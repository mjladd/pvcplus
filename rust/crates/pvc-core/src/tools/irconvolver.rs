//! Ports `irconvolver.c`: fast FFT-based convolution (or, `-a 0`,
//! deconvolution) of each input channel against a spectrum loaded from an
//! `.ir` file (`pvc-io::ir`, written by `pvc-core::tools::impulseresponse`
//! or the real `impulseresponse` binary), via classic non-overlapping-
//! input/overlapping-output-tail block convolution.
//!
//! **Doc bug in the C, corrected here**: `usage()` prints `-a` as
//! "Spectrum DB Plot File", but the actual `switch` binds `-a` to
//! `deconvolution_0__convolution_1` - the convolution/deconvolution mode
//! switch (default `1`, convolution). There is no spectrum-plot flag at
//! all; `usage()`'s text for `-a` is simply wrong. [`Mode`] is this port's
//! replacement for that flag.
//!
//! **Real, faithfully-reproduced output-truncation bug**: real
//! `irconvolver` output length is always a multiple of `BLOCKSIZE`
//! (`1024`) - the shared `bufferout()`'s final "flush and close" call is
//! `bufferout(outbuffer, 0, 1)`, and passing `I = 0` means its own
//! `while (outbuffpt < I)` loop never runs at all, `flushflag`
//! notwithstanding, so a genuinely pending sub-`BLOCKSIZE` remainder is
//! silently never written. See [`process`]'s doc comment for the detail
//! and how it was confirmed against a real compiled binary.
//!
//! Not ported (documented, not silently dropped):
//! - `-M` (`multichannel_output_mode__standard_0__alternate_1`): an
//!   "alternate" output mode that expands a single input channel into
//!   `numberOfImpulseChannels` duplicate output channels, one per impulse
//!   channel - a channel-*count*-changing mode, unlike every other flag
//!   this port exposes. Matches this project's established practice of
//!   not porting channel-topology-changing flags (`pvc impulseresponse`'s
//!   `-C`/`-M` precedent).
//! - `-C` (`channelout`): the same "process one input channel, optionally
//!   shifted" knob already left unported in `pvc impulseresponse` - this
//!   port always processes every channel present in the input.
//! - `-x`/`-P`/`-B`/`-F`/`-Z`/`-z` (impulse-response "signal shaping"): a
//!   per-*sample* (not per-bin) nonlinear waveshaper applied to the dry
//!   input before the source-gain mix, off by default
//!   (`impulseShapingFlag_Off_0__On_1 = 0`). A real but secondary feature,
//!   deferred per this project's "port what's exercised/on by default,
//!   document the rest" precedent (`tools::reshape`'s mode subset,
//!   `tools::pitchtracker`'s `ANALYSIS_DATA_FILE` input mode).
//! - `-d`'s usage-text label ("amplitude normalization level in dB") is
//!   *also* wrong - the actual switch binds `-d` to
//!   `add_ring_time__off_0__on_1`, this port's [`IrconvolverParams::
//!   add_ring_time`]. There's no separate normalization-level flag on
//!   this tool at all (normalization happens once, in `impulseresponse`,
//!   before the `.ir` file is ever written).
//!
//! The bandpass rolloff filters (`-s`/`-t`/`-g`/`-G` on the impulse
//! spectrum, `-D`/`-f`/`-h`/`-H` on the input spectrum) share one real
//! quirk, reproduced faithfully in [`apply_bandpass_rolloff`]: the C's
//! rolloff loop only ever runs `k` from `0` to `n2 - 1` (`k < N/2`), and
//! bin `0`'s iteration (`i=0, j=1`) packs *both* the DC term (index `0`)
//! and the Nyquist term (index `1`) - but the `k == 0` branch (taken
//! whenever the low-rolloff cutoff excludes DC) only ever zeroes index
//! `0`, never touching index `1`. The Nyquist bin is consequently *never*
//! attenuated by either rolloff, low or high, regardless of parameters -
//! a real asymmetry between the DC and Nyquist ends of the spectrum, not
//! a design choice worth second-guessing here.

