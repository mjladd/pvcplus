//! Ports `impulseresponse.c`: analyzes a `[begin_secs, end_secs)` window of
//! each input channel into a zero-padded, peak-normalized rfft-format
//! spectrum. The four values (channel count, FFT size, impulse length,
//! sample rate) plus the per-channel spectra are exactly the fields
//! `pvc_io::ir::IrData` persists - this module computes them, `pvc-cli`'s
//! `impulseresponse` command writes the file (`pvc-core` has no I/O
//! dependency).
//!
//! Not ported: `-C` (`channelout`), a "shift which channel range gets
//! processed" knob left over from the C's own per-channel-file-loop
//! architecture (`beginchan = channelout - 1`, `endchan = beginchan +
//! ochan`, where `ochan` is the *output* channel count from `setupfiles`).
//! This port always processes every channel actually present in the input
//! `AudioBuffer`, matching every other multichannel tool ported this
//! session (`compander`/`noisefilter`/`filter`/... never expose a
//! channel-subset flag either). Also not ported: `-a` (decibels spectrum
//! plot file) and `-P` (frequency-response printout) - both are
//! stderr/display-only debug dumps that never feed back into the
//! persisted `.ir` file, matching this project's established practice of
//! not porting debug/display-only output paths.
//!
//! FFT size `fft_size` is the smallest power of two `>= 2*impulse_len - 1`
//! (signed arithmetic, matching the C's `int L = 2*Lh - 1` exactly,
//! including its behavior for a degenerate zero-length window) - the
//! classic "zero-pad to at least twice your own length minus one" rule for
//! linear (not circular) FFT convolution, sized only against this
//! impulse's own length (the other operand's length, supplied later by
//! `irconvolver`, isn't known yet). A deliberate simplification from the
//! C: `impulseresponse.c` computes `Lh` twice - once arithmetically
//! (`dur * R`) to size the FFT *before* reading any samples, then again as
//! the real sample count read back from a per-channel temp file for the
//! header's actual `Lh` field - and these can differ by a sample under
//! fractional-second `-b`/`-e` values due to independent rounding in two
//! different places. This port uses one resolved sample count
//! (`end_sample - begin_sample`) for both, which matches the C for any
//! `-b`/`-e` that lands on a whole number of samples (this tool's own
//! golden case does) but doesn't chase the rare rounding mismatch.
//!
//! `convert()`'s frequency computation (used in the C only to find each
//! channel's peak magnitude for normalization, via `findPeakAmp`, which
//! only ever reads the *magnitude* slots) is skipped entirely here -
//! [`peak_magnitude`] computes just the values `findPeakAmp` actually
//! uses, directly from the raw rfft-format spectrum, rather than routing
//! through a full magnitude/frequency conversion whose frequency half is
//! provably dead for this tool's purposes (confirmed by reading
//! `findPeakAmp`'s only two lines).
//!
//! Normalization (`-N`, default "together"; `-d`, default `0` dB) uses
//! [`crate::units::DbToAmp`] (the lookup-table convention, *not* exact
//! `pow`) - confirmed by reading: `impulseresponse.c` calls the shared
//! `dB_to_amp()` library function directly, same convention as
//! `filter.c`. A real, faithfully-reproduced footgun: the C's
//! normalization loop gates *both* modes on the *global* peak amplitude
//! being nonzero (`if(peakAmp!=0.)`), but the actual per-channel divisor
//! for [`Normalization::Independent`] is that channel's *own* peak - so a
//! silent channel sitting alongside a non-silent one under independent
//! normalization divides by exactly `0.0`, producing `+inf`/`NaN` for
//! that channel's entire spectrum. Not special-cased here; plain `f32`
//! division reproduces the same IEEE 754 behavior the C's `double`
//! division does.

use crate::fft::rfft;
use crate::units::DbToAmp;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Normalization {
    Off,
    Independent,
    Together,
}

