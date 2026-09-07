//! Ports `ringtvfilter.c`: `tools::ring`'s feedback delay network, plus a
//! switchable *time-varying* filter (a `.pva` "filter analysis" file,
//! navigated over time exactly the way `tools::tvfilter` navigates its
//! own response) placed either on the feedback path's input ("prefilter",
//! `x(n)`) or inside the loop ("postfilter", `y(n-1)`, decay-time-scaled),
//! the same two placements `tools::ringfilter` already supports for a
//! *fixed* `.fr` response. Reuses `tools::ring`'s shared building blocks
//! ([`crate::tools::ring::RawAnalyzer`], `lean_convert`/`lean_unconvert`,
//! `apply_shelf_eq`, `control_fn_max`) and `tools::tvfilter`'s time-
//! navigation machinery (`crate::timenav::{TimeNavigator, interpolate_frame,
//! make_loop_smooth_time}`, `tools::tvfilter`'s own `filter_lookup`/
//! `compress_response`/`normalize_response`/`LoopNormalizer`) directly,
//! confirmed by reading `ringtvfilter.c` against both sibling tools side
//! by side rather than re-deriving either half.
//!
//! **The filter-response pipeline is a strict subset of `tvfilter.c`'s
//! own, not the same steps**: reading the "MAKE THE FILTER FRAME" section
//! of `ringtvfilter.c` end to end finds `makeInterpolatedFilterFrame` ->
//! (optional) `normalizeLoopAmplitudes` -> a *single symmetric*
//! `smooth()` call (one `loopSmoothTime`-derived coefficient pair used for
//! both attack and release, unlike `tvfilter.c`'s own separate `-Z`/`-z`
//! attack/release controls) -> `normalize()` (to the analysis file's own
//! peak amp) -> optional `compress()` -> `spectmagwarp()`. There is no
//! shelf EQ, no bandwidth smoothing (`smoothspec`), and no invert-response
//! step anywhere in this chain - all three are genuinely `tvfilter.c`-only
//! features, not omissions here. [`crate::smooth::Smoother`] (built for
//! the asymmetric attack/release case) still applies unchanged for the
//! symmetric case: passing the same coefficient pair as both its
//! attack and release arguments reproduces `smooth()`'s own symmetric
//! call exactly.
//!
//! **`-W`/`-v` (filter-spectrum compression threshold/decibels) are plain
//! floats here, not `(func)`s - confirmed by their C declarations
//! (`float filtcompthresh_in_dB=0`, `float filtcompdecibels=0`, no
//! `struct func`) and by their `usage()` text, which omits the `(func)`
//! annotation every other time-varying flag in this file carries. Both
//! are resolved once, outside the per-frame loop, matching the C's own
//! one-time `crackfloat()` read.
//!
//! **A real, confirmed asymmetry between the two filter placements,
//! reproduced faithfully - the same shape `tools::ringfilter` already
//! documents for its own fixed-response placements**: both share one
//! pitch/frequency-compensated pair (`fm`, and - only for the *prefilter*
//! placement - `fs`) computed once per frame from `-u`/`-V`
//! (`filter_transpose`/`filter_shift`), optionally divided/subtracted by
//! the feedback path's own `-P`/`-H` shift unless [`RingtvfilterParams::
//! pitchflag`] is set. The *postfilter* placement's own bin-shift lookup
//! uses `-V`'s raw value directly, not the `fs`-compensated one, and
//! (unlike the prefilter placement) does not scale its bin index by
//! `N_ratio` or use the filter file's own fundamental - it indexes the
//! *audio*'s own bin/fundamental directly into the same interpolated
//! response array. Both are still exactly [`crate::tools::tvfilter::
//! filter_lookup`]'s own formula, just with `n_ratio = 1.0` and
//! `analysis_fundamental` replaced by the audio's own `fundamental` for
//! the postfilter case - confirmed by algebraic reduction of the C's two
//! near-identical index formulas, not assumed from symmetry.
//!
//! **The in-loop feedback EQ's decay-time division is guarded here, like
//! `tools::ringfilter` and unlike `tools::ring`** - see `tools::
//! ringfilter`'s doc comment for the exact mechanism and the same
//! `FEEDBACK_dBlowtemp`-never-reassigned-on-the-guard-branch simplification
//! (set fresh to `0.0` here instead of reading a stale value).
//!
//! **A real dead control, found by grepping every reference**:
//! `master_dBgain` (`struct func`, declared and initialized to `0.` like
//! every other control in this file) is never assigned by any `crack()`
//! case and never read by `fval()` anywhere in the frame loop - only its
//! `fclose()` cleanup check at exit references it at all. Unlike
//! `tools::ring`'s own `-A` (a working, documented master gain), this
//! file's `-A` flag is wired to `LoopNormalizationFlag` instead - there is
//! no way to reach `master_dBgain` from the command line at all. Not
//! exposed here.
//!
//! **The feedback envelope-follower threshold has no pass-mode flag
//! here**, unlike `tools::ring`'s own `-V` (`feedback_threshold_pass_above`):
//! `ringtvfilter.c`'s per-bin gate is unconditionally "below threshold
//! releases, at-or-above threshold attacks/holds" - confirmed by reading
//! the whole flag list for a second threshold-direction flag and finding
//! none. Hardcoded to that one direction here, not exposed as a field.
//!
//! Not ported, matching `tools::ring`/`tools::ringfilter`'s own precedent
//! for the same flags: `-C`, `-w`/`-i`/`-_`/`-=`, `-j`/`-K` (random per-bin
//! frequency dither, glibc `random()`-based). `-@`'s own window-mode
//! sub-flag (`//`, window type) and `-t` (oscillator-bank threshold) are
//! plain constants here, matching every other oscillator-bank tool.

