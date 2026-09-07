//! Ports `tvfilter.c`: like `tools::filter`, a phase-vocoder filter that
//! multiplies each frame's amplitude by a shaped copy of a response, but
//! the response itself is a *time-varying* sequence of frames (a legacy
//! `.pva`-format "filter analysis" file, `pvc_io::read_legacy_pva`)
//! navigated over time exactly the way `tools::twarp` navigates its own
//! resynthesis source - reusing `timenav`'s `TimeNavigator`/
//! `interpolate_frame`/`make_loop_smooth_time` directly, not
//! re-deriving them.
//!
//! **Not yet ported: the oscillator-bank resynthesis path** - `tvfilter.c`
//! picks it dynamically (`obank = 1` whenever `-P`/`-a` are ever nonzero,
//! `0` otherwise, matching `plainpv.c`'s original condition), but this
//! port always overlap-adds, matching `tools::filter`'s own already-
//! established simplification for the identical condition (see that
//! module's doc comment) - not a new gap this port introduces. `-t`
//! (the oscillator bank's own resynthesis threshold) is consequently not
//! exposed either, matching `tools::filter`'s own precedent: `getthresh`
//! is only ever consumed by `noscbank()`'s per-bin skip check, which the
//! overlap-add path never calls.
//!
//! **`-u` (`imode`, "FILTER data access mode") is dead**: declared,
//! parsed, and printed, but never read anywhere else in the file -
//! confirmed by grepping every reference. The time-navigation formula
//! (`TimeNavigator::advance`, shared with `twarp`) already combines
//! `-Q`'s origin and `-Y`'s rate unconditionally on every call; there is
//! no second code path `imode` could ever select between. Not exposed
//! here.
//!
//! Unlike `tools::twarp`, this tool has no time-position smoothing
//! (no `-time-response`-equivalent flag exists in `tvfilter.c`'s own
//! usage text, and `makeInterpolatedFilterFrame` is called with the
//! navigator's raw `filttnow` directly, never a smoothed blend) - one
//! real difference from its sibling, confirmed by reading the whole
//! frame loop rather than assumed from symmetry with `twarp`.
//!
//! The bin-shift/transpose lookup is the same shape as `tools::filter`'s
//! and `tools::ringfilter`'s (`filter_lookup`), with one addition: since
//! this tool's own audio FFT size (`-N`) can differ from the filter
//! file's FFT size (`analysis_n`, unlike `tools::filter`, which forces
//! them equal), each bin index is first scaled by `N_ratio = analysis_n /
//! n` before the shift/transpose - `analysis_fundamental` (used for the
//! shift term) is also computed from the *audio*'s own Nyquist over the
//! *filter*'s own bin count (`nyquist / (analysis_n / 2)`), not either
//! FFT size alone - reproduced exactly as the C computes it.

use crate::eq::eq;
use crate::filter_response::{invert_response, smooth_response};
use crate::pvoc::{Analyzer, Frame, Synthesizer};
use crate::smooth::{smooth_setup, Smoother};
use crate::timenav::{
    interpolate_frame, make_loop_smooth_time, LoopMode, TimeNavConfig, TimeNavigator,
};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum InvertMode {
    Pass,
    InvertFixedPeak,
    InvertFramePeak,
}