use crate::fft::rfft;
use crate::response::hz_to_midi;
use crate::units::DbToAmp;
use crate::ControlFn;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Mode {
    Convolution,
    Deconvolution,
}

/// One channel's rfft-format spectrum, plus the two values every channel
/// shares (`fft_size` and `impulse_len` - `pvc_io::ir::IrHeader`'s
/// `channels` count is just `channels.len()` here). A `pvc-core`-local
/// mirror of `pvc_io::ir::IrData` (`pvc-core` has no I/O dependency of its
/// own) - the CLI layer converts one into the other.
pub struct ImpulseSpectra {
    pub fft_size: usize,
    pub impulse_len: usize,
    pub channels: Vec<Vec<f32>>,
}

pub struct IrconvolverParams {
    /// `-b`: analysis window start, in seconds.
    pub begin_secs: f32,
    /// `-e`: analysis window end, in seconds. `<= 0.0` means "whole file".
    pub end_secs: f32,
    /// `-d`: extends the processed window by one impulse-length of
    /// trailing silence, so the convolution's reverb tail isn't cut off
    /// at the source's own end.
    pub add_ring_time: bool,
    /// `-J`: which `.ir` channel to use. `0` means "auto": start at
    /// impulse channel 0, advancing to the next impulse channel (wrapping)
    /// after each audio channel - only meaningful when the `.ir` file has
    /// more than one channel. A nonzero (1-based) value fixes one impulse
    /// channel for every audio channel.
    pub impulse_channel: usize,
    /// `-a` (see this module's doc comment for the usage-text bug).
    pub mode: Mode,
    /// `-s`: impulse response bandpass low rolloff point, Hz.
    pub ir_low_freq: f32,
    /// `-t`: impulse response bandpass high rolloff point, Hz. `<= 0.0`
    /// means Nyquist.
    pub ir_high_freq: f32,
    /// `-g`.
    pub ir_low_rolloff_db_per_octave: f32,
    /// `-G`.
    pub ir_high_rolloff_db_per_octave: f32,
    /// `-D`: input sound bandpass low rolloff point, Hz.
    pub source_low_freq: f32,
    /// `-f`: input sound bandpass high rolloff point, Hz. `<= 0.0` means
    /// Nyquist.
    pub source_high_freq: f32,
    /// `-h`.
    pub source_low_rolloff_db_per_octave: f32,
    /// `-H`.
    pub source_high_rolloff_db_per_octave: f32,
    /// `-A`: dry-signal gain mixed back in after convolution.
    pub source_gain_db: ControlFn,
    /// `-q`: gain applied to the signal *before* convolution.
    pub input_gain_db: ControlFn,
    /// `-r`: gain applied to the convolution output, before the dry mix.
    pub output_gain_db: ControlFn,
}

/// A resolved (post-Nyquist-default) low/high bandpass edge, each with its
/// own dB/octave rolloff slope - one instance for the impulse-response
/// filter band (`-s`/`-t`/`-g`/`-G`), another for the input-sound filter
/// band (`-D`/`-f`/`-h`/`-H`).
struct RolloffBand {
    low_freq: f32,
    high_freq: f32,
    low_db_per_octave: f32,
    high_db_per_octave: f32,
}

