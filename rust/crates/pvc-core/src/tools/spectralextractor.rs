//! Ports `spectralextractor.c`: a phase-vocoder "periodic vs. noise"
//! separator. Every bin's frame-to-frame frequency deviation is tracked
//! and exponentially smoothed; bins whose deviation stays under a
//! threshold are gated on (or off, depending on `-q`), producing either
//! the tonal/periodic part of a sound or its noise residue.
//!
//! **A genuinely dead computation, confirmed by reading the whole file**:
//! `-P` (pitch transpose) and `-a` (frequency shift) never actually
//! change the resynthesized signal's pitch or frequency. The C's own
//! per-bin loop that would apply them (`temp = pm * (harmadd.A[0] +
//! channel[i])`) computes `temp` and then never uses it - no assignment
//! back into `channel[i]`/`channel[i-1]`, no bounds gating, despite the
//! `// NEUTOR OUT OF BOUNDS FREQ BINS` comment implying intent to do so
//! (the same shape as `plainpv`'s/`twarp`'s own bin loops, which *do*
//! write `channel[i]`/`channel[i-1]` back - this one just never got the
//! assignment added). `-P`/`-a` still have two real, narrower effects:
//! (1) whichever of them is nonzero at all (checked once via a `ControlFn`
//! being anything other than `Const(0.0)`, matching `tools::twarp`'s own
//! precedent) selects oscillator-bank resynthesis over overlap-add, and
//! (2) their accumulated values feed `channel_freqdev`, which [`eq2`]
//! uses to compute each bin's *effective* frequency for shelf-EQ banding
//! only. Reproduced exactly here, not "fixed" - the golden harness
//! compares against the real tool's equally pitch-shift-inert output.
//!
//! Two more dead computations, confirmed the same way and simply not
//! implemented here (no observable effect either way): the C's first
//! `channelAmpSum` accumulation (during the per-bin gate loop) sums
//! *frequency* values under an amplitude-sounding name and is fully
//! overwritten by a second, correctly-computed `channelAmpSum` before
//! ever being read; and the `binfreq`/`wouldbephasepoint` arrays are
//! filled once at startup and never read again.
//!
//! Real, sample-accurate input trimming: `-b`/`-e` (begin/end time) are
//! **not** `plainpv`/`pvanalysis`-style `dur`-only bookkeeping (see
//! `docs/dev/rust-verification.md`'s Task 3.6 note on that pair) -
//! `spectralextractor.c` calls the same `setupfiles()`/`openfiles()`
//! pair as `filter.c`/`noisefilter.c`, whose `getInputFileDataToSetOutputChannels()`
//! (`legacy/pvc_lib/fileio.c`) seeks to `begint*R` and buffers only
//! `(endt-begint)*R` frames before the frame loop ever starts. This
//! port's caller is expected to pass an already-trimmed `input` slice
//! (see `commands::spectralextractor::run`), with `dur` computed from
//! that trimmed length, matching the C's own `dur = (endt-begint) * I /
//! D` (`I/D` reduces to `time_factor`).
//!
//! Like `tools::twarp`, the oscillator-bank-vs-overlap-add choice here is
//! genuinely conditional (unlike `plainpv`'s hardcoded-always-oscbank
//! quirk), decided once per channel.
//!
//! `-C` (single-channel-only resynthesis) is out of scope, matching
//! `tools::pv`'s own established precedent for the same flag letter and
//! meaning - this port always processes every channel.
//!
//! Several flags `crack()` declares (`s`, a duplicate `T`, `v`, `S`, `n`,
//! `x`, `u`, `U`, `d`, `B`, `f`, a duplicate `F`, `l`, `h`) have no `case`
//! in the switch at all - confirmed dead, not exposed here.

use crate::eq::{eq2, ShelfEq};
use crate::pvoc::{getthresh, Analyzer, Frame, OscBank, Synthesizer};
use crate::smooth::smooth_setup;
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `-q`: which part of the spectrum to keep.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SpectralType {
    /// Keep bins whose frequency deviation stays *below* the threshold.
    Periodic,
    /// Keep bins whose frequency deviation stays *above* the threshold.
    Noise,
}

pub struct SpectralExtractorParams {
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

    /// `-q`.
    pub spectral_type: SpectralType,
    /// `-Q`: max (periodic) / min (noise) frequency change allowed every
    /// 5 milliseconds, in Hz.
    pub freq_change_threshold_hz: ControlFn,
    /// `-g`: response time of the frequency-change threshold accumulator.
    pub freq_change_response_secs: ControlFn,
    /// `-L`: amplitude-gate release time.
    pub release_secs: ControlFn,

