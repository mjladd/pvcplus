//! Low/high shelf EQ, ported from `legacy/pvc_lib/eq2.c`. Verified
//! against real compiled output via `legacy/tools/dumputils.c` (see
//! `docs/dev/rust-verification.md`).

use crate::units::DbToAmp;

/// The shelf curve's four control values (legacy `dBlow`/`dBhi`/
/// `freqlow`/`freqhi`, each independently `fval()`-resolved per frame in
/// `plainpv` - grouped here mainly to keep [`eq2`]'s argument count sane).
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct ShelfEq {
    pub d_blow: f32,
    pub d_bhi: f32,
    pub freqlow: f32,
    pub freqhi: f32,
}

/// Ports `eq2(SP, N, dBlow, dBhi, freqlow, freqhi, fundamental,
/// channel_freqdev, normflag)`: applies a low/high shelf gain curve to
/// the amplitude (even-indexed) slots of a mag/freq-interleaved spectrum
/// array. `channel_freqdev` holds `plainpv`'s accumulated per-bin
/// frequency-shift/scale factors (interleaved the same way as `sp`) -
/// used here to compute each bin's *original* (pre-shift) frequency for
/// EQ banding, not its possibly-already-modified one.
///
/// Bins at or below `shelf.freqlow` get `shelf.d_blow`, at or above
/// `shelf.freqhi` get `shelf.d_bhi`, and bins in between are linearly
/// interpolated in dB. When `d_blow == d_bhi`, this degenerates to a flat
/// gain (or a no-op if that gain is `0.0`) - matching the C's own
/// special-cased fast path, not just an edge case of the general formula.
///
/// Panics if `shelf.freqlow > shelf.freqhi` (matches the C's own
/// `exit(0)` there) or, when `normalize` is requested, if every
/// amplitude is zero.
pub fn eq2(
    sp: &mut [f32],
    shelf: &ShelfEq,
    fundamental: f32,
    channel_freqdev: &[f32],
    normalize: bool,
    db_to_amp: &DbToAmp,
) {
    let ShelfEq {
        d_blow,
        d_bhi,
        freqlow,
        freqhi,
    } = *shelf;
    assert!(
        freqlow <= freqhi,
        "eq2: low shelf frequency ({freqlow}) must be <= high shelf frequency ({freqhi})"
    );

    if d_bhi - d_blow != 0.0 {
        let lowamp = db_to_amp.convert(d_blow);
        let hiamp = db_to_amp.convert(d_bhi);
        let d_bdiff = d_bhi - d_blow;
        let freqdiff = freqhi - freqlow;
        for j in 0..sp.len() / 2 {
            let i = 1 + 2 * j;
            let freq = (j as f32 * fundamental + channel_freqdev[i - 1]) * channel_freqdev[i];
            if freq <= freqlow {
                sp[i - 1] *= lowamp;
            } else if freq >= freqhi {
                sp[i - 1] *= hiamp;
            } else {
                // Exact `pow`, not the `DbToAmp` lookup table - confirmed
                // against the oracle: this transition value matched only
                // when computed with the real formula, not the table.
                let temp1 = d_blow + d_bdiff * ((freq - freqlow) / freqdiff);
                let temp1 = 10.0f64.powf(temp1 as f64 / 20.0) as f32;
                sp[i - 1] *= temp1;
            }
        }
    } else if d_bhi != 0.0 {
        let temp1 = db_to_amp.convert(d_bhi);
        for m in sp.iter_mut().step_by(2) {
            *m *= temp1;
        }
    }

    if normalize {
        let peak = sp.iter().step_by(2).copied().fold(0.0f32, f32::max);
        assert!(peak > 0.0, "eq2: cannot normalize, peak bin amp is 0");
        for m in sp.iter_mut().step_by(2) {
            *m /= peak;
        }
    }
}