/// Ports the C's inline bandpass rolloff loop (`for k in 0..n2`,
/// `binFreq = k * fundamental`): MIDI-space linear dB/octave rolloff below
/// `low_freq` and above `high_freq`. See this module's doc comment for the
/// DC-zeroed/Nyquist-untouched quirk this reproduces exactly.
fn apply_bandpass_rolloff(
    spectrum: &mut [f32],
    n2: usize,
    fundamental: f32,
    band: &RolloffBand,
    db_to_amp: &DbToAmp,
) {
    for k in 0..n2 {
        let i = 2 * k;
        let j = i + 1;
        let bin_freq = k as f32 * fundamental;
        if bin_freq < band.low_freq {
            if k == 0 {
                spectrum[i] = 0.0;
            } else {
                let rolloff_db = (hz_to_midi(band.low_freq) - hz_to_midi(bin_freq)) / 12.0
                    * band.low_db_per_octave;
                let rolloff_amp = db_to_amp.convert(rolloff_db);
                spectrum[i] *= rolloff_amp;
                spectrum[j] *= rolloff_amp;
            }
        } else if bin_freq > band.high_freq {
            let rolloff_db = (hz_to_midi(bin_freq) - hz_to_midi(band.high_freq)) / 12.0
                * band.high_db_per_octave;
            let rolloff_amp = db_to_amp.convert(rolloff_db);
            spectrum[i] *= rolloff_amp;
            spectrum[j] *= rolloff_amp;
        }
    }
}

