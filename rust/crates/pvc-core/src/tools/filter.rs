//! Ports `filter.c`: a fixed-spectrum phase-vocoder filter that
//! multiplies each frame's amplitude by a time-varying-positioned,
//! shaped copy of a pre-computed `.fr` response (see `pvc-core::
//! filter_response`/`pvc-io::response`), then resynthesizes *both* that
//! filtered signal and (unless disabled) a separately delayed/shifted
//! copy of the original analyzed signal, mixed together - not a
//! subtractive filter that replaces the source, an additive one that
//! layers a shaped copy alongside it. Confirmed by reading the source-
//! mixing block and its default: `SOURCE_dB` (source gain) defaults to a
//! constant `0.0`, which is `> -96.0`, so `sourceflag` is `true` by
//! default - the source-mixing path isn't a rare option, it's what a
//! bare `pvc filter` invocation actually does (with all-default source
//! parameters, the mixed-in source is unmodified - `0dB` gain, `0`
//! semitones, `0Hz` shift - so it's an identity mix, not an audible
//! effect, until a source parameter is actually set away from its
//! default).
//!
//! The response's own frequency values are never read after a one-time,
//! dead "convert frequencies to phase differences" step - confirmed by
//! reading the whole file: nothing downstream of that conversion ever
//! indexes an odd (frequency) slot of `F`/`FF` again, only even
//! (amplitude) ones. So this port (and `pvc-core::filter_response`)
//! represent both `F` and `FF` as plain amplitude arrays, skipping that
//! conversion entirely - not a simplification that changes behavior,
//! since the C's own conversion result is provably unused.
//!
//! The per-frame response-shaping chain is exactly what `filter.c`'s own
//! usage text documents: `EQ -> COMPANDING -> WARP -> INVERSION ->
//! SMOOTHING -> NORMALIZATION` - except EQ and companding happen once,
//! up front (their inputs - shelf EQ decibels/frequencies, compression/
//! expansion thresholds - are plain floats, not control functions,
//! confirmed by their `crackfloat` parsing), while warp/inversion/
//! smoothing/normalization repeat every frame against a time-varying
//! warpshape/smoothing bandwidth/frame-normalization limit.
//!
//! This port always forces the FFT size to the response file's own size
//! (`analysis_N`) rather than exposing an independent `--fft` that has
//! to be kept in sync - the real tool only *warns* on a mismatch (`-N`
//! not matching the response file's size) without actually correcting
//! for it anywhere in the code that follows, which reads as an
//! incomplete safeguard rather than a real feature worth preserving.
//!
//! Not yet ported: the oscillator-bank resynthesis path (`noscbank2`,
//! needed only when a pitch/frequency shift is requested - unmodified
//! frequencies use overlap-add, this module's only resynthesis path so
//! far) and time-varying `--filter-time-delay`/`--source-time-delay`
//! beyond their zero defaults (the delay-line machinery itself is fully
//! implemented and exercised even at zero delay).

use crate::eq::eq;
use crate::filter_response::{compand, invert_response, smooth_response};
use crate::pvoc::{Analyzer, Frame, Synthesizer};
use crate::smooth::{smooth_setup, Smoother};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::filter_warp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

#[derive(Debug, Clone)]
pub struct FilterParams {
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,

    pub filter_pitch_transpose: ControlFn,
    pub filter_freq_shift: ControlFn,
    pub filter_gain_db: ControlFn,
    pub filter_time_delay: ControlFn,
    pub delay_time_scaler: ControlFn,

    pub source_enabled: bool,
    pub source_gain_db: ControlFn,
    pub source_pitch_transpose: ControlFn,
    pub source_freq_shift: ControlFn,
    pub source_time_delay: ControlFn,

    pub response_pitch_transpose: ControlFn,
    pub response_freq_shift: ControlFn,
    pub response_warpshape: ControlFn,
    pub response_smoothing_bw: ControlFn,
    pub source_floor_db: ControlFn,
    /// `-B`: when `false` (the default), the filter-output's own pitch/
    /// frequency shift is subtracted back out of the response's
    /// positioning, so the response tracks the source's shift instead of
    /// staying fixed while only the source moves.
    pub pitch_shift_source_only: bool,

    pub band_reject: bool,

    pub shelf_low_db: f32,
    pub shelf_high_db: f32,
    pub shelf_low_freq: f32,
    pub shelf_high_freq: f32,

    pub comp_threshold_db: f32,
    pub comp_db: f32,
    pub exp_threshold_db: f32,
    pub exp_db: f32,

