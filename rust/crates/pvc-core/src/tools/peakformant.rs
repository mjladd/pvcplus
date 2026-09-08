//! Ports `peakformant.c`: confirmed byte-for-byte identical to
//! `centroid.c` (`diff legacy/pvc_src/{centroid,peakformant}.c` - every
//! difference is either cosmetic text (banner, usage, scratch-file
//! prefix) or the one real difference below), so this module reuses
//! `tools::centroid`'s whole two-pass pipeline (band-bound resolution,
//! per-channel attack/release smoothing, multi-channel average/peak
//! combination, warp, output-format conversion) and only replaces the
//! per-frame core analysis - see `tools::centroid`'s doc comment for
//! everything shared, including the provably-dead `-T`/`-S`/`-H` flags
//! (peakformant's `crack()` switch has no cases for them either,
//! confirmed the same way) and the frame-0 `old_temp` seeding
//! convention this port also follows unmodified for consistency with
//! its sibling (not independently re-verified against
//! `peakformant.c`'s own analogous multi-channel state-carry behavior -
//! see that module's own doc comment on the same point).
//!
//! **The one real difference**: `centroid.c` calls `find_centroid()`
//! (amplitude²-weighted mean frequency over the band); `peakformant.c`
//! calls `findFreqOfPeakFormant()` (`legacy/pvc_lib/
//! findFreqOfPeakFormant.c`) - simple peak-picking, returning the
//! frequency of whichever bin in the band has the highest amplitude, no
//! weighting or averaging at all. Confirmed by reading both library
//! functions side by side, not assumed from the tools' similar shape -
//! per this project's own established methodology (`groupdelaymaker`'s
//! usage-vs-actual-math mismatch is the canonical example of why).
//! `findFreqOfPeakFormant`'s bin-range derivation (`i1 = 1 + 2*(int)
//! (lowf/fundamental)`, clamped, bumped apart if equal) is the exact
//! same formula `find_centroid` uses, re-expressed in this crate's
//! `(amp, freq)`-pair terms by `tools::centroid::resolve_bin_range` -
//! reused unmodified here since the formula is genuinely identical, not
//! merely similar-looking.
//!
//! Unlike `find_centroid` (which falls back to `old_value` when every
//! bin in the band has zero amplitude), `findFreqOfPeakFormant` always
//! has a well-defined answer (it just returns the first bin's own
//! frequency if nothing beats it), so no such fallback is needed here.

use crate::pvoc::Analyzer;
use crate::tools::centroid::{resolve_band_bound, resolve_bin_range};
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

pub use crate::tools::centroid::OutputFormat;
pub use crate::tools::envelope::ChannelMethod;

#[derive(Debug, Clone)]
pub struct PeakformantParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub band_octave_pitchclass: bool,
    pub band_low: ControlFn,
    pub band_high: ControlFn,
    pub channel_method: ChannelMethod,
    pub attack: ControlFn,
    pub release: ControlFn,
    pub warp: ControlFn,
    pub output_rate: f32,
    pub output_format: OutputFormat,
    /// `-G`: reference pitch in octave.pitchclass notation, used only by
    /// `SemitonesDeviation`/`NegSemitonesDeviation`.
    pub reference_pitch: f32,
}

/// Ports `findFreqOfPeakFormant()`: the frequency of the loudest bin in
/// `[k1, k2]`. `k1`/`k2` must already be resolved by
/// [`resolve_bin_range`] (bounds-clamped, `k1==k2` bumped apart).
fn find_peak_formant(bins: &[(f32, f32)], k1: usize, k2: usize) -> f32 {
    let mut peak_amp = bins[k1].0;
    let mut freq = bins[k1].1;
    for &(amp, f) in &bins[k1 + 1..=k2] {
        if amp > peak_amp {
            peak_amp = amp;
            freq = f;
        }
    }
    freq
}

fn analyze_channel(
    input: &[f32],
    sample_rate: u32,
    params: &PeakformantParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
    let d = (r / params.frames_per_sec) as usize;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;
    let ir = d as f32 / r;
    let ar_db = 10.0f64.powf(-60.0 / 20.0);

    let mut nw = params.window_size;
    if nw == 0 {
        nw = 2 * n;
    }

    let window_pair = make_windows(params.window, nw, n, 0);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;
    let mut samples_seen: usize = 0;

    let mut old_temp = 0.0f32;
    let mut out = Vec::new();

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

        let frame = analyzer.push(&hop).expect("hop is exactly d samples");

        let t = samples_seen as f32 / r;
        let lowf = resolve_band_bound(
            params.band_low.at(t, dur),
            params.band_octave_pitchclass,
            true,
            nyquist,
        );
        let hif = resolve_band_bound(
            params.band_high.at(t, dur),
            params.band_octave_pitchclass,
            false,
            nyquist,
        );

        let (k1, k2) = resolve_bin_range(lowf, hif, fundamental, n2);

        let mut temp = find_peak_formant(&frame.bins, k1, k2);

        let release = params.release.at(t, dur);
        let (releasec, minusreleasec) = if release <= 0.0 {
            (0.0, 1.0)
        } else {
            let c = ar_db.powf(ir as f64 / release as f64) as f32;
            (c, 1.0 - c)
        };
        let attack = params.attack.at(t, dur);
        let (attackc, minusattackc) = if attack <= 0.0 {
            (0.0, 1.0)
        } else {
            let c = ar_db.powf(ir as f64 / attack as f64) as f32;
            (c, 1.0 - c)
        };

        temp = if temp > old_temp {
            attackc * old_temp + minusattackc * temp
        } else {
            releasec * old_temp + minusreleasec * temp
        };
        old_temp = temp;

        out.push(temp);
        samples_seen += d;

        if eof_after_this_hop {
            break;
        }
    }

    out
}