    /// `-c`: complement amplitude spectrum proportion (0-1).
    pub complement_prop: ControlFn,
    /// `-E`: frame normalization decibel limit (0 disables normalization).
    pub frame_norm_limit_db: ControlFn,

    /// `-W`.
    pub warpshape: ControlFn,

    /// `-H`.
    pub shelf_low_db: ControlFn,
    /// `-X`.
    pub shelf_high_db: ControlFn,
    /// `-m`.
    pub shelf_low_freq: ControlFn,
    /// `-R`.
    pub shelf_high_freq: ControlFn,

    /// `-t`: oscillator-bank resynthesis threshold in dB, only consumed
    /// on the oscillator-bank path.
    pub threshold_db: f32,
}

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant.
const OSCILBANKGAIN: f32 = 1.7782794;

/// Resynthesizes one channel. `input` should already be trimmed to
/// `[begint, endt)` by the caller (see this module's doc comment); `dur`
/// is the control-function normalization duration in seconds (legacy
/// `dur = (endt-begint) * I / D`, i.e. `input`'s own duration times
/// `params.time_factor`).
pub fn process_channel(
    input: &[f32],
    sample_rate: u32,
    params: &SpectralExtractorParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
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

    let threshfac = 10.0f64.powf(params.threshold_db as f64 / 20.0) as f32;

    // Selected once, matching spectralextractor.c's own startup-time (not
    // per-frame) obank decision - see this module's doc comment and
    // `tools::twarp`'s identical precedent.
    let ptrans_is_const_zero = matches!(&params.pitch_transpose, ControlFn::Const(v) if *v == 0.0);
    let harmadd_is_const_zero = matches!(&params.freq_shift, ControlFn::Const(v) if *v == 0.0);
    let obank = !(ptrans_is_const_zero && harmadd_is_const_zero);

    // `freqChangeBWnormalizer = 10.0 / (0.5 * fundamental)`.
    let freq_change_bw_normalizer = 10.0 / (0.5 * fundamental);

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);
    let mut osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, sample_rate);

    // Per-channel state the C keeps in `previous_channel1`/
    // `previous_gain_mult`/`avg_change`, primed on `frame_count == 0`
    // rather than a global counter.
    let mut previous_channel1: Option<Vec<f32>> = None;
    let mut previous_gain_mult = vec![1.0f32; n2 + 1];
    let mut avg_change = vec![0.0f32; n + 2];

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

        let frame = analyzer.push(&hop).expect("hop is exactly d samples");
        let mut channel = frame.to_pva_floats();

        if previous_channel1.is_none() {
            previous_channel1 = Some(channel.clone());
        }
        let previous_channel1 = previous_channel1.as_mut().unwrap();

        // SETUP FREQDEV HISTORY ARRAY: reset every frame, matching the C
        // (`channel_freqdev` is not carried across frames despite its
        // name - see this module's doc comment on `-P`/`-a`).
        let mut channel_freqdev = vec![0.0f32; n + 2];
        for k in (1..n).step_by(2) {
            channel_freqdev[k] = 1.0;
        }

        // ---- CONTROL VALUES ----
        let harmadd = params.freq_shift.at(t, dur);
        let gain = db_to_amp.convert(params.gain_db.at(t, dur));
        let pm = semitones_to_mult.convert(params.pitch_transpose.at(t, dur));

        let release = params.release_secs.at(t, dur);
        let (releasec, minusreleasec) = smooth_setup(release, ir);

        let warpshape = params.warpshape.at(t, dur);
        let shelf_low_db = params.shelf_low_db.at(t, dur);
        let shelf_high_db = params.shelf_high_db.at(t, dur);
        let shelf_low_freq = params.shelf_low_freq.at(t, dur);
        let shelf_high_freq = params.shelf_high_freq.at(t, dur);

        let f_spect_thresh = params.freq_change_threshold_hz.at(t, dur);
        // `f_spect_t = 200. * f_spect_thresh / frames_per_sec`.
        let f_spect_t = 200.0 * f_spect_thresh / params.frames_per_sec;

        let avgresponse = params.freq_change_response_secs.at(t, dur);
        let (avgresponsec, minusavgresponsec) = smooth_setup(avgresponse, ir);

        let complement_prop = params.complement_prop.at(t, dur);

        let frame_norm_amp_limit = db_to_amp.convert(params.frame_norm_limit_db.at(t, dur));

        // ---- PERIODIC/NOISE GATE ----
        // Amplitude/frequency slots are interleaved [amp0, freq0, amp1,
        // freq1, ...] (see `pvoc::Frame`). `j` indexes bins (0..=n2); `i`
        // is the matching odd (frequency) slot in `channel`/`tempchannel`.
        let mut tempchannel = channel.clone();
        #[allow(clippy::needless_range_loop)]
        for j in 0..=n2 {
            let i = 1 + 2 * j;
            tempchannel[i] = channel[i];
            tempchannel[i - 1] = channel[i - 1];

            let freqc = (tempchannel[i] - previous_channel1[i]).abs() * freq_change_bw_normalizer;
            avg_change[i] = minusavgresponsec * freqc + avgresponsec * avg_change[i];

            let turn_on = match params.spectral_type {
                SpectralType::Periodic => avg_change[i] <= f_spect_t,
                SpectralType::Noise => avg_change[i] >= f_spect_t,
            };
            let gate_mult = if turn_on {
                releasec * previous_gain_mult[j] + minusreleasec
            } else {
                releasec * previous_gain_mult[j]
            };

            tempchannel[i - 1] *= gate_mult;
            previous_gain_mult[j] = gate_mult;

            previous_channel1[i - 1] = tempchannel[i - 1];
            previous_channel1[i] = tempchannel[i];
        }

        // ---- SOURCE/COMPLEMENT MIX ----
        for j in 0..=n2 {
            let i = 2 * j;
            tempchannel[i] += complement_prop * (channel[i] - 2.0 * tempchannel[i]);
        }

        // ---- NORMALIZE AND REPLACE INTO CHANNEL ----
        let channel_amp_sum: f32 = (0..=n2).map(|j| channel[2 * j]).sum();
        let temp_channel_amp_sum: f32 = (0..=n2).map(|j| tempchannel[2 * j]).sum();
        if temp_channel_amp_sum > 0.0 && frame_norm_amp_limit != 1.0 {
            let mut normalization_amp = channel_amp_sum / temp_channel_amp_sum;
            if normalization_amp > frame_norm_amp_limit {
                normalization_amp = frame_norm_amp_limit;
            }
            for j in 0..=n2 {
                channel[2 * j] = tempchannel[2 * j] * normalization_amp;
            }
        } else {
            for j in 0..=n2 {
                channel[2 * j] = tempchannel[2 * j];
            }
        }

        // ---- CHANNEL_FREQDEV BOOKKEEPING (see this module's doc comment:
        // `pm`/`harmadd` never touch `channel`'s own amp/freq slots) ----
        for j in 0..=n2 {
            let i = 1 + 2 * j;
            channel_freqdev[i] *= pm;
            channel_freqdev[i - 1] += harmadd;
        }

        // ADD GAIN.
        for j in 0..=n2 {
            channel[2 * j] *= gain;
        }

        spectmagwarp(&mut channel, warpshape, false);
        eq2(
            &mut channel,
            &ShelfEq {
                d_blow: shelf_low_db,
                d_bhi: shelf_high_db,
                freqlow: shelf_low_freq,
                freqhi: shelf_high_freq,
            },
            fundamental,
            &channel_freqdev,
            false,
            &db_to_amp,
        );

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

    fn default_params(fft_size: usize) -> SpectralExtractorParams {
        SpectralExtractorParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            pitch_transpose: ControlFn::Const(0.0),
            freq_shift: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            spectral_type: SpectralType::Periodic,
            freq_change_threshold_hz: ControlFn::Const(0.0),
            freq_change_response_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
            complement_prop: ControlFn::Const(0.0),
            frame_norm_limit_db: ControlFn::Const(0.0),
            warpshape: ControlFn::Const(0.0),
            shelf_low_db: ControlFn::Const(0.0),
            shelf_high_db: ControlFn::Const(0.0),
            shelf_low_freq: ControlFn::Const(200.0),
            shelf_high_freq: ControlFn::Const(2000.0),
            threshold_db: -96.0,
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let fft = 1024;
        let params = default_params(fft);
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(&input, 44100, &params, 1.0);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn sine_input_produces_bounded_nonzero_output_periodic_mode() {
        let fft = 1024;
        let params = default_params(fft);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate * 2)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(&input, sample_rate, &params, 2.0);

        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.05, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn sine_input_produces_bounded_output_noise_mode() {
        // A pure, stable tone has ~zero frequency deviation frame to
        // frame, so noise mode (keep bins *above* the threshold) should
        // gate nearly everything off once the release envelope settles.
        let fft = 1024;
        let mut params = default_params(fft);
        params.spectral_type = SpectralType::Noise;
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate * 2)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(&input, sample_rate, &params, 2.0);

        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn obank_selected_when_pitch_transpose_nonzero() {
        let fft = 1024;
        let mut params = default_params(fft);
        params.pitch_transpose = ControlFn::Const(3.0);
        let input = vec![0.1f32; 44100 / 4];
        // Just needs to run to completion without panicking on either path.
        let output = process_channel(&input, 44100, &params, 1.0);
        assert!(!output.is_empty());
    }
}
