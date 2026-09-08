//! Ports `tvfiltdeviator.c`: `tools::tvfilter`'s own time-varying
//! cross-synthetic filter (a live-analyzed raw-audio source, filtered by
//! a time-varying `.pva` response file navigated with the same
//! wrap/fold/clip sampler-loop machinery) plus two subsystems it doesn't
//! have: a per-bin, response-shaped time delay into the *source*'s own
//! delay line (conceptually `tools::delayfilter`'s per-bin fetch, but
//! into a plain ring rather than a `.pva` file), and a per-bin frequency
//! deviation shaped by a second, independently-warped copy of the same
//! response (`tools::filtdeviator`'s own "response-correlated frequency
//! deviation", reused here almost formula-for-formula).
//!
//! **The tool's own main filter fetch has the same stride bug already
//! found and reproduced in `tools::convolver`**: `tvfiltdeviator.c` calls
//! `makeInterpolatedFilterFrame(..., analysis_N, ...)`, but that
//! function's own parameter is named `analysis_Nplus2` - passing
//! `analysis_N` instead of `analysis_N + 2` makes every frame fetch
//! start `2` floats short of where the next real frame actually begins,
//! drifting further every frame. Confirmed by reading
//! `legacy/pvc_lib/makeInterpolatedFilterFrame.c` directly, the same
//! function `twarp.c`/`tvfilter.c`/`ringtvfilter.c` also call - correctly,
//! with `analysis_N + 2` - and the same one `convolver.c` calls
//! incorrectly. Reproduced here via [`buggy_filter_frame`], adapted from
//! `tools::convolver`'s own (not exported - a small enough helper that
//! duplicating it here beats a forced shared abstraction across two
//! otherwise-unrelated tools). That same function's `filtfprop` is
//! declared `int`, not `float` (confirmed in the same file) - the same
//! "declared-as-int, never actually interpolates" bug `crate::timenav::
//! interpolate_frame` already documents for `twarp.c`'s own, correctly-
//! called, use of this function - so `buggy_filter_frame` returns a
//! single floor-frame fetch, not an interpolation, matching the real
//! compiled behavior.
//!
//! **`normalizeLoopAmplitudes()`'s own internal frame fetches are not
//! affected by that bug** - it receives `analysis_N + 2` as an explicit,
//! correct parameter at its own call site (confirmed by reading that
//! call), so `tools::tvfilter::LoopNormalizer` (which already reproduces
//! that shared function against a correctly-parsed per-channel frame
//! list) is reused here unmodified, fed from a *separately* correctly-
//! parsed frame list - not from the same buggy raw stream the main fetch
//! uses.
//!
//! **A second, narrower stride bug in the frequency-deviation and time-
//! delay response warps**: `spectmagwarp(Ffreq, N, ...)` and
//! `spectmagwarp(FtimeDelay, N, ...)` are both called with the *source
//! audio*'s own FFT size `N`, not `analysis_N + 2` (`spectmagwarp`'s own
//! second parameter, confirmed by reading `legacy/pvc_lib/
//! spectmagwarp.c`, is `Nplus2` - a literal array length). Whenever `N`
//! differs from `analysis_N + 2` (an independent response FFT size, this
//! tool's whole reason for having its own `N_ratio`), only the first `N`
//! floats of each warped copy actually get warped; the remainder keeps
//! whatever `F`'s own (already-shaped) value was. Reproduced here by
//! slicing to `..N` (never `analysis_N + 2`) before calling
//! `spectmagwarp` on those two copies specifically - `F`'s own warp call
//! passes the correct full length and is unaffected.
//!
//! **A confirmed, real difference from `tools::filtdeviator`'s own
//! otherwise-identical frequency-deviation formulas**: file mode here
//! smooths `freqdevmode`'s own value through the same one-pole filter
//! random mode uses (`ranfreqdevresponse`/`previous_ranfreqv`) before
//! using it as a blend weight; `filtdeviator.c`'s own file mode uses the
//! raw, unsmoothed value. Confirmed by reading both files side by side,
//! not assumed from the otherwise near-identical surrounding formulas -
//! reproduced as read.
//!
//! **No per-bin control-function time-shift** (unlike `tools::
//! filtdeviator`'s own `t - scaler*delay`): every `fval()` call in this
//! tool's per-frame setup happens once, at plain `t`, before the per-bin
//! loop even starts - confirmed by reading the whole loop, not assumed
//! from the sibling tool's own pattern.
//!
//! **Random frequency deviation mode (`-U 1`) is not ported**, matching
//! `tools::ring`'s own established `randf()` precedent - `[`FreqDevMode`]
//! only distinguishes response/file modes, the same restriction
//! `tools::filtdeviator`'s own `FreqDevMode` already applies.
//!
//! **A third instance of the "`usage()` claims one default, the
//! initializer sets another" bug** (after `pvc delayfilter`'s `-T` and
//! `pvc filtdeviator`'s `-q`/`-B`): `-O`'s own `usage()` text claims
//! `[1.]`, but `fdevcontrol.A[0] = 0.` in the real initializer - unlike
//! `filtdeviator.c`'s *own* `-O` (whose default genuinely is `1.`,
//! matching its own `usage()` text correctly), so this is a fresh,
//! independent instance of the bug in this tool, not one inherited from
//! the sibling it otherwise borrows so much from. Reproduced here: this
//! port's own default is `0.`, matching the real behavior.
//!
//! **`ringTime`, a global, extends the output past the trimmed input's
//! own length** exactly the way `tools::filtdeviator`'s own doc comment
//! describes - set here from the time delay's own max, read back by
//! `legacy/pvc_lib/fileio.c`'s `shiftin`-adjacent code. Modeled by
//! padding the input with that many zero samples, same as that sibling.
//!
//! Not ported (no source mixing exists in this tool at all - `sourceflag`
//! and every reference to a separately-delayed/gained "source" signal are
//! commented out in `main()`, confirmed by reading the whole file):
//! anything resembling `tools::filter`'s or `tools::filtdeviator`'s own
//! additive source path. `sourcedB`/`filtamp`/`sourceamp` here instead
//! blend the filtered response against an unfiltered floor of the *same*
//! signal, exactly like `tools::tvfilter`'s own `filter_source_db`.