pub struct TvfilterParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,

    /// `-P`.
    pub pitch_transpose: ControlFn,
    /// `-a`.
    pub freq_shift: ControlFn,
    /// `-A`.
    pub gain_db: ControlFn,
    /// `-B`: `false` (the default) compensates the filter's own
    /// transpose/shift by the source's own `-P`/`-a`; `true` applies no
    /// compensation.
    pub pitchflag: bool,

    /// `-q`.
    pub invert_mode: InvertMode,

    /// `-Q`.
    pub time_origin: ControlFn,
    /// `-Y`.
    pub rate: ControlFn,
    /// `-g`.
    pub window_low: ControlFn,
    /// `-G`: raw value: `< 0.0` means "use the filter file's own
    /// duration" (resolved once per frame against the loaded file, since
    /// the CLI layer that builds a plain `ControlFn::Const(-1.0)` default
    /// doesn't know that duration).
    pub window_high: ControlFn,
    /// `-o`.
    pub loop_mode: LoopMode,
    /// `-r`.
    pub onset_release: bool,
    /// `-d`.
    pub autostop: bool,

    /// `-j`.
    pub loop_normalization: bool,
    /// `-k`.
    pub peak_loop_smooth_time: ControlFn,

    /// `-E`: unlike `tools::filter`'s plain-float compand thresholds,
    /// this one is a `(func)`, re-evaluated every frame.
    pub comp_threshold_db: ControlFn,
    /// `-c`.
    pub comp_db: ControlFn,

    /// `-T`.
    pub filter_transpose: ControlFn,
    /// `-V`.
    pub filter_shift: ControlFn,
    /// `-Z`.
    pub filter_release_secs: ControlFn,
    /// `-z`.
    pub filter_attack_secs: ControlFn,
    /// `-S`.
    pub filter_source_db: ControlFn,
    /// `-W`.
    pub filter_warpshape: ControlFn,
    /// `-f`.
    pub filter_smoothing_bw: ControlFn,

    /// `-H`.
    pub shelf_low_db: ControlFn,
    /// `-X`.
    pub shelf_high_db: ControlFn,
    /// `-m`.
    pub shelf_low_freq: ControlFn,
    /// `-R`.
    pub shelf_high_freq: ControlFn,

    /// `-n`.
    pub frame_norm_limit_db: ControlFn,
    /// `-v`: `false` (the default) normalizes toward the input sound's
    /// own amplitude sum; `true` toward the filter's.
    pub normalize_to_filter: bool,

    /// `-l`.
    pub attack_secs: ControlFn,
    /// `-L`.
    pub release_secs: ControlFn,
}

/// Ports the shared bin-shift/transpose lookup (see this module's doc
/// comment on `N_ratio`). `this_f` is `analysis_n / 2 + 1` amplitudes
/// (already shaped for this frame); `bin` is the *audio*'s own bin index
/// (`0..=n2`).
pub(crate) fn filter_lookup(
    this_f: &[f32],
    bin: usize,
    n_ratio: f32,
    analysis_fundamental: f32,
    shift_hz: f32,
    transpose_mult: f32,
) -> f32 {
    let analysis_n2 = this_f.len() - 1;
    let mut temp = bin as f32 * n_ratio;
    temp -= shift_hz / analysis_fundamental;
    temp /= transpose_mult;
    let i1 = temp as i64;
    let i2p = temp - i1 as f32;
    let i1p = 1.0 - i2p;
    let i2 = i1 + 1;
    if i1 < 0 {
        this_f[0]
    } else if i2 as usize >= analysis_n2 {
        this_f[analysis_n2 - 1]
    } else {
        this_f[i1 as usize] * i1p + this_f[i2 as usize] * i2p
    }
}

/// Ports `compress()`: single-sided upward compression - bins above
/// `threshold_amp` have the amount they exceed it scaled by `comp_amp`,
/// then every bin (compressed or not) is rescaled by `norm_amp`.
pub(crate) fn compress_response(
    amps: &mut [f32],
    threshold_amp: f32,
    comp_amp: f32,
    norm_amp: f32,
) {
    for a in amps.iter_mut() {
        *a = if *a > threshold_amp {
            norm_amp * (threshold_amp + comp_amp * (*a - threshold_amp))
        } else {
            norm_amp * *a
        };
    }
}

/// Ports `normalize()`: gain-scales every bin by `1 / peak_amp`, then
/// hard-limits anything still over `1.0` (not a real peak-normalize if
/// `peak_amp` is wrong for the data - just what the C does).
pub(crate) fn normalize_response(amps: &mut [f32], peak_amp: f32) {
    for a in amps.iter_mut() {
        *a /= peak_amp;
        if *a > 1.0 {
            *a = 1.0;
        }
    }
}

/// Ports `normalizeLoopAmplitudes()`: in sampler-loop mode (not
/// autostop) with `-j` on, keeps the filter's own amplitude roughly
/// continuous across a loop's seam by comparing the current frame's
/// amplitude sum against the window boundaries' own sums - full gain
/// during the loop body, a proportional blend during the onset (before
/// the low boundary is first reached) and release (after the high
/// boundary) segments. The boundary sums are cached and only refreshed
/// when a control function actually moves that boundary (matching the
/// C's own `last_filtwinlow`/`last_filtwinhi` staleness check) - safe to
/// reproduce exactly here since it's a real cross-frame cache, not an
/// artifact of one call site's incomplete state like `tools::ring`'s
/// `prebalancesum`.
pub(crate) struct LoopNormalizer {
    low_sum: f32,
    high_sum: f32,
    peak_sum: f32,
    last_win_low: f32,
    last_win_high: f32,
    initialized: bool,
}

