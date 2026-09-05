//! Analysis/synthesis window pairs, ported from `legacy/pvc_lib/makewindows.c`.
//!
//! Faithfully reproduces the legacy shape computation, the Kaiser window's
//! custom Bessel-function series approximation (not a "proper" I0 Bessel
//! function from a math library - the exact series `makewindows.c` uses,
//! so a Kaiser window here matches the C oracle rather than a textbook
//! Kaiser window), the sinc-interpolation step applied when `Nw > N`, and
//! the two-stage unity-gain normalization. See the plan's risk notes: port
//! first, "fix" only if a real bug turns up (none did here - the window
//! math translates directly).
//!
//! One behavioral difference from the C: `window_type` is a *parameter*
//! here (an explicit `Window` enum), not a global variable the caller sets
//! before calling - the legacy `makewindows()` signature doesn't even take
//! it as an argument, which is exactly the kind of implicit-global-state
//! wart the new CLI is meant to get away from (plan §2.1).

use std::f32::consts::PI as PI_F32;

/// Window shape. The legacy Kaiser windows are parameterized by an integer
/// "alpha" from 4 to 12 (also controls the reported sidelobe level,
/// `-7.5 * alpha` dB); kept as an `f32` here since `besselfunc` is defined
/// over reals, but the legacy CLI only ever exercises the integers 4..=12.
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Window {
    Hamming,
    Rectangular,
    Blackman,
    Bartlett,
    Kaiser(f32),
    BlackmanHarris,
    Nuttall,
    BlackmanNuttall,
    FlatTop,
}

/// `H`: the raw window shape before sinc-interpolation or normalization.
/// Computed and returned for parity with the legacy `Hwin` output, but note
/// legacy callers allocate it and then never read it again after the
/// `makewindows()` call (confirmed in `plainpv.c`) - it's dead in practice.
/// `analysis`/`synthesis` (legacy `A`/`S`): sinc-interpolated (if
/// `nw > n`) and unity-gain-normalized - what real callers use.
#[derive(Debug, Clone, PartialEq)]
pub struct WindowPair {
    pub raw: Vec<f32>,
    pub analysis: Vec<f32>,
    pub synthesis: Vec<f32>,
}

/// Legacy `besselfunc()`: a power-series approximation of the modified
/// Bessel function of the first kind, I0(x), summing `(x^k / (2^k k!))^2`
/// until a term drops below `1e-7`. Ported verbatim rather than substituted
/// with a library I0 implementation, since the exact series and threshold
/// are what the C oracle's Kaiser windows are computed from.
fn besselfunc(x: f32) -> f32 {
    const THRESHOLD: f32 = 0.0000001;
    let mut ssum = 0.0f32;
    let mut k: i32 = 1;
    loop {
        let mut factsum = 1.0f32;
        for i in 2..=k {
            factsum *= i as f32;
        }
        let s = (x.powi(k) / (2.0f32.powi(k) * factsum)).powi(2);
        ssum += s;
        if s <= THRESHOLD {
            break;
        }
        k += 1;
    }
    ssum
}

