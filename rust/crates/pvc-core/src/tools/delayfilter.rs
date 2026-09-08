//! Ports `delayfilter.c`: resynthesizes a pre-analyzed `.pva` source file
//! (`-F`) through a per-bin, time-varying delay line shaped by a
//! `groupdelaymaker`-produced `(amp, delay-seconds)` response file (`-B`,
//! see `pvc_core::tools::groupdelaymaker`'s own doc comment identifying
//! this as "the delay-line tool" that consumes its output). For each
//! output frame and each frequency bin, the tool looks up that bin's own
//! delay time from the response, then fetches (and linearly interpolates)
//! the *source* analysis frame from `binTimeDelays[bin]` seconds earlier
//! than the tool's own navigated time position - different bins can
//! effectively read from different points in the source's own timeline
//! within the same output frame, a comb/diffusion-style effect.
//!
//! Structurally sits between `tools::twarp` (a `.pva` source, no live FFT
//! of raw audio) and `tools::tvfilter` (a per-bin frequency-shifted/
//! transposed lookup into a second response file - see
//! [`group_delay_lookup`]'s doc comment on its shared index math with
//! `tvfilter::filter_lookup`) - but genuinely its own tool, not a
//! composition of either's Rust code: confirmed by reading `delayfilter.c`
//! in full, not assumed from either sibling's `usage()` text (matching
//! this project's own reuse-before-rederive convention).
//!
//! **Real per-bin interpolation, unlike `twarp`'s [`crate::timenav::
//! interpolate_frame`]**: `delayfilter.c` declares its own `filtfprop` as
//! `float *` (`twarp.c`'s equivalent is a scalar `int`, the reason that
//! function never actually interpolates) - here, the fractional position
//! between the two fetched analysis frames genuinely blends them. A fresh
//! lookup is written for this module rather than reusing
//! `interpolate_frame`.
//!
//! **`func_dur` vs `dur`**: the C evaluates every control function against
//! `funcDur` (the *pre-ringout* duration - either `-d`'s value or the
//! source's own analysis duration), while the frame loop itself runs
//! until the *extended* `dur = funcDur + ringTime` (a tail long enough for
//! the largest delay the response file, scaled by `-T`'s own maximum, could
//! ever produce, so a long group delay doesn't get truncated). Modeled
//! here as two distinct values threaded through explicitly, not derived
//! from each other after setup.
//!
//! **The frequency-shift term divides by the *source*'s own fundamental,
//! not the response file's** (`fs / fundamental`, where `fundamental =
//! sample_rate / n`, `n` being the source `.pva`'s own FFT size):
//! confirmed by reading the exact line, not assumed from
//! `tools::tvfilter::filter_lookup`'s analogous term (which divides by
//! *its* response file's own fundamental instead) - the two tools'
//! otherwise-identical-looking shift/transpose formulas differ in exactly
//! this one respect. Reproduced as read, not "fixed" to match its sibling.
//!
//! **Not reproduced**: the C's per-bin *group-batching* optimization (bins
//! sharing one integer virtual analysis-frame index reuse a single
//! `fseek`/`fread` pair - a pure disk-I/O saving with no reason to exist
//! once the whole file is loaded into memory up front, as this port does)
//! has one small semantic side effect this port does not carry over: the
//! three curve-shaping control functions (`-V`/`-y`/`-z`, zero/max delay
//! decibels and the curve's own warp index) are evaluated, for every bin
//! in a batch, at the *first* (lowest-index) bin's own time-shift rather
//! than each bin's own. This is only observable when at least one of
//! those three is a real time-varying function rather than its
//! `Const(0.0)` default (under which `ControlFn::at` ignores its time
//! argument entirely, so the batching quirk has zero effect) - deferred
//! rather than reproducing the C's exact non-contiguous forward-scan
//! grouping order for a quirk invisible at every default setting. This
//! port evaluates each bin's own time-shift instead.
//!
//! **Not ported, matching `tools::ring`'s own established precedent for
//! the same kind of flag** (bit-exact glibc `randf()`/`random()` is a
//! disproportionate side quest for a feature off by default): `-v`/`-r`/
//! `-q` (the per-bin time-position diffusion/dither switch and its two
//! smoothing controls) - `binranv` is always `0.0` with the diffusion
//! switch off, which is the C's own default.
//!
//! **Not ported, matching every other multi-channel tool's established
//! precedent in this project**: `-C` (a "process one channel only" flag
//! whose own numeric value is never actually used to select *which*
//! channel - `beginchan` is never assigned from it anywhere in the file,
//! confirmed by grepping `legacy/pvc_src/delayfilter.c` and
//! `legacy/pvc_lib/fileio.c` - so a nonzero `-C` always processes channel 1
//! regardless of the number given; not worth reproducing a fixed-to-
//! channel-1 restriction with no real selection behavior behind it). Also
//! not ported: `-Z` (a stderr-only response printout) and `-_`/`-=`
//! (auto-play and the peak-rescale-level override - every resynthesis tool
//! in this project skips the print/play flags and always rescales the
//! whole output to the input's own peak; see `commands::twarp::run`'s doc
//! comment on the same convention applied here).
//!
//! **Dead flags** (in `crack()`'s own flag list but handled by no `case`
//! in the `switch` - confirmed by cross-referencing both, matching this
//! project's per-tool delivery checklist): `-h`, `-I`, `-K`, `-N`, `-s`.
//! None appear in the tool's own `usage()` text either.
//!
//! **`-w`'s delay-warpshape is silently truncated to an integer** at parse
//! time (`(int) crackfloat(...)`, stored back into a `float`) - unlike
//! `-W`'s own (fully continuous) spectrum warpshape. The CLI layer
//! truncates the same way before this module ever sees the value.

