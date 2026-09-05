//! Spectrum magnitude distribution warping, ported from
//! `legacy/pvc_lib/spectmagwarp.c` and its `curve()` primitive
//! (`legacy/pvc_lib/curve.c`). Verified against real compiled output via
//! `legacy/tools/dumputils.c` (see `docs/dev/rust-verification.md`).

/// Ports `curve(V1, V2, n, warp)`: maps a `[0, 1]`-normalized value `n`
/// into `[V1, V2]` along an exponential curve shaped by `warp` (`0.0` is
/// a straight line; the sign/magnitude of `warp` bends it toward `V1` or
/// `V2`). The same shape as `gen.rs`'s `trans`, just parameterized by a
/// continuous `n` instead of a discrete sample index.
pub fn curve(v1: f32, v2: f32, n: f32, warp: f32) -> f32 {
    if warp == 0.0 {
        v1 + (v2 - v1) * n
    } else {
        // `n * warp` is float multiplication (no double literal), only
        // then cast to double for `exp`; the rest of the expression
        // (`1. - ...`, the division, the final `V1 + ...`) mixes in
        // double literals throughout and stays double precision until
        // the single narrowing assignment to the C's `float v` at the
        // very end - no intermediate narrow like `trans`'s `denom`.
        let numerator = 1.0 - ((n * warp) as f64).exp();
        let denominator = 1.0 - (warp as f64).exp();
        (v1 as f64 + (v2 - v1) as f64 * numerator / denominator) as f32
    }
}

/// Ports `spectmagwarp(SP, Nplus2, warpshape, normflag)`: warps the
/// amplitude (even-indexed) slots of a mag/freq-interleaved spectrum
/// array against its own peak amplitude. Returns `false` when nothing
/// happened (peak amplitude is zero or negative, or `warpshape == 0.0`
/// with normalization off - a pure no-op in the C, not an error).
pub fn spectmagwarp(sp: &mut [f32], warpshape: f32, normalize: bool) -> bool {
    let peak = sp.iter().step_by(2).copied().fold(f32::MIN, f32::max);

    if normalize {
        if peak <= 0.0 {
            return false;
        }
        for m in sp.iter_mut().step_by(2) {
            *m /= peak;
        }
        if warpshape == 0.0 {
            return false;
        }
        for m in sp.iter_mut().step_by(2) {
            *m = curve(0.0, 1.0, *m, warpshape);
        }
        true
    } else if warpshape != 0.0 {
        if peak <= 0.0 {
            return false;
        }
        for m in sp.iter_mut().step_by(2) {
            *m = curve(0.0, peak, *m / peak, warpshape);
        }
        true
    } else {
        false
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn curve_matches_c_oracle() {
        let cases: [(f32, f32, f32, f32, f32); 5] = [
            (0.0, 1.0, 0.5, 0.0, 0.5),
            (0.0, 1.0, 0.5, 4.0, 0.119202919),
            (0.0, 1.0, 0.5, -4.0, 0.880797088),
            (0.0, 1.0, 0.25, 8.0, 0.00214400887),
            (10.0, 20.0, 0.75, -2.0, 18.9846363),
        ];
        for (v1, v2, n, warp, want) in cases {
            let got = curve(v1, v2, n, warp);
            assert_eq!(got, want, "curve({v1},{v2},{n},{warp})");
        }
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn spectmagwarp_unnormalized_matches_c_oracle() {
        // Amps [1.0, 4.0, 2.0, 0.5, 3.0] interleaved with arbitrary
        // frequencies (unused by spectmagwarp).
        let mut sp = [
            1.0f32, 100.0, 4.0, 200.0, 2.0, 300.0, 0.5, 400.0, 3.0, 500.0,
        ];
        let changed = spectmagwarp(&mut sp, 2.0, false);
        assert!(changed);
        let mags = [sp[0], sp[2], sp[4], sp[6], sp[8]];
        assert_eq!(
            mags,
            [0.406145304, 4.0, 1.07576573, 0.177819952, 2.17978311]
        );
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn spectmagwarp_normalized_matches_c_oracle() {
        let mut sp = [
            1.0f32, 100.0, 4.0, 200.0, 2.0, 300.0, 0.5, 400.0, 3.0, 500.0,
        ];
        let changed = spectmagwarp(&mut sp, -3.0, true);
        assert!(changed);
        let mags = [sp[0], sp[2], sp[4], sp[6], sp[8]];
        assert_eq!(
            mags,
            [0.555279195, 1.0, 0.817574501, 0.329095423, 0.94147402]
        );
    }

    #[test]
    fn zero_warpshape_without_normalization_is_a_noop() {
        let mut sp = [1.0f32, 0.0, 4.0, 0.0];
        assert!(!spectmagwarp(&mut sp, 0.0, false));
        assert_eq!(sp, [1.0, 0.0, 4.0, 0.0]);
    }
}
