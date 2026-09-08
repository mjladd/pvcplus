//! Ports `specflattracker.c`: like `tools::peakformant`, confirmed by
//! `diff legacy/pvc_src/{centroid,specflattracker}.c` to share
//! `centroid.c`'s whole two-pass shape (band-bound resolution, per-frame
//! attack/release smoothing, multi-channel average/peak combination),
//! so this module reuses `tools::centroid::{resolve_band_bound,
//! resolve_bin_range}` directly (the band-bound-parsing and
//! bin-range-derivation blocks are byte-for-byte identical, confirmed by
//! diffing those specific blocks in isolation, not just eyeballing the
//! whole-file diff).
//!
//! Three real differences from `centroid`/`peakformant`, all confirmed
//! by reading the differing code rather than assumed from the shared
//! shape:
//!
//! 1. **The per-frame core** is `find_spectralflatness()`
//!    (`legacy/pvc_lib/find_spectralflatness.c`): the ratio of the
//!    geometric mean to the arithmetic mean of a per-bin `value` over the
//!    detection band - `1.0` for a perfectly flat (noise-like) spectrum,
//!    approaching `0.0` for a spectrum concentrated in a few bins.
//!    `value` is selected by `-m` (`methodFlag`): `0` = raw amplitude,
//!    `1` = frame-to-frame amplitude change, `2` = frame-to-frame
//!    frequency change - the latter two need the *previous* frame's own
//!    bins, seeded to the current frame's own bins on `frame_count == 0`
//!    (so the very first frame's amplitude/frequency-change methods
//!    always see all-zero deltas). The geometric mean is computed via
//!    `exp(mean(ln(value)))`, not a running product - a real
//!    log(0) = -inf floor: **a single zero-valued bin anywhere in the
//!    band collapses that whole frame's flatness to the amplitude-
//!    threshold floor**, since `-inf` propagates through the mean and
//!    `exp(-inf) == 0.0`. Reproduced exactly via `f64::ln`/`f64::exp`
//!    (which follow the same IEEE-754 behavior as the C's `log`/`exp` at
//!    these limits), not specially guarded - this is what the real tool
//!    does with silence in the band, not a port bug to fix.
//!
//! 2. **A real pass-2 interpolation quirk absent from `centroid.c`**:
//!    `specflattracker.c`'s output loop has an extra `if (tp == 0.)
//!    old_temp = temp;` inside the `while (tp < 1.)` sub-sample
//!    interpolation loop, immediately before using `old_temp` in
//!    `curve()`. Since `tp` only lands on exactly `0.0` at the very
//!    start of the very first outer iteration (every later iteration
//!    carries a nonzero fractional remainder forward via `tp -= (int)
//!    tp`, for any `--output-rate` that doesn't evenly divide back to a
//!    whole number - confirmed by tracing the arithmetic for the
//!    library's own default `output_rate=500`/`frames_per_sec=200`),
//!    this only ever fires on frame 0's first output sample, but it is
//!    real and reproducible there: without it, that sample would
//!    interpolate from the declared-but-never-assigned `old_temp = 0.`
//!    towards the first frame's own value; with it, `curve(old_temp,
//!    old_temp, 0, warp)` trivially returns the first frame's own value
//!    instead. Reproduced here as `if tp == 0.0 { old_temp = temp; }`,
//!    matching the C's own placement inside the loop exactly (not hoisted
//!    out as a one-time special case), since a `--output-rate` that
//!    exactly divides `frames-per-sec` (e.g. equal rates) makes `tp`
//!    land on `0.0` every single frame, not just the first.
//!
//! 3. **`-G`/reference-pitch is fully dead here**, unlike in `centroid`/
//!    `peakformant`: `specflattracker.c`'s own `outformat` only has two
//!    branches (`0` = raw 0-1 coefficient, `1` = decibels via
//!    `amp_to_dB`) - the whole octave/semitones-of-deviation branch
//!    family (and therefore `midC`/`log_of_2`/`refoctave`, all still
//!    computed at startup) is unreachable, confirmed by reading the
//!    `outformat` `if`/`else` chain directly. `-G` is still parsed by
//!    `crack()` and printed in the startup banner but has zero effect on
//!    output. Not exposed as a CLI flag here, matching this project's
//!    established treatment of provably-dead legacy flags.
//!
//! **A doc-vs-code default mismatch, resolved in favor of the code**:
//! `specflattracker.c`'s own `usage()` text claims `-c`'s default is
//! `-200` dB, but the variable it sets
//! (`amplitudeThresholdInDecibels=-96.`) is declared with `-96.` as its
//! actual initializer - confirmed by reading the declaration, not the
//! usage string. This port's default matches the code (`-96.0`), which
//! is what an un-flagged real invocation actually runs with.