    pub frame_normalization_limit_db: ControlFn,
    pub normalize_to_filter: bool,

    pub attack_secs: ControlFn,
    pub release_secs: ControlFn,
}

fn control_fn_max(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(vals) => vals.iter().copied().fold(f32::MIN, f32::max),
    }
}

/// A ring of the last `capacity` analysis frames, indexed by
/// `frame_count % capacity` - ports the `channel_delay` flat array plus
/// its `frameNowChannelDelayIndex`/`thisFrameDelay` lookups.
struct DelayLine {
    frames: Vec<Frame>,
    capacity: usize,
}

impl DelayLine {
    fn new(capacity: usize, n_plus_2: usize) -> Self {
        DelayLine {
            frames: vec![Frame::from_pva_floats(&vec![0.0; n_plus_2]); capacity],
            capacity,
        }
    }

    fn push(&mut self, frame_count: usize, frame: Frame) -> usize {
        let index = frame_count % self.capacity;
        self.frames[index] = frame;
        index
    }

    /// Looks up the frame `delay_secs` seconds before `now_index`,
    /// matching `thisFrameDelay = now - (int)(delay*frames_per_sec+0.5);
    /// while (thisFrameDelay < 0) thisFrameDelay += capacity;`.
    fn get(&self, now_index: usize, delay_secs: f32, frames_per_sec: f32) -> &Frame {
        let offset = (delay_secs * frames_per_sec + 0.5) as i64;
        let mut index = now_index as i64 - offset;
        while index < 0 {
            index += self.capacity as i64;
        }
        &self.frames[(index as usize) % self.capacity]
    }
}

