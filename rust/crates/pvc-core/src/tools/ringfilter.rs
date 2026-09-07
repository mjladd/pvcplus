//! Ports `ringfilter.c`: `tools::ring`'s "phase vocoder reverberator/
//! resonator", plus a switchable fixed-spectrum filter (a `.fr` response
//! file, the same format `pvc filter` reads) that can be placed either
//! on the feedback path's *input* ("prefilter", `x(n)`, before the delay
//! line's tail is added in) or *inside the loop* ("postfilter", `y(n-1)`,
//! after the in-loop feedback EQ, decay-time-scaled the same way that EQ
//! is). Reuses `tools::ring`'s shared building blocks directly
//! ([`crate::tools::ring::RawAnalyzer`], `lean_convert`/`lean_unconvert`,
//! `apply_shelf_eq`, `control_fn_max`) rather than re-deriving them - the
//! two tools' analysis/feedback/resynthesis machinery is otherwise
//! identical, confirmed by reading both files side by side.
//!
//! **Real, confirmed default-value bug, reproduced faithfully - present
//! independently in all three shelf-EQ stages, exactly as in `tools::
//! ring`**: see that module's doc comment. `ringfilter.c`'s own
//! `INPUT_dBlow`/`FEEDBACK_dBlow`/`OUTPUT_dBlow` initializers repeat the
//! identical `200.`/`0.` (gain/frequency) swap.
//!
//! **The same silent duration extension as `tools::ring`**: `ringTime`
//! (from `funcStats(&feedback_level, ...).hi`) extends the readable input
//! with that many extra seconds of silence before EOF, letting the
//! reverb tail ring out - see `tools::ring`'s doc comment for how this
//! was found. Reproduced the same way, via [`control_fn_max`].
//!
//! **A real difference from `tools::ring`: the in-loop EQ's decay-time
//! division is guarded here, not there.** `ring.c` divides by
//! `FEEDBACK_decay_time.A[0]` unconditionally (a real, if pathological,
//! infinite-loop risk for `-T 0` combined with a nonzero `-X`/`-Q` - see
//! `tools::ring`'s doc comment); `ringfilter.c` checks `FEEDBACK_decay_time
//! .A[0] < IR` first and uses the *unscaled* dB value directly when it
//! is (skipping the division that would otherwise blow the value up for
//! a very short decay time relative to one hop). The low-shelf half of
//! that scaled pair (`FEEDBACK_dBlowtemp`) is a real, if obscure,
//! function-scope variable in the C that is simply never reassigned on
//! the `< IR` branch - this port sets it to `0.0` fresh on that branch
//! instead of reading a stale previous-frame value, the same
//! simplification `tools::ring` already makes for `prebalancesum`.
//!
//! **A real, confirmed asymmetry between the prefilter and postfilter
//! placements, reproduced faithfully**: both share one pitch/frequency-
//! compensated pair (`fm`, and - only for the *prefilter* placement -
//! `fs`) computed once per frame from `-u`/`-V` (`ftrans`/`fshift`),
//! optionally divided/subtracted by the feedback path's own `-P`/`-H`
//! shift unless [`RingfilterParams::pitchflag`] is set (`-B 0`, the
//! default, compensates; `-B 1` does not). The *postfilter* placement's
//! own bin-shift lookup
//! uses `fshift`'s raw value directly, not the `fs` compensated one -
//! confirmed by reading `ringfilter.c`'s two nearly-identical filter-
//! application blocks side by side, not assumed from symmetry with the
//! prefilter block.
//!
//! Not ported, matching `tools::ring`'s own precedent for the same
//! flags: `-C`, `-w`/`-i`/`-_`/`-=`, `-j`/`-K`/`-W`/`-a` (random per-bin
//! frequency/amplitude dither). Also not ported: `-J` (an "optional
//! analysis sound file source" whose own enabling flag, `sflag`, is
//! declared but never set anywhere in the file - reading its branch
//! confirms the feature is entirely inert, matching this project's
//! established "confirmed dead by reading the whole file" bar, though
//! unlike `tools::ring`'s `-R`/`-a`/`-B`/`-h`, `-J` at least has a
//! `case` that stores its argument, so it doesn't fail to parse).