#[derive(Debug, Clone, Copy)]
pub struct ImpulseResponseParams {
    /// `-b`: analysis window start, in seconds.
    pub begin_secs: f32,
    /// `-e`: analysis window end, in seconds. `<= 0.0` means "whole file"
    /// (`impulseresponse.c`'s own `setupfiles()`-inherited `if (endt <=
    /// 0.) endt = idur;` default).
    pub end_secs: f32,
    /// `-N`: default `Together`.
    pub normalization: Normalization,
    /// `-d`: normalization target level, in dB. Default `0.0`.
    pub normalization_db: f32,
}

pub struct ImpulseResponseOutput {
    pub fft_size: usize,
    pub impulse_len: usize,
    pub sample_rate: u32,
    /// `channels[ch]` is that channel's `fft_size` rfft-format floats,
    /// zero-padded and normalized.
    pub channels: Vec<Vec<f32>>,
}

/// `findPeakAmp()`: the largest magnitude among an rfft-format spectrum's
/// `n2 + 1` bins - bin `0` (DC) and bin `n2` (Nyquist) are each packed as
/// a single real value with no imaginary counterpart, every other bin is
/// a `(re, im)` pair.
fn peak_magnitude(spectrum: &[f32], n2: usize) -> f32 {
    let mut peak = f32::MIN;
    for i in 0..=n2 {
        let mag = if i == 0 {
            spectrum[0].abs()
        } else if i == n2 {
            spectrum[1].abs()
        } else {
            spectrum[2 * i].hypot(spectrum[2 * i + 1])
        };
        if mag > peak {
            peak = mag;
        }
    }
    peak
}

