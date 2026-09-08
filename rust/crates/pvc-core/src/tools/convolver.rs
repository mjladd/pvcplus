//! Ports `convolver.c`'s audio-processing path ("Sound A", a live input
//! file, spectrally multiplied against "Sound B", a pre-analyzed `.pva`
//! filter file navigated over time the same way `tools::tvfilter`
//! navigates its own filter response).
//!
//! **A real, confirmed-by-reading bug central to this tool's own stated
//! purpose**: despite `convolver.c`'s own banner ("SHORT-TERM FFT
//! SPECTRAL MULTIPLICATION") and a correctly-written complex-multiply
//! block sitting right there in the source, that block is wrapped in
//! `/* ... */` and never runs. The *active* code instead does a naive
//! element-wise multiply of the two rfft-packed real/imaginary arrays
//! (`C_buffer[i] = normamp*C_dB*buffer[i]*Fbuffer[i]` for every `i`),
//! which is not a complex product at all - it silently produces the
//! *wrong* convolution result (mixing real\*real and imag\*imag terms
//! into wherever they happen to land, never the cross terms a true
//! complex multiply needs). Reproduced exactly here (`convolve()`'s own
//! naive per-index multiply, not the commented-out one) since this is
//! what the real tool's output actually is - not "fixed," which would
//! make this port disagree with its own oracle.
//!
//! **A real, confirmed uninitialized-memory bug, not ported at all**:
//! `envattack`/`envrelease`/`minusattack`/`minusrelease` (the Cartesian/
//! polar spectral-smoothing coefficients `-l`/`-L` would drive) are
//! declared with no initializer and are only ever *assigned* inside a
//! `/* ... */`-commented-out block - the active `CartesianSmooth()` call
//! that consumes them therefore reads whatever garbage happens to be on
//! the stack at that point, a genuine C-level bug (not merely "unwired,"
//! since the call site *is* live and *does* run whenever `-l`/`-L` make
//! `smoothingFlag` truthy). There is no well-defined value to reproduce
//! here - unlike a deterministic-but-wrong formula, uninitialized memory
//! has no formula at all - so `-l`/`-L`/`-k` (attack, release, and the
//! polar-vs-Cartesian smoothing-mode selector they gate) are not exposed
//! by this port. Every other feature this tool has is unaffected: the
//! frame-normalization path this smoothing block sits next to inside the
//! same `if` is fully real and ported.
//!
//! **Real, working pitch/frequency-shift** (unlike `tools::
//! spectralextractor`'s otherwise-similar-looking but entirely dead
//! version): `-P`/`-a` select oscillator-bank resynthesis over overlap-
//! add exactly once per channel (matching `tools::twarp`'s own startup-
//! time, not per-frame, decision), and their per-bin loop *does* write
//! the shifted frequency back (`channel[i] = temp`) and zero out-of-
//! range bins (`channel[i-1] = 0.`) - confirmed by reading the loop body
//! itself rather than assuming the same shape as the broken sibling.
//!
//! **Real, sample-accurate input trimming**: like `tools::
//! spectralextractor` (see that module's doc comment), `convolver.c`
//! calls the same `setupfiles()`/`openfiles()` pair that `getInputFileDataToSetOutputChannels()`
//! (`legacy/pvc_lib/fileio.c`) uses to seek/trim the input to
//! `[begint, endt)` before the frame loop ever starts - this port's
//! caller is expected to pass an already-trimmed `input` slice.
//!
//! **`-N` is dead**: `crack()`'s own flag list includes `N`, but there is
//! no `case 'N':` in the switch at all - the FFT size is always taken
//! from Sound B's own analysis file header (`N = analysis_N`), matching
//! the tool's own usage text ("FFT and framerate/decimation are taken
//! from Sound B analysis file"). Not exposed as a separate flag here
//! either.
//!
//! `-C` (single-channel-only resynthesis) is out of scope, matching
//! `tools::pv`'s/`tools::spectralextractor`'s own established precedent
//! for the same flag letter and meaning.
//!
//! One confirmed-dead call, left out entirely (no observable effect):
//! `leanunconvert(C_channel, C_buffer, ...)`, called early in the frame
//! loop using the *previous* frame's stale `C_channel` (this frame's own
//! hasn't been computed yet), writes into `C_buffer` - which is then
//! unconditionally overwritten by the real convolution multiply before
//! ever being read.
//!
//! **A real, severe bug in `convolver.c`'s own filter-frame fetch,
//! confirmed against the real compiled binary, not just by reading**:
//! its call to `makeInterpolatedFilterFrame()` passes `analysis_N` where
//! that function's own parameter is *named* `analysis_Nplus2`:
//! ```c
//! makeInterpolatedFilterFrame ( &filter, F_lower, F_higher, F,
//!         iframes_per_sec, analysis_N, filttnow, ainchan, analysis_chan
//! ) ;
//! ```
//! Both that function's seek-offset formula and its `fread` count use
//! this parameter as the per-frame float stride, so every fetch actually
//! advances by `analysis_N` floats (not the true on-disk `analysis_N +
//! 2`) - two floats short of a real frame, drifting further with every
//! additional frame fetched (frame 0's fetch happens to land correctly;
//! by frame *k* the read start has drifted `2*k` floats *before* the
//! true frame *k*, silently reading into what's actually the *previous*
//! true frame's own tail). The fetched array's own last bin (the Nyquist
//! bin, floats `analysis_N`/`analysis_N+1`) is never written by the
//! `fread` at all - it stays at `0.0` amplitude/`0.0` frequency for the
//! entire program run, since nothing else in `convolver.c` ever touches
//! those two array slots (`F` is `fvec()`-allocated once, before the
//! channel loop, and presumably zero-initialized on allocation).
//! [`buggy_filter_frame`] reproduces this exactly - confirmed by patching
//! a debug build of the real `convolver.c` to dump `F[]` directly and
//! diffing bin-for-bin against this port's own fetch, not merely
//! reasoned from the source.

