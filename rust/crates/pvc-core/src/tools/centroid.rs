//! Ports `centroid.c`: a two-pass analysis tool outputting a time-series
//! of the amplitude²-weighted mean frequency ("spectral centroid") over
//! a detection band - see `pvc-core::tools::envelope`'s doc comment for
//! the shared two-pass shape (this tool included), which this doc
//! comment only calls out real *differences* from.
//!
//! **Real bug, confirmed by grepping every reference: `-T`/`-S`
//! (compress-threshold/gate-threshold) have no `case` in `centroid.c`'s
//! `crack()` switch at all** - unlike `envelope`/`fluxoid`, centroid has
//! **no compress/gate/output-scale stage whatsoever**; its pass 2 only
//! warps the raw centroid frequency (`-W`, via
//! [`crate::warp::curve`]) onto `[band_low, band_high]` and converts it
//! to the requested output format. Not exposed as CLI flags at all,
//! matching this project's established treatment of provably-dead
//! legacy flags (`pvc compand`'s dead `-L`, `pvc harmonize`'s dead
//! `-J`).
//!
//! **Real bug, also dead: `-H`/`--warp`** (a *second*, different warp
//! from `-W`'s distribution warp) is parsed and printed in the startup
//! banner, but its only call site (`spectmagwarp(channel, N+2,
//! swarpshape.A[0], 0)`) is commented out in the source - confirmed by
//! reading `centroid.c` directly. Not exposed here either.
//!
//! **`-G`/`--reference-pitch` does not collide with anything internally**
//! despite `-G` meaning "compression amount" in most other tools in this
//! family - `centroid.c`'s `crack()` has exactly one `case 'G'`
//! (`refpitch = crackfloat(...)`, a plain float, not a control
//! function), used only by the `semitones-deviation`/
//! `neg-semitones-deviation` output formats. The doc's warning about a
//! "collision" is a cross-tool naming coincidence, not an internal
//! ambiguity.
//!
//! **Frame-0 asymmetry vs. `fluxoid`** (a sibling that looks similar):
//! `find_centroid` seeds its `old_value` state to the *band midpoint*
//! `(hif+lowf)/2` on the very first frame, and `centroid.c` never
//! overrides that afterward - so frame 0's attack/release smoothing
//! compares the real computed centroid against the band midpoint, not
//! against itself (unlike `fluxoid`, whose caller forces `old_temp =
//! temp` on frame 0, making its own smoothing step a true no-op there).

use crate::pvoc::Analyzer;
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

pub use crate::tools::envelope::ChannelMethod;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OutputFormat {
    Freq,
    Octave,
    OctavePitchclass,
    SemitonesDeviation,
    NegSemitonesDeviation,
}

#[derive(Debug, Clone)]
pub struct CentroidParams {
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

/// Ports `find_centroid()` (`legacy/pvc_lib/find_centroid.c`):
/// power(amplitude²)-weighted mean frequency over `[k1, k2]`, clamped to
/// `[lowf, hif]`. Falls back to `old_value` if every bin in the band has
/// zero amplitude (`sum == 0`). `k1`/`k2` must already be resolved by
/// [`resolve_bin_range`] (bounds-clamped, `k1==k2` bumped apart) -
/// matching the C's own clamp-then-bump order exactly, see that
/// function's doc comment.
fn find_centroid(
    bins: &[(f32, f32)],
    k1: usize,
    k2: usize,
    lowf: f32,
    hif: f32,
    old_value: f32,
) -> f32 {
    let mut sum = 0.0f32;
    let mut weighted = 0.0f32;
    for &(amp, freq) in &bins[k1..=k2] {
        let amp2 = amp * amp;
        sum += amp2;
        weighted += freq * amp2;
    }
    let value = if sum > 0.0 { weighted / sum } else { old_value };
    value.clamp(lowf, hif)
}

/// Ports the shared `i1`/`i2` bin-range derivation used by
/// `find_centroid`/`find_fluxoid` (`legacy/pvc_lib/find_centroid.c:26-32`,
/// `find_fluxoid.c`'s equivalent): unlike `envelope.c`'s own *inline*
/// band-sum loop (which clamps nothing, a real unguarded OOB read for a
/// user-supplied `--band-high` past Nyquist - see `tools::envelope`'s
/// doc comment), these library functions clamp `k2` to the array's last
/// valid bin **before** checking `k2 == k1` and bumping it apart - not
/// the reverse order. That order matters: a `k1`/`k2` that both land
/// exactly on the clamped max bin bumps `k2` one *past* it
/// (`k2 = k1 + 1`), a real one-past-the-end read in the C for that
/// narrow edge case (`lowf`'s own bin exactly equal to the clamped max)
/// that this port re-clamps a second time afterward instead of
/// reproducing, to stay panic-safe on a Rust slice index.
fn resolve_bin_range(lowf: f32, hif: f32, fundamental: f32, n2: usize) -> (usize, usize) {
    let k1 = (lowf / fundamental) as usize;
    let mut k2 = (hif / fundamental) as usize;
    k2 = k2.min(n2);
    if k2 == k1 {
        k2 = k1 + 1;
    }
    (k1, k2.min(n2))
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

fn analyze_channel(input: &[f32], sample_rate: u32, params: &CentroidParams, dur: f32) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
    let d = (r / params.frames_per_sec) as usize;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;
    let ir = d as f32 / r;
    // `ar_dB = 10^(-60/20)`: same recipe as `smooth_setup`, but
    // centroid/fluxoid inline it and branch attack-vs-decay based on
    // `temp > old_temp` directly, rather than calling
    // `smooth_setup`/`smooth_one_value` - see this module's doc comment
    // on the frame-0 midpoint seed for why a bespoke loop (not
    // `smooth_one_value`) is used here.
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

        if frame_count == 0 {
            old_temp = (lowf + hif) * 0.5;
        }

        let mut temp = find_centroid(&frame.bins, k1, k2, lowf, hif, old_temp);

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
    params: &CentroidParams,
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
        "centroid: output rate must be >= frames-per-sec"
    );

    let mut tp = 0.0f32;
    let mut out = Vec::new();
    for (frame_count, &raw) in combined.iter().enumerate() {
        let t = frame_count as f32 * ir;
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

    fn default_params(fft_size: usize) -> CentroidParams {
        CentroidParams {
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
    fn sine_input_centroid_near_440hz() {
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
        assert!((mid - 440.0).abs() < 50.0, "centroid {mid} not near 440Hz");
    }
}