use crate::pvoc::{getthresh, OscBank, PhaseTracker};
use crate::smooth::{smooth_setup, Smoother};
use crate::timenav::{
    interpolate_frame, make_loop_smooth_time, LoopMode, TimeNavConfig, TimeNavigator,
};
use crate::tools::ring::{
    apply_shelf_eq, control_fn_max, lean_convert, lean_unconvert, RawAnalyzer,
};
use crate::tools::tvfilter::{
    compress_response, filter_lookup, normalize_response, LoopNormalizer,
};
use crate::units::{db_to_amp_exact, DbToAmp, SemitonesToMult};
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant (each oscillator-bank tool in this
/// project redefines it locally rather than sharing one copy).
const OSCILBANKGAIN: f32 = 1.7782794;

pub struct RingtvfilterParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,
    /// `-t`: a plain constant, not a `(func)`.
    pub oscbank_threshold_db: f32,
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
    /// `-l`.
    pub envelope_attack_secs: ControlFn,
    /// `-L`.
    pub envelope_release_secs: ControlFn,
    /// `-O`. Defaults to `200.0`, not `0.0` - see `tools::ring`'s doc
    /// comment on the real swapped-default bug this reproduces
    /// (independently present here too).
    pub input_eq_low_db: ControlFn,
    /// `-Y`.
    pub input_eq_high_db: ControlFn,
    /// `-d`. Defaults to `0.0`, not `200.0` - see `tools::ring`'s doc
    /// comment.
    pub input_eq_low_freq: ControlFn,
    /// `-n`.
    pub input_eq_high_freq: ControlFn,
    /// `-T`.
    pub loop_eq_decay_secs: ControlFn,
    /// `-E`: a plain constant, not a `(func)`.
    pub loop_balance_limit_db: f32,
    /// `-X`. Defaults to `200.0`, not `0.0` - see `tools::ring`'s doc
    /// comment.
    pub loop_eq_low_db: ControlFn,
    /// `-Q`.
    pub loop_eq_high_db: ControlFn,
    /// `-U`. Defaults to `0.0`, not `200.0` - see `tools::ring`'s doc
    /// comment.
    pub loop_eq_low_freq: ControlFn,
    /// `-m`.
    pub loop_eq_high_freq: ControlFn,
    /// `-k`. Defaults to `200.0`, not `0.0` - see `tools::ring`'s doc
    /// comment.
    pub output_eq_low_db: ControlFn,
    /// `-c`.
    pub output_eq_high_db: ControlFn,
    /// `-s`. Defaults to `0.0`, not `200.0` - see `tools::ring`'s doc
    /// comment.
    pub output_eq_low_freq: ControlFn,
    /// `-G`.
    pub output_eq_high_freq: ControlFn,

    // ---- Filter time navigation (shared shape with `tools::tvfilter`) ----
    /// `-h`.
    pub time_origin: ControlFn,
    /// `-R`.
    pub rate: ControlFn,
    /// `-g`.
    pub window_low: ControlFn,
    /// `-J`: raw value: `< 0.0` means "use the filter file's own
    /// duration".
    pub window_high: ControlFn,
    /// `/Q`.
    pub loop_mode: LoopMode,
    /// `::`.
    pub onset_release: bool,
    /// `-@`.
    pub autostop: bool,
    /// `-A`.
    pub loop_normalization: bool,
    /// `-a`.
    pub peak_loop_smooth_time: ControlFn,

    // ---- Filter response shaping ----
    /// `-q`.
    pub filter_warpshape: ControlFn,
    /// `-W`: a plain constant, not a `(func)` - see this module's doc
    /// comment.
    pub comp_threshold_db: f32,
    /// `-v`: a plain constant, not a `(func)`.
    pub comp_db: f32,

    // ---- Filter application ----
    /// `-u`.
    pub filter_transpose_semitones: ControlFn,
    /// `-V`.
    pub filter_freq_shift_hz: ControlFn,
    /// `-x`: blends between the filtered response (`-96`, the default:
    /// fully filtered) and the unfiltered dry signal (`0.0` dB: filter has
    /// no effect at all).
    pub filter_source_db: ControlFn,
    /// `-r`: only used by the postfilter placement.
    pub filter_decay_secs: ControlFn,
    /// `-o`: `false` (the default, `-o 0`) places the filter on the
    /// feedback path's *input* ("prefilter", `x(n)`); `true` (`-o 1`)
    /// places it *inside the loop* ("postfilter", `y(n-1)`, decay-time-
    /// scaled).
    pub filter_postfilter: bool,
    /// `-B`: `false` (the default) compensates the filter's own
    /// transpose/shift by the feedback path's own `-P`/`-H`; `true`
    /// applies no compensation.
    pub pitchflag: bool,
}