use crate::pvoc::{getthresh, Frame, OscBank, PhaseTracker, Synthesizer};
use crate::timenav::{LoopMode, TimeNavConfig, TimeNavigator};
use crate::tools::ring::{apply_shelf_eq, lean_convert, lean_unconvert, RawAnalyzer};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant.
const OSCILBANKGAIN: f32 = 1.7782794;

#[derive(Debug, Clone)]
pub struct ConvolverParams {
    pub window_size: usize,
    pub window: Window,
    pub time_factor: f32,

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
    /// `-g`.
    pub filter_window_low: ControlFn,
    /// `-G`: raw value; `< 0.0` means "use the filter file's own
    /// duration" (resolved per-frame here since the CLI layer's default
    /// doesn't know that duration).
    pub filter_window_high: ControlFn,
    /// `-o`.
    pub loop_mode: LoopMode,
    /// `-r`.
    pub onset_release: bool,
    /// `-y`.
    pub autostop: bool,

    /// `-q`: Sound A's own gain (yes, `-q` - confirmed by reading the
    /// `crack()` switch, an unusual letter choice for this purpose but
    /// not a typo).
    pub sound_a_db: ControlFn,
    /// `-B`.
    pub sound_b_db: ControlFn,
    /// `-Z`.
    pub convolve_db: ControlFn,
    /// `-S`.
    pub pan: ControlFn,
    /// `-j`.
    pub panwarp_a: f32,
    /// `-J`.
    pub panwarp_b: f32,

    /// `-H`. Unlike most other tools' shelf EQ, `convolver.c` parses
    /// these four with plain `crackfloat`, not `crackstring` - fixed for
    /// the whole run, not a per-frame control function.
    pub shelf_low_db: f32,
    /// `-X`.
    pub shelf_high_db: f32,
    /// `-m`.
    pub shelf_low_freq: f32,
    /// `-R`.
    pub shelf_high_freq: f32,

    /// `-n`.
    pub frame_norm_limit_db: ControlFn,
    /// `-v`: `false` (the default) normalizes toward Sound A's own
    /// amplitude sum; `true` toward Sound B's (the filter analysis's).
    pub normalize_to_filter: bool,

    /// `-t`: oscillator-bank resynthesis threshold in dB.
    pub threshold_db: f32,
}

/// `frameNormalizationDecibelLimit.n > 1 || frameNormalizationDecibelLimit.A[0] > 0.`:
/// evaluated once from the control function's own static shape (a
/// multi-entry table, or a positive constant), not from any per-frame
/// value - matches the C's own one-time-at-startup `frameNormalizeFlag`
/// exactly.
fn frame_norm_enabled(cf: &ControlFn) -> bool {
    match cf {
        ControlFn::Table(vals) => vals.len() > 1,
        ControlFn::Const(v) => *v > 0.0,
    }
}