/// Ports `eq(SP, N, dBlow, dBhi, freqlow, freqhi, fundamental, pmult,
/// freqadd, normflag)` from `legacy/pvc_lib/eq.c` - `pvanalysis.c`'s shelf
/// EQ, distinct from [`eq2`] (`plainpv`'s). Where `eq2` computes each
/// bin's *actual* frequency (via a per-bin `channel_freqdev` factor) and
/// compares that against the shelf frequencies every frame, `eq`
/// converts `freqlow`/`freqhi` to a fixed *bin-index* range once per call
/// (`ilow`/`ihigh`, from `freqlow`/`freqhi` and `fundamental` alone) and
/// gains by index instead of by frequency - so a bin's gain here depends
/// only on its position in the array, never on any per-frame frequency
/// shift. `pmult`/`freqadd` (transposition multiplier / frequency adder)
/// let a caller move the shelf boundaries the same way `eq2`'s
/// `channel_freqdev` would; `pvanalysis.c` always calls this with
/// `pmult=1.0, freqadd=0.0, normflag=false` (no transposition to
/// compensate for).
///
/// `sp` is the full mag/freq-interleaved array (legacy `N` = `sp.len()`,
/// i.e. `n_fft + 2` for a real call) - the loops below only ever touch
/// `sp`'s even indices (amplitudes), matching the C.
///
/// Panics if `freqlow > freqhi` (matches the C's own `exit(0)` there) or,
/// when `normalize` is requested, if every amplitude is zero.
#[allow(clippy::too_many_arguments)]
pub fn eq(
    sp: &mut [f32],
    d_blow: f32,
    d_bhi: f32,
    freqlow: f32,
    freqhi: f32,
    fundamental: f32,
    pmult: f32,
    freqadd: f32,
    normalize: bool,
    db_to_amp: &DbToAmp,
) {
    assert!(
        freqlow <= freqhi,
        "eq: low shelf frequency ({freqlow}) must be <= high shelf frequency ({freqhi})"
    );

    let n = sp.len() as i64;

    if d_bhi - d_blow != 0.0 {
        let freqlow = (freqlow - freqadd) / pmult;
        let freqhi = (freqhi - freqadd) / pmult;

        let mut ilow = 1 + 2 * ((freqlow / fundamental) + 0.5) as i64;
        if ilow < 0 {
            ilow = 1;
        }
        if ilow > n {
            ilow = n - 1;
        }
        let mut ihigh = 1 + 2 * ((freqhi / fundamental) + 0.5) as i64;
        if ihigh < 0 {
            ihigh = 1;
        }
        if ihigh > n {
            ihigh = n - 1;
        }

        let lowamp = db_to_amp.convert(d_blow);
        let hiamp = db_to_amp.convert(d_bhi);

        let mut i = 1;
        while i < ilow {
            sp[(i - 1) as usize] *= lowamp;
            i += 2;
        }

        let d_bdiff = d_bhi - d_blow;
        let n_transition_bins = ((ihigh - ilow) / 2) as f32; // integer division, then cast - matches the C
        let db_per_bin = d_bdiff / n_transition_bins;
        let mut i = ilow;
        let mut j = 0.0f32;
        while i < ihigh {
            let gain_db = d_blow + db_per_bin * j;
            sp[(i - 1) as usize] *= db_to_amp.convert(gain_db);
            i += 2;
            j += 1.0;
        }

        let mut i = ihigh;
        while i < n {
            sp[(i - 1) as usize] *= hiamp;
            i += 2;
        }
    } else if d_bhi != 0.0 {
        let amp = db_to_amp.convert(d_bhi);
        let mut i = 1;
        while i < n {
            sp[(i - 1) as usize] *= amp;
            i += 2;
        }
    }

    if normalize {
        let mut i = 1;
        let mut peak = 0.0f32;
        while i < n {
            peak = peak.max(sp[(i - 1) as usize]);
            i += 2;
        }
        assert!(peak > 0.0, "eq: cannot normalize, peak bin amp is 0");
        let mut i = 1;
        while i < n {
            sp[(i - 1) as usize] /= peak;
            i += 2;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn shelf_transition_matches_c_oracle() {
        let db_to_amp = DbToAmp::new();
        let mut sp = [
            1.0f32, 100.0, 1.0, 200.0, 1.0, 300.0, 1.0, 400.0, 1.0, 500.0,
        ];
        let freqdev = [0.0f32, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0];
        let shelf = ShelfEq {
            d_blow: -12.0,
            d_bhi: 6.0,
            freqlow: 150.0,
            freqhi: 350.0,
        };
        eq2(&mut sp, &shelf, 100.0, &freqdev, false, &db_to_amp);
        let mags = [sp[0], sp[2], sp[4], sp[6], sp[8]];
        assert_eq!(
            mags,
            [
                0.250703752,
                0.250703752,
                0.421696514,
                1.18850219,
                1.99058378
            ]
        );
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn eq_shelf_matches_c_oracle() {
        let db_to_amp = DbToAmp::new();
        let mut sp = [
            1.0f32, 100.0, 1.0, 200.0, 1.0, 300.0, 1.0, 400.0, 1.0, 500.0,
        ];
        eq(
            &mut sp, -12.0, 6.0, 150.0, 350.0, 100.0, 1.0, 0.0, false, &db_to_amp,
        );
        let mags = [sp[0], sp[2], sp[4], sp[6], sp[8]];
        assert_eq!(
            mags,
            [
                0.250703752,
                0.250703752,
                0.250703752,
                0.706431746,
                1.99058378
            ]
        );
    }

    #[test]
    #[allow(clippy::excessive_precision)]
    fn eq_gain_only_mode_matches_c_oracle() {
        let db_to_amp = DbToAmp::new();
        let mut sp = [
            1.0f32, 100.0, 1.0, 200.0, 1.0, 300.0, 1.0, 400.0, 1.0, 500.0,
        ];
        eq(
            &mut sp, 6.0, 6.0, 150.0, 350.0, 100.0, 1.0, 0.0, false, &db_to_amp,
        );
        let mags = [sp[0], sp[2], sp[4], sp[6], sp[8]];
        assert_eq!(mags, [1.99058378; 5]);
    }

    #[test]
    #[allow(clippy::excessive_precision)]
    fn eq_wide_transition_matches_c_oracle() {
        let db_to_amp = DbToAmp::new();
        let mut sp = [0.0f32; 22];
        for i in 0..11 {
            sp[2 * i] = 1.0;
            sp[2 * i + 1] = (i * 44) as f32;
        }
        eq(
            &mut sp, -18.0, 9.0, 50.0, 400.0, 44.0, 1.0, 0.0, false, &db_to_amp,
        );
        let mags: Vec<f32> = (0..11).map(|i| sp[2 * i]).collect();
        assert_eq!(
            mags,
            [
                0.125666901,
                0.125666901,
                0.185326263,
                0.273308337,
                0.403058976,
                0.594408929,
                0.876600921,
                1.29276133,
                1.90649021,
                2.81158137,
                2.81158137
            ]
        );
    }

    #[test]
    #[should_panic(expected = "must be <=")]
    fn reversed_shelf_frequencies_panics() {
        let db_to_amp = DbToAmp::new();
        let mut sp = [1.0f32, 0.0];
        let freqdev = [0.0f32, 1.0];
        let shelf = ShelfEq {
            d_blow: -12.0,
            d_bhi: 6.0,
            freqlow: 350.0,
            freqhi: 150.0,
        };
        eq2(&mut sp, &shelf, 100.0, &freqdev, false, &db_to_amp);
    }
}
