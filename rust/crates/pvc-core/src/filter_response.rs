//! `filter.c`'s response-shaping primitives that operate on a bare
//! amplitude array (one value per bin) rather than a mag/freq-
//! interleaved spectrum: `compand()`, `invertresponse()`,
//! `smoothspec()`. All three only ever read/write the *amplitude*
//! (even-indexed) slots of their legacy interleaved arrays - confirmed
//! by reading each - so this port represents the response as a plain
//! `Vec<f32>`/`&mut [f32]` of amplitudes instead of carrying along a
//! frequency slot nothing here (or `filter.c`'s own per-frame filter-
//! application logic, which recomputes bin positions independently) ever
//! reads.

use crate::units::{amp_to_db, DbToAmp};

/// Ports `compand()`: normalizes to peak `1.0`, compresses bins above
/// `comp_threshold_amp` toward it by `comp_amp`, expands bins below
/// `exp_threshold_amp` away from it by `exp_amp` (clamped at `0`), then
/// re-normalizes to peak `1.0`. Panics if the spectrum's peak is `<= 0`
/// at either normalization (matches the C's own `exit(0)` there - not a
/// runtime condition callers should recover from, since a peak of zero
/// there means the caller already has a malformed spectrum).
pub fn compand(
    amps: &mut [f32],
    comp_threshold_amp: f32,
    comp_amp: f32,
    exp_threshold_amp: f32,
    exp_amp: f32,
) {
    let peak = amps.iter().copied().fold(f32::MIN, f32::max);
    assert!(peak > 0.0, "compand: cannot normalize, peak bin amp is 0");
    for a in amps.iter_mut() {
        *a /= peak;
    }

    let mut peak = f32::MIN;
    for a in amps.iter_mut() {
        if *a > comp_threshold_amp {
            *a = comp_threshold_amp + comp_amp * (*a - comp_threshold_amp);
        } else if *a < exp_threshold_amp {
            *a = exp_threshold_amp - exp_amp * (exp_threshold_amp - *a);
            if *a < 0.0 {
                *a = 0.0;
            }
        }
        if *a > peak {
            peak = *a;
        }
    }

    assert!(peak > 0.0, "compand: cannot normalize, peak bin amp is 0");
    for a in amps.iter_mut() {
        *a /= peak;
    }
}

/// Ports `invertresponse(SP, N, normflag)`: inverts each amplitude in the
/// dB domain (`-96dB` floor for anything already at or below that,
/// matching the C's `athresh = dB_to_amp(-96.)` guard), against either a
/// fixed peak of `1.0` (`peak_relative = false` - `filter.c`'s only call
/// site, confirmed by reading the whole file) or that frame's own actual
/// peak amplitude (`peak_relative = true` - `tvfilter.c`'s `-q 2` mode,
/// its other call site; `-q 1` passes `false` here). A non-positive peak
/// under `peak_relative` is a silent no-op, matching the C's own early
/// `return(0)` there (unlike `compand`'s peak checks elsewhere in this
/// module, this one isn't a caller error worth panicking over - a
/// response frame can be legitimately silent for a stretch of a
/// time-varying file).
pub fn invert_response(amps: &mut [f32], peak_relative: bool, db_to_amp: &DbToAmp) {
    let athresh = db_to_amp.convert(-96.0);
    let (peakamp, normamp) = if peak_relative {
        let peak = amps.iter().copied().fold(f32::MIN, f32::max);
        if peak <= 0.0 {
            return;
        }
        (peak, 1.0 / peak)
    } else {
        (1.0, 1.0)
    };
    for a in amps.iter_mut() {
        let scaled = *a * normamp;
        let db = if scaled <= athresh {
            -96.0
        } else {
            amp_to_db(scaled)
        };
        let inverted_db = -96.0 - db;
        *a = db_to_amp.convert(inverted_db) * peakamp;
    }
}

