//! Amplitude/dB/semitone conversions, ported from
//! `legacy/pvc_lib/miscellania.c`.
//!
//! `dB_to_amp` and `semitones_to_mult` are *not* the exact formulas
//! (`10^(dB/20)`, `2^(semitones/12)`) in the C - they're 5000-entry
//! lookup tables with linear interpolation, a 1990s performance shortcut
//! that introduces real, deliberate approximation error: confirmed
//! against the real compiled binary (a throwaway `legacy/tools/
//! dumputils.c` linked against `libpvoc.a`, the same technique as
//! `dumpwin.c`), `dB_to_amp(0.0)` is `0.997791529`, not `1.0`. Reproduced
//! here exactly - table construction included - not replaced with the
//! exact formula, since `pv`'s actual output depends on this specific
//! approximation.
//!
//! Every expression here that mixes a `float` value with one of the C's
//! bare numeric literals (`96.`, `4999.`, `.5`, ...) computes in
//! `double` precision, C's usual arithmetic promotion rule for any
//! literal without an `f` suffix - narrowed back to `float` only where
//! the C actually assigns into a `float` variable. Getting this
//! precision cascade wrong (`f32` throughout, narrowing only at the
//! very end) was already caught once in Task 3.1's `gen.rs` (`trans`'s
//! `denom`/angle expressions) and is caught again by this module's own
//! oracle tests below: a first draft using `f32` arithmetic throughout
//! matched the real C's `dB_to_amp(0.0)` to only 6 significant figures,
//! not bit-for-bit.

/// Ports `dB_to_amp()`'s lookup table (a `pv`-tool-lifetime object here,
/// rather than the C's function-local `static` - matches this port's
/// existing convention of caller-owned state, e.g. `Analyzer`/`OscBank`).
pub struct DbToAmp {
    table: [f32; 5000],
    mult1: f32,
}

impl Default for DbToAmp {
    fn default() -> Self {
        Self::new()
    }
}

impl DbToAmp {
    pub fn new() -> Self {
        let mut table = [0.0f32; 5000];
        for (i, t) in table.iter_mut().enumerate() {
            // `( 192. * ( (float) i / 4999. ) ) - 96.`: every literal is
            // a double, so this whole expression is double arithmetic,
            // narrowed to float only on assignment to `temp`.
            let temp = (192.0 * (i as f32 as f64 / 4999.0) - 96.0) as f32;
            // `pow( (double) 10.0, (double) (temp / 20.) )`: `temp / 20.`
            // promotes to double (20. is a double literal).
            *t = 10.0f64.powf(temp as f64 / 20.0) as f32;
        }
        DbToAmp {
            table,
            mult1: (4998.0 / 192.0) as f32,
        }
    }

    /// `dB_to_amp(dB)`: range-reduces by 6dB (halving/doubling the
    /// result) until `dB` is within the table's `[-96, 96]` domain, then
    /// linearly interpolates.
    pub fn convert(&self, mut db: f32) -> f32 {
        let mut div = 1.0f32;
        if db > 96.0 {
            while db > 96.0 {
                db = (db as f64 - 6.0) as f32;
                div = (div as f64 * 2.0) as f32;
            }
        } else if db < -96.0 {
            while db < -96.0 {
                db = (db as f64 + 6.0) as f32;
                div = (div as f64 * 0.5) as f32;
            }
        }
        // `mult1 * ( dB + 96. )`: `dB + 96.` promotes to double.
        let prop = (self.mult1 as f64 * (db as f64 + 96.0)) as f32;
        let iy1 = prop as usize;
        // No double literals below - plain float arithmetic, matching
        // the C exactly.
        let frac = prop - iy1 as f32;
        let y1 = self.table[iy1];
        let y2 = self.table[iy1 + 1];
        (y1 + frac * (y2 - y1)) * div
    }
}

/// Ports `semitones_to_mult()`'s lookup table.
pub struct SemitonesToMult {
    table: [f32; 5000],
    mult: f32,
}

impl Default for SemitonesToMult {
    fn default() -> Self {
        Self::new()
    }
}