/// Processes every channel. `ir.channels.len()` must be nonzero and
/// `params.impulse_channel` (if nonzero) must be `<= ir.channels.len()` -
/// both are user-facing validation errors checked by the CLI layer before
/// calling in, matching this project's established convention (e.g.
/// `pvc impulseresponse`'s sample-rate-mismatch check).
pub fn process(
    channels: &[Vec<f32>],
    sample_rate: u32,
    ir: &ImpulseSpectra,
    params: &IrconvolverParams,
    dur: f32,
) -> Vec<Vec<f32>> {
    let r = sample_rate as f32;
    let n = ir.fft_size;
    let n2 = n / 2;
    let lh = ir.impulse_len;
    let lhm1 = lh - 1;
    let fundamental = r / n as f32;
    let nyquist = r / 2.0;

    let ir_band = RolloffBand {
        low_freq: params.ir_low_freq.max(0.0),
        high_freq: if params.ir_high_freq <= 0.0 {
            nyquist
        } else {
            params.ir_high_freq
        },
        low_db_per_octave: params.ir_low_rolloff_db_per_octave,
        high_db_per_octave: params.ir_high_rolloff_db_per_octave,
    };
    let source_band = RolloffBand {
        low_freq: params.source_low_freq.max(0.0),
        high_freq: if params.source_high_freq <= 0.0 {
            nyquist
        } else {
            params.source_high_freq
        },
        low_db_per_octave: params.source_low_rolloff_db_per_octave,
        high_db_per_octave: params.source_high_rolloff_db_per_octave,
    };

    let db_to_amp = DbToAmp::new();

    let begin_sample = (params.begin_secs.max(0.0) * r) as usize;
    let end_secs = if params.end_secs <= 0.0 {
        channels[0].len() as f32 / r
    } else {
        params.end_secs
    };
    let end_sample = ((end_secs * r) as usize).clamp(begin_sample, channels[0].len());
    let ring_extension = if params.add_ring_time { lh } else { 0 };
    let total_len = (end_sample - begin_sample) + ring_extension;

    let num_impulse_channels = ir.channels.len();
    let mut impulse_channel_now = params.impulse_channel.saturating_sub(1);

    let mut outputs = Vec::with_capacity(channels.len());

    for channel in channels {
        let mut impulse_spectrum = ir.channels[impulse_channel_now].clone();
        apply_bandpass_rolloff(&mut impulse_spectrum, n2, fundamental, &ir_band, &db_to_amp);

        let mut b = vec![0.0f32; lhm1];
        let mut output = Vec::with_capacity(total_len);
        let mut frame_begin = 0usize;

        while frame_begin < total_len {
            let samps_read = (total_len - frame_begin).min(lh);

            let mut inbuffer = vec![0.0f32; n];
            for (i, slot) in inbuffer.iter_mut().enumerate().take(samps_read) {
                let src_idx = begin_sample + frame_begin + i;
                *slot = if src_idx < end_sample {
                    channel[src_idx]
                } else {
                    0.0
                };
            }

            let mut input_save = vec![0.0f32; samps_read];
            for i in 0..samps_read {
                let t = (frame_begin + i) as f32 / r;
                let source_gain = db_to_amp.convert(params.source_gain_db.at(t, dur));
                input_save[i] = inbuffer[i] * source_gain;

                let input_gain = db_to_amp.convert(params.input_gain_db.at(t, dur));
                inbuffer[i] *= input_gain;
            }

            rfft(&mut inbuffer, n2, true);
            apply_bandpass_rolloff(&mut inbuffer, n2, fundamental, &source_band, &db_to_amp);

            let mut outbuffer = vec![0.0f32; n];
            match params.mode {
                Mode::Convolution => {
                    outbuffer[0] = inbuffer[0] * impulse_spectrum[0];
                    outbuffer[1] = inbuffer[1] * impulse_spectrum[1];
                    for k in 1..n2 {
                        let i = 2 * k;
                        let j = i + 1;
                        outbuffer[i] =
                            inbuffer[i] * impulse_spectrum[i] - inbuffer[j] * impulse_spectrum[j];
                        outbuffer[j] =
                            inbuffer[i] * impulse_spectrum[j] + inbuffer[j] * impulse_spectrum[i];
                    }
                }
                Mode::Deconvolution => {
                    outbuffer[0] = inbuffer[0] / impulse_spectrum[0];
                    outbuffer[1] = inbuffer[1] / impulse_spectrum[1];
                    for k in 1..n2 {
                        let i = 2 * k;
                        let j = i + 1;
                        let denom = impulse_spectrum[i] * impulse_spectrum[i]
                            + impulse_spectrum[j] * impulse_spectrum[j];
                        outbuffer[i] = (inbuffer[i] * impulse_spectrum[i]
                            + inbuffer[j] * impulse_spectrum[j])
                            / denom;
                        outbuffer[j] = (inbuffer[j] * impulse_spectrum[i]
                            - inbuffer[i] * impulse_spectrum[j])
                            / denom;
                    }
                }
            }

            rfft(&mut outbuffer, n2, false);

            for (out_sample, carry) in outbuffer[..lhm1].iter_mut().zip(&b) {
                *out_sample += carry;
            }
            b.copy_from_slice(&outbuffer[lh..lh + lhm1]);

            for i in 0..samps_read {
                let t = (frame_begin + i) as f32 / r;
                let out_gain = db_to_amp.convert(params.output_gain_db.at(t, dur));
                outbuffer[i] *= out_gain;
                outbuffer[i] += input_save[i];
            }

            output.extend_from_slice(&outbuffer[..samps_read]);
            frame_begin += samps_read;
        }

        // `bufferout()`'s shared output buffering (`legacy/pvc_lib/
        // fileio.c`) only ever writes complete `BLOCKSIZE`-sample chunks
        // to the per-channel temp file, keeping any trailing remainder in
        // a `static` scratch buffer until a later call tops it off. The
        // final "flush and close" call the C makes is `bufferout(
        // outbuffer, 0, 1)` - `I = 0` - and the function's outer loop is
        // `while (outbuffpt < I)`, which with `I = 0` never runs even
        // once, `flushflag` notwithstanding. So that trailing remainder
        // is never written at all: real `irconvolver` output is always
        // silently truncated to a multiple of `BLOCKSIZE`, confirmed
        // against a real compiled binary (a 3-second, 132300-sample
        // input produced exactly 132096 = 129*1024 samples of output,
        // not 132300). Reproduced exactly, not smoothed over - for an
        // input under one `BLOCKSIZE` long, this makes the real tool's
        // output *empty*.
        const BLOCKSIZE: usize = 1024;
        output.truncate((output.len() / BLOCKSIZE) * BLOCKSIZE);
        outputs.push(output);

        if params.impulse_channel == 0 {
            impulse_channel_now = (impulse_channel_now + 1) % num_impulse_channels;
        }
    }

    outputs
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params() -> IrconvolverParams {
        IrconvolverParams {
            begin_secs: 0.0,
            end_secs: 0.0,
            add_ring_time: false,
            impulse_channel: 0,
            mode: Mode::Convolution,
            ir_low_freq: 0.0,
            ir_high_freq: 0.0,
            ir_low_rolloff_db_per_octave: 0.0,
            ir_high_rolloff_db_per_octave: 0.0,
            source_low_freq: 0.0,
            source_high_freq: 0.0,
            source_low_rolloff_db_per_octave: 0.0,
            source_high_rolloff_db_per_octave: 0.0,
            source_gain_db: ControlFn::Const(-999.0), // silence the dry mix for these tests
            input_gain_db: ControlFn::Const(0.0),
            output_gain_db: ControlFn::Const(0.0),
        }
    }

    fn unit_impulse_spectrum(fft_size: usize, impulse_len: usize) -> ImpulseSpectra {
        let mut buf = vec![0.0f32; fft_size];
        buf[0] = 1.0;
        rfft(&mut buf, fft_size / 2, true);
        ImpulseSpectra {
            fft_size,
            impulse_len,
            channels: vec![buf],
        }
    }

    // Every test here uses inputs well over `BLOCKSIZE` (`1024`) samples -
    // short enough to run instantly, long enough that the real tool's
    // trailing-truncation quirk (see `process`'s doc comment) doesn't
    // degenerate the whole output to empty and defeat the test.

    #[test]
    fn convolving_with_a_unit_impulse_preserves_shape() {
        // `rfft`'s forward transform scales by `1/N` and its inverse by a
        // flat `2.0` (see `fft.rs`) - convolving through this exact pair
        // once each, with no extra compensation (matching the C, which
        // doesn't add any either), does not reproduce the input at unity
        // gain. What it must do is preserve *shape*: every sample scaled
        // by the same constant.
        let fft_size = 64;
        let impulse_len = 8;
        let ir = unit_impulse_spectrum(fft_size, impulse_len);
        let input: Vec<f32> = (0..3000).map(|i| (i as f32 * 0.1).sin()).collect();
        let out = process(
            std::slice::from_ref(&input),
            44100,
            &ir,
            &default_params(),
            input.len() as f32 / 44100.0,
        );
        assert_eq!(out[0].len(), 2048); // 3000 truncated down to a multiple of 1024.
        let k = out[0][5] / input[5];
        assert!(k.abs() > 1e-6, "degenerate scale factor {k}");
        for (a, b) in input.iter().zip(&out[0]) {
            assert!((a * k - b).abs() < 1e-3, "a={a} b={b} k={k}");
        }
    }

    #[test]
    fn silence_in_zero_out() {
        let ir = unit_impulse_spectrum(64, 8);
        let input = vec![0.0f32; 3000];
        let out = process(
            std::slice::from_ref(&input),
            44100,
            &ir,
            &default_params(),
            3000.0 / 44100.0,
        );
        assert_eq!(out[0].len(), 2048);
        assert!(out[0].iter().all(|&v| v == 0.0));
    }

    #[test]
    fn output_length_is_truncated_to_a_multiple_of_blocksize() {
        // Under one `BLOCKSIZE`: output is entirely empty.
        let ir = unit_impulse_spectrum(64, 8);
        let short_input = vec![1.0f32; 500];
        let out = process(
            std::slice::from_ref(&short_input),
            44100,
            &ir,
            &default_params(),
            500.0 / 44100.0,
        );
        assert_eq!(out[0].len(), 0);

        // Comfortably over one `BLOCKSIZE`: truncated down to the nearest
        // multiple, not rounded or padded.
        let input = vec![1.0f32; 2500];
        let out = process(
            std::slice::from_ref(&input),
            44100,
            &ir,
            &default_params(),
            2500.0 / 44100.0,
        );
        assert_eq!(out[0].len(), 2048);
    }

    #[test]
    fn ring_time_extends_output_past_a_blocksize_boundary() {
        // A round `impulse_len` of exactly one `BLOCKSIZE` makes the
        // effect of `-d` easy to state exactly: adding it pushes total
        // length from 2000 to 3024, crossing one more `BLOCKSIZE`
        // boundary (1024 -> 2048).
        let fft_size = 2048;
        let impulse_len = 1024;
        let ir = unit_impulse_spectrum(fft_size, impulse_len);
        let input = vec![1.0f32; 2000];

        let without_ring = process(
            std::slice::from_ref(&input),
            44100,
            &ir,
            &default_params(),
            2000.0 / 44100.0,
        );
        assert_eq!(without_ring[0].len(), 1024);

        let params = IrconvolverParams {
            add_ring_time: true,
            ..default_params()
        };
        let with_ring = process(
            std::slice::from_ref(&input),
            44100,
            &ir,
            &params,
            2000.0 / 44100.0,
        );
        assert_eq!(with_ring[0].len(), 2048);
    }

    #[test]
    fn multi_channel_impulse_round_robins_across_audio_channels() {
        let fft_size = 64;
        let n2 = fft_size / 2;
        let mut spec0 = vec![0.0f32; fft_size];
        spec0[0] = 1.0;
        rfft(&mut spec0, n2, true);
        let mut spec1 = vec![0.0f32; fft_size];
        spec1[0] = 2.0;
        rfft(&mut spec1, n2, true);
        let ir = ImpulseSpectra {
            fft_size,
            impulse_len: 8,
            channels: vec![spec0, spec1],
        };
        let ch = vec![1.0f32; 3000];
        let out = process(
            &[ch.clone(), ch.clone(), ch],
            44100,
            &ir,
            &default_params(),
            3000.0 / 44100.0,
        );
        // Channel 0 uses impulse 0, channel 1 uses impulse 1 (2x the
        // amplitude), channel 2 wraps back to impulse 0.
        assert!(out[0][0].abs() > 1e-6, "{}", out[0][0]);
        let ratio_1_to_0 = out[1][0] / out[0][0];
        assert!((ratio_1_to_0 - 2.0).abs() < 1e-3, "{ratio_1_to_0}");
        assert!(
            (out[2][0] - out[0][0]).abs() < 1e-6,
            "{} {}",
            out[2][0],
            out[0][0]
        );
    }

    #[test]
    fn deconvolution_undoes_convolution() {
        let fft_size = 64;
        let n2 = fft_size / 2;
        let mut spec = vec![0.0f32; fft_size];
        spec[0] = 2.0;
        rfft(&mut spec, n2, true);
        let ir = ImpulseSpectra {
            fft_size,
            impulse_len: 8,
            channels: vec![spec],
        };
        let input = vec![1.0f32; 3000];
        let convolved = process(
            std::slice::from_ref(&input),
            44100,
            &ir,
            &default_params(),
            3000.0 / 44100.0,
        );
        assert_eq!(convolved[0].len(), 2048);
        let params = IrconvolverParams {
            mode: Mode::Deconvolution,
            ..default_params()
        };
        let round_trip = process(&convolved, 44100, &ir, &params, 2048.0 / 44100.0);
        assert_eq!(round_trip[0].len(), 2048);
        // Default input/output gain is `0dB` via `ControlFn::Const(0.0)`,
        // and `DbToAmp::convert(0.0)` is `0.997791529`, not `1.0` (a
        // well-established, deliberately-reproduced quirk of the lookup
        // table this tool's `dB_to_amp()` calls use throughout this
        // project - see `units.rs`). Both the convolve and deconvolve
        // passes apply it twice each (input gain, output gain), so a
        // round trip compounds four non-unity multiplies:
        // `0.997791529^4 =~ 0.9913` - not a bug, an expected consequence
        // of the real tool's own default gain convention.
        for (a, b) in input.iter().zip(&round_trip[0]) {
            assert!((a - b).abs() < 0.01, "a={a} b={b}");
        }
    }
}