impl LoopNormalizer {
    pub(crate) fn new() -> Self {
        LoopNormalizer {
            low_sum: 0.0,
            high_sum: 0.0,
            peak_sum: 0.0,
            last_win_low: -1.0,
            last_win_high: -1.0,
            initialized: false,
        }
    }

    #[allow(clippy::too_many_arguments)]
    pub(crate) fn apply(
        &mut self,
        channel: &mut [f32],
        filter_frames: &[Vec<f32>],
        iframes_per_sec: f32,
        win_low: f32,
        win_high: f32,
        analysis_dur: f32,
        filttnow: f32,
    ) {
        if !self.initialized || win_low != self.last_win_low {
            let frame = interpolate_frame(filter_frames, iframes_per_sec, win_low);
            self.low_sum = frame.iter().step_by(2).sum();
        }
        if !self.initialized || win_high != self.last_win_high {
            let frame = interpolate_frame(filter_frames, iframes_per_sec, win_high);
            self.high_sum = frame.iter().step_by(2).sum();
        }
        if !self.initialized || win_low != self.last_win_low || win_high != self.last_win_high {
            self.peak_sum = self.low_sum.max(self.high_sum);
        }
        self.initialized = true;
        self.last_win_low = win_low;
        self.last_win_high = win_high;

        let channel_sum: f32 = channel.iter().step_by(2).sum();
        if channel_sum <= 0.0 {
            return;
        }

        let db_to_amp = DbToAmp::new();
        let gain_scale = if filttnow <= win_low {
            let prop = filttnow / win_low;
            let high_db = crate::units::amp_to_db(self.peak_sum / self.low_sum);
            db_to_amp.convert(prop * high_db)
        } else if filttnow >= win_high {
            let prop = (filttnow - win_high) / (analysis_dur - win_high);
            let low_db = crate::units::amp_to_db(self.peak_sum / self.high_sum);
            db_to_amp.convert(low_db + prop * -low_db)
        } else {
            self.peak_sum / channel_sum
        };

        for a in channel.iter_mut().step_by(2) {
            *a *= gain_scale;
        }
    }
}