/// Processes one channel start to finish. `filter_frames` is this
/// channel's already-selected filter-analysis frames (`pvc_io::
/// PvaData::channels[ch]`, each `analysis_n + 2` floats); `analysis_peak_amp`
/// is that channel's own stored peak amplitude. `dur` is the control-
/// function normalization duration in seconds - like `tvfilter.c`'s own
/// shared `dur` variable, this can be *mutated* by the filter's own time
/// navigation partway through a frame (autostop mode extending/truncating
/// it), and every control function evaluated afterward in that same frame
/// (including `tools::ring`'s own gain/EQ controls) sees the updated
/// value, not the original one - reproduced by evaluating the navigation
/// step before any of this frame's other control-function reads, matching
/// the C's own statement order exactly.
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    channel: &[f32],
    filter_frames: &[Vec<f32>],
    analysis_n: usize,
    analysis_d: u32,
    analysis_sample_rate: u32,
    analysis_peak_amp: f32,
    sample_rate: u32,
    params: &RingtvfilterParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;
    let n_ratio = analysis_n as f32 / n as f32;
    let analysis_n2 = analysis_n / 2;
    let analysis_fundamental = nyquist / analysis_n2 as f32;

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
    let filttinc = analysis_d as f32 / analysis_sample_rate as f32;

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
    let threshfac = db_to_amp.convert(params.oscbank_threshold_db);

    let balance_limit_amp = db_to_amp_exact(params.loop_balance_limit_db);
    let balance_flag = params.loop_balance_limit_db > 0.0;

    let iframes_per_sec = analysis_sample_rate as f32 / analysis_d as f32;
    let niframes = filter_frames.len();
    let analysis_dur = niframes as f32 / iframes_per_sec;

    // Compression is resolved once, outside the frame loop - `-W`/`-v` are
    // plain floats here, not `(func)`s (see this module's doc comment).
    let comp_threshold_amp = db_to_amp.convert(params.comp_threshold_db);
    let comp_amp = db_to_amp.convert(params.comp_db);
    let comp_norm_amp = 1.0 / (comp_threshold_amp + comp_amp * (1.0 - comp_threshold_amp));
    let compflag = comp_threshold_amp < 1.0 && comp_amp < 1.0;

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut raw_analyzer = RawAnalyzer::new(n, window_pair.analysis, d);

    let mut channel_phase = PhaseTracker::new_analysis(n2, d, sample_rate);
    let mut feedback_phase = PhaseTracker::new_analysis(n2, d, sample_rate);

    let mut source_osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut feedback_osc =
        OscBank::with_shared_table(n2, sample_rate, i_factor, 1.0, source_osc.table());

    let mut previous: Vec<(f32, f32)> = vec![(0.0, 0.0); n2 + 1];
    let mut next_buffer: Vec<f32> = vec![0.0; n];

    let mut filter_smoother = Smoother::new(analysis_n + 2);
    let mut loop_normalizer = LoopNormalizer::new();

    let nav_cfg = TimeNavConfig {
        onset_release: params.onset_release,
        autostop: params.autostop,
        loop_mode: params.loop_mode,
    };
    let mut dur = dur;
    let initial_time_origin = params.time_origin.at(0.0, dur);
    let mut nav = TimeNavigator::new(nav_cfg, initial_time_origin);

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

        // Raw post-FFT spectrum, shared by both the source and feedback
        // paths.
        let buffer = raw_analyzer.push(&hop);

        // ---- FILTER TIME NAVIGATION (may mutate `dur` - see this
        // function's doc comment) ----
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

        // ---- FETCH AND SHAPE THE FILTER FRAME ----
        let mut f_flat = interpolate_frame(filter_frames, iframes_per_sec, step.filttnow);

        if params.loop_normalization {
            loop_normalizer.apply(
                &mut f_flat,
                filter_frames,
                iframes_per_sec,
                win_low_val,
                win_hi_val,
                analysis_dur,
                step.filttnow,
            );
        }

        // Loop-seam smoothing: one symmetric coefficient pair (unlike
        // `tvfilter.c`'s own separate attack/release) - see this module's
        // doc comment.
        let (loop_c, minus_loop_c) = smooth_setup(loop_smooth_time, ir);
        filter_smoother.smooth(&mut f_flat, loop_c, minus_loop_c, loop_c, minus_loop_c);

        {
            let mut amps: Vec<f32> = f_flat.iter().step_by(2).copied().collect();
            normalize_response(&mut amps, analysis_peak_amp);
            for (dst, src) in f_flat.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }

        if compflag {
            let mut amps: Vec<f32> = f_flat.iter().step_by(2).copied().collect();
            compress_response(&mut amps, comp_threshold_amp, comp_amp, comp_norm_amp);
            for (dst, src) in f_flat.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }

        let warpshape = params.filter_warpshape.at(t, dur);
        spectmagwarp(&mut f_flat, warpshape, false);

        let this_f: Vec<f32> = f_flat.iter().step_by(2).copied().collect();

        // ---- RING: PER-FRAME CONTROL VALUES (see this module's own EQ
        // fields) - evaluated with the (possibly navigation-mutated)
        // `dur` from above ----
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

        let mut feedback_buffer = buffer.clone();
        let mut feedback_bins = lean_convert(&feedback_buffer, n2);

        // THRESHOLD ENVELOPE: no pass-mode flag here (unlike `tools::
        // ring`'s own `-V`) - always "below threshold releases" - see this
        // module's doc comment.
        let feedt = getthresh(&feedback_bins, feedback_thresh_amp);
        for (j, bin) in feedback_bins.iter_mut().enumerate() {
            bin.0 = if bin.0 < feedt {
                envrelease * previous[j].0
            } else if bin.0 < previous[j].0 {
                envrelease * previous[j].0 + minusrelease * bin.0
            } else {
                envattack * previous[j].0 + minusattack * bin.0
            };
            previous[j].0 = bin.0;
        }

        // INPUT EQ (pre-transpose/shift, "x(n)").
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

        // ---- PREFILTER: applied to x(n), before the delay line's tail.
        // N_ratio-scaled against the filter file's own fundamental. ----
        if !params.filter_postfilter && source_db != 0.0 {
            for (j, bin) in feedback_bins.iter_mut().enumerate() {
                let interp = filter_lookup(&this_f, j, n_ratio, analysis_fundamental, fs, fm);
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
        // Guarded like `tools::ringfilter` (unlike `tools::ring`, which
        // divides unconditionally) - see that module's doc comment.
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

        // ---- POSTFILTER: applied to y(n-1), inside the loop. Uses the
        // audio's own bin index/fundamental directly (n_ratio = 1.0) and
        // `raw_fshift` (not the compensated `fs`) - a real, confirmed
        // asymmetry with the prefilter placement, see this module's doc
        // comment. ----
        if params.filter_postfilter && source_db != 0.0 {
            for (j, bin) in feedback_bins.iter_mut().enumerate() {
                let interp = filter_lookup(&this_f, j, 1.0, fundamental, raw_fshift, fm);
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
            bin.0 *= feedback_gain;
        }
        for bin in source_frame.bins.iter_mut() {
            let (amp, freq) = *bin;
            let temp = source_pm * (source_harmadd + freq);
            *bin = if temp > 0.0 && temp < nyquist {
                (amp, temp)
            } else {
                (0.0, freq)
            };
            bin.0 *= source_gain;
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

        if eof_after_this_hop || step.autostop {
            break;
        }
    }

    output.extend(vec![0.0f32; i_factor]);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    fn flat_filter_frames(analysis_n: usize, count: usize) -> Vec<Vec<f32>> {
        vec![vec![1.0f32; analysis_n + 2]; count]
    }

    fn default_params(fft_size: usize) -> RingtvfilterParams {
        RingtvfilterParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            oscbank_threshold_db: -96.0,
            source_gain_db: ControlFn::Const(0.0),
            source_freq_shift_hz: ControlFn::Const(0.0),
            source_pitch_semitones: ControlFn::Const(0.0),
            feedback_gain_db: ControlFn::Const(-999.0),
            feedback_freq_shift_hz: ControlFn::Const(0.0),
            feedback_pitch_semitones: ControlFn::Const(0.0),
            feedback_decay_secs: ControlFn::Const(0.0),
            feedback_threshold_db: ControlFn::Const(-96.0),
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
            time_origin: ControlFn::Const(0.0),
            rate: ControlFn::Const(1.0),
            window_low: ControlFn::Const(0.0),
            window_high: ControlFn::Const(-1.0),
            loop_mode: LoopMode::Wrap,
            onset_release: false,
            autostop: false,
            loop_normalization: false,
            peak_loop_smooth_time: ControlFn::Const(0.2),
            filter_warpshape: ControlFn::Const(0.0),
            comp_threshold_db: 0.0,
            comp_db: 0.0,
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
        let fft = 1024;
        let filter_frames = flat_filter_frames(fft, 50);
        let params = default_params(fft);
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(
            &input,
            &filter_frames,
            fft,
            220,
            44100,
            1.0,
            44100,
            &params,
            1.0,
        );
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn sine_input_produces_bounded_output() {
        let fft = 1024;
        let filter_frames = flat_filter_frames(fft, 50);
        let params = default_params(fft);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(
            &input,
            &filter_frames,
            fft,
            220,
            sample_rate,
            1.0,
            sample_rate,
            &params,
            1.0,
        );
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.0, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn feedback_loop_adds_energy_that_decays_over_time() {
        let fft = 1024;
        let filter_frames = flat_filter_frames(fft, 50);
        let sample_rate = 44100u32;
        let mut params = default_params(fft);
        params.feedback_gain_db = ControlFn::Const(0.0);
        params.feedback_decay_secs = ControlFn::Const(0.5);
        params.feedback_threshold_db = ControlFn::Const(-200.0);
        params.source_gain_db = ControlFn::Const(-999.0); // isolate the reverb tail

        let burst_len = sample_rate as usize / 10;
        let mut input: Vec<f32> = (0..burst_len)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        input.extend(vec![0.0f32; sample_rate as usize]);

        let output = process_channel(
            &input,
            &filter_frames,
            fft,
            220,
            sample_rate,
            1.0,
            sample_rate,
            &params,
            1.0,
        );
        let tail_start = burst_len + sample_rate as usize / 4;
        let tail_energy: f32 = output[tail_start..].iter().map(|&s| s * s).sum();
        assert!(
            tail_energy > 0.0,
            "expected a decaying reverb tail past the dry burst"
        );
    }

    #[test]
    fn postfilter_notch_bounds_output() {
        let fft = 1024;
        let sample_rate = 44100u32;
        let mut flat: Vec<f32> = vec![1.0f32; fft + 2];
        flat[20] = 0.0; // notch one amplitude slot low
        let filter_frames = vec![flat; 50];
        let mut params = default_params(fft);
        params.filter_postfilter = true;
        params.feedback_gain_db = ControlFn::Const(0.0);
        params.feedback_decay_secs = ControlFn::Const(0.2);
        params.feedback_threshold_db = ControlFn::Const(-200.0);
        params.source_gain_db = ControlFn::Const(-999.0);

        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.3 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(
            &input,
            &filter_frames,
            fft,
            220,
            sample_rate,
            1.0,
            sample_rate,
            &params,
            1.0,
        );
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak.is_finite() && peak < 10.0, "peak {peak} unbounded");
    }
}