use crate::eq::eq;
use crate::pvoc::{getthresh, Frame, OscBank, Synthesizer};
use crate::smooth::smooth_setup;
use crate::tools::tvfilter::{compress_response, normalize_response};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::{curve, spectmagwarp};
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::twarp`'s doc
/// comment on the same constant.
const OSCILBANKGAIN: f32 = 1.7782794;

pub struct DelayfilterParams {
    /// `-M`. `0` means auto (`2 * n`, `n` being the source file's own FFT
    /// size - there's no independent `-N`).
    pub window_size: usize,
    /// `-S`.
    pub window: Window,
    /// `-D`. Values under `32.0` reset to `200.0`, matching the C's own
    /// safety clamp.
    pub frames_per_sec: f32,
    /// `-d`. `<= 0.0` means "use the source file's own analysis duration".
    pub duration: f32,

    /// `-x`.
    pub delay_time_scaler: ControlFn,
    /// `-P`.
    pub pitch_transpose: ControlFn,
    /// `-a`.
    pub freq_shift: ControlFn,
    /// `-A`.
    pub gain_db: ControlFn,

    /// `-Q`.
    pub filter_time_origin: ControlFn,
    /// `-Y`.
    pub filter_rate: ControlFn,

    /// `-b`.
    pub delay_transpose: ControlFn,
    /// `-e`.
    pub delay_shift: ControlFn,
    /// `-w`, already truncated to an integer by the caller (see this
    /// module's doc comment).
    pub delay_warpshape: f32,

    /// `-V`.
    pub zero_delay_db: ControlFn,
    /// `-y`.
    pub max_delay_db: ControlFn,
    /// `-z`.
    pub delay_db_warp: ControlFn,
    /// `-T`.
    pub max_delay_time_mult: ControlFn,

    /// `-E`.
    pub comp_threshold_db: f32,
    /// `-c`.
    pub comp_db: f32,

    /// `-L`.
    pub release_secs: ControlFn,
    /// `-l`.
    pub attack_secs: ControlFn,
    /// `-f`.
    pub freq_smooth_secs: ControlFn,

    /// `-W`.
    pub input_warpshape: ControlFn,

    /// `-H`.
    pub shelf_low_db: f32,
    /// `-X`.
    pub shelf_high_db: f32,
    /// `-m`.
    pub shelf_low_freq: f32,
    /// `-R`.
    pub shelf_high_freq: f32,

    /// `-t`.
    pub threshold_db: f32,
}

fn control_fn_max(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(vals) => vals.iter().copied().fold(f32::MIN, f32::max),
    }
}

/// Mirrors `tools::tvfilter::filter_lookup`'s index math exactly (bin-
/// shift-then-transpose, then linear interpolation with the same
/// second-to-last-pair/first-pair edge clamping) but resolves an `(amp,
/// delay-seconds)` pair instead of a single amplitude - see this module's
/// doc comment on why `source_fundamental` (not the response's own) is
/// the right divisor for `shift_hz`.
fn group_delay_lookup(
    delay_pairs: &[(f32, f32)],
    bin: usize,
    n_ratio: f32,
    source_fundamental: f32,
    shift_hz: f32,
    transpose_mult: f32,
) -> (f32, f32) {
    let delay_n2 = delay_pairs.len() - 1;
    let mut temp = bin as f32 * n_ratio;
    temp -= shift_hz / source_fundamental;
    temp /= transpose_mult;
    let i1 = temp as i64;
    let i2p = temp - i1 as f32;
    let i1p = 1.0 - i2p;
    let i2 = i1 + 1;
    if i1 < 0 {
        delay_pairs[0]
    } else if i2 as usize >= delay_n2 {
        delay_pairs[delay_n2 - 1]
    } else {
        let (a1, d1) = delay_pairs[i1 as usize];
        let (a2, d2) = delay_pairs[i2 as usize];
        (a1 * i1p + a2 * i2p, d1 * i1p + d2 * i2p)
    }
}

/// Resynthesizes one channel. `frames` is this channel's source analysis
/// frames (`pvc_io::PvaData::channels[ch]`, each `analysis_n + 2` floats);
/// `master_peak_amp` is the *maximum* of `pvc_io::PvaData::peak_amps`
/// across *every* channel (not just this one), matching the C's own
/// `normamppk` (computed once, before the per-channel loop, from all of
/// `normamp[]`). `delay_pairs` is the group-delay response's own `(amp,
/// delay-seconds)` bins (`pvc_io::read_fr_pairs`'s output).
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    frames: &[Vec<f32>],
    analysis_n: usize,
    analysis_d: u32,
    analysis_sample_rate: u32,
    master_peak_amp: f32,
    delay_pairs: &[(f32, f32)],
    params: &DelayfilterParams,
) -> Vec<f32> {
    let r = analysis_sample_rate as f32;
    let n = analysis_n;
    let n2 = n / 2;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;

    let delay_analysis_n = (delay_pairs.len() - 1) * 2;
    let n_ratio = delay_analysis_n as f32 / n as f32;

    let frames_per_sec = if params.frames_per_sec < 32.0 {
        200.0
    } else {
        params.frames_per_sec
    };
    let d = (r / frames_per_sec) as usize;
    let i_factor = d;
    let filttinc = d as f32 / r;
    let ir = i_factor as f32 / r;

    let mut nw = params.window_size;
    if nw == 0 {
        nw = 2 * n;
    }
    if nw < i_factor {
        nw = 2;
        while nw <= i_factor {
            nw *= 2;
        }
    }

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();

    let iframes_per_sec = analysis_sample_rate as f32 / analysis_d as f32;
    let niframes = frames.len();
    let analysis_dur = niframes as f32 / iframes_per_sec;

    let func_dur = if params.duration <= 0.0 {
        analysis_dur
    } else {
        params.duration
    };

    let max_delay_secs = delay_pairs
        .iter()
        .map(|&(_, delay)| delay)
        .fold(f32::MIN, f32::max);
    let ring_time = max_delay_secs * control_fn_max(&params.max_delay_time_mult);
    let dur = func_dur + ring_time;

    assert!(
        params.comp_threshold_db <= 0.0,
        "delayfilter: compression threshold must be <= 0dB"
    );
    assert!(
        params.comp_db <= 0.0,
        "delayfilter: compression decibels must be <= 0dB"
    );
    let comp_threshold_amp = db_to_amp.convert(params.comp_threshold_db);
    let comp_amp = db_to_amp.convert(params.comp_db);
    let comp_norm_amp = 1.0 / (comp_threshold_amp + comp_amp * (1.0 - comp_threshold_amp));
    let compflag = comp_threshold_amp < 1.0 && comp_amp < 1.0;

    let threshfac = db_to_amp.convert(params.threshold_db);

    // Startup-time obank/overlap-add selection - matches `tools::twarp`'s
    // own precedent (see that module's doc comment).
    let ptrans_is_const_zero = matches!(&params.pitch_transpose, ControlFn::Const(v) if *v == 0.0);
    let harmadd_is_const_zero = matches!(&params.freq_shift, ControlFn::Const(v) if *v == 0.0);
    let obank = !(ptrans_is_const_zero && harmadd_is_const_zero);

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut osc = OscBank::new(n2, nw, analysis_sample_rate, i_factor, 1.0);
    let mut synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, analysis_sample_rate);

    let mut previous_channel = vec![0.0f32; n + 2];
    let mut frame_count: usize = 0;

    let mut filttorigin_prev = params.filter_time_origin.at(0.0, func_dur);
    let mut filttnow = filttorigin_prev;

    let mut on: i64 = (-(nw as i64) * i_factor as i64) / d as i64;

    let mut output = Vec::new();
    let mut samps_written: usize = 0;
    let mut t_for_check = 0.0f32;

    loop {
        if t_for_check >= dur {
            break;
        }
        let t = samps_written as f32 / r;

        // ---- FILTER TIME NAVIGATION (no window/loop/autostop here - see
        // this module's doc comment) ----
        let scaler = params.delay_time_scaler.at(t, func_dur);
        let filtrate_val = params.filter_rate.at(t, func_dur);
        let new_origin = params.filter_time_origin.at(t, func_dur);
        let origin_change = new_origin - filttorigin_prev;
        filttorigin_prev = new_origin;
        filttnow += origin_change + filttinc * filtrate_val;

        let fs = params.delay_shift.at(t, func_dur);
        let fm = semitones_to_mult.convert(params.delay_transpose.at(t, func_dur));
        let dwin = params.max_delay_time_mult.at(t, func_dur);

        // ---- MAKE EACH BIN'S FILTER TIME, THEN FETCH ITS FRAME PAIR ----
        let mut channel = vec![0.0f32; n + 2];
        let mut bin_time_delays = vec![0.0f32; n2 + 1];

        for j in 0..=n2 {
            let (bin_amp_mult, delay_secs) =
                group_delay_lookup(delay_pairs, j, n_ratio, fundamental, fs, fm);
            let bin_time_delay = delay_secs * dwin;
            bin_time_delays[j] = bin_time_delay;

            let bindb_prop = if dwin <= 0.0 {
                0.0
            } else {
                bin_time_delay / (max_delay_secs * dwin)
            };

            let filtbint_time = filttnow - bin_time_delay;
            let filtf = iframes_per_sec * filtbint_time;
            let max_frame = (niframes as i64 - 2).max(0);
            let filtflow = (filtf as i64).clamp(0, max_frame);
            let filtfprop = filtf - filtflow as f32;

            let f_low = &frames[filtflow as usize];
            let f_high = &frames[filtflow as usize + 1];

            let shifted_t = t - scaler * bin_time_delay;
            let curve_db = curve(
                params.zero_delay_db.at(shifted_t, func_dur),
                params.max_delay_db.at(shifted_t, func_dur),
                bindb_prop,
                params.delay_db_warp.at(shifted_t, func_dur),
            );
            let bin_amp = bin_amp_mult * db_to_amp.convert(curve_db);

            channel[2 * j] = bin_amp * (f_low[2 * j] + filtfprop * (f_high[2 * j] - f_low[2 * j]));
            channel[2 * j + 1] =
                f_low[2 * j + 1] + filtfprop * (f_high[2 * j + 1] - f_low[2 * j + 1]);
        }

        if frame_count == 0 {
            previous_channel.copy_from_slice(&channel);
        }

        // ---- NORMALIZE -> COMPRESS -> WARP -> EQ -> GAIN SCALE BACK ----
        {
            let mut amps: Vec<f32> = channel.iter().step_by(2).copied().collect();
            normalize_response(&mut amps, master_peak_amp);
            for (dst, src) in channel.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }
        if compflag {
            let mut amps: Vec<f32> = channel.iter().step_by(2).copied().collect();
            compress_response(&mut amps, comp_threshold_amp, comp_amp, comp_norm_amp);
            for (dst, src) in channel.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }
        let input_warpshape_val = params.input_warpshape.at(t, func_dur);
        spectmagwarp(&mut channel, input_warpshape_val, false);
        eq(
            &mut channel,
            params.shelf_low_db,
            params.shelf_high_db,
            params.shelf_low_freq,
            params.shelf_high_freq,
            fundamental,
            1.0,
            0.0,
            false,
            &db_to_amp,
        );
        for a in channel.iter_mut().step_by(2) {
            *a *= master_peak_amp;
        }

        // ---- PER-BIN ATTACK/RELEASE, FREQ SMOOTH, SHIFT/TRANSPOSE, GAIN ----
        for j in 0..=n2 {
            let shifted_t = t - scaler * bin_time_delays[j];

            if channel[2 * j] < previous_channel[2 * j] {
                let release_val = params.release_secs.at(shifted_t, func_dur);
                let (releasec, minus_releasec) = smooth_setup(release_val, ir);
                channel[2 * j] =
                    releasec * previous_channel[2 * j] + minus_releasec * channel[2 * j];
            } else {
                let attack_val = params.attack_secs.at(shifted_t, func_dur);
                let (attackc, minus_attackc) = smooth_setup(attack_val, ir);
                channel[2 * j] = attackc * previous_channel[2 * j] + minus_attackc * channel[2 * j];
            }

            let fsmooth_val = params.freq_smooth_secs.at(t, func_dur);
            let (fsmoothc, minus_fsmoothc) = smooth_setup(fsmooth_val, ir);
            channel[2 * j + 1] =
                fsmoothc * previous_channel[2 * j + 1] + minus_fsmoothc * channel[2 * j + 1];

            previous_channel[2 * j] = channel[2 * j];
            previous_channel[2 * j + 1] = channel[2 * j + 1];

            let harmadd = params.freq_shift.at(shifted_t, func_dur);
            let pm = semitones_to_mult.convert(params.pitch_transpose.at(shifted_t, func_dur));
            let temp = pm * (channel[2 * j + 1] + harmadd);
            if temp <= 0.0 || temp >= nyquist {
                channel[2 * j] = 0.0;
            } else {
                channel[2 * j + 1] = temp;
            }

            let gain = db_to_amp.convert(params.gain_db.at(shifted_t, func_dur));
            channel[2 * j] *= gain;
        }

        let frame = Frame::from_pva_floats(&channel);
        let synt = getthresh(&frame.bins[..n2], threshfac);

        if obank {
            let hop_out = osc.synthesize(&frame, synt);
            on += i_factor as i64;
            if on + nw as i64 - i_factor as i64 >= 0 {
                output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
                samps_written += i_factor;
            }
        } else {
            let hop_out = synth.overlap_add(&frame);
            if !hop_out.is_empty() {
                output.extend(hop_out);
                samps_written += i_factor;
            }
        }

        frame_count += 1;
        t_for_check = t;
    }

    if obank {
        output.extend(vec![0.0f32; i_factor]);
    } else {
        output.extend(synth.flush());
    }

    output
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params() -> DelayfilterParams {
        DelayfilterParams {
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            duration: 0.0,
            delay_time_scaler: ControlFn::Const(0.0),
            pitch_transpose: ControlFn::Const(0.0),
            freq_shift: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            filter_time_origin: ControlFn::Const(0.0),
            filter_rate: ControlFn::Const(1.0),
            delay_transpose: ControlFn::Const(0.0),
            delay_shift: ControlFn::Const(0.0),
            delay_warpshape: 0.0,
            zero_delay_db: ControlFn::Const(0.0),
            max_delay_db: ControlFn::Const(0.0),
            delay_db_warp: ControlFn::Const(0.0),
            max_delay_time_mult: ControlFn::Const(0.0),
            comp_threshold_db: 0.0,
            comp_db: 0.0,
            release_secs: ControlFn::Const(0.0),
            attack_secs: ControlFn::Const(0.0),
            freq_smooth_secs: ControlFn::Const(0.0),
            input_warpshape: ControlFn::Const(0.0),
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            threshold_db: -60.0,
        }
    }

    fn silent_frames(n: usize, count: usize) -> Vec<Vec<f32>> {
        vec![vec![0.0f32; n + 2]; count]
    }

    fn flat_delay_pairs(n2: usize) -> Vec<(f32, f32)> {
        vec![(1.0, 0.0); n2 + 1]
    }

    #[test]
    fn silence_in_silence_out() {
        let fft = 1024;
        let frames = silent_frames(fft, 100);
        let delay_pairs = flat_delay_pairs(fft / 2);
        let params = default_params();
        let output = process_channel(&frames, fft, 220, 44100, 1.0, &delay_pairs, &params);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn zero_delay_response_bounded_for_real_analyzed_sine() {
        use crate::tools::analyze::{process_channel as analyze_channel, AnalyzeParams};

        let sample_rate = 44100u32;
        let fft = 1024;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let analyze_params = AnalyzeParams {
            fft_size: fft,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            gain_db: 0.0,
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            warpshape: 0.0,
        };
        let (analyzed, peak_amp) = analyze_channel(&input, sample_rate, &analyze_params);
        let d = (sample_rate as f32 / analyze_params.frames_per_sec) as u32;
        let frames: Vec<Vec<f32>> = analyzed.iter().map(|f| f.to_pva_floats()).collect();

        let delay_pairs = flat_delay_pairs(fft / 2);
        let params = default_params();
        let output = process_channel(
            &frames,
            fft,
            d,
            sample_rate,
            peak_amp,
            &delay_pairs,
            &params,
        );
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.01, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn nonzero_delay_extends_output_past_the_undelayed_duration() {
        let fft = 1024;
        let frames = silent_frames(fft, 200);
        // A uniform 0.5s delay everywhere - the output should ring out
        // past the source's own ~1s analysis duration.
        let delay_pairs = vec![(1.0, 0.5); fft / 2 + 1];
        let mut params = default_params();
        params.max_delay_time_mult = ControlFn::Const(1.0);
        let output = process_channel(&frames, fft, 220, 44100, 1.0, &delay_pairs, &params);
        let secs = output.len() as f32 / 44100.0;
        let no_delay_output = process_channel(
            &frames,
            fft,
            220,
            44100,
            1.0,
            &flat_delay_pairs(fft / 2),
            &default_params(),
        );
        let no_delay_secs = no_delay_output.len() as f32 / 44100.0;
        assert!(
            secs > no_delay_secs,
            "expected the delayed run ({secs}s) to be longer than the undelayed one ({no_delay_secs}s)"
        );
    }
}
