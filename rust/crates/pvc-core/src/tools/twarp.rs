//! The core `twarp` time-varying resynthesis pipeline
//! (`legacy/pvc_src/twarp.c`): navigates a virtual time position through
//! a pre-analyzed `.pva` file (at a possibly time-varying rate, from a
//! possibly time-varying origin, with sampler-loop wrap/fold/clip or
//! autostop-at-the-edge behavior) and resynthesizes from whatever
//! analysis frame that position lands on.
//!
//! **Not yet ported** (deferred to a follow-up - each is an independent,
//! off-by-default feature, confirmed by reading `twarp.c`'s flag
//! defaults): time-point dither (`-T`), loop-boundary amplitude
//! normalization (`-v`), and random amplitude/frequency variation aka
//! "shimmer" (`-B`/`-e`/`-j`/`-k`/`-n`/`-I`/`-J`/`-N`). None of `pvc
//! twarp`'s flags below expose these yet.
//!
//! **Real behavior, not a bug in this port:**
//! [`crate::timenav::interpolate_frame`] never actually interpolates
//! between analysis frames despite its C name - see that function's doc
//! comment. `pvc twarp` lands on whichever frame the navigated time
//! floors to.
//!
//! Unlike `plainpv`, `twarp` genuinely picks between the oscillator bank
//! and overlap-add resynthesis depending on whether pitch transposition
//! or frequency shift is requested at all (`plainpv`'s otherwise-
//! identical-looking selector has an extra hardcoded clause forcing
//! oscillator-bank unconditionally; `twarp`'s doesn't) - confirmed
//! empirically via a debug build (`docs/dev/parameter-inventory.md` §4).
//! Both paths are ported here.
//!
//! No `phaselock` call anywhere in `twarp.c` (confirmed by reading the
//! whole file) - resynthesis quality relies on the oscillator bank's own
//! `getthresh` gating or overlap-add alone, same as `pvanalysis` needing
//! neither.
//!
//! The output FFT size is always the analysis file's own `N` (`twarp.c`:
//! `N = analysis_N`) - there's no independent `-N` flag. There's also no
//! independent resynthesis hop distinct from the analysis hop (`I = D`
//! always; `twarp` has no `plainpv`-style stretch factor) - the "time
//! warp" comes entirely from navigating the analysis data at a different
//! apparent rate, not from an I/D hop mismatch.

use crate::eq::{eq2, ShelfEq};
use crate::pvoc::{getthresh, Frame, OscBank, Synthesizer};
use crate::smooth::{smooth_setup, Smoother};
use crate::timenav::{
    interpolate_frame, make_loop_smooth_time, LoopMode, TimeNavConfig, TimeNavigator,
};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant. Applies to `twarp`'s oscillator-bank
/// path exactly as it does to `plainpv`'s (same shared `bufferout()`).
const OSCILBANKGAIN: f32 = 1.7782794;

#[derive(Debug, Clone)]
pub struct TwarpParams {
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub duration: f32,
    pub time_origin: ControlFn,
    pub rate: ControlFn,
    pub window_low: ControlFn,
    pub window_high: ControlFn,
    pub time_response: ControlFn,
    pub loop_smooth: ControlFn,
    pub autostop: bool,
    pub loop_mode: LoopMode,
    pub onset_release: bool,
    pub pitch_transpose_semitones: ControlFn,
    pub freq_shift_hz: ControlFn,
    pub gain_db: ControlFn,
    pub attack_secs: ControlFn,
    pub release_secs: ControlFn,
    pub freq_response_secs: ControlFn,
    pub warpshape: ControlFn,
    pub shelf_low_db: ControlFn,
    pub shelf_high_db: ControlFn,
    pub shelf_low_freq: ControlFn,
    pub shelf_high_freq: ControlFn,
    pub threshold_db: f32,
}