use crate::pvoc::Analyzer;
use crate::tools::centroid::{resolve_band_bound, resolve_bin_range};
use crate::units::DbToAmp;
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

pub use crate::tools::envelope::ChannelMethod;

/// `-m`: which per-bin quantity feeds the flatness ratio.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FlatnessMethod {
    Amplitude,
    AmplitudeChange,
    FrequencyChange,
}

/// `-q`: output value shape.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OutputFormat {
    /// `0`: the raw 0-1 flatness coefficient.
    Coefficient,
    /// `1`: the coefficient converted to decibels.
    Decibels,
}

#[derive(Debug, Clone)]
pub struct SpecflattrackerParams {
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
    pub method: FlatnessMethod,
    /// `-c`: amplitude threshold in dB for excluding/flooring
    /// low-valued bins.
    pub amplitude_threshold_db: f32,
}

/// Ports `find_spectralflatness()` (`legacy/pvc_lib/
/// find_spectralflatness.c`): geometric-mean-over-arithmetic-mean ratio
/// of a per-bin `value` (selected by `method`) over `[k1, k2]`. `k1`/`k2`
/// must already be resolved by `tools::centroid::resolve_bin_range`.
/// Returns `0.0` if the arithmetic mean is `<= 0.0` (matching the C's own
/// fallback), *before* the caller applies the amplitude-threshold floor -
/// see this module's doc comment on the `log(0) = -inf` collapse.
fn find_spectralflatness(
    bins: &[(f32, f32)],
    previous_bins: &[(f32, f32)],
    k1: usize,
    k2: usize,
    method: FlatnessMethod,
) -> f32 {
    let mut arithmetic_sum = 0.0f64;
    let mut geometric_log_sum = 0.0f64;
    let mut count = 0.0f64;
    for k in k1..=k2 {
        let value: f64 = match method {
            FlatnessMethod::Amplitude => bins[k].0 as f64,
            FlatnessMethod::AmplitudeChange => (bins[k].0 as f64 - previous_bins[k].0 as f64).abs(),
            FlatnessMethod::FrequencyChange => (bins[k].1 as f64 - previous_bins[k].1 as f64).abs(),
        };
        arithmetic_sum += value;
        geometric_log_sum += value.ln();
        count += 1.0;
    }
    if count <= 0.0 {
        return 0.0;
    }
    let arithmetic_mean = arithmetic_sum / count;
    let geometric_mean = (geometric_log_sum / count).exp();
    if arithmetic_mean > 0.0 {
        (geometric_mean / arithmetic_mean) as f32
    } else {
        0.0
    }
}