pub fn process(
    channels: &[Vec<f32>],
    sample_rate: u32,
    params: &ImpulseResponseParams,
) -> ImpulseResponseOutput {
    let r = sample_rate as f32;
    let begin_sample = (params.begin_secs.max(0.0) * r) as usize;
    let end_secs = if params.end_secs <= 0.0 {
        channels[0].len() as f32 / r
    } else {
        params.end_secs
    };
    let end_sample = ((end_secs * r) as usize).clamp(begin_sample, channels[0].len());
    let impulse_len = end_sample - begin_sample;

    // `int L = 2*Lh - 1; for(N=1; N<L; N<<=1);` - signed arithmetic,
    // matching the C exactly including a degenerate `impulse_len == 0`
    // window (`L` goes negative, the loop never runs, `N` stays `1`).
    let l = 2 * impulse_len as i64 - 1;
    let mut fft_size: i64 = 1;
    while fft_size < l {
        fft_size <<= 1;
    }
    let fft_size = fft_size as usize;
    let n2 = fft_size / 2;

    let mut spectra = Vec::with_capacity(channels.len());
    let mut peak_per_channel = Vec::with_capacity(channels.len());
    for channel in channels {
        let mut buffer = vec![0.0f32; fft_size];
        let slice = &channel[begin_sample..end_sample];
        buffer[..slice.len()].copy_from_slice(slice);
        rfft(&mut buffer, n2, true);
        peak_per_channel.push(peak_magnitude(&buffer, n2));
        spectra.push(buffer);
    }

    let peak_amp = peak_per_channel
        .iter()
        .copied()
        .fold(peak_per_channel[0], f32::max);

    if params.normalization != Normalization::Off && peak_amp != 0.0 {
        let db_to_amp = DbToAmp::new();
        let target = db_to_amp.convert(params.normalization_db);
        for (i, buffer) in spectra.iter_mut().enumerate() {
            let scale = match params.normalization {
                Normalization::Together => target / peak_amp,
                Normalization::Independent => target / peak_per_channel[i],
                Normalization::Off => unreachable!(),
            };
            for v in buffer.iter_mut() {
                *v *= scale;
            }
        }
    }

    ImpulseResponseOutput {
        fft_size,
        impulse_len,
        sample_rate,
        channels: spectra,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params() -> ImpulseResponseParams {
        ImpulseResponseParams {
            begin_secs: 0.0,
            end_secs: 0.0,
            normalization: Normalization::Together,
            normalization_db: 0.0,
        }
    }

    fn impulse(len: usize) -> Vec<f32> {
        let mut v = vec![0.0f32; len];
        v[0] = 1.0;
        v
    }

    #[test]
    fn silence_in_zero_spectrum_out() {
        let channels = vec![vec![0.0f32; 512]];
        let out = process(&channels, 44100, &default_params());
        assert!(out.channels[0].iter().all(|&v| v == 0.0));
    }

    #[test]
    fn fft_size_is_next_power_of_two_past_twice_the_impulse_length() {
        let channels = vec![impulse(300)];
        let out = process(&channels, 44100, &default_params());
        assert_eq!(out.impulse_len, 300);
        // L = 2*300 - 1 = 599, next power of two >= 599 is 1024.
        assert_eq!(out.fft_size, 1024);
    }

    #[test]
    fn together_normalization_brings_peak_to_target_level() {
        let channels = vec![impulse(64), {
            let mut v = impulse(64);
            v[0] = 0.25;
            v
        }];
        let params = default_params();
        let out = process(&channels, 44100, &params);
        let n2 = out.fft_size / 2;
        let peak0 = peak_magnitude(&out.channels[0], n2);
        let peak1 = peak_magnitude(&out.channels[1], n2);
        // The louder channel's peak should land at 0dB (amp ~= DbToAmp(0)).
        let db_to_amp = DbToAmp::new();
        let target = db_to_amp.convert(0.0);
        assert!(
            (peak0 - target).abs() < 1e-3,
            "peak0={peak0}, target={target}"
        );
        // The quieter channel is scaled by the *same* factor, so its peak
        // should be exactly a quarter of the target.
        assert!((peak1 - target / 4.0).abs() < 1e-3, "peak1={peak1}");
    }

    #[test]
    fn independent_normalization_brings_every_channel_to_target_level() {
        let channels = vec![impulse(64), {
            let mut v = impulse(64);
            v[0] = 0.25;
            v
        }];
        let params = ImpulseResponseParams {
            normalization: Normalization::Independent,
            ..default_params()
        };
        let out = process(&channels, 44100, &params);
        let n2 = out.fft_size / 2;
        let db_to_amp = DbToAmp::new();
        let target = db_to_amp.convert(0.0);
        for ch in &out.channels {
            let peak = peak_magnitude(ch, n2);
            assert!((peak - target).abs() < 1e-3, "peak={peak}, target={target}");
        }
    }

    #[test]
    fn off_normalization_leaves_raw_spectrum_unscaled() {
        let channels = vec![impulse(64)];
        let params = ImpulseResponseParams {
            normalization: Normalization::Off,
            ..default_params()
        };
        let out = process(&channels, 44100, &params);
        let n2 = out.fft_size / 2;
        let peak = peak_magnitude(&out.channels[0], n2);
        // An unscaled unit impulse has a flat spectrum of magnitude
        // `1/fft_size` at every bin (`rfft`'s forward transform divides by
        // `N`) - much smaller than the normalized target.
        let want = 1.0 / out.fft_size as f32;
        assert!((peak - want).abs() < 1e-6, "peak={peak}, want={want}");
    }

    #[test]
    fn independent_normalization_produces_non_finite_output_for_a_silent_channel_alongside_a_loud_one(
    ) {
        // Faithfully-reproduced C footgun: the global-peak gate passes
        // (channel 0 is loud), but channel 1's own peak is exactly zero,
        // so its divisor is zero too.
        let channels = vec![impulse(64), vec![0.0f32; 64]];
        let params = ImpulseResponseParams {
            normalization: Normalization::Independent,
            ..default_params()
        };
        let out = process(&channels, 44100, &params);
        assert!(out.channels[1].iter().all(|v| !v.is_finite() || *v == 0.0));
    }
}
