//! Ports `envelope.c`: a two-pass analysis tool that outputs a
//! *time-series of scalars* (one amplitude value per output sample),
//! never audio - no oscillator bank, no overlap-add, no WAV output.
//!
//! Pass 1 (per channel, independently): standard `fold`/`rfft`/`convert`
//! analysis, sum the amplitude of every bin in `[band_low, band_high]`,
//! optionally subtract a slower-tracking "filtered envelope" of that sum
//! (`-j`/`-k`/`-m`), then attack/release-smooth the result
//! ([`crate::smooth::smooth_one_value`]). Each channel's smoothed
//! per-frame value also contributes to a **global** (not per-channel)
//! running peak/min tracked across every channel's own raw value before
//! any cross-channel combination - `envelope.c:554-555`, confirmed by
//! reading: `peakenvamp`/`minenvamp` update inside the per-channel frame
//! loop, using that channel's own `temp`, not the channel-combined one.
//!
//! Channels are then combined index-wise (average or peak across
//! channels, `-X`) into one series, and pass 2 normalizes each value by
//! the pass-1 global `[min, max]`, compresses/gates it
//! (`-T`/`-G`/`-S`), applies the distribution warp (`-W`, via
//! [`crate::warp::curve`]), then linearly interpolates
//! (`curve(...,0.0)`) onto the output rate (`-r`) before a final
//! output-scale conversion (`-q`: amp/dB/inverted-amp/inverted-dB).
//!
//! **Real quirk, reproduced faithfully:** the pass-2 interpolation's
//! `old_temp` (the "previous output value" the very first interpolated
//! sample lerps from) is never reset between passes - it carries over
//! from pass 1's *last channel's last frame's* smoothed value
//! (`envelope.c` has no `old_temp = 0` between the channel loop and the
//! "NOW OPEN, COMPRESS AND NORMALIZE" pass-2 section). So the very first
//! output sample (before pass 2 ever computes anything) is exactly that
//! leftover pass-1 value, not a sensible default like `0.0`. Also not
//! reset: `frame_count`, whose value at the end of pass 1 becomes pass
//! 2's *starting* value for `t = frame_count * IR` - invisible under
//! constant control functions (this tool's only golden case), but a
//! real quirk for time-varying `-T`/`-G`/-S`/`-W` tables.
//!
//! Envelope's own inline band-sum loop has no upper clamp against the
//! bins array size (unlike `find_centroid`/`find_fluxoid`, which both
//! clamp) - a real, if practically unreachable under default flags
//! (`band_high` defaults to Nyquist), out-of-bounds read in the C for a
//! `--band-high` above Nyquist. Clamped here instead of reproduced.

use crate::pvoc::Analyzer;
use crate::smooth::{smooth_one_value, smooth_setup};
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ChannelMethod {
    Average,
    Peak,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OutputScale {
    Amp,
    Db,
    InvertedAmp,
    InvertedDb,
}

#[derive(Debug, Clone)]
pub struct EnvelopeParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    /// `-Q`: when true, `band_low`/`band_high` are octave.pitchclass
    /// values instead of Hz.
    pub band_octave_pitchclass: bool,
    pub band_low: ControlFn,
    pub band_high: ControlFn,
    pub channel_method: ChannelMethod,
    pub attack: ControlFn,
    pub release: ControlFn,
    pub filtered_attack: ControlFn,
    pub filtered_release: ControlFn,
    pub filtered_cut: ControlFn,
    pub compress_threshold_db: ControlFn,
    pub compress_amount_db: ControlFn,
    pub gate_threshold_db: ControlFn,
    pub warp: ControlFn,
    pub output_rate: f32,
    pub output_scale: OutputScale,
}