impl SemitonesToMult {
    pub fn new() -> Self {
        let mut table = [0.0f32; 5000];
        for (i, t) in table.iter_mut().enumerate() {
            let temp = (144.0 * (i as f32 as f64 / 4999.0) - 72.0) as f32;
            *t = 2.0f64.powf(temp as f64 / 12.0) as f32;
        }
        SemitonesToMult {
            table,
            mult: (4999.0 / 144.0) as f32,
        }
    }

    /// `semitones_to_mult(semidev)`: clamps to the table's `[-72, 72]`
    /// domain (unlike `dB_to_amp`, no range-reduction trick - values past
    /// the ends just clamp) then linearly interpolates.
    pub fn convert(&self, semidev: f32) -> f32 {
        let prop = ((self.mult as f64 * (semidev as f64 + 72.0)) as f32).clamp(0.0, 4998.0);
        let iy1 = prop as usize;
        let frac = prop - iy1 as f32;
        let y1 = self.table[iy1];
        let y2 = self.table[iy1 + 1];
        y1 + frac * (y2 - y1)
    }
}

/// Ports `amp_to_dB()`: this one *is* exact math, no lookup table.
pub fn amp_to_db(amp: f32) -> f32 {
    (20.0 * (amp as f64).log10()) as f32
}

/// Ports `dBtoamp.c`'s own inline conversion - a *third* dB/amplitude
/// convention alongside this module's other two. Unlike `dB_to_amp`
/// above (the `pv`/`filter`/`envelope`-family lookup table) and unlike
/// `amp_to_db` (exact, but the opposite direction), the standalone
/// `dBtoamp` tool never calls into `miscellania.c` at all - it computes
/// `pow(10., dB/20.)` directly inline, so there is no shared table-based
/// counterpart to reuse here. Confirmed exact (not merely close) against
/// the real compiled `dBtoamp` binary in the unit test below.
pub fn db_to_amp_exact(db: f32) -> f32 {
    10.0f64.powf(db as f64 / 20.0) as f32
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn db_to_amp_matches_c_oracle() {
        let conv = DbToAmp::new();
        let cases: [(f32, f32); 10] = [
            (0.0, 0.997791529),
            (-6.0, 0.500150204),
            (-96.0, 1.58489311e-05),
            (96.0, 62817.3594),
            (-200.0, 9.58121499e-11),
            (200.0, 1.03910646e+10),
            (3.0, 1.40932178),
            (-40.0, 0.00998712797),
            (-12.5, 0.236682341),
            (48.0, 250.357635),
        ];
        for (db, want) in cases {
            let got = conv.convert(db);
            assert_eq!(got, want, "dB_to_amp({db})");
        }
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn semitones_to_mult_matches_c_oracle() {
        let conv = SemitonesToMult::new();
        let cases: [(f32, f32); 8] = [
            (0.0, 1.00000036),
            (12.0, 2.00000072),
            (-12.0, 0.500000119),
            (7.0, 1.49830794),
            (-7.0, 0.667420268),
            (100.0, 63.893589),
            (-100.0, 0.015625),
            (0.5, 1.02930236),
        ];
        for (semi, want) in cases {
            let got = conv.convert(semi);
            assert_eq!(got, want, "semitones_to_mult({semi})");
        }
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn db_to_amp_exact_matches_c_oracle() {
        // `/tmp/pvcbuild/pvc_src/dBtoamp -6.0 0.0 20.0 -96.0 96.5`
        let cases: [(f32, f32); 5] = [
            (-6.0, 0.501187),
            (0.0, 1.000000),
            (20.0, 10.000000),
            (-96.0, 0.000016),
            (96.5, 66834.390625),
        ];
        for (db, want) in cases {
            let got = db_to_amp_exact(db);
            assert!(
                (got - want).abs() < 1e-3,
                "db_to_amp_exact({db}) = {got}, want {want}"
            );
        }
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn amp_to_db_matches_c_oracle() {
        let cases: [(f32, f32); 5] = [
            (1.0, 0.0),
            (0.5, -6.02059984),
            (2.0, 6.02059984),
            (0.001, -60.0),
            (100.0, 40.0),
        ];
        for (amp, want) in cases {
            let got = amp_to_db(amp);
            assert!(
                (got - want).abs() < 1e-4,
                "amp_to_dB({amp}) = {got}, want {want}"
            );
        }
    }
}