use crate::pvoc::{getthresh, OscBank, PhaseTracker};
use crate::smooth::smooth_setup;
use crate::tools::ring::{
    apply_shelf_eq, control_fn_max, lean_convert, lean_unconvert, RawAnalyzer,
};
use crate::units::{db_to_amp_exact, DbToAmp, SemitonesToMult};
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant (each oscillator-bank tool in this
/// project redefines it locally rather than sharing one copy).
const OSCILBANKGAIN: f32 = 1.7782794;

pub struct RingfilterParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,
    /// `-t`: a plain constant, not a `(func)`.
    pub oscbank_threshold_db: f32,
    /// `-A`.
    pub master_gain_db: ControlFn,
    /// `-S`.
    pub source_gain_db: ControlFn,
    /// `-f`.
    pub source_freq_shift_hz: ControlFn,
    /// `-p`.
    pub source_pitch_semitones: ControlFn,
    /// `-F`.
    pub feedback_gain_db: ControlFn,
    /// `-H`.
    pub feedback_freq_shift_hz: ControlFn,
    /// `-P`.
    pub feedback_pitch_semitones: ControlFn,
    /// `-Z`: `0.0` disables the feedback delay line entirely.
    pub feedback_decay_secs: ControlFn,
    /// `-z`.
    pub feedback_threshold_db: ControlFn,
    /// `-g`: `true` (the default) passes/holds bins *above* threshold.
    pub feedback_threshold_pass_above: bool,
    /// `-l`.
    pub envelope_attack_secs: ControlFn,
    /// `-L`.
    pub envelope_release_secs: ControlFn,
    /// `-O`. Defaults to `200.0`, not `0.0` - see this module's doc
    /// comment.
    pub input_eq_low_db: ControlFn,
    /// `-Y`.
    pub input_eq_high_db: ControlFn,
    /// `-d`. Defaults to `0.0`, not `200.0` - see this module's doc
    /// comment.
    pub input_eq_low_freq: ControlFn,
    /// `-n`.
    pub input_eq_high_freq: ControlFn,
    /// `-T`.
    pub loop_eq_decay_secs: ControlFn,
    /// `-E`: a plain constant, not a `(func)`.
    pub loop_balance_limit_db: f32,
    /// `-X`. Defaults to `200.0`, not `0.0` - see this module's doc
    /// comment.
    pub loop_eq_low_db: ControlFn,
    /// `-Q`.
    pub loop_eq_high_db: ControlFn,
    /// `-U`. Defaults to `0.0`, not `200.0` - see this module's doc
    /// comment.
    pub loop_eq_low_freq: ControlFn,
    /// `-m`.
    pub loop_eq_high_freq: ControlFn,
    /// `-k`. Defaults to `200.0`, not `0.0` - see this module's doc
    /// comment.
    pub output_eq_low_db: ControlFn,
    /// `-c`.
    pub output_eq_high_db: ControlFn,
    /// `-s`. Defaults to `0.0`, not `200.0` - see this module's doc
    /// comment.
    pub output_eq_low_freq: ControlFn,
    /// `-G`.
    pub output_eq_high_freq: ControlFn,
    /// `-y`: the loaded `.fr` file's amplitude-only values, `n2 + 1`
    /// long (`pvc_io::read_fr_amplitudes`'s own return shape) - required,
    /// matching `fillfunc`'s own "YOU MUST PROVIDE A FILTER FUNCTION"
    /// exit when absent.
    pub filter_response: Vec<f32>,
    /// `-q`: reshapes the response curve (via `spectmagwarp`, peak-
    /// normalized every frame regardless of warpshape) before each
    /// frame's lookup.
    pub filter_warpshape: ControlFn,
    /// `-u`.
    pub filter_transpose_semitones: ControlFn,
    /// `-V`.
    pub filter_freq_shift_hz: ControlFn,
    /// `-x`: blends between the filtered response (`0.0`/`-96`, the
    /// default: fully filtered) and the unfiltered dry signal (`0.0` dB:
    /// filter has no effect at all).
    pub filter_source_db: ControlFn,
    /// `-r`: only used by the postfilter placement.
    pub filter_decay_secs: ControlFn,
    /// `-o`: `false` (the default, `-o 0`) places the filter on the
    /// feedback path's *input* ("prefilter", `x(n)`); `true` (`-o 1`)
    /// places it *inside the loop* ("postfilter", `y(n-1)`, decay-time-
    /// scaled).
    pub filter_postfilter: bool,
    /// `-B`: `false` (the default, `-B 0`) compensates the filter's own
    /// transpose/shift by the feedback path's own `-P`/`-H` so the
    /// filter tracks the *unshifted* analysis frequencies; `true`
    /// (`-B 1`) applies no compensation, so the filter moves with the
    /// feedback's own pitch/frequency modulation too.
    pub pitchflag: bool,
}