use crate::eq::eq;
use crate::filter_response::smooth_response;
use crate::pvoc::{getthresh, Analyzer, Frame, OscBank, Synthesizer};
use crate::smooth::{smooth_setup, Smoother};
use crate::timenav::{make_loop_smooth_time, LoopMode, TimeNavConfig, TimeNavigator};
use crate::tools::tvfilter::{compress_response, normalize_response, InvertMode, LoopNormalizer};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

const OSCILBANKGAIN: f32 = 1.7782794;

/// `-U`'s own mode, resolved once at startup - see this module's doc
/// comment on why "random mode" has no variant here.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FreqDevMode {
    Response,
    File,
}

impl FreqDevMode {
    pub fn resolve(freq_dev_mode: &ControlFn) -> FreqDevMode {
        match freq_dev_mode {
            ControlFn::Table(_) => FreqDevMode::File,
            ControlFn::Const(v) if *v == 0.0 => FreqDevMode::Response,
            other => panic!(
                "tvfiltdeviator: illegal frequency deviation mode {other:?} (only response mode \
                 [0] and file mode [a table] are supported - random mode [1] is not ported, see \
                 pvc-core::tools::tvfiltdeviator's doc comment)"
            ),
        }
    }
}

pub struct TvfiltdeviatorParams {
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,

    pub time_origin: ControlFn,
    pub rate: ControlFn,
    pub window_low: ControlFn,
    pub window_high: ControlFn,
    pub loop_mode: LoopMode,
    pub onset_release: bool,
    pub autostop: bool,
    pub loop_normalization: bool,
    pub peak_loop_smooth_time: ControlFn,

    pub pitch_transpose: ControlFn,
    pub freq_shift: ControlFn,
    pub gain_db: ControlFn,
    pub pitchflag: bool,

    pub invert_mode: InvertMode,