/// Ports `makewindows()`. `n` must be a power of two (the legacy function
/// exits the whole process on a bad FFT size; here that's the caller's
/// responsibility to validate before calling - this function assumes `n`
/// is already valid and just computes windows of length `nw`).
///
/// `i_factor` is the synthesis interpolation factor (legacy `I`); pass `0`
/// for the overlap-add case where there is no separate interpolation
/// stride (matches the legacy `if (I)` guards).
pub fn make_windows(window: Window, nw: usize, n: usize, i_factor: usize) -> WindowPair {
    let mut h = vec![0.0f32; nw];
    let mut a = vec![0.0f32; nw];

    let twopi: f32 = (8.0f64 * 1.0f64.atan()) as f32;

    match window {
        Window::Hamming => {
            for (i, hv) in h.iter_mut().enumerate() {
                *hv = 0.54 - 0.46 * (twopi * i as f32 / (nw as f32 - 1.0)).cos();
            }
        }
        Window::Rectangular => {
            h.fill(1.0);
        }
        Window::Blackman => {
            for (i, hv) in h.iter_mut().enumerate() {
                let x = i as f32;
                *hv = 0.42 - 0.5 * ((twopi / (nw as f32 - 1.0)) * x).cos()
                    + 0.08 * ((2.0 * twopi / (nw as f32 - 1.0)) * x).cos();
            }
        }
        Window::Bartlett => {
            for (i, hv) in h.iter_mut().enumerate() {
                *hv = 1.0 - (1.0 / (nw as f32 + 1.0)) * (2.0 * i as f32 - nw as f32 + 1.0).abs();
            }
        }
        Window::Kaiser(alpha) => {
            let bessel_alpha = besselfunc(alpha);
            for (i, hv) in h.iter_mut().enumerate() {
                let mut v = (2.0 * i as f32 - nw as f32 + 1.0).abs();
                v *= 1.0 / (nw as f32 - 1.0);
                v *= v;
                v = (1.0 - v).sqrt();
                v *= alpha;
                *hv = besselfunc(v) / bessel_alpha;
            }
        }
        Window::BlackmanHarris => {
            for (i, hv) in h.iter_mut().enumerate() {
                let x = i as f32;
                *hv = 0.35875 - 0.48829 * ((twopi / (nw as f32 - 1.0)) * x).cos()
                    + 0.14128 * ((2.0 * twopi / (nw as f32 - 1.0)) * x).cos()
                    - 0.01168 * (((3.0 / 2.0) * twopi / (nw as f32 - 1.0)) * x).cos();
            }
        }
        Window::Nuttall => {
            for (i, hv) in h.iter_mut().enumerate() {
                let x = i as f32;
                *hv = 0.355768 - 0.487396 * ((twopi / (nw as f32 - 1.0)) * x).cos()
                    + 0.144232 * ((2.0 * twopi / (nw as f32 - 1.0)) * x).cos()
                    - 0.012604 * (((3.0 / 2.0) * twopi / (nw as f32 - 1.0)) * x).cos();
            }
        }
        Window::BlackmanNuttall => {
            for (i, hv) in h.iter_mut().enumerate() {
                let x = i as f32;
                *hv = 0.3635819 - 0.4891775 * ((twopi / (nw as f32 - 1.0)) * x).cos()
                    + 0.1365995 * ((2.0 * twopi / (nw as f32 - 1.0)) * x).cos()
                    - 0.0106411 * (((3.0 / 2.0) * twopi / (nw as f32 - 1.0)) * x).cos();
            }
        }
        Window::FlatTop => {
            for (i, hv) in h.iter_mut().enumerate() {
                let x = i as f32;
                *hv = (1.0 - 1.93 * ((twopi * x) / (nw as f32 - 1.0)).cos()
                    + 1.29 * ((2.0 * twopi * x) / (nw as f32 - 1.0)).cos()
                    - 0.388 * ((3.0 * twopi * x) / (nw as f32 - 1.0)).cos()
                    + 0.032 * ((4.0 * twopi * x) / (nw as f32 - 1.0)).cos())
                    / 5.0;
            }
        }
    }
    a.copy_from_slice(&h);
    let mut s = a.clone();

    // Sinc-interpolation, when the window is wider than the FFT: ensures
    // the window is zero at multiples of N (analysis) / I (synthesis) away
    // from center.
    if nw > n {
        let mut x = -((nw as f32 - 1.0) / 2.0);
        for (av, sv) in a.iter_mut().zip(s.iter_mut()) {
            if x != 0.0 {
                *av *= n as f32 * (PI_F32 * x / n as f32).sin() / (PI_F32 * x);
                if i_factor != 0 {
                    *sv *= i_factor as f32 * (PI_F32 * x / i_factor as f32).sin() / (PI_F32 * x);
                }
            }
            x += 1.0;
        }
    }

    // Unity-gain normalization.
    let sum: f32 = a.iter().sum();
    let afac = 2.0 / sum;
    let sfac = if nw > n { 1.0 / afac } else { afac };
    for (av, sv) in a.iter_mut().zip(s.iter_mut()) {
        *av *= afac;
        *sv *= sfac;
    }

    if nw <= n && i_factor != 0 {
        let mut sum2 = 0.0f32;
        let mut i = 0;
        while i < nw {
            sum2 += s[i] * s[i];
            i += i_factor;
        }
        let inv = 1.0 / sum2;
        for v in s.iter_mut() {
            *v *= inv;
        }
    }

    WindowPair {
        raw: h,
        analysis: a,
        synthesis: s,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn hamming_matches_known_endpoints() {
        // Hamming: 0.54 - 0.46*cos(2*pi*i/(N-1)); at i=0 and i=N-1 this is
        // 0.54 - 0.46*1 = 0.08 before normalization.
        let pair = make_windows(Window::Hamming, 8, 8, 0);
        assert!((pair.raw[0] - 0.08).abs() < 1e-5);
        assert!((pair.raw[7] - 0.08).abs() < 1e-5);
        // Symmetric.
        for i in 0..8 {
            assert!((pair.raw[i] - pair.raw[7 - i]).abs() < 1e-6);
        }
    }

    #[test]
    fn rectangular_is_all_ones_before_normalization() {
        let pair = make_windows(Window::Rectangular, 16, 16, 0);
        assert!(pair.raw.iter().all(|&v| (v - 1.0).abs() < 1e-6));
    }

    #[test]
    fn analysis_window_sums_to_two_when_nw_le_n() {
        // afac = 2/sum(A) is applied so that, when Nw<=N, sum(A) == 2
        // after normalization (the "unity gain" the C comment refers to).
        for w in [
            Window::Hamming,
            Window::Blackman,
            Window::Bartlett,
            Window::BlackmanHarris,
        ] {
            let pair = make_windows(w, 1024, 1024, 220);
            let sum: f32 = pair.analysis.iter().sum();
            assert!((sum - 2.0).abs() < 1e-3, "{w:?}: sum = {sum}");
        }
    }

    #[test]
    fn kaiser_window_is_symmetric_and_peaks_at_one() {
        let pair = make_windows(Window::Kaiser(8.0), 15, 1024, 220);
        // Odd length: exact center sample should be the peak (raw, before
        // normalization skews the overall scale) since besselfunc(0)/
        // besselfunc(alpha) is the max value the shape function takes.
        let center = pair.raw[7];
        assert!(pair.raw.iter().all(|&v| v <= center + 1e-6));
        for i in 0..15 {
            assert!((pair.raw[i] - pair.raw[14 - i]).abs() < 1e-5);
        }
    }

    #[test]
    fn sinc_interpolation_only_applies_when_nw_greater_than_n() {
        // Same window type/length, different N: Nw==N should skip the sinc
        // step, so analysis differs from the Nw>N case (which also mixes
        // in the sinc factor before normalizing).
        let no_sinc = make_windows(Window::Hamming, 1024, 1024, 220);
        let with_sinc = make_windows(Window::Hamming, 1024, 512, 220);
        assert_ne!(no_sinc.analysis, with_sinc.analysis);
    }

    /// Direct comparison against real `legacy/pvc_lib/makewindows.c` output
    /// (via `legacy/tools/dumpwin.c`, Nw=N=1024, I=220) - not a
    /// hand-derived invariant. The synthesis window's final normalization
    /// stage (`sum(S[i]^2)` at stride `I`, then `S[i] *= 1/that_sum` for
    /// every `i`) produces values far outside [0,1] here (~12 to ~149) -
    /// confirmed against the C oracle rather than assumed, since an
    /// earlier version of this test wrongly assumed the result should
    /// renormalize back to a sum of 1.
    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn hamming_matches_c_oracle_values() {
        let pair = make_windows(Window::Hamming, 1024, 1024, 220);
        let expected = [
            (0usize, 0.0799999982f32, 0.000289592805f32, 11.9454193f32),
            (1, 0.0800086781, 0.000289624208, 11.9467144),
            (500, 0.998853028, 0.00361575815, 149.146484),
            (1023, 0.0799999982, 0.000289592805, 11.9454193),
        ];
        for (i, raw, analysis, synthesis) in expected {
            assert!(
                (pair.raw[i] - raw).abs() < 1e-4,
                "raw[{i}] = {}",
                pair.raw[i]
            );
            assert!(
                (pair.analysis[i] - analysis).abs() < 1e-5,
                "analysis[{i}] = {}",
                pair.analysis[i]
            );
            assert!(
                (pair.synthesis[i] - synthesis).abs() < 1e-2,
                "synthesis[{i}] = {}",
                pair.synthesis[i]
            );
        }
    }

    /// Same idea for a Kaiser window (exercises besselfunc), Nw=N=1024,
    /// I=220, alpha=8.
    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn kaiser_matches_c_oracle_values() {
        let pair = make_windows(Window::Kaiser(8.0), 1024, 1024, 220);
        let expected = [
            (0usize, 0.0f32, 0.0f32, 0.0f32),
            (1, 0.000148824925, 6.69755195e-07, 0.0225280691),
            (500, 0.998106122, 0.00449176598, 151.086273),
            (1023, 0.0, 0.0, 0.0),
        ];
        for (i, raw, analysis, synthesis) in expected {
            assert!(
                (pair.raw[i] - raw).abs() < 1e-4,
                "raw[{i}] = {}",
                pair.raw[i]
            );
            assert!(
                (pair.analysis[i] - analysis).abs() < 1e-5,
                "analysis[{i}] = {}",
                pair.analysis[i]
            );
            assert!(
                (pair.synthesis[i] - synthesis).abs() < 1e-2,
                "synthesis[{i}] = {}",
                pair.synthesis[i]
            );
        }
    }
}