/// Resynthesizes one channel. `response_amps` is the loaded `.fr` file's
/// amplitudes (`analysis_n / 2 + 1` values, already separated from its
/// frequency slots - dead for this tool, see this module's doc comment).
/// `analysis_n` is the response file's own FFT size - this port's FFT
/// size, per this module's doc comment.
pub fn process_channel(
    input: &[f32],
    response_amps: &[f32],
    analysis_n: usize,
    sample_rate: u32,
    params: &FilterParams,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = analysis_n;
    let n2 = n / 2;
    let nyquist = r / 2.0;
    let n_plus_2 = n + 2;
    let fundamental = r / n as f32;

    let d = (r / params.frames_per_sec) as usize;
    let i_factor = (d as f32 * params.time_factor) as usize;
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

    // ---- One-time response setup: EQ -> COMPANDING ----
    let mut response = response_amps.to_vec();
    let filter_channel_amp_sum: f32 = response.iter().sum();

    // eq() operates on the full mag/freq-interleaved layout; adapt with
    // a throwaway interleaved buffer (frequency slots unused, per this
    // module's doc comment - zero is fine).
    {
        let mut interleaved = vec![0.0f32; n_plus_2];
        for (j, &a) in response.iter().enumerate() {
            interleaved[2 * j] = a;
        }
        eq(
            &mut interleaved,
            params.shelf_low_db,
            params.shelf_high_db,
            params.shelf_low_freq,
            params.shelf_high_freq,
            fundamental,
            1.0,
            0.0,
            true,
            &db_to_amp,
        );
        for (j, a) in response.iter_mut().enumerate() {
            *a = interleaved[2 * j];
        }
    }

    let comp_threshold_amp = db_to_amp.convert(params.comp_threshold_db);
    let comp_amp = db_to_amp.convert(params.comp_db);
    let exp_threshold_amp = db_to_amp.convert(params.exp_threshold_db);
    let exp_amp = 1.0 / db_to_amp.convert(params.exp_db);
    if comp_threshold_amp > exp_threshold_amp && comp_amp <= 1.0 && exp_amp >= 1.0 {
        compand(
            &mut response,
            comp_threshold_amp,
            comp_amp,
            exp_threshold_amp,
            exp_amp,
        );
    }

    // ---- Delay line sizing ----
    let mut max_delay_t = control_fn_max(&params.filter_time_delay);
    if params.source_enabled {
        max_delay_t = max_delay_t.max(control_fn_max(&params.source_time_delay));
    }
    let max_num_delay_frames = 1 + (max_delay_t * params.frames_per_sec + 0.5) as usize;

    // ---- Per-channel state ----
    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);
    let mut synth = Synthesizer::new(n, window_pair.synthesis.clone(), i_factor, d, sample_rate);
    // Independent phase-tracking state for the source path (matching
    // `unconvert`/`unconvert1`'s separate static phase memory in the C -
    // see `Synthesizer::unconvert_only`'s doc comment) - only its
    // `unconvert_only` is ever called; `synth` above owns the shared
    // ring/window that `finish` uses for both combined.
    let mut source_synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, sample_rate);
    let mut smoother = Smoother::new(n_plus_2);
    let mut delay_line = DelayLine::new(max_num_delay_frames, n_plus_2);

    // Control-function normalization duration - matches `pv`'s own
    // "natural output duration" convention (`dur = (endt - begint) * I /
    // D` in the C, but `-b`/`-e` aren't ported here either, same as
    // `pv`/`pvanalysis` - see `tools::pv`'s doc comment on why callers
    // pick `dur` directly instead of replicating that indirection).
    let dur = input.len() as f32 / r * params.time_factor;

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;

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
        let now_index = delay_line.push(frame_count, channel_frame);

        let filter_delay = params.filter_time_delay.at(t, dur);
        let mut channel_filter = delay_line
            .get(now_index, filter_delay, params.frames_per_sec)
            .clone();

        let scaler = params.delay_time_scaler.at(t, dur);
        let t_shifted = t - scaler * filter_delay;

        let harmadd = params.filter_freq_shift.at(t_shifted, dur);
        let gain = db_to_amp.convert(params.filter_gain_db.at(t_shifted, dur));
        let pm = semitones_to_mult.convert(params.filter_pitch_transpose.at(t_shifted, dur));

        let mut fs = params.response_freq_shift.at(t_shifted, dur);
        if !params.pitch_shift_source_only {
            fs -= harmadd;
        }
        let mut fm = semitones_to_mult.convert(params.response_pitch_transpose.at(t_shifted, dur));
        if !params.pitch_shift_source_only {
            fm /= pm;
        }

        let filterampfloor = db_to_amp.convert(params.source_floor_db.at(t_shifted, dur));
        let filtamp = 1.0 - filterampfloor;

        let warpshape = params.response_warpshape.at(t_shifted, dur);
        let (attackc, minusattackc) = smooth_setup(params.attack_secs.at(t_shifted, dur), ir);
        let (releasec, minusreleasec) = smooth_setup(params.release_secs.at(t_shifted, dur), ir);
        let frame_normalization_amp_limit =
            db_to_amp.convert(params.frame_normalization_limit_db.at(t_shifted, dur));

        // ---- WARP -> INVERSION -> SMOOTHING ----
        let response_interleaved: Vec<f32> = response.iter().flat_map(|&a| [a, 0.0]).collect();
        let mut ff = filter_warp(&response_interleaved, warpshape, true);
        smooth_response(
            &mut ff,
            params.response_smoothing_bw.at(t_shifted, dur),
            sample_rate,
        );
        if params.band_reject {
            invert_response(&mut ff, false, &db_to_amp);
        }

        let channel_amp_sum: f32 = channel_filter.bins.iter().map(|&(a, _)| a).sum();

        // ---- APPLY FILTER (bin-shifted/transposed interpolation of ff) ----
        for j in 0..=n2 {
            let mut temp = j as f32; // N_ratio is always 1.0 (N forced equal to analysis_N).
            temp -= fs / fundamental;
            temp /= fm;
            // `(int) temp` in the C truncates toward zero, not floors -
            // matters once `temp` goes negative (a source/filter shift
            // large enough to push a low bin below the response's
            // origin), so `as i64` (which also truncates toward zero in
            // Rust) is used here rather than `.floor()`.
            let i1 = temp as i64;
            let i2p = temp - i1 as f32;
            let i1p = 1.0 - i2p;
            let i2 = i1 + 1;

            let interp = if i1 < 0 {
                ff[0]
            } else if i2 as usize >= n2 {
                ff[n2 - 1]
            } else {
                ff[i1 as usize] * i1p + ff[i2 as usize] * i2p
            };

            channel_filter.bins[j].0 *= filtamp * interp + filterampfloor;
        }

        // ---- FRAME NORMALIZATION ----
        let temp_channel_amp_sum: f32 = channel_filter.bins.iter().map(|&(a, _)| a).sum();
        if temp_channel_amp_sum > 0.0 && frame_normalization_amp_limit != 1.0 {
            let target = if params.normalize_to_filter {
                filter_channel_amp_sum
            } else {
                channel_amp_sum
            };
            let mut normalization_amp = target / temp_channel_amp_sum;
            if normalization_amp > frame_normalization_amp_limit {
                normalization_amp = frame_normalization_amp_limit;
            }
            for (a, _) in channel_filter.bins.iter_mut() {
                *a *= normalization_amp;
            }
        }

        // ---- ATTACK/RELEASE SMOOTHING ----
        let mut flat = channel_filter.to_pva_floats();
        smoother.smooth(&mut flat, attackc, minusattackc, releasec, minusreleasec);
        channel_filter = Frame::from_pva_floats(&flat);

        // ---- FILTER OUTPUT PITCH/FREQ SHIFT + GAIN ----
        // Loop bound matches the C's `for (i=1; i<(N+2); i+=2)`: `i`
        // steps through *every* frequency slot from bin 0's (flat index
        // 1) through the Nyquist bin's (flat index N+1, since N is
        // even) - i.e. bins 0..=n2 inclusive, the *whole* array. Unlike
        // `pv`/`twarp`'s analogous loops (bounded by `N`, not `N+2`,
        // deliberately excluding Nyquist), this one covers everything -
        // confirmed by re-reading the exact bound, not assumed by
        // analogy with those other tools.
        for j in 0..=n2 {
            let freq = channel_filter.bins[j].1;
            let temp = pm * (freq + harmadd);
            if temp <= 0.0 || temp >= nyquist {
                channel_filter.bins[j].0 = 0.0;
            } else {
                channel_filter.bins[j].1 = temp;
            }
            channel_filter.bins[j].0 *= gain;
        }

        // ---- SOURCE ----
        let source_contrib = if params.source_enabled {
            let source_delay = params.source_time_delay.at(t, dur);
            let mut source_frame = delay_line
                .get(now_index, source_delay, params.frames_per_sec)
                .clone();

            let source_t_shifted = t - scaler * source_delay;
            let source_gain = db_to_amp.convert(params.source_gain_db.at(source_t_shifted, dur));
            let source_pm =
                semitones_to_mult.convert(params.source_pitch_transpose.at(source_t_shifted, dur));
            let source_fshift = params.source_freq_shift.at(source_t_shifted, dur);

            for (amp, freq) in source_frame.bins.iter_mut() {
                *amp *= source_gain;
                *freq = *freq * source_pm + source_fshift;
                if *freq < 0.0 {
                    *freq = 0.0;
                    *amp = 0.0;
                }
            }
            Some(source_frame)
        } else {
            None
        };

        // ---- RESYNTHESIS (overlap-add only - see module doc comment) ----
        let mut combined = synth.unconvert_only(&channel_filter);
        if let Some(source_frame) = source_contrib {
            let source_buf = source_synth.unconvert_only(&source_frame);
            for (c, s) in combined.iter_mut().zip(&source_buf) {
                *c += s;
            }
        }

        let hop_out = synth.finish(&mut combined);
        if !hop_out.is_empty() {
            output.extend(hop_out);
            samps_written += i_factor;
        }

        frame_count += 1;
        if eof_after_this_hop {
            break;
        }
    }

    output.extend(synth.flush());

    output
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params() -> FilterParams {
        FilterParams {
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            filter_pitch_transpose: ControlFn::Const(0.0),
            filter_freq_shift: ControlFn::Const(0.0),
            filter_gain_db: ControlFn::Const(0.0),
            filter_time_delay: ControlFn::Const(0.0),
            delay_time_scaler: ControlFn::Const(1.0),
            source_enabled: true,
            source_gain_db: ControlFn::Const(0.0),
            source_pitch_transpose: ControlFn::Const(0.0),
            source_freq_shift: ControlFn::Const(0.0),
            source_time_delay: ControlFn::Const(0.0),
            response_pitch_transpose: ControlFn::Const(0.0),
            response_freq_shift: ControlFn::Const(0.0),
            response_warpshape: ControlFn::Const(0.0),
            response_smoothing_bw: ControlFn::Const(0.0),
            source_floor_db: ControlFn::Const(-96.0),
            pitch_shift_source_only: false,
            band_reject: false,
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            comp_threshold_db: 0.0,
            comp_db: 0.0,
            exp_threshold_db: -96.0,
            exp_db: 0.0,
            frame_normalization_limit_db: ControlFn::Const(0.0),
            normalize_to_filter: false,
            attack_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let fft = 1024;
        let response = vec![1.0f32; fft / 2 + 1];
        let params = default_params();
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(&input, &response, fft, 44100, &params);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn flat_response_passes_sine_input_through_at_bounded_amplitude() {
        let fft = 1024;
        let response = vec![1.0f32; fft / 2 + 1];
        let params = default_params();
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(&input, &response, fft, sample_rate, &params);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.1, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }
}