/// Ports `smoothspec(F, N2plus1, octavesOrFreqBW, R)`: replaces each
/// bin with the average of a symmetric window of bins around it (in
/// fixed-Hz or (if `octaves_or_freq_bw < 0`) octave-proportional units),
/// then rescales the whole result so its peak matches the original
/// peak.
///
/// `fundamental = (float) R / (float) (N2plus1 * 2)` in the C - the same
/// "which N" mixup as `get_formants`' internal fundamental (see
/// `pvc-core::formant`'s doc comment): `N2plus1 * 2` is `N_actual + 2`
/// (`Nplus2`), not `N_actual`, so this is really `R / (N_actual + 2)`,
/// not the true fundamental `R / N_actual`. Reproduced faithfully -
/// confirmed against the oracle with a Hz-mode bandwidth that only
/// produces a real (non-single-bin) smoothing window under the buggy
/// formula, not the "true" one.
///
/// Not reproduced: a real out-of-bounds read in the C whenever a
/// smoothing window's upper edge reaches at or past the last bin -
/// `hibin`'s own clamp only fires on strictly-greater
/// (`if (hibin > N2plus1) hibin = N2plus1;`), so `hibin` can land
/// exactly on `N2plus1` (one past the last valid index) *unclamped* and
/// the loop still reads it inclusively. This is not a narrow edge case:
/// *any* nonzero smoothing width reaches this for bins near the top
/// (there's no width for which every bin's window safely stays in
/// bounds, short of `0`, i.e. no smoothing at all - unreachable for
/// `filter.c`'s own default `smoothingBW = 0.0`, cleanly verified as a
/// no-op against the oracle). Worse, because the whole array is
/// peak-rescaled by one shared factor at the end, this corrupted region
/// can shift *every* bin's output, not just the ones whose own window
/// touches the edge - confirmed while writing this module's oracle
/// tests, which anchor the rescale with one deliberately-huge bin (so
/// the real peak reliably dominates over whatever adjacent memory the
/// C's OOB read happens to return) and compare only the bins whose own
/// window never reaches the last few indices. Clamped to the last valid
/// bin index in this port instead of reproducing the OOB read.
pub fn smooth_response(amps: &mut [f32], octaves_or_freq_bw: f32, sample_rate: u32) {
    let n2plus1 = amps.len();
    let fundamental = sample_rate as f32 / (2 * n2plus1) as f32;

    let (octave_units, half_width_octaves) = if octaves_or_freq_bw < 0.0 {
        (true, octaves_or_freq_bw.abs() / 2.0)
    } else {
        (false, 0.0)
    };
    let pma = 2.0f64.powf(half_width_octaves as f64) as f32;
    let pmb = 2.0f64.powf(-half_width_octaves as f64) as f32;

    let peak_before = amps.iter().copied().fold(f32::MIN, f32::max);

    let last = n2plus1 - 1;
    let mut smoothed = vec![0.0f32; n2plus1];
    for (j, out) in smoothed.iter_mut().enumerate() {
        let (lowbin, hibin) = if octave_units {
            ((0.5 + pmb * j as f32) as i64, (0.5 + pma * j as f32) as i64)
        } else {
            let bins_half_band = (((octaves_or_freq_bw * 0.5) / fundamental) + 0.5) as i64;
            (j as i64 - bins_half_band, j as i64 + bins_half_band)
        };
        let lowbin = lowbin.max(0) as usize;
        let hibin = (hibin as usize).min(last);

        let sum: f32 = amps[lowbin..=hibin].iter().sum();
        *out = sum / (hibin - lowbin + 1) as f32;
    }

    let peak_after = smoothed.iter().copied().fold(f32::MIN, f32::max);
    let norm = if peak_after > 0.0 {
        peak_before / peak_after
    } else {
        1.0
    };
    for (a, s) in amps.iter_mut().zip(&smoothed) {
        *a = s * norm;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[allow(clippy::excessive_precision)]
    fn compand_matches_c_oracle() {
        let mut amps = [0.1, 0.5, 1.0, 0.05, 0.02];
        compand(&mut amps, 0.8, 0.5, 0.1, 2.0);
        assert_eq!(amps, [0.111111119, 0.555555582, 1.0, 0.0, 0.0]);
    }

    #[test]
    #[allow(clippy::excessive_precision)]
    fn invert_response_matches_c_oracle() {
        let db_to_amp = DbToAmp::new();
        let mut amps = [1.0, 0.1];
        invert_response(&mut amps, false, &db_to_amp);
        assert_eq!(amps, [1.58489311e-05, 0.000158416646]);
    }

    #[test]
    fn invert_response_peak_relative_leaves_the_peak_bin_at_the_peak() {
        // At the frame's own peak, `scaled == 1.0` exactly, so the
        // inverted dB is `-96 - 0 = -96`, converted back through
        // `db_to_amp` and rescaled by `peakamp` - i.e. the peak bin maps
        // to `db_to_amp(-96) * peakamp`, not back to `peakamp` itself
        // (this is a real inversion, not a round trip).
        let db_to_amp = DbToAmp::new();
        let mut amps = [2.0, 0.2];
        invert_response(&mut amps, true, &db_to_amp);
        let expected_peak = db_to_amp.convert(-96.0) * 2.0;
        assert!((amps[0] - expected_peak).abs() < 1e-6, "{amps:?}");
        // A quieter bin (10% of peak, so -20dB down) inverts to a louder
        // one, still scaled back up by peakamp.
        assert!(amps[1] > amps[0], "{amps:?}");
    }

    #[test]
    fn invert_response_peak_relative_is_a_noop_when_peak_is_zero() {
        let db_to_amp = DbToAmp::new();
        let mut amps = [0.0, 0.0];
        invert_response(&mut amps, true, &db_to_amp);
        assert_eq!(amps, [0.0, 0.0]);
    }

    #[test]
    fn smooth_response_is_noop_at_zero_bandwidth() {
        let mut amps = vec![0.1, 0.9, 0.2, 0.8, 0.3];
        let before = amps.clone();
        smooth_response(&mut amps, 0.0, 44100);
        for (a, b) in amps.iter().zip(&before) {
            assert!((a - b).abs() < 1e-6);
        }
    }

    #[test]
    #[allow(clippy::excessive_precision)]
    fn smooth_response_hz_mode_matches_c_oracle() {
        // This bandwidth's window reaches a real out-of-bounds read in
        // the C once `j` is within 3 bins of the top edge (`hibin`'s own
        // clamp only fires on strictly-greater, so it can land exactly
        // on the one-past-the-end index and the loop still reads it -
        // see this module's doc comment on `smooth_response`), which
        // then contaminates *every* bin's result through the shared
        // peak-rescale step, not just the bins whose own window touches
        // the edge - so bins 17..20 are excluded from the array itself,
        // not just from comparison, and bin 0 is set far louder than any
        // plausible adjacent-memory garbage so the rescale is reliably
        // anchored there regardless.
        let mut amps: Vec<f32> = (0..20).map(|j: i32| 0.1 + 0.05 * (j % 5) as f32).collect();
        amps[0] = 100.0;
        smooth_response(&mut amps, 100.0, 1000);
        assert_eq!(
            amps[0..=16],
            [
                100.0,
                75.1868439,
                60.3288498,
                0.597907305,
                0.597907305,
                0.597907364,
                0.597907305,
                0.597907305,
                0.597907305,
                0.597907305,
                0.597907364,
                0.597907305,
                0.597907305,
                0.597907305,
                0.597907305,
                0.597907364,
                0.597907305
            ]
        );
    }

    #[test]
    #[allow(clippy::excessive_precision)]
    fn smooth_response_octave_mode_matches_c_oracle() {
        // Same real out-of-bounds/global-rescale-contamination issue as
        // the Hz-mode test above (see its comment): bin 0's window never
        // grows (an octave-proportional window is 0-width at bin 0), so
        // making it far louder than anything plausible from adjacent
        // memory anchors the peak-rescale there regardless of whatever
        // the corrupted top bins (14..20 for this bandwidth) contain.
        let mut amps: Vec<f32> = (0..20).map(|j: i32| 0.1 + 0.05 * (j % 5) as f32).collect();
        amps[0] = 100.0;
        smooth_response(&mut amps, -1.0, 1000);
        assert_eq!(
            amps[0..=13],
            [
                100.0,
                0.150000006,
                0.200000003,
                0.25,
                0.200000018,
                0.1875,
                0.200000003,
                0.183333337,
                0.191666663,
                0.200000003,
                0.21875,
                0.200000018,
                0.200000018,
                0.200000003
            ]
        );
    }
}