    pub filter_transpose: ControlFn,
    pub filter_shift: ControlFn,
    pub filter_release_secs: ControlFn,
    pub filter_attack_secs: ControlFn,
    pub filter_source_db: ControlFn,
    pub filter_warpshape: ControlFn,
    pub filter_smoothing_bw: ControlFn,

    pub comp_threshold_db: ControlFn,
    pub comp_db: ControlFn,

    pub shelf_low_db: f32,
    pub shelf_high_db: f32,
    pub shelf_low_freq: f32,
    pub shelf_high_freq: f32,

    pub attack_secs: ControlFn,
    pub release_secs: ControlFn,

    pub time_delay_base: ControlFn,
    pub time_delay_peak: ControlFn,
    pub time_delay_dev_control: ControlFn,
    pub time_delay_warpshape: ControlFn,

    pub freq_dev_base: ControlFn,
    pub freq_dev_peak: ControlFn,
    pub freq_shift_dev_base: ControlFn,
    pub freq_shift_dev_peak: ControlFn,
    pub freq_dev_control: ControlFn,
    pub freq_dev_mode: ControlFn,
    pub freq_dev_response_secs: ControlFn,
    pub freq_warpshape: ControlFn,

    pub threshold_db: f32,
}

fn control_fn_max(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(vals) => vals.iter().copied().fold(f32::MIN, f32::max),
    }
}

/// Reproduces `tvfiltdeviator.c`'s own buggy filter-frame fetch (see this
/// module's doc comment). `raw_stream` is the *true* on-disk interleaved
/// float stream (all channels' frames, real `analysis_n + 2` stride,
/// frame-major then channel-minor), reconstructed by the caller from an
/// already-correctly-parsed `pvc_io::PvaData`. Returns `analysis_n + 2`
/// amplitude/frequency floats; anything past the drifted `analysis_n`
/// floats actually read is `0.0`.
fn buggy_filter_frame(
    raw_stream: &[f32],
    analysis_n: usize,
    analysis_chan: usize,
    ainchan: usize,
    filtflow: i64,
) -> Vec<f32> {
    let mut out = vec![0.0f32; analysis_n + 2];
    if filtflow < 0 {
        return out;
    }
    let start = (ainchan as i64 + analysis_chan as i64 * filtflow) as usize * analysis_n;
    if start >= raw_stream.len() {
        return out;
    }
    let end = (start + analysis_n).min(raw_stream.len());
    out[..end - start].copy_from_slice(&raw_stream[start..end]);
    out
}

/// Ports the shared per-bin shift/transpose index math, applied
/// identically to `f_amp`/`f_freq` (the "in array"/"under" cases) but
/// with a genuinely different "over the array" fallback for `f_delay`
/// than for the other two - see this module's doc comment on why: the C
/// reassigns `i1`/`i2` to the *source audio*'s own `N - 2` there, not
/// `analysis_N - 2`, only for the array holding the ultimately-unused
/// reassigned indices' own lerp (`f_delay`'s), while `f_amp`/`f_freq`
/// bypass the reassigned indices entirely and read `analysis_N - 2`
/// directly. All three arrays share one length (`analysis_n2 + 1`
/// pairs); `source_n2` is the source audio's own bin count, used only
/// for `f_delay`'s own over-array fallback index.
#[allow(clippy::too_many_arguments)]
fn per_bin_lookup(
    f_amp: &[f32],
    f_freq: &[f32],
    f_delay: &[f32],
    bin: usize,
    n_ratio: f32,
    fundamental: f32,
    shift_hz: f32,
    transpose_mult: f32,
    source_n2: usize,
) -> (f32, f32, f32) {
    let analysis_n2 = f_amp.len() - 1;
    let mut temp = bin as f32 * n_ratio;
    temp -= shift_hz / fundamental;
    temp /= transpose_mult;
    let i1 = temp as i64;
    let i2p = temp - i1 as f32;
    let i1p = 1.0 - i2p;
    let i2 = i1 + 1;
    if i1 < 0 {
        (f_amp[0], f_freq[0], f_delay[0])
    } else if i2 as usize >= analysis_n2 {
        (
            f_amp[analysis_n2 - 1],
            f_freq[analysis_n2 - 1],
            f_delay[source_n2 - 1],
        )
    } else {
        let (i1, i2) = (i1 as usize, i2 as usize);
        (
            f_amp[i1] * i1p + f_amp[i2] * i2p,
            f_freq[i1] * i1p + f_freq[i2] * i2p,
            f_delay[i1] * i1p + f_delay[i2] * i2p,
        )
    }
}