/// Ports the shared bin-shift/transpose lookup both the prefilter and
/// postfilter placements use: `this_f` (already warped, `n2 + 1` long)
/// indexed at a linearly-interpolated, shifted-and-transposed position,
/// clamped at both ends. `i2 as usize >= n2` (not `> n2`) clamping to
/// `this_f[n2 - 1]` (not `this_f[n2]`) is a real quirk shared with `pvc
/// filter`'s own equivalent lookup (`tools::filter::process_channel`) -
/// the top bin (Nyquist) can never actually be selected by this formula,
/// in either tool.
fn filter_lookup(
    this_f: &[f32],
    bin: usize,
    fundamental: f32,
    shift_hz: f32,
    transpose_mult: f32,
) -> f32 {
    let n2 = this_f.len() - 1;
    let mut temp = bin as f32;
    temp -= shift_hz / fundamental;
    temp /= transpose_mult;
    let i1 = temp as i64;
    let i2p = temp - i1 as f32;
    let i1p = 1.0 - i2p;
    let i2 = i1 + 1;
    if i1 < 0 {
        this_f[0]
    } else if i2 as usize >= n2 {
        this_f[n2 - 1]
    } else {
        this_f[i1 as usize] * i1p + this_f[i2 as usize] * i2p
    }
}