/// Ports `RIfindpeak()`: the maximum value in a real/imaginary-packed
/// rfft buffer (no absolute value, and no clamping to non-negative -
/// matches the C exactly; its `flag`-guarded `0.0` fallback only ever
/// triggers for an empty buffer, never the case here).
fn ri_find_peak(buffer: &[f32]) -> f32 {
    buffer.iter().copied().fold(f32::MIN, f32::max)
}

/// Reproduces `convolver.c`'s own buggy filter-frame fetch - see this
/// module's doc comment. `raw_stream` is the *true* on-disk interleaved
/// float stream (all channels' frames, real `analysis_n + 2` stride,
/// frame-major then channel-minor order - i.e. frame 0 channel 0, frame
/// 0 channel 1, ..., frame 1 channel 0, ...), reconstructed by the
/// caller from an already-correctly-parsed `pvc_io::PvaData`. Returns an
/// `analysis_n + 2`-float array whose last two floats (the Nyquist bin)
/// are always `0.0` (see the doc comment on why), and whose first
/// `analysis_n` floats are read starting `analysis_n` floats (not
/// `analysis_n + 2`) past the previous frame's own start - reproducing
/// the real drift exactly, not just a similar-looking approximation.
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

/// Resynthesizes one channel. `filter_raw_stream` is Sound B's *true*
/// on-disk interleaved frame stream (see [`buggy_filter_frame`]'s doc
/// comment) - not a per-channel frame list, since this tool's own real
/// filter-fetch bug reads across true frame boundaries in a way that
/// only makes sense against the raw stream. `input` should already be
/// trimmed to `[begint, endt)` by the caller (see this module's doc
/// comment). `dur` is the control-function normalization duration in
/// seconds (`input`'s own duration times `params.time_factor`).
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    input: &[f32],
    filter_raw_stream: &[f32],
    analysis_n: usize,
    analysis_d: u32,
    analysis_chan: usize,
    ainchan: usize,
    sample_rate: u32,
    params: &ConvolverParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = analysis_n;
    let n2 = n / 2;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;

    let d = analysis_d as usize;
    let i_factor = (d as f32 * params.time_factor) as usize;
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

    let threshfac = DbToAmp::new().convert(params.threshold_db);
    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();

    let iframes_per_sec = r / analysis_d as f32;
    // The *true* frame count (matching `convolver.c`'s own correctly-
    // computed `niframes`, which uses `analysis_N + 2` - only the
    // per-frame *fetch* is buggy, not this duration/window-bound math).
    let true_frame_floats = n + 2;
    let niframes = filter_raw_stream.len() / (analysis_chan * true_frame_floats);
    let analysis_dur = niframes as f32 / iframes_per_sec;

    let ptrans_is_const_zero = matches!(&params.pitch_transpose, ControlFn::Const(v) if *v == 0.0);
    let harmadd_is_const_zero = matches!(&params.freq_shift, ControlFn::Const(v) if *v == 0.0);
    let obank = !(ptrans_is_const_zero && harmadd_is_const_zero);

    let frame_normalize = frame_norm_enabled(&params.frame_norm_limit_db);

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = RawAnalyzer::new(n, window_pair.analysis, d);
    let mut osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut synth = Synthesizer::new(n, window_pair.synthesis.clone(), i_factor, d, sample_rate);
    // Sound B's own persistent phase-tracking state for `unconvert1()` -
    // a separate instance from `synth`'s (matching `filter.c`'s
    // established "hand-duplicated function -> separate per-instance
    // state" pattern already documented in `pvoc.rs`), reused every
    // frame purely for its `unconvert_only` half.
    let mut filter_synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, sample_rate);
    let mut obank_phase = PhaseTracker::new_analysis(n2, d, sample_rate);

    let nav_cfg = TimeNavConfig {
        onset_release: params.onset_release,
        autostop: params.autostop,
        loop_mode: params.loop_mode,
    };
    let mut dur = dur;
    let initial_time_origin = params.filter_time_origin.at(0.0, dur);
    let mut nav = TimeNavigator::new(nav_cfg, initial_time_origin);

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;
    let mut on: i64 = (-(nw as i64) * i_factor as i64) / d as i64;

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

        let mut buffer = analyzer.push(&hop);

        // ---- SOUND A'S OWN AMPLITUDE SUM (frame normalization only) ----
        let channel_amp_sum = if frame_normalize {
            lean_convert(&buffer, n2)[..n2].iter().map(|b| b.0).sum()
        } else {
            0.0
        };

        // ---- FILTER TIME NAVIGATION ----
        let time_origin_val = params.filter_time_origin.at(t, dur);
        let rate_val = params.filter_rate.at(t, dur);
        let win_low_val = params.filter_window_low.at(t, dur);
        let win_hi_raw = params.filter_window_high.at(t, dur);
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

        // ---- FETCH SOUND B'S FILTER FRAME (real bug reproduced - see
        // this module's doc comment) ----
        let filtflow = (iframes_per_sec * step.filttnow) as i64;
        let f_flat = buggy_filter_frame(filter_raw_stream, n, analysis_chan, ainchan, filtflow);
        let filter_channel_amp_sum: f32 = f_flat.iter().step_by(2).take(n2).copied().sum();

        let f_buffer = filter_synth.unconvert_only(&Frame::from_pva_floats(&f_flat));

        // ---- CONVOLVE (naive per-index multiply - see this module's
        // doc comment) ----
        let a_peak = ri_find_peak(&buffer);
        let b_peak = ri_find_peak(&f_buffer);
        let ab_peak = a_peak.max(b_peak);
        let normamp = if ab_peak == 0.0 { 1.0 } else { 1.0 / ab_peak };

        let sound_a_db = params.sound_a_db.at(t, dur);
        let sound_b_db = params.sound_b_db.at(t, dur);
        let convolve_db = params.convolve_db.at(t, dur);

        let pan_raw = params.pan.at(t, dur);
        let pans_abs = pan_raw.abs();
        let panwarp = if pan_raw < 0.0 {
            params.panwarp_a
        } else {
            params.panwarp_b
        };
        let pans = curve(0.0, 1.0, pans_abs, panwarp);

        let a_db = db_to_amp.convert(sound_a_db) * pans;
        let b_db = db_to_amp.convert(sound_b_db) * pans;
        let c_db = db_to_amp.convert(convolve_db) * (1.0 - pans);

        let mut c_buffer = vec![0.0f32; n];
        for i in 0..n {
            c_buffer[i] = normamp * c_db * buffer[i] * f_buffer[i];
        }

        let mut c_bins = lean_convert(&c_buffer, n2);
        apply_shelf_eq(
            &mut c_bins,
            params.shelf_low_db,
            params.shelf_high_db,
            params.shelf_low_freq,
            params.shelf_high_freq,
            fundamental,
            &db_to_amp,
        );
        let c_buffer = lean_unconvert(&c_bins, n);

        let gain = db_to_amp.convert(params.gain_db.at(t, dur));

        if pan_raw < 0.0 {
            for i in 0..n {
                buffer[i] = gain * (c_buffer[i] + a_db * buffer[i]);
            }
        } else {
            for i in 0..n {
                buffer[i] = gain * (c_buffer[i] + b_db * f_buffer[i]);
            }
        }

        // ---- FRAME NORMALIZATION ----
        if frame_normalize {
            let mut channel_bins = lean_convert(&buffer, n2);
            let temp_channel_amp_sum: f32 = channel_bins[..n2].iter().map(|b| b.0).sum();
            if temp_channel_amp_sum > 0.0 {
                let frame_norm_amp_limit = db_to_amp.convert(params.frame_norm_limit_db.at(t, dur));
                let target = if params.normalize_to_filter {
                    filter_channel_amp_sum
                } else {
                    channel_amp_sum
                };
                let normalization_amp = (target / temp_channel_amp_sum).min(frame_norm_amp_limit);
                for b in channel_bins[..n2].iter_mut() {
                    b.0 *= normalization_amp;
                }
            }
            buffer = lean_unconvert(&channel_bins, n);
        }

        // ---- OSCILLATOR BANK OR OVERLAP-ADD OUT ----
        if obank {
            let frame = obank_phase.convert(&buffer);
            let mut flat = frame.to_pva_floats();

            let harmadd = params.freq_shift.at(t, dur);
            let pm = semitones_to_mult.convert(params.pitch_transpose.at(t, dur));
            for j in 0..n2 {
                let i = 1 + 2 * j;
                let temp = pm * (flat[i] + harmadd);
                if temp <= 0.0 || temp >= nyquist {
                    flat[i - 1] = 0.0;
                } else {
                    flat[i] = temp;
                }
            }

            let frame = Frame::from_pva_floats(&flat);
            let synt = getthresh(&frame.bins[..n2], threshfac);
            let hop_out = osc.synthesize(&frame, synt);

            on += i_factor as i64;
            if on + nw as i64 - i_factor as i64 >= 0 {
                output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
                samps_written += i_factor;
            }
        } else {
            let hop_out = synth.finish(&mut buffer);
            if !hop_out.is_empty() {
                output.extend(hop_out);
                samps_written += i_factor;
            }
        }

        if eof_after_this_hop || step.autostop {
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

    /// A raw (true-stride) mono filter stream: `count` frames, each
    /// `analysis_n + 2` floats, all filled with `1.0` (a flat, nonzero
    /// spectrum at every bin including the Nyquist one).
    fn flat_raw_stream(analysis_n: usize, count: usize) -> Vec<f32> {
        vec![1.0f32; count * (analysis_n + 2)]
    }

    fn default_params() -> ConvolverParams {
        ConvolverParams {
            window_size: 0,
            window: Window::Hamming,
            time_factor: 1.0,
            pitch_transpose: ControlFn::Const(0.0),
            freq_shift: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            filter_time_origin: ControlFn::Const(0.0),
            filter_rate: ControlFn::Const(1.0),
            filter_window_low: ControlFn::Const(0.0),
            filter_window_high: ControlFn::Const(-1.0),
            loop_mode: LoopMode::Wrap,
            onset_release: false,
            autostop: false,
            sound_a_db: ControlFn::Const(0.0),
            sound_b_db: ControlFn::Const(0.0),
            convolve_db: ControlFn::Const(0.0),
            pan: ControlFn::Const(0.0),
            panwarp_a: 0.0,
            panwarp_b: 0.0,
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            frame_norm_limit_db: ControlFn::Const(0.0),
            normalize_to_filter: false,
            threshold_db: -96.0,
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let fft = 1024;
        let raw_stream = flat_raw_stream(fft, 50);
        let params = default_params();
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(&input, &raw_stream, fft, 220, 1, 0, 44100, &params, 1.0);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn sine_input_produces_bounded_output() {
        let fft = 1024;
        let raw_stream = flat_raw_stream(fft, 50);
        let params = default_params();
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(
            &input,
            &raw_stream,
            fft,
            220,
            1,
            0,
            sample_rate,
            &params,
            1.0,
        );
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak < 10.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn obank_selected_when_pitch_transpose_nonzero() {
        let fft = 1024;
        let raw_stream = flat_raw_stream(fft, 50);
        let mut params = default_params();
        params.pitch_transpose = ControlFn::Const(3.0);
        let input = vec![0.1f32; 44100 / 4];
        let output = process_channel(&input, &raw_stream, fft, 220, 1, 0, 44100, &params, 1.0);
        assert!(!output.is_empty());
    }

    #[test]
    fn buggy_filter_frame_drifts_two_floats_per_frame_and_zeroes_nyquist() {
        // Regression test for the real convolver.c bug (see this
        // module's doc comment): a raw stream where frame k's floats are
        // all set to (k+1) as a marker, true stride analysis_n+2=6
        // (analysis_n=4, n2=2, 3 bins/frame). The buggy fetch at
        // filtflow=k reads starting analysis_n*k=4k floats in, not
        // (n+2)*k=6k - by filtflow=2 that's already reading into frame
        // 1's tail, not frame 2's own start.
        let analysis_n = 4;
        let n_frames = 4;
        let mut raw = Vec::new();
        for f in 0..n_frames {
            raw.extend(vec![(f + 1) as f32; analysis_n + 2]);
        }

        let frame0 = buggy_filter_frame(&raw, analysis_n, 1, 0, 0);
        assert_eq!(frame0, vec![1.0, 1.0, 1.0, 1.0, 0.0, 0.0]);

        // filtflow=2: start = 2*4 = 8 floats in, which is 2 floats into
        // true frame 1 (floats 6..12), not the start of true frame 2.
        let frame2 = buggy_filter_frame(&raw, analysis_n, 1, 0, 2);
        assert_eq!(frame2[0..4], raw[8..12]);
        // Nyquist bin always zero, regardless of what's actually there.
        assert_eq!(frame2[4], 0.0);
        assert_eq!(frame2[5], 0.0);
    }
}