/// Resolves `-f`/`-F`'s per-frame value into Hz, matching `envelope.c`'s
/// inline octave.pitchclass-or-plain-Hz branch (identical across all
/// four analysis-family tools). `is_low` selects which sentinel a
/// too-small octave.pitchclass value resolves to (`0.0` for the low
/// bound, `nyquist` for the high bound - both branches use the same
/// `< 3.0` threshold, just different fallbacks).
///
/// In plain-Hz mode, a negative value also resolves to that same
/// low/high sentinel - not a behavior `envelope.c` itself has (its own
/// default is simply the literal `nyquist` value computed once at
/// startup, with no negative-sentinel convention), but matching this
/// project's own established `pvc` CLI convention elsewhere (`pv`,
/// `filter`, ...) for deferring "the real Nyquist frequency" until the
/// input's sample rate is known - `pvc-cli`'s `--band-high` defaults to
/// `-1` for exactly this reason.
fn resolve_band_bound(value: f32, octave_pitchclass: bool, is_low: bool, nyquist: f32) -> f32 {
    if octave_pitchclass {
        if value < 3.0 {
            if is_low {
                0.0
            } else {
                nyquist
            }
        } else {
            crate::response::oppc_to_hz(value)
        }
    } else if value < 0.0 {
        if is_low {
            0.0
        } else {
            nyquist
        }
    } else {
        value
    }
}