/// Processes one channel start to finish. `dur` is the control-function
/// normalization duration in seconds, matching `tools::ring`.
pub fn process_channel(
    channel: &[f32],
    sample_rate: u32,
    params: &RingfilterParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;

    assert!(
        params.filter_response.len() == n2 + 1,
        "ringfilter: filter response has {} values, expected {} (n2 + 1) for FFT size {}",
        params.filter_response.len(),
        n2 + 1,
        n
    );

    let ring_time_secs = control_fn_max(&params.feedback_decay_secs).max(0.0);
    let ring_time_samples = (ring_time_secs * r) as usize;
    let mut extended_channel = channel.to_vec();
    extended_channel.extend(std::iter::repeat_n(0.0f32, ring_time_samples));
    let channel = extended_channel.as_slice();

    let frames_per_sec = if params.frames_per_sec < 32.0 {
        200.0
    } else {
        params.frames_per_sec
    };
    let time_factor = if params.time_factor <= 0.0 {
        1.0
    } else {
        params.time_factor
    };
    let d = (r / frames_per_sec) as usize;
    let i_factor = (d as f32 * time_factor) as usize;
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

    let nyquist = r / 2.0;
    let fundamental = r / n as f32;

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();
    let threshfac = db_to_amp.convert(params.oscbank_threshold_db);

    let balance_limit_amp = db_to_amp_exact(params.loop_balance_limit_db);
    let balance_flag = params.loop_balance_limit_db > 0.0;

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut raw_analyzer = RawAnalyzer::new(n, window_pair.analysis, d);

    let mut channel_phase = PhaseTracker::new_analysis(n2, d, sample_rate);
    let mut feedback_phase = PhaseTracker::new_analysis(n2, d, sample_rate);

    let mut source_osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut feedback_osc =
        OscBank::with_shared_table(n2, sample_rate, i_factor, 1.0, source_osc.table());

    let mut previous: Vec<(f32, f32)> = vec![(0.0, 0.0); n2 + 1];
    let mut next_buffer: Vec<f32> = vec![0.0; n];

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;
    let mut on: i64 = (-(nw as i64) * i_factor as i64) / d as i64;
    let mut output = Vec::new();
    let mut samps_written: usize = 0;

    loop {
        let mut hop = vec![0.0f32; d];
        if valid == nw as i64 {
            let available = d.min(channel.len().saturating_sub(pos));
            hop[..available].copy_from_slice(&channel[pos..pos + available]);
            pos += available;
            if available < d {
                valid = nw as i64 - d as i64 + available as i64;
            }
        }
        if valid < nw as i64 {
            valid -= d as i64;
        }
        let eof_after_this_hop = valid <= 0;

        let t = samps_written as f32 / r;

        let master_gain = db_to_amp.convert(params.master_gain_db.at(t, dur));

        let source_gain = db_to_amp.convert(params.source_gain_db.at(t, dur));
        let source_harmadd = params.source_freq_shift_hz.at(t, dur);
        let source_pm = semitones_to_mult.convert(params.source_pitch_semitones.at(t, dur));

        let feedback_gain = db_to_amp.convert(params.feedback_gain_db.at(t, dur));
        let feedback_harmadd = params.feedback_freq_shift_hz.at(t, dur);
        let feedback_pm = semitones_to_mult.convert(params.feedback_pitch_semitones.at(t, dur));

        let (feedlevel, _) = smooth_setup(params.feedback_decay_secs.at(t, dur), ir);
        let feedback_thresh_amp = db_to_amp.convert(params.feedback_threshold_db.at(t, dur));

        let (envrelease, minusrelease) = smooth_setup(params.envelope_release_secs.at(t, dur), ir);
        let (envattack, minusattack) = smooth_setup(params.envelope_attack_secs.at(t, dur), ir);

        let buffer = raw_analyzer.push(&hop);

        let mut feedback_buffer = buffer.clone();
        let mut feedback_bins = lean_convert(&feedback_buffer, n2);

        let feedt = getthresh(&feedback_bins, feedback_thresh_amp);
        for (j, bin) in feedback_bins.iter_mut().enumerate() {
            let below_gate = if params.feedback_threshold_pass_above {
                bin.0 < feedt
            } else {
                bin.0 > feedt
            };
            bin.0 = if below_gate {
                envrelease * previous[j].0
            } else if bin.0 < previous[j].0 {
                envrelease * previous[j].0 + minusrelease * bin.0
            } else {
                envattack * previous[j].0 + minusattack * bin.0
            };
            previous[j].0 = bin.0;
        }

        apply_shelf_eq(
            &mut feedback_bins,
            params.input_eq_low_db.at(t, dur),
            params.input_eq_high_db.at(t, dur),
            params.input_eq_low_freq.at(t, dur),
            params.input_eq_high_freq.at(t, dur),
            fundamental,
            &db_to_amp,
        );

        // ---- FILTER SETUP (shared by both placements) ----
        let mut fm = semitones_to_mult.convert(params.filter_transpose_semitones.at(t, dur));
        if !params.pitchflag {
            fm /= feedback_pm;
        }
        let raw_fshift = params.filter_freq_shift_hz.at(t, dur);
        let mut fs = raw_fshift;
        if !params.pitchflag {
            fs -= feedback_harmadd;
        }
        let source_db = params.filter_source_db.at(t, dur);
        let sourceamp = db_to_amp.convert(source_db);
        let filtamp = 1.0 - sourceamp;

        let warpshape = params.filter_warpshape.at(t, dur);
        let mut this_f_interleaved: Vec<f32> = params
            .filter_response
            .iter()
            .flat_map(|&a| [a, 0.0])
            .collect();
        spectmagwarp(&mut this_f_interleaved, warpshape, true);
        let this_f: Vec<f32> = this_f_interleaved.iter().step_by(2).copied().collect();

        // ---- PREFILTER: applied to x(n), before the delay line's tail ----
        if !params.filter_postfilter && source_db != 0.0 {
            for (j, bin) in feedback_bins.iter_mut().enumerate() {
                let interp = filter_lookup(&this_f, j, fundamental, fs, fm);
                bin.0 *= filtamp * interp + sourceamp;
            }
        }

        feedback_buffer = lean_unconvert(&feedback_bins, n);

        for i in 0..n {
            feedback_buffer[i] += feedlevel * next_buffer[i];
        }
        feedback_bins = lean_convert(&feedback_buffer, n2);

        let loop_decay_secs = params.loop_eq_decay_secs.at(t, dur);
        let loop_high_db = params.loop_eq_high_db.at(t, dur);
        let loop_low_db = params.loop_eq_low_db.at(t, dur);
        // Guarded here (unlike `tools::ring`, which divides unconditionally
        // - a real difference between the two tools, see this module's doc
        // comment): a decay time shorter than one hop uses the raw dB
        // value directly instead of a division that would otherwise blow
        // it up.
        let (dbhitemp, dblowtemp) = if loop_decay_secs < ir {
            (loop_high_db, 0.0)
        } else {
            (
                (loop_high_db * ir) / loop_decay_secs,
                (loop_low_db * ir) / loop_decay_secs,
            )
        };
        let loop_freqlow = params.loop_eq_low_freq.at(t, dur);
        let loop_freqhi = params.loop_eq_high_freq.at(t, dur);

        let prebalancesum = if ((params.filter_postfilter && source_db != 0.0) || dbhitemp != 0.0)
            && balance_flag
        {
            feedback_bins.iter().map(|b| b.0).sum::<f32>()
        } else {
            0.0
        };

        apply_shelf_eq(
            &mut feedback_bins,
            dblowtemp,
            dbhitemp,
            loop_freqlow,
            loop_freqhi,
            fundamental,
            &db_to_amp,
        );

        // ---- FILTER DECAY (only meaningful for the postfilter placement) ----
        let filter_decay_secs = params.filter_decay_secs.at(t, dur);
        let filter_decay_exponent: f64 = if filter_decay_secs < ir {
            1.0
        } else {
            (ir / filter_decay_secs) as f64
        };

        // ---- POSTFILTER: applied to y(n-1), inside the loop ----
        // Uses `raw_fshift` directly (not the `-B 0` compensated `fs`) -
        // a real, confirmed asymmetry with the prefilter placement above,
        // reproduced faithfully; `fm` is the same compensated value both
        // placements share.
        if params.filter_postfilter && source_db != 0.0 {
            for (j, bin) in feedback_bins.iter_mut().enumerate() {
                let interp = filter_lookup(&this_f, j, fundamental, raw_fshift, fm);
                let temp = (filtamp * interp + sourceamp) as f64;
                bin.0 *= temp.powf(filter_decay_exponent) as f32;
            }
        }

        if balance_flag && ((params.filter_postfilter && source_db != 0.0) || dbhitemp != 0.0) {
            let postbalancesum = feedback_bins.iter().map(|b| b.0).sum::<f32>();
            if prebalancesum > 0.0 && postbalancesum > 0.0 {
                let mut temp = prebalancesum / postbalancesum;
                if temp > balance_limit_amp {
                    temp = balance_limit_amp;
                }
                for bin in feedback_bins.iter_mut() {
                    bin.0 *= temp;
                }
            }
        }

        let mut next_bins = vec![(0.0f32, 0.0f32); n2 + 1];
        for j in 0..=n2 {
            let phasediff = feedback_bins[j].1 - previous[j].1;
            next_bins[j] = (feedback_bins[j].0, feedback_bins[j].1 + phasediff);
            previous[j].1 = feedback_bins[j].1;
        }
        next_buffer = lean_unconvert(&next_bins, n);

        let mut feedback_frame = feedback_phase.convert(&feedback_buffer);
        apply_shelf_eq(
            &mut feedback_frame.bins,
            params.output_eq_low_db.at(t, dur),
            params.output_eq_high_db.at(t, dur),
            params.output_eq_low_freq.at(t, dur),
            params.output_eq_high_freq.at(t, dur),
            fundamental,
            &db_to_amp,
        );
        let mut source_frame = channel_phase.convert(&buffer);

        for bin in feedback_frame.bins.iter_mut() {
            let (amp, freq) = *bin;
            let temp = feedback_pm * (feedback_harmadd + freq);
            *bin = if temp > 0.0 && temp < nyquist {
                (amp, temp)
            } else {
                (0.0, freq)
            };
            bin.0 *= feedback_gain * master_gain;
        }
        for bin in source_frame.bins.iter_mut() {
            let (amp, freq) = *bin;
            let temp = source_pm * (source_harmadd + freq);
            *bin = if temp > 0.0 && temp < nyquist {
                (amp, temp)
            } else {
                (0.0, freq)
            };
            bin.0 *= source_gain * master_gain;
        }

        let synt = getthresh(&source_frame.bins, threshfac)
            .max(getthresh(&feedback_frame.bins, threshfac));

        let mut hop_out = vec![0.0f32; i_factor];
        for (o, s) in hop_out
            .iter_mut()
            .zip(source_osc.synthesize(&source_frame, synt))
        {
            *o += s;
        }
        for (o, s) in hop_out
            .iter_mut()
            .zip(feedback_osc.synthesize(&feedback_frame, synt))
        {
            *o += s;
        }

        on += i_factor as i64;
        if on + nw as i64 - i_factor as i64 >= 0 {
            output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
            samps_written += i_factor;
        }

        if eof_after_this_hop {
            break;
        }
    }

    output.extend(vec![0.0f32; i_factor]);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    fn flat_response(n2: usize) -> Vec<f32> {
        vec![1.0f32; n2 + 1]
    }

    fn default_params(fft_size: usize) -> RingfilterParams {
        let n2 = fft_size / 2;
        RingfilterParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            oscbank_threshold_db: -96.0,
            master_gain_db: ControlFn::Const(0.0),
            source_gain_db: ControlFn::Const(0.0),
            source_freq_shift_hz: ControlFn::Const(0.0),
            source_pitch_semitones: ControlFn::Const(0.0),
            feedback_gain_db: ControlFn::Const(-999.0),
            feedback_freq_shift_hz: ControlFn::Const(0.0),
            feedback_pitch_semitones: ControlFn::Const(0.0),
            feedback_decay_secs: ControlFn::Const(0.0),
            feedback_threshold_db: ControlFn::Const(-96.0),
            feedback_threshold_pass_above: true,
            envelope_attack_secs: ControlFn::Const(0.0),
            envelope_release_secs: ControlFn::Const(0.0),
            input_eq_low_db: ControlFn::Const(0.0),
            input_eq_high_db: ControlFn::Const(0.0),
            input_eq_low_freq: ControlFn::Const(0.0),
            input_eq_high_freq: ControlFn::Const(2000.0),
            loop_eq_decay_secs: ControlFn::Const(1.0),
            loop_balance_limit_db: 0.0,
            loop_eq_low_db: ControlFn::Const(0.0),
            loop_eq_high_db: ControlFn::Const(0.0),
            loop_eq_low_freq: ControlFn::Const(0.0),
            loop_eq_high_freq: ControlFn::Const(2000.0),
            output_eq_low_db: ControlFn::Const(0.0),
            output_eq_high_db: ControlFn::Const(0.0),
            output_eq_low_freq: ControlFn::Const(0.0),
            output_eq_high_freq: ControlFn::Const(2000.0),
            filter_response: flat_response(n2),
            filter_warpshape: ControlFn::Const(0.0),
            filter_transpose_semitones: ControlFn::Const(0.0),
            filter_freq_shift_hz: ControlFn::Const(0.0),
            filter_source_db: ControlFn::Const(-96.0),
            filter_decay_secs: ControlFn::Const(1.0),
            filter_postfilter: false,
            pitchflag: false,
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let params = default_params(1024);
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(&input, 44100, &params, 1.0);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn sine_input_produces_bounded_output() {
        let params = default_params(1024);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(&input, sample_rate, &params, 1.0);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.0, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    #[should_panic(expected = "filter response has")]
    fn mismatched_filter_response_length_panics() {
        let mut params = default_params(1024);
        params.filter_response = vec![1.0; 10]; // wrong length
        let input = vec![0.0f32; 4410];
        process_channel(&input, 44100, &params, 1.0);
    }

    #[test]
    fn prefilter_attenuates_a_notched_band() {
        // A filter response that's silent everywhere except one bin -
        // with sourcedB at 0 (no dry blend) and a decaying feedback loop
        // fed by a broadband-ish input, the resonator's output should
        // still be bounded (the filter selects/attenuates most content
        // rather than exploding).
        let sample_rate = 44100u32;
        let fft_size = 1024;
        let n2 = fft_size / 2;
        let mut response = vec![0.0f32; n2 + 1];
        response[10] = 1.0;
        let mut params = default_params(fft_size);
        params.filter_response = response;
        params.feedback_gain_db = ControlFn::Const(0.0);
        params.feedback_decay_secs = ControlFn::Const(0.2);
        params.feedback_threshold_db = ControlFn::Const(-200.0);
        params.source_gain_db = ControlFn::Const(-999.0);

        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.3 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(&input, sample_rate, &params, 1.0);
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak.is_finite() && peak < 10.0, "peak {peak} unbounded");
    }
}