/// Resynthesizes one channel. `filter_frames` is this channel's already-
/// selected filter-analysis frames (`pvc_io::PvaData::channels[ch]`,
/// each `analysis_n + 2` floats); `analysis_peak_amp` is that channel's
/// own stored peak amplitude (`pvc_io::PvaData::peak_amps[ch]`).
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    input: &[f32],
    filter_frames: &[Vec<f32>],
    analysis_n: usize,
    analysis_d: u32,
    analysis_sample_rate: u32,
    analysis_peak_amp: f32,
    sample_rate: u32,
    params: &TvfilterParams,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;
    let n_ratio = analysis_n as f32 / n as f32;
    let analysis_n2 = analysis_n / 2;
    let analysis_fundamental = nyquist / analysis_n2 as f32;

    let d = (r / params.frames_per_sec) as usize;
    let i_factor = (d as f32 * params.time_factor) as usize;
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

    let iframes_per_sec = analysis_sample_rate as f32 / analysis_d as f32;
    let niframes = filter_frames.len();
    let analysis_dur = niframes as f32 / iframes_per_sec;

    let mut dur = (input.len() as f32 / r) * params.time_factor;

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);
    let mut synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, sample_rate);
    let mut channel_smoother = Smoother::new(n + 2);
    // Filter-response smoothing: a separate instance keyed on the
    // *filter*'s own bin layout (`analysis_n + 2`, interleaved amp/phase-
    // like slots - see `lean_convert`-style layouts elsewhere; here it's
    // just amp/freq like everything else, phase slots unused).
    let mut filter_smoother = Smoother::new(analysis_n + 2);
    let mut loop_normalizer = LoopNormalizer::new();

    let nav_cfg = TimeNavConfig {
        onset_release: params.onset_release,
        autostop: params.autostop,
        loop_mode: params.loop_mode,
    };
    let initial_time_origin = params.time_origin.at(0.0, dur);
    let mut nav = TimeNavigator::new(nav_cfg, initial_time_origin);

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;

    let mut output = Vec::new();
    let mut samps_written: usize = 0;

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
        let mut channel_flat = channel_frame.to_pva_floats();

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

        {
            let mut amps: Vec<f32> = f_flat.iter().step_by(2).copied().collect();
            normalize_response(&mut amps, analysis_peak_amp);
            for (dst, src) in f_flat.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }
        let filter_channel_amp_sum: f32 = f_flat.iter().step_by(2).sum();

        let comp_threshold_db = params.comp_threshold_db.at(t, dur);
        let comp_db = params.comp_db.at(t, dur);
        assert!(
            comp_threshold_db <= 0.0,
            "tvfilter: compression threshold must be <= 0dB"
        );
        assert!(
            comp_db <= 0.0,
            "tvfilter: compression decibels must be <= 0dB"
        );
        let comp_threshold_amp = db_to_amp.convert(comp_threshold_db);
        let comp_amp = db_to_amp.convert(comp_db);
        let comp_norm_amp = 1.0 / (comp_threshold_amp + comp_amp * (1.0 - comp_threshold_amp));
        if comp_threshold_amp < 1.0 && comp_amp < 1.0 {
            let mut amps: Vec<f32> = f_flat.iter().step_by(2).copied().collect();
            compress_response(&mut amps, comp_threshold_amp, comp_amp, comp_norm_amp);
            for (dst, src) in f_flat.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }

        match params.invert_mode {
            InvertMode::Pass => {}
            InvertMode::InvertFixedPeak | InvertMode::InvertFramePeak => {
                let mut amps: Vec<f32> = f_flat.iter().step_by(2).copied().collect();
                invert_response(
                    &mut amps,
                    params.invert_mode == InvertMode::InvertFramePeak,
                    &db_to_amp,
                );
                for (dst, src) in f_flat.iter_mut().step_by(2).zip(&amps) {
                    *dst = *src;
                }
            }
        }

        let warpshape = params.filter_warpshape.at(t, dur);
        spectmagwarp(&mut f_flat, warpshape, false);

        let shelf_low_db = params.shelf_low_db.at(t, dur);
        let shelf_high_db = params.shelf_high_db.at(t, dur);
        let shelf_low_freq = params.shelf_low_freq.at(t, dur);
        let shelf_high_freq = params.shelf_high_freq.at(t, dur);
        eq(
            &mut f_flat,
            shelf_low_db,
            shelf_high_db,
            shelf_low_freq,
            shelf_high_freq,
            fundamental,
            1.0,
            0.0,
            false,
            &db_to_amp,
        );

        let smoothing_bw = params.filter_smoothing_bw.at(t, dur);
        {
            let mut amps: Vec<f32> = f_flat.iter().step_by(2).copied().collect();
            smooth_response(&mut amps, smoothing_bw, sample_rate);
            for (dst, src) in f_flat.iter_mut().step_by(2).zip(&amps) {
                *dst = *src;
            }
        }

        // Filter's own attack/release smoothing, boosted by the loop's
        // seam-smoothing time.
        let (freleasec, minusfreleasec) =
            smooth_setup(params.filter_release_secs.at(t, dur) + loop_smooth_time, ir);
        let (fattackc, minusfattackc) =
            smooth_setup(params.filter_attack_secs.at(t, dur) + loop_smooth_time, ir);
        filter_smoother.smooth(
            &mut f_flat,
            fattackc,
            minusfattackc,
            freleasec,
            minusfreleasec,
        );

        let this_f: Vec<f32> = f_flat.iter().step_by(2).copied().collect();

        // ---- SOURCE/FILTER CONTROL VALUES ----
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

        let (releasec, minusreleasec) = smooth_setup(params.release_secs.at(t, dur), ir);
        let (attackc, minusattackc) = smooth_setup(params.attack_secs.at(t, dur), ir);
        let frame_norm_amp_limit = db_to_amp.convert(params.frame_norm_limit_db.at(t, dur));

        let channel_amp_sum: f32 = channel_flat.iter().step_by(2).sum();

        // ---- APPLY THE FILTER TO THE SOURCE SPECTRUM ----
        for j in 0..=n2 {
            let interp = filter_lookup(&this_f, j, n_ratio, analysis_fundamental, fs, fm);
            channel_flat[2 * j] *= filtamp * interp + sourceamp;
        }

        // ---- FRAME NORMALIZATION ----
        let temp_channel_amp_sum: f32 = channel_flat.iter().step_by(2).sum();
        if temp_channel_amp_sum > 0.0 && frame_norm_amp_limit != 1.0 {
            let target = if params.normalize_to_filter {
                filter_channel_amp_sum
            } else {
                channel_amp_sum
            };
            let mut normalization_amp = target / temp_channel_amp_sum;
            if normalization_amp > frame_norm_amp_limit {
                normalization_amp = frame_norm_amp_limit;
            }
            for a in channel_flat.iter_mut().step_by(2) {
                *a *= normalization_amp;
            }
        }

        channel_smoother.smooth(
            &mut channel_flat,
            attackc,
            minusattackc,
            releasec,
            minusreleasec,
        );

        // ---- SOURCE PITCH/FREQ SHIFT + GAIN ----
        for j in 0..=n2 {
            let freq = channel_flat[2 * j + 1];
            let temp = pm * (freq + harmadd);
            if temp <= 0.0 || temp >= nyquist {
                channel_flat[2 * j] = 0.0;
            } else {
                channel_flat[2 * j + 1] = temp;
            }
            channel_flat[2 * j] *= gain;
        }

        // `-t`'s own oscillator-bank threshold is dead weight here - only
        // ever consumed by `noscbank()`'s per-bin skip check, which the
        // overlap-add path (this port's only resynthesis path, see this
        // module's doc comment) never calls. Matches `tools::filter`'s
        // own precedent of not computing `getthresh` at all for the same
        // reason.
        let frame = Frame::from_pva_floats(&channel_flat);
        let hop_out = synth.overlap_add(&frame);
        if !hop_out.is_empty() {
            output.extend(hop_out);
            samps_written += i_factor;
        }

        if eof_after_this_hop || step.autostop {
            break;
        }
    }

    output.extend(synth.flush());
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    fn flat_filter_frames(analysis_n: usize, count: usize) -> Vec<Vec<f32>> {
        vec![vec![1.0f32; analysis_n + 2]; count]
    }

    fn default_params(fft_size: usize) -> TvfilterParams {
        TvfilterParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            pitch_transpose: ControlFn::Const(0.0),
            freq_shift: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            pitchflag: false,
            invert_mode: InvertMode::Pass,
            time_origin: ControlFn::Const(0.0),
            rate: ControlFn::Const(1.0),
            window_low: ControlFn::Const(0.0),
            window_high: ControlFn::Const(-1.0),
            loop_mode: LoopMode::Wrap,
            onset_release: false,
            autostop: false,
            loop_normalization: false,
            peak_loop_smooth_time: ControlFn::Const(0.2),
            comp_threshold_db: ControlFn::Const(0.0),
            comp_db: ControlFn::Const(0.0),
            filter_transpose: ControlFn::Const(0.0),
            filter_shift: ControlFn::Const(0.0),
            filter_release_secs: ControlFn::Const(0.0),
            filter_attack_secs: ControlFn::Const(0.0),
            filter_source_db: ControlFn::Const(-96.0),
            filter_warpshape: ControlFn::Const(0.0),
            filter_smoothing_bw: ControlFn::Const(0.0),
            shelf_low_db: ControlFn::Const(0.0),
            shelf_high_db: ControlFn::Const(0.0),
            shelf_low_freq: ControlFn::Const(200.0),
            shelf_high_freq: ControlFn::Const(2000.0),
            frame_norm_limit_db: ControlFn::Const(0.0),
            normalize_to_filter: false,
            attack_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let fft = 1024;
        let filter_frames = flat_filter_frames(fft, 50);
        let params = default_params(fft);
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(&input, &filter_frames, fft, 220, 44100, 1.0, 44100, &params);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn flat_response_passes_sine_input_through_at_bounded_amplitude() {
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
        );
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.1, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn autostop_ends_output_before_input_is_exhausted() {
        let fft = 1024;
        let filter_frames = flat_filter_frames(fft, 50);
        let mut params = default_params(fft);
        params.autostop = true;
        params.window_low = ControlFn::Const(0.0);
        params.window_high = ControlFn::Const(0.05); // a narrow window, quickly exited
        let sample_rate = 44100u32;
        let input = vec![0.1f32; sample_rate as usize * 2]; // a long input
        let output = process_channel(
            &input,
            &filter_frames,
            fft,
            220,
            sample_rate,
            1.0,
            sample_rate,
            &params,
        );
        // Should stop well short of the full 2-second input once the
        // navigator's time position exits the narrow window.
        assert!(
            output.len() < input.len(),
            "expected autostop to cut output short"
        );
    }
}