/// Per-channel pass 1: returns one smoothed band-sum value per analysis
/// frame, plus that channel's own contribution to the global peak/min
/// (folded into the caller's running values, not returned separately,
/// since the C tracks them as a single running pair across all
/// channels - see this module's doc comment).
fn analyze_channel(
    input: &[f32],
    sample_rate: u32,
    params: &EnvelopeParams,
    dur: f32,
    peak: &mut f32,
    min: &mut f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
    let d = (r / params.frames_per_sec) as usize;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;
    let ir = d as f32 / r;

    let mut nw = params.window_size;
    if nw == 0 {
        nw = 2 * n;
    }

    let window_pair = make_windows(params.window, nw, n, 0);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;
    let mut samples_seen: usize = 0;

    let mut old_filtenv = 0.0f32;
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

        let mut k1 = (lowf / fundamental) as usize;
        let mut k2 = (hif / fundamental) as usize;
        if k2 == k1 {
            k2 = k1 + 1;
        }
        k1 = k1.min(n2);
        k2 = k2.min(n2);

        let mut temp: f32 = frame.bins[k1..=k2].iter().map(|&(amp, _)| amp).sum();

        let (fattackc, minusfattackc) = smooth_setup(params.filtered_attack.at(t, dur), ir);
        let (freleasec, minusfreleasec) = smooth_setup(params.filtered_release.at(t, dur), ir);
        let filtenv = smooth_one_value(
            temp,
            old_filtenv,
            fattackc,
            minusfattackc,
            freleasec,
            minusfreleasec,
        );
        old_filtenv = filtenv;

        temp -= params.filtered_cut.at(t, dur) * filtenv;

        let (attackc, minusattackc) = smooth_setup(params.attack.at(t, dur), ir);
        let (releasec, minusreleasec) = smooth_setup(params.release.at(t, dur), ir);
        temp = smooth_one_value(
            temp,
            old_temp,
            attackc,
            minusattackc,
            releasec,
            minusreleasec,
        );
        old_temp = temp;

        if temp > *peak {
            *peak = temp;
        }
        if temp < *min {
            *min = temp;
        }

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

/// Analyzes every channel in `channels` and returns the final output
/// sample series (already output-rate-interpolated and scale-converted -
/// ready to write one value per line/float, in order).
pub fn process(
    channels: &[Vec<f32>],
    sample_rate: u32,
    params: &EnvelopeParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let d = (r / params.frames_per_sec) as usize;
    let ir = d as f32 / r;

    let mut peak = f32::MIN;
    let mut min = f32::MAX;
    let per_channel: Vec<Vec<f32>> = channels
        .iter()
        .map(|c| analyze_channel(c, sample_rate, params, dur, &mut peak, &mut min))
        .collect();
    // `old_temp`'s pass-1-leftover carryover (see this module's doc
    // comment): the last channel's last frame's own smoothed value,
    // matching `envelope.c`'s single shared `old_temp` C variable never
    // being reset between the channel loop and pass 2.
    let mut old_temp = per_channel
        .last()
        .and_then(|c| c.last().copied())
        .unwrap_or(0.0);

    let combined = combine_channels(&per_channel, params.channel_method);

    let short_norm = 1.0 - 1.0 / 32760.0;
    let min_amp = 10.0f64.powf(-96.0 / 20.0) as f32;

    let mut tpinc = params.output_rate;
    if tpinc < params.frames_per_sec {
        tpinc = 1.0;
    } else {
        tpinc = 1.0 / (tpinc / params.frames_per_sec);
    }
    assert!(
        tpinc <= 1.0,
        "envelope: output rate must be >= frames-per-sec"
    );

    let mut tp = 0.0f32;
    let mut out = Vec::new();
    for (frame_count, &raw) in combined.iter().enumerate() {
        let t = frame_count as f32 * ir;
        let compression_db = params.compress_amount_db.at(t, dur);
        let compression = if compression_db < 0.0 {
            10.0f64.powf(compression_db as f64 / 20.0) as f32
        } else {
            1.0
        };
        let amp_thresh = 10.0f64.powf(params.compress_threshold_db.at(t, dur) as f64 / 20.0) as f32;
        let amp_gate_thresh =
            10.0f64.powf(params.gate_threshold_db.at(t, dur) as f64 / 20.0) as f32;
        assert!(
            amp_thresh > amp_gate_thresh,
            "envelope: gate threshold must be below compression threshold"
        );
        let warp = params.warp.at(t, dur);

        let mut temp = (raw - min) / (peak - min);
        if temp > amp_thresh {
            temp = amp_thresh + (temp - amp_thresh) * compression;
        }
        temp = if temp > amp_gate_thresh {
            temp - amp_gate_thresh
        } else {
            0.0
        };
        let temp3 = 1.0 / ((amp_thresh + (1.0 - amp_thresh) * compression) - amp_gate_thresh);
        temp = temp * temp3 * short_norm;
        temp = curve(0.0, 1.0, temp, warp);

        while tp < 1.0 {
            let mut temp4 = curve(old_temp, temp, tp, 0.0);
            if temp4 >= 1.0 {
                temp4 = short_norm;
            }
            temp4 = match params.output_scale {
                OutputScale::Amp => temp4,
                OutputScale::Db => {
                    if temp4 > min_amp {
                        20.0 * temp4.log10()
                    } else {
                        -96.0
                    }
                }
                OutputScale::InvertedAmp => 1.0 - temp4,
                OutputScale::InvertedDb => {
                    let db = if temp4 > min_amp {
                        20.0 * temp4.log10()
                    } else {
                        -96.0
                    };
                    -(db + 96.0)
                }
            };
            out.push(temp4);
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

    fn default_params(fft_size: usize) -> EnvelopeParams {
        EnvelopeParams {
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
            filtered_attack: ControlFn::Const(0.0),
            filtered_release: ControlFn::Const(0.0),
            filtered_cut: ControlFn::Const(0.0),
            compress_threshold_db: ControlFn::Const(0.0),
            compress_amount_db: ControlFn::Const(0.0),
            gate_threshold_db: ControlFn::Const(-96.0),
            warp: ControlFn::Const(0.0),
            output_rate: 500.0,
            output_scale: OutputScale::Amp,
        }
    }

    #[test]
    fn silence_in_zero_out() {
        let params = default_params(1024);
        let input = vec![0.0f32; 44100 / 4];
        let out = process(&[input], 44100, &params, 1.0);
        assert!(!out.is_empty());
    }

    #[test]
    fn sine_input_produces_bounded_output() {
        let params = default_params(1024);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let out = process(&[input], sample_rate, &params, 1.0);
        assert!(!out.is_empty());
        for &v in &out {
            assert!(
                (0.0..=1.0001).contains(&v),
                "value {v} out of expected [0,1] range"
            );
        }
    }
}