fn analyze_channel(
    input: &[f32],
    sample_rate: u32,
    params: &SpecflattrackerParams,
    amplitude_threshold: f32,
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
    let mut frame_count: usize = 0;

    let mut previous_bins: Vec<(f32, f32)> = Vec::new();
    let mut old_flatness = amplitude_threshold;
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
            previous_bins = frame.bins.clone();
        }

        let mut flatness =
            find_spectralflatness(&frame.bins, &previous_bins, k1, k2, params.method);
        if flatness < amplitude_threshold {
            flatness = amplitude_threshold;
        }
        previous_bins = frame.bins.clone();

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

        flatness = if flatness > old_flatness {
            attackc * old_flatness + minusattackc * flatness
        } else {
            releasec * old_flatness + minusreleasec * flatness
        };
        old_flatness = flatness;

        out.push(flatness);
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
    params: &SpecflattrackerParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let nyquist = r / 2.0;
    let d = (r / params.frames_per_sec) as usize;
    let ir = d as f32 / r;

    let db_to_amp = DbToAmp::new();
    let amplitude_threshold = db_to_amp.convert(params.amplitude_threshold_db);

    let per_channel: Vec<Vec<f32>> = channels
        .iter()
        .map(|c| analyze_channel(c, sample_rate, params, amplitude_threshold, dur))
        .collect();
    let mut old_temp = per_channel
        .last()
        .and_then(|c| c.last().copied())
        .unwrap_or(0.0);

    let combined = combine_channels(&per_channel, params.channel_method);

    let mut tpinc = params.output_rate;
    if tpinc < params.frames_per_sec {
        tpinc = 1.0;
    } else {
        tpinc = 1.0 / (tpinc / params.frames_per_sec);
    }
    assert!(
        tpinc <= 1.0,
        "specflattracker: output rate must be >= frames-per-sec"
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
            // Real quirk, not present in `centroid.c` - see this
            // module's doc comment.
            if tp == 0.0 {
                old_temp = temp;
            }
            let mut value = curve(old_temp, temp, tp, 0.0);
            if params.output_format == OutputFormat::Decibels {
                value = crate::units::amp_to_db(value);
            }
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

    fn default_params(fft_size: usize) -> SpecflattrackerParams {
        SpecflattrackerParams {
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
            output_format: OutputFormat::Coefficient,
            method: FlatnessMethod::Amplitude,
            amplitude_threshold_db: -96.0,
        }
    }

    #[test]
    fn pure_tone_has_low_flatness() {
        // A single strong sinusoid concentrates almost all energy in one
        // bin, so its amplitude-method flatness should sit well below 1
        // (perfectly flat/noise-like) once the envelope settles.
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
        assert!((0.0..1.0).contains(&mid), "flatness {mid} out of range");
        assert!(mid < 0.5, "pure tone flatness {mid} unexpectedly high");
    }

    #[test]
    fn white_noise_has_higher_flatness_than_a_pure_tone() {
        let params = default_params(1024);
        let sample_rate = 44100u32;
        // A simple deterministic pseudo-noise signal (summed odd
        // harmonics at incommensurate ratios) stands in for true white
        // noise well enough to spread energy across many bins.
        let noise: Vec<f32> = (0..sample_rate)
            .map(|i| {
                let t = i as f32 / sample_rate as f32;
                let mut s = 0.0;
                for k in 1..40 {
                    s += (1.0 / k as f32)
                        * (2.0 * std::f32::consts::PI * (137.0 * k as f32 + 3.0) * t).sin();
                }
                0.3 * s / 10.0
            })
            .collect();
        let tone: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();

        let noise_out = process(&[noise], sample_rate, &params, 1.0);
        let tone_out = process(&[tone], sample_rate, &params, 1.0);
        let mid_noise = noise_out[noise_out.len() / 2];
        let mid_tone = tone_out[tone_out.len() / 2];
        assert!(
            mid_noise > mid_tone,
            "expected noise flatness {mid_noise} > tone flatness {mid_tone}"
        );
    }

    #[test]
    fn silence_floors_at_amplitude_threshold() {
        let params = default_params(1024);
        let input = vec![0.0f32; 44100 / 2];
        let out = process(&[input], 44100, &params, 1.0);
        let db_to_amp = DbToAmp::new();
        let floor = db_to_amp.convert(params.amplitude_threshold_db);
        for &v in &out {
            assert!(
                (v - floor).abs() < 1e-6,
                "silent input should floor at {floor}, got {v}"
            );
        }
    }
}