fn interleave(amps: &[f32]) -> Vec<f32> {
    let mut out = vec![0.0f32; amps.len() * 2];
    for (j, &a) in amps.iter().enumerate() {
        out[2 * j] = a;
    }
    out
}

fn deinterleave_amps(interleaved: &[f32], amps: &mut [f32]) {
    for (j, a) in amps.iter_mut().enumerate() {
        *a = interleaved[2 * j];
    }
}

/// Resynthesizes one channel. `filter_raw_stream` is the response file's
/// *true* on-disk interleaved frame stream (see [`buggy_filter_frame`]'s
/// doc comment); `filter_frames` is the *same* file's correctly-parsed
/// per-channel frame list (`pvc_io::PvaData::channels[ainchan]`), used
/// only by the loop-normalization path. `analysis_peak_amp` is that
/// channel's own stored peak amplitude.
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    input: &[f32],
    filter_raw_stream: &[f32],
    filter_frames: &[Vec<f32>],
    analysis_n: usize,
    analysis_d: u32,
    analysis_sample_rate: u32,
    analysis_chan: usize,
    ainchan: usize,
    analysis_peak_amp: f32,
    fft_size: usize,
    sample_rate: u32,
    params: &TvfiltdeviatorParams,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = fft_size;
    let n2 = n / 2;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;
    let n_ratio = analysis_n as f32 / n as f32;
    let analysis_n2 = analysis_n / 2;
    let analysis_fundamental = nyquist / analysis_n2 as f32;

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
    let filttinc = d as f32 / r;

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
    let freq_dev_mode = FreqDevMode::resolve(&params.freq_dev_mode);

    let iframes_per_sec = analysis_sample_rate as f32 / analysis_d as f32;
    let niframes = filter_frames.len();
    let analysis_dur = niframes as f32 / iframes_per_sec;

    let mut dur = (input.len() as f32 / r) * time_factor;

    // ---- RING-TIME INPUT PADDING (see this module's doc comment) ----
    let max_delay_t = control_fn_max(&params.time_delay_base)
        .max(control_fn_max(&params.time_delay_peak))
        .max(0.0);
    let max_num_delay_frames = 1 + (max_delay_t * frames_per_sec + 0.5) as usize;
    let ring_time_samples = (max_delay_t * r) as usize;
    let padded_input;
    let input: &[f32] = if ring_time_samples > 0 {
        let mut v = input.to_vec();
        v.extend(std::iter::repeat_n(0.0f32, ring_time_samples));
        padded_input = v;
        &padded_input
    } else {
        input
    };

    let obank = !(matches!(&params.pitch_transpose, ControlFn::Const(v) if *v == 0.0)
        && matches!(&params.freq_shift, ControlFn::Const(v) if *v == 0.0));

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);
    let mut synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, sample_rate);
    let mut osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut channel_smoother = Smoother::new(n + 2);
    let mut filter_smoother = Smoother::new(analysis_n + 2);
    let mut loop_normalizer = LoopNormalizer::new();

    let mut channel_ring: Vec<Vec<f32>> = vec![vec![0.0f32; n + 2]; max_num_delay_frames];
    let mut previous_ranfreqv = vec![0.0f32; n2 + 1];

    let nav_cfg = TimeNavConfig {
        onset_release: params.onset_release,
        autostop: params.autostop,
        loop_mode: params.loop_mode,
    };
    let initial_time_origin = params.time_origin.at(0.0, dur);
    let mut nav = TimeNavigator::new(nav_cfg, initial_time_origin);

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;
    let mut on: i64 = (-(nw as i64) * i_factor as i64) / d as i64;

    let mut output = Vec::new();
    let mut samps_written: usize = 0;
    let mut frame_count: usize = 0;

    loop {
        let mut hop = vec![0.0f32; d];
        if valid == nw as i64 {
            let available = d.min(input.len().saturating_sub(pos));
            hop[..available].copy_from_slice(&input[pos..pos + available]);
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

        let channel_frame = analyzer.push(&hop).expect("hop is exactly d samples");
        let channel_flat = channel_frame.to_pva_floats();

        let now_index = frame_count % max_num_delay_frames;
        channel_ring[now_index] = channel_flat.clone();

        // ---- FILTER TIME NAVIGATION ----
        let time_origin_val = params.time_origin.at(t, dur);
        let rate_val = params.rate.at(t, dur);
        let win_low_val = params.window_low.at(t, dur);
        let win_hi_raw = params.window_high.at(t, dur);
        let win_hi_val = if win_hi_raw < 0.0 {
            analysis_dur
        } else {
            win_hi_raw
        };

        let step = nav.advance(
            filttinc,
            time_origin_val,
            rate_val,
            win_low_val,
            win_hi_val,
            analysis_dur,
            t,
            &mut dur,
        );

        let peak_loop_smooth_time = params.peak_loop_smooth_time.at(t, dur);
        let loop_smooth_time = make_loop_smooth_time(
            step.filttnow,
            params.loop_mode,
            params.autostop,
            win_low_val,
            win_hi_val,
            peak_loop_smooth_time,
        );

        // ---- MAIN (BUGGY-STRIDE) FILTER FETCH ----
        let filtflow = (iframes_per_sec * step.filttnow) as i64;
        let mut f_amp_full = buggy_filter_frame(
            filter_raw_stream,
            analysis_n,
            analysis_chan,
            ainchan,
            filtflow,
        );

        if params.loop_normalization {
            loop_normalizer.apply(
                &mut f_amp_full,
                filter_frames,
                iframes_per_sec,
                win_low_val,
                win_hi_val,
                analysis_dur,
                step.filttnow,
            );
        }

        {
            let mut amps: Vec<f32> = f_amp_full.iter().step_by(2).copied().collect();
            normalize_response(&mut amps, analysis_peak_amp);
            for (dst, src) in f_amp_full.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }

        let comp_threshold_db = params.comp_threshold_db.at(t, dur);
        let comp_db = params.comp_db.at(t, dur);
        assert!(
            comp_threshold_db <= 0.0,
            "tvfiltdeviator: compression threshold must be < 0dB"
        );
        assert!(
            comp_db <= 0.0,
            "tvfiltdeviator: compression decibels must be < 0dB"
        );
        let comp_threshold_amp = db_to_amp.convert(comp_threshold_db);
        let comp_amp = db_to_amp.convert(comp_db);
        let comp_norm_amp = 1.0 / (comp_threshold_amp + comp_amp * (1.0 - comp_threshold_amp));
        if comp_threshold_amp < 1.0 && comp_amp < 1.0 {
            let mut amps: Vec<f32> = f_amp_full.iter().step_by(2).copied().collect();
            compress_response(&mut amps, comp_threshold_amp, comp_amp, comp_norm_amp);
            for (dst, src) in f_amp_full.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }

        match params.invert_mode {
            InvertMode::Pass => {}
            InvertMode::InvertFixedPeak | InvertMode::InvertFramePeak => {
                let mut amps: Vec<f32> = f_amp_full.iter().step_by(2).copied().collect();
                crate::filter_response::invert_response(
                    &mut amps,
                    params.invert_mode == InvertMode::InvertFramePeak,
                    &db_to_amp,
                );
                for (dst, src) in f_amp_full.iter_mut().step_by(2).zip(&amps) {
                    *dst = *src;
                }
            }
        }

        let filter_warpshape = params.filter_warpshape.at(t, dur);
        spectmagwarp(&mut f_amp_full, filter_warpshape, false);

        eq(
            &mut f_amp_full,
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

        let smoothing_bw = params.filter_smoothing_bw.at(t, dur);
        {
            let mut amps: Vec<f32> = f_amp_full.iter().step_by(2).copied().collect();
            smooth_response(&mut amps, smoothing_bw, sample_rate);
            for (dst, src) in f_amp_full.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }

        let (freleasec, minus_freleasec) =
            smooth_setup(params.filter_release_secs.at(t, dur) + loop_smooth_time, ir);
        let (fattackc, minus_fattackc) =
            smooth_setup(params.filter_attack_secs.at(t, dur) + loop_smooth_time, ir);
        filter_smoother.smooth(
            &mut f_amp_full,
            fattackc,
            minus_fattackc,
            freleasec,
            minus_freleasec,
        );

        // ---- BUILD THE TWO EXTRA, INDEPENDENTLY-WARPED RESPONSE COPIES ----
        // `spectmagwarp` is called on these with the *source audio*'s
        // own `N`, not `analysis_N + 2` - see this module's doc comment.
        // Represented here as pair arrays (length `analysis_n2 + 1`); the
        // slice bound is expressed in that same unit (`n2`, clamped to
        // the array's own length).
        let f_amp: Vec<f32> = f_amp_full.iter().step_by(2).copied().collect();
        let mut f_freq = f_amp.clone();
        let mut f_delay = f_amp.clone();
        let warp_bound = n2.min(analysis_n2 + 1);

        let freq_warpshape = params.freq_warpshape.at(t, dur);
        {
            let mut interleaved = interleave(&f_freq[..warp_bound]);
            spectmagwarp(&mut interleaved, freq_warpshape, true);
            deinterleave_amps(&interleaved, &mut f_freq[..warp_bound]);
        }
        let time_delay_warpshape = params.time_delay_warpshape.at(t, dur);
        {
            let mut interleaved = interleave(&f_delay[..warp_bound]);
            spectmagwarp(&mut interleaved, time_delay_warpshape, true);
            deinterleave_amps(&interleaved, &mut f_delay[..warp_bound]);
        }

        // ---- PER-FRAME CONTROL VALUES ----
        let harmadd = params.freq_shift.at(t, dur);
        let gain = db_to_amp.convert(params.gain_db.at(t, dur));
        let pm = semitones_to_mult.convert(params.pitch_transpose.at(t, dur));

        let mut fm = semitones_to_mult.convert(params.filter_transpose.at(t, dur));
        let mut fs = params.filter_shift.at(t, dur);
        if !params.pitchflag {
            fs -= harmadd;
            fm /= pm;
        }
        let source_db = params.filter_source_db.at(t, dur);
        let sourceamp = db_to_amp.convert(source_db);
        let filtamp = 1.0 - sourceamp;

        let (releasec, minus_releasec) = smooth_setup(params.release_secs.at(t, dur), ir);
        let (attackc, minus_attackc) = smooth_setup(params.attack_secs.at(t, dur), ir);

        let fdev_base = params.freq_dev_base.at(t, dur);
        let fdev_peak = params.freq_dev_peak.at(t, dur);
        let fdm_diff = fdev_peak - fdev_base;
        let fshift_base = params.freq_shift_dev_base.at(t, dur);
        let fshift_peak = params.freq_shift_dev_peak.at(t, dur);
        let fs_diff = fshift_peak - fshift_base;
        let ranfreqdevresponse = params.freq_dev_response_secs.at(t, dur);
        let (ranfreqsmoothc, minus_ranfreqsmoothc) = smooth_setup(ranfreqdevresponse, ir);
        let freqdevmode_val = params.freq_dev_mode.at(t, dur);
        let fdevcontrol = params.freq_dev_control.at(t, dur);
        let fdevminus = 1.0 - fdevcontrol;

        let time_delay_base = params.time_delay_base.at(t, dur);
        let time_delay_peak = params.time_delay_peak.at(t, dur);
        let time_range = time_delay_peak - time_delay_base;
        let time_delay_dev_control = params.time_delay_dev_control.at(t, dur);

        // ---- PER-BIN LOOP ----
        let mut channel_filter = vec![0.0f32; n + 2];
        for j in 0..=n2 {
            let (this_f, this_ffreq, interp_tdelay) = per_bin_lookup(
                &f_amp,
                &f_freq,
                &f_delay,
                j,
                n_ratio,
                analysis_fundamental,
                fs,
                fm,
                n2 + 1,
            );

            let this_delay_t =
                time_delay_dev_control * (time_delay_base + interp_tdelay * time_range);
            let delay_frames = (this_delay_t * frames_per_sec + 0.5) as i64;
            let mut this_frame_delay = now_index as i64 - delay_frames;
            while this_frame_delay < 0 {
                this_frame_delay += max_num_delay_frames as i64;
            }
            let delayed = &channel_ring[(this_frame_delay as usize) % max_num_delay_frames];
            channel_filter[2 * j] = delayed[2 * j];
            channel_filter[2 * j + 1] = delayed[2 * j + 1];

            channel_filter[2 * j] *= filtamp * this_f + sourceamp;

            match freq_dev_mode {
                FreqDevMode::File => {
                    let smoothed = ranfreqsmoothc * previous_ranfreqv[j]
                        + minus_ranfreqsmoothc * freqdevmode_val;
                    previous_ranfreqv[j] = smoothed;
                    let fully_deviated = semitones_to_mult.convert(fdev_base + smoothed * fdm_diff)
                        * (channel_filter[2 * j + 1] + (fshift_base + smoothed * fs_diff));
                    channel_filter[2 * j + 1] +=
                        this_ffreq * fdevcontrol * (fully_deviated - channel_filter[2 * j + 1]);
                }
                FreqDevMode::Response => {
                    let amp_factor = fdevminus
                        + fdevcontrol
                            * semitones_to_mult.convert(fdev_base + this_ffreq * fdm_diff);
                    channel_filter[2 * j + 1] = amp_factor
                        * (channel_filter[2 * j + 1]
                            + fdevcontrol * (fshift_base + this_ffreq * fs_diff));
                }
            }
        }

        // `if (!frame_count) previous_channel_filter[i] = channel[i];` -
        // seeds the smoother's own "previous" state from the *fresh,
        // undelayed* analysis frame on the first frame only, not from
        // `channel_filter`'s own delayed/deviated content that
        // `Smoother::new`'s usual auto-seed-from-first-call would use.
        // `smooth()`'s own attack/release math is a no-op when both
        // coefficients are `0.0`/`1.0` respectively, so this call only
        // primes the internal state, exactly matching the C's plain
        // array copy.
        if frame_count == 0 {
            let mut seed = channel_flat.clone();
            channel_smoother.smooth(&mut seed, 0.0, 1.0, 0.0, 1.0);
        }

        channel_smoother.smooth(
            &mut channel_filter,
            attackc,
            minus_attackc,
            releasec,
            minus_releasec,
        );

        for j in 0..=n2 {
            let temp = pm * (channel_filter[2 * j + 1] + harmadd);
            if temp <= 0.0 || temp >= nyquist {
                channel_filter[2 * j] = 0.0;
            } else {
                channel_filter[2 * j + 1] = temp;
            }
            channel_filter[2 * j] *= gain;
        }

        let frame = Frame::from_pva_floats(&channel_filter);
        let threshfac = db_to_amp.convert(params.threshold_db);
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
        if eof_after_this_hop {
            break;
        }
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

    fn default_params() -> TvfiltdeviatorParams {
        TvfiltdeviatorParams {
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            time_origin: ControlFn::Const(0.0),
            rate: ControlFn::Const(1.0),
            window_low: ControlFn::Const(0.0),
            window_high: ControlFn::Const(-1.0),
            loop_mode: LoopMode::Wrap,
            onset_release: false,
            autostop: false,
            loop_normalization: false,
            peak_loop_smooth_time: ControlFn::Const(0.2),
            pitch_transpose: ControlFn::Const(0.0),
            freq_shift: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            pitchflag: false,
            invert_mode: InvertMode::Pass,
            filter_transpose: ControlFn::Const(0.0),
            filter_shift: ControlFn::Const(0.0),
            filter_release_secs: ControlFn::Const(0.0),
            filter_attack_secs: ControlFn::Const(0.0),
            filter_source_db: ControlFn::Const(-96.0),
            filter_warpshape: ControlFn::Const(0.0),
            filter_smoothing_bw: ControlFn::Const(0.0),
            comp_threshold_db: ControlFn::Const(0.0),
            comp_db: ControlFn::Const(0.0),
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            attack_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
            time_delay_base: ControlFn::Const(0.0),
            time_delay_peak: ControlFn::Const(0.0),
            time_delay_dev_control: ControlFn::Const(1.0),
            time_delay_warpshape: ControlFn::Const(0.0),
            freq_dev_base: ControlFn::Const(0.0),
            freq_dev_peak: ControlFn::Const(0.0),
            freq_shift_dev_base: ControlFn::Const(0.0),
            freq_shift_dev_peak: ControlFn::Const(0.0),
            freq_dev_control: ControlFn::Const(0.0),
            freq_dev_mode: ControlFn::Const(0.0),
            freq_dev_response_secs: ControlFn::Const(0.0),
            freq_warpshape: ControlFn::Const(0.0),
            threshold_db: -60.0,
        }
    }

    fn flat_raw_stream(analysis_n: usize, count: usize) -> Vec<f32> {
        vec![1.0f32; count * (analysis_n + 2)]
    }

    fn flat_frames(analysis_n: usize, count: usize) -> Vec<Vec<f32>> {
        vec![vec![1.0f32; analysis_n + 2]; count]
    }

    #[test]
    fn silence_in_silence_out() {
        let fft = 1024;
        let raw = flat_raw_stream(fft, 50);
        let frames = flat_frames(fft, 50);
        let params = default_params();
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(
            &input, &raw, &frames, fft, 220, 44100, 1, 0, 1.0, fft, 44100, &params,
        );
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn flat_response_passes_sine_input_through_at_bounded_amplitude() {
        let fft = 1024;
        let raw = flat_raw_stream(fft, 50);
        let frames = flat_frames(fft, 50);
        let params = default_params();
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(
            &input,
            &raw,
            &frames,
            fft,
            220,
            sample_rate,
            1,
            0,
            1.0,
            fft,
            sample_rate,
            &params,
        );
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.1, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn nonzero_time_delay_runs_without_panicking_and_stays_bounded() {
        let fft = 1024;
        let raw = flat_raw_stream(fft, 50);
        let frames = flat_frames(fft, 50);
        let mut params = default_params();
        params.time_delay_base = ControlFn::Const(0.02);
        params.time_delay_peak = ControlFn::Const(0.05);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(
            &input,
            &raw,
            &frames,
            fft,
            220,
            sample_rate,
            1,
            0,
            1.0,
            fft,
            sample_rate,
            &params,
        );
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak.is_finite());
        assert!(peak < 10.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn oscbank_path_selected_when_pitch_shifted() {
        let fft = 1024;
        let raw = flat_raw_stream(fft, 50);
        let frames = flat_frames(fft, 50);
        let mut params = default_params();
        params.pitch_transpose = ControlFn::Const(7.0);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(
            &input,
            &raw,
            &frames,
            fft,
            220,
            sample_rate,
            1,
            0,
            1.0,
            fft,
            sample_rate,
            &params,
        );
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.01, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn file_mode_freq_deviation_runs_without_panicking() {
        let fft = 1024;
        let raw = flat_raw_stream(fft, 50);
        let frames = flat_frames(fft, 50);
        let mut params = default_params();
        params.freq_dev_mode = ControlFn::Table(vec![0.0, 1.0, 0.5]);
        params.freq_dev_peak = ControlFn::Const(2.0);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(
            &input,
            &raw,
            &frames,
            fft,
            220,
            sample_rate,
            1,
            0,
            1.0,
            fft,
            sample_rate,
            &params,
        );
        assert!(!output.is_empty());
        assert!(output.iter().all(|s| s.is_finite()));
    }

    #[test]
    #[should_panic(expected = "illegal frequency deviation mode")]
    fn random_freq_dev_mode_is_rejected() {
        FreqDevMode::resolve(&ControlFn::Const(1.0));
    }
}