fn combine_channels(per_channel: &[Vec<f32>], method: ChannelMethod) -> Vec<f32> {
    let n = per_channel.iter().map(|c| c.len()).min().unwrap_or(0);
    (0..n)
        .map(|i| match method {
            ChannelMethod::Average => {
                per_channel.iter().map(|c| c[i]).sum::<f32>() / per_channel.len() as f32
            }
            ChannelMethod::Peak => per_channel.iter().map(|c| c[i]).fold(f32::MIN, f32::max),
        })
        .collect()
}

pub fn process(
    channels: &[Vec<f32>],
    sample_rate: u32,
    params: &PeakformantParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let nyquist = r / 2.0;
    let d = (r / params.frames_per_sec) as usize;
    let ir = d as f32 / r;

    let per_channel: Vec<Vec<f32>> = channels
        .iter()
        .map(|c| analyze_channel(c, sample_rate, params, dur))
        .collect();
    let mut old_temp = per_channel
        .last()
        .and_then(|c| c.last().copied())
        .unwrap_or(0.0);

    let combined = combine_channels(&per_channel, params.channel_method);

    let mid_c = (220.0 * 2.0f64.powf(3.0 / 12.0)) as f32;
    let log_of_2 = 2.0f32.log10();
    let ref_int = params.reference_pitch.trunc();
    let ref_octave = ref_int + (params.reference_pitch - ref_int) / 0.12;

    let mut tpinc = params.output_rate;
    if tpinc < params.frames_per_sec {
        tpinc = 1.0;
    } else {
        tpinc = 1.0 / (tpinc / params.frames_per_sec);
    }
    assert!(
        tpinc <= 1.0,
        "peakformant: output rate must be >= frames-per-sec"
    );

    let mut tp = 0.0f32;
    let mut out = Vec::new();
    for &raw in combined.iter() {
        let t = out.len() as f32 * ir;
        let lowf = resolve_band_bound(
            params.band_low.at(t, dur),
            params.band_octave_pitchclass,
            true,
            nyquist,
        );
        let hif = resolve_band_bound(
            params.band_high.at(t, dur),
            params.band_octave_pitchclass,
            false,
            nyquist,
        );
        let diff = hif - lowf;
        let warp = params.warp.at(t, dur);

        let temp = curve(lowf, hif, (raw - lowf) / diff, warp);

        while tp < 1.0 {
            let mut value = curve(old_temp, temp, tp, 0.0);
            value = match params.output_format {
                OutputFormat::Freq => value,
                OutputFormat::Octave => 8.0 + (value / mid_c).log10() / log_of_2,
                OutputFormat::OctavePitchclass => 8.0 + 0.12 * ((value / mid_c).log10() / log_of_2),
                OutputFormat::SemitonesDeviation => {
                    let octave = 8.0 + (value / mid_c).log10() / log_of_2;
                    12.0 * (octave - ref_octave)
                }
                OutputFormat::NegSemitonesDeviation => {
                    let octave = 8.0 + (value / mid_c).log10() / log_of_2;
                    -12.0 * (octave - ref_octave)
                }
            };
            out.push(value);
            tp += tpinc;
        }
        old_temp = temp;
        tp -= tp.floor();
    }

    out
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params(fft_size: usize) -> PeakformantParams {
        PeakformantParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            band_octave_pitchclass: false,
            band_low: ControlFn::Const(0.0),
            band_high: ControlFn::Const(-1.0),
            channel_method: ChannelMethod::Average,
            attack: ControlFn::Const(0.0),
            release: ControlFn::Const(0.0),
            warp: ControlFn::Const(0.0),
            output_rate: 500.0,
            output_format: OutputFormat::Freq,
            reference_pitch: 8.0,
        }
    }

    #[test]
    fn sine_input_peak_formant_near_440hz() {
        let params = default_params(1024);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let out = process(&[input], sample_rate, &params, 1.0);
        assert!(!out.is_empty());
        let mid = out[out.len() / 2];
        let fundamental = sample_rate as f32 / 1024.0;
        assert!(
            (mid - 440.0).abs() < fundamental,
            "peak formant {mid} not near 440Hz"
        );
    }

    #[test]
    fn two_tone_input_picks_the_louder_bin() {
        // A loud 440Hz tone plus a much quieter 1000Hz tone: the peak
        // formant should track the loud tone, unlike a centroid (which
        // would land somewhere between the two).
        let params = default_params(1024);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                let t = i as f32 / sample_rate as f32;
                0.9 * (2.0 * std::f32::consts::PI * 440.0 * t).sin()
                    + 0.05 * (2.0 * std::f32::consts::PI * 1000.0 * t).sin()
            })
            .collect();
        let out = process(&[input], sample_rate, &params, 1.0);
        let mid = out[out.len() / 2];
        let fundamental = sample_rate as f32 / 1024.0;
        assert!(
            (mid - 440.0).abs() < fundamental,
            "peak formant {mid} should track the louder 440Hz tone"
        );
    }
}