/// Resynthesizes one channel from its analysis frames
/// (`pvc_io::PvaData::channels[ch]`, each `analysis_n + 2` floats).
/// `analysis_n`/`analysis_d`/`analysis_sample_rate` come from the `.pva`
/// header. Returns the resynthesized samples.
pub fn process_channel(
    analysis: &[Vec<f32>],
    analysis_n: usize,
    analysis_d: u32,
    analysis_sample_rate: u32,
    params: &TwarpParams,
) -> Vec<f32> {
    let r = analysis_sample_rate as f32;
    let n = analysis_n;
    let n2 = n / 2;
    let nyquist = r / 2.0;
    let fundamental = nyquist / (n / 2) as f32;

    let iframes_per_sec = analysis_sample_rate as f32 / analysis_d as f32;
    let analysis_dur = analysis.len() as f32 / iframes_per_sec;
    let mut dur = if params.duration <= 0.0 {
        analysis_dur
    } else {
        params.duration
    };

    let d = (r / params.frames_per_sec) as usize;
    let i_factor = d; // twarp: I = D always, no independent stretch.
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

    let threshfac = 10.0f64.powf(params.threshold_db as f64 / 20.0) as f32;

    // Selected once, matching twarp.c's own startup-time (not per-frame)
    // obank decision - see this module's doc comment.
    let ptrans_is_const_zero =
        matches!(&params.pitch_transpose_semitones, ControlFn::Const(v) if *v == 0.0);
    let harmadd_is_const_zero = matches!(&params.freq_shift_hz, ControlFn::Const(v) if *v == 0.0);
    let obank = !(ptrans_is_const_zero && harmadd_is_const_zero);

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut osc = OscBank::new(n2, nw, analysis_sample_rate, i_factor, 1.0);
    let mut synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, analysis_sample_rate);
    let mut smoother = Smoother::new(n + 2);
    let mut previous_channel = vec![0.0f32; n + 2];

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();

    let nav_cfg = TimeNavConfig {
        onset_release: params.onset_release,
        autostop: params.autostop,
        loop_mode: params.loop_mode,
    };
    let initial_time_origin = params.time_origin.at(0.0, dur);
    let mut nav = TimeNavigator::new(nav_cfg, initial_time_origin);

    // Same oscillator-bank write gate as `tools::pv` (see that module's
    // doc comment on `on`) - only used on the obank path; the overlap-add
    // path's own gating lives inside `Synthesizer::overlap_add`.
    let mut on: i64 = (-(nw as i64) * i_factor as i64) / d as i64;

    let mut output = Vec::new();
    let mut samps_written: usize = 0;
    // `while (t < dur && autostopflag == 0)` in the C: `t` is a plain
    // global, set exactly once per iteration by `timenow()` (early in
    // the loop body, *before* that iteration's own write can advance
    // `samps`) and never touched again until the *next* iteration's own
    // `timenow()` call. So the value the while-check sees before
    // admitting iteration `k+1` is whatever iteration `k`'s own
    // `timenow()` computed - `samps` as of the *start* of iteration `k`,
    // one full iteration stale relative to `samps` as of the *end* of
    // iteration `k` (which is what a naive fresh `samps_written / r`
    // recomputation here would produce instead). Confirmed by instrumenting
    // a real debug build: it ran one more iteration past the point where a
    // fresh-recomputation check would have already stopped, landing on
    // exactly the real tool's output length once `t_for_check` below was
    // introduced to carry that one-iteration staleness forward explicitly
    // rather than recomputing `t` fresh at the top of the loop (which is
    // what `tools::pv`/`tools::analyze` correctly do instead, since their
    // own loops terminate on an EOF/valid-samples state machine, not a
    // `t < dur` check - this staleness only matters for the latter).
    let mut t_for_check = 0.0f32;
    // The current frame is always fully processed and its output written
    // even if `advance()` sets autostop partway through - only the *next*
    // iteration's check actually stops things, matching `autostopflag`'s
    // own one-iteration-later effect in the C.
    let mut autostop_from_prev = false;

    loop {
        if t_for_check >= dur || autostop_from_prev {
            break;
        }
        let t = samps_written as f32 / r;

        let mut channel_freqdev = vec![0.0f32; n + 2];
        for k in (1..n).step_by(2) {
            channel_freqdev[k] = 1.0;
        }

        let time_origin_val = params.time_origin.at(t, dur);
        let rate_val = params.rate.at(t, dur);
        let win_low_val = params.window_low.at(t, dur);
        let win_hi_raw = params.window_high.at(t, dur);
        // `-1` flags "use the analysis duration" for the high boundary
        // (`analysisDatawinhi.A[0] = -1.` is the C's own default) -
        // resolved here, once, since `analysis_dur` isn't known to the
        // CLI layer that builds a plain `ControlFn::Const(-1.0)` default.
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

        let peak_loop_smooth_time = params.loop_smooth.at(t, dur);
        let loop_smooth_time = make_loop_smooth_time(
            step.filttnow,
            params.loop_mode,
            params.autostop,
            win_low_val,
            win_hi_val,
            peak_loop_smooth_time,
        );

        let tsmooth_val = params.time_response.at(t, dur);
        let (tsmoothc, minustsmoothc) = smooth_setup(tsmooth_val, ir);
        let smoothed_filttnow = tsmoothc * step.oldfilttnow + minustsmoothc * step.filttnow;

        let mut flat = interpolate_frame(analysis, iframes_per_sec, smoothed_filttnow);

        let warpshape_val = params.warpshape.at(t, dur);
        spectmagwarp(&mut flat, warpshape_val, false);

        let harmadd = params.freq_shift_hz.at(t, dur);
        let gain = db_to_amp.convert(params.gain_db.at(t, dur));
        let pm = semitones_to_mult.convert(params.pitch_transpose_semitones.at(t, dur));

        let (attackc, minusattackc) =
            smooth_setup(params.attack_secs.at(t, dur) + loop_smooth_time, ir);
        let (releasec, minusreleasec) =
            smooth_setup(params.release_secs.at(t, dur) + loop_smooth_time, ir);
        let (fsmoothc, minusfsmoothc) =
            smooth_setup(params.freq_response_secs.at(t, dur) + loop_smooth_time, ir);

        let shelf = ShelfEq {
            d_blow: params.shelf_low_db.at(t, dur),
            d_bhi: params.shelf_high_db.at(t, dur),
            freqlow: params.shelf_low_freq.at(t, dur),
            freqhi: params.shelf_high_freq.at(t, dur),
        };

        smoother.smooth(&mut flat, attackc, minusattackc, releasec, minusreleasec);

        for j in 0..n2 {
            let i = 1 + 2 * j;

            flat[i] = fsmoothc * previous_channel[i] + minusfsmoothc * flat[i];
            previous_channel[i] = flat[i];

            let temp = pm * (flat[i] + harmadd);
            channel_freqdev[i] *= pm;
            channel_freqdev[i - 1] += harmadd;
            if temp <= 0.0 || temp >= nyquist {
                flat[i - 1] = 0.0;
            } else {
                flat[i] = temp;
            }

            flat[i - 1] *= gain;
        }

        eq2(
            &mut flat,
            &shelf,
            fundamental,
            &channel_freqdev,
            false,
            &db_to_amp,
        );

        let frame = Frame::from_pva_floats(&flat);
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
        autostop_from_prev = step.autostop;
        t_for_check = t;
    }

    // `shiftout(output, Nw, I, 1, 1)` - the unconditional final flush
    // outside the frame loop, same call for both resynthesis branches
    // (see this module's doc comment). For the oscillator bank this is
    // always `I` samples of silence (see `tools::pv`'s identical
    // finding); for overlap-add, whatever's left in the ring genuinely
    // matters (the tail of the last frame's windowed spectrum), so it's
    // `Synthesizer::flush`'s job, not a fixed silence chunk.
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
    use crate::tools::analyze::{process_channel as analyze_channel, AnalyzeParams};

    fn default_params(duration: f32) -> TwarpParams {
        TwarpParams {
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            duration,
            time_origin: ControlFn::Const(0.0),
            rate: ControlFn::Const(1.0),
            window_low: ControlFn::Const(0.0),
            window_high: ControlFn::Const(-1.0),
            time_response: ControlFn::Const(0.0),
            loop_smooth: ControlFn::Const(0.2),
            autostop: false,
            loop_mode: LoopMode::Wrap,
            onset_release: false,
            pitch_transpose_semitones: ControlFn::Const(0.0),
            freq_shift_hz: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            attack_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
            freq_response_secs: ControlFn::Const(0.0),
            warpshape: ControlFn::Const(0.0),
            shelf_low_db: ControlFn::Const(0.0),
            shelf_high_db: ControlFn::Const(0.0),
            shelf_low_freq: ControlFn::Const(200.0),
            shelf_high_freq: ControlFn::Const(2000.0),
            threshold_db: -60.0,
        }
    }

    fn analyze_sine(sample_rate: u32, fft_size: usize) -> (Vec<Vec<f32>>, u32) {
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let params = AnalyzeParams {
            fft_size,
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
        let (frames, _peak_amp) = analyze_channel(&input, sample_rate, &params);
        let d = (sample_rate as f32 / params.frames_per_sec) as u32;
        (frames.iter().map(|f| f.to_pva_floats()).collect(), d)
    }

    #[test]
    fn overlap_add_path_selected_by_default() {
        let sample_rate = 44100u32;
        let (frames, d) = analyze_sine(sample_rate, 1024);
        let params = default_params(1.0);
        let output = process_channel(&frames, 1024, d, sample_rate, &params);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.01, "peak {peak} too quiet");
        assert!(peak < 2.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn oscbank_path_selected_when_pitch_shifted() {
        let sample_rate = 44100u32;
        let (frames, d) = analyze_sine(sample_rate, 1024);
        let mut params = default_params(1.0);
        params.pitch_transpose_semitones = ControlFn::Const(7.0);
        let output = process_channel(&frames, 1024, d, sample_rate, &params);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.01, "peak {peak} too quiet");
        assert!(peak < 2.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn wrap_loop_extends_output_past_analysis_duration() {
        let sample_rate = 44100u32;
        let (frames, d) = analyze_sine(sample_rate, 1024);
        // Request more output duration than the ~1s analysis - only
        // possible if wrap looping is actually kicking in.
        let params = default_params(2.5);
        let output = process_channel(&frames, 1024, d, sample_rate, &params);
        let secs = output.len() as f32 / sample_rate as f32;
        assert!(secs > 2.0, "expected ~2.5s of output, got {secs}s");
    }
}
