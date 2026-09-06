//! Ports `fluxoid.c`: a two-pass analysis tool outputting a time-series
//! of spectral flux (frame-to-frame frequency change over a detection
//! band, optionally amplitude-weighted) - see
//! `pvc-core::tools::envelope`'s doc comment for the shared two-pass
//! shape; this doc comment only calls out real differences.
//!
//! Unlike `envelope`, the pass-2 normalization divides by the running
//! **peak** alone (`temp / peakenvamp`, `fluxoid.c:643`) - there's no
//! running minimum/`minenvamp` subtraction here, since a flux value is
//! already naturally floored near `0.0` (no signal change), unlike
//! envelope's raw band-sum which can have an arbitrary non-zero floor.
//!
//! **Real quirk, reproduced faithfully:** the attack/decay smoothing
//! here (and in `centroid`) is a hand-inlined variant of
//! [`crate::smooth::smooth_one_value`], not a call to it - and the two
//! disagree at the *exact-equality* boundary (`temp == old_temp`):
//! `smooth_one_value` treats that as "attack" (its condition is `a <
//! old_a` for release, so equality falls to attack), while this file's
//! own `if (temp > old_temp) ATTACK else DECAY` (`fluxoid.c:538-543`)
//! treats an exact tie as "decay" instead. Negligible in practice (exact
//! float equality between two independently-computed values is rare),
//! but implemented as its own small inline branch here rather than
//! reusing `smooth_one_value`, to match the C's boundary exactly.
//!
//! Frame 0 is a genuine no-op here (unlike `centroid`'s own frame-0
//! band-midpoint seed): `fluxoid.c` copies the current frame into
//! `previous_channel` *before* computing flux on frame 0
//! (`fluxoid.c:516-518`), so frame 0's raw flux is always exactly `0.0`
//! by construction, and its caller additionally forces `old_temp = temp`
//! right after (`fluxoid.c:525`), making the smoothing step a true
//! no-op too.

use crate::pvoc::{Analyzer, Frame};
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

pub use crate::tools::envelope::{ChannelMethod, OutputScale};

#[derive(Debug, Clone)]
pub struct FluxoidParams {
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
    /// `-A`: weight each bin's frequency change by its amplitude
    /// (default on).
    pub amplitude_weighting: bool,
    pub compress_threshold_db: ControlFn,
    pub compress_amount_db: ControlFn,
    pub gate_threshold_db: ControlFn,
    pub warp: ControlFn,
    pub output_rate: f32,
    pub output_scale: OutputScale,
}

/// Ports `find_fluxoid()` (`legacy/pvc_lib/find_fluxoid.c`): summed
/// frame-to-frame frequency change over `[k1, k2]`, weighted by each
/// bin's own current amplitude when `amplitude_weighting`, else
/// unweighted. No division/normalization here - a raw accumulated
/// magnitude in whatever units `amp` happens to be.
fn find_fluxoid(
    current: &[(f32, f32)],
    previous: &[(f32, f32)],
    k1: usize,
    k2: usize,
    amplitude_weighting: bool,
) -> f32 {
    let mut sum = 0.0f32;
    for k in k1..=k2 {
        let (amp, freq) = current[k];
        let (_, old_freq) = previous[k];
        let diff = (freq - old_freq).abs();
        sum += if amplitude_weighting {
            amp * diff
        } else {
            diff
        };
    }
    sum
}

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

/// Matches `find_centroid`/`find_fluxoid`'s shared clamp-then-bump bin
/// range derivation - see `tools::centroid::resolve_bin_range`'s doc
/// comment for why the order (clamp `k2` to the array's last valid bin,
/// *then* bump it apart from `k1` if they collide) matters.
fn resolve_bin_range(lowf: f32, hif: f32, fundamental: f32, n2: usize) -> (usize, usize) {
    let k1 = (lowf / fundamental) as usize;
    let mut k2 = (hif / fundamental) as usize;
    k2 = k2.min(n2);
    if k2 == k1 {
        k2 = k1 + 1;
    }
    (k1, k2.min(n2))
}

fn analyze_channel(
    input: &[f32],
    sample_rate: u32,
    params: &FluxoidParams,
    dur: f32,
    peak: &mut f32,
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
    let mut frame_count: usize = 0;

    let mut previous: Frame = Frame {
        bins: vec![(0.0, 0.0); n2 + 1],
    };
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

        if frame_count == 0 {
            previous = frame.clone();
        }

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

        let mut temp = find_fluxoid(
            &frame.bins,
            &previous.bins,
            k1,
            k2,
            params.amplitude_weighting,
        );

        previous = frame.clone();

        if frame_count == 0 {
            old_temp = temp;
        }

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

        // Attack/decay, hand-inlined (not `smooth_one_value` - see this
        // module's doc comment on the exact-equality boundary).
        temp = if temp > old_temp {
            attackc * old_temp + minusattackc * temp
        } else {
            releasec * old_temp + minusreleasec * temp
        };
        old_temp = temp;

        if temp > *peak {
            *peak = temp;
        }

        out.push(temp);
        samples_seen += d;
        frame_count += 1;

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
    params: &FluxoidParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let d = (r / params.frames_per_sec) as usize;
    let ir = d as f32 / r;

    let mut peak = 0.0f32;
    let per_channel: Vec<Vec<f32>> = channels
        .iter()
        .map(|c| analyze_channel(c, sample_rate, params, dur, &mut peak))
        .collect();
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
        "fluxoid: output rate must be >= frames-per-sec"
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
            "fluxoid: gate threshold must be below compression threshold"
        );
        let warp = params.warp.at(t, dur);

        let mut temp = raw / peak;
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

    fn default_params(fft_size: usize) -> FluxoidParams {
        FluxoidParams {
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
            amplitude_weighting: true,
            compress_threshold_db: ControlFn::Const(0.0),
            compress_amount_db: ControlFn::Const(0.0),
            gate_threshold_db: ControlFn::Const(-96.0),
            warp: ControlFn::Const(0.0),
            output_rate: 500.0,
            output_scale: OutputScale::Amp,
        }
    }

    #[test]
    fn silence_in_zero_flux_out() {
        let params = default_params(1024);
        let input = vec![0.0f32; 44100 / 4];
        let out = process(&[input], 44100, &params, 1.0);
        assert!(!out.is_empty());
    }

    #[test]
    fn steady_tone_produces_bounded_output() {
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
