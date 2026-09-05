//! Time-varying control values, ported from `legacy/pvc_lib/fval.c`
//! (`fval()`). Most `pvc` tool parameters (gain, pitch transposition,
//! filter cutoffs, ...) can be either a single constant or a breakpoint
//! table interpolated linearly over the sound's duration; `fval` is what
//! every tool calls each analysis frame to resolve "what's this
//! parameter's value at time T".
//!
//! The legacy `fval` reads breakpoint values on demand from an open file
//! (`struct func`'s `fp`/`L`/`n` fields), re-seeking on every call - a
//! memory-saving measure for 1990s hardware, not part of the algorithm.
//! `pvc-io::control::read_control_file` already made the equivalent
//! simplification (load the whole table into a `Vec<f32>` once); `ControlFn`
//! continues that here by holding the table in memory and doing plain
//! indexed lookups instead of file seeks - same interpolated values, no
//! I/O in this crate.

/// A time-varying control value: either a fixed constant (`fval`'s
/// `p->n <= 1` case) or a breakpoint table linearly interpolated across
/// `[0, dur]` (`fval`'s general case).
#[derive(Debug, Clone, PartialEq)]
pub enum ControlFn {
    Const(f32),
    Table(Vec<f32>),
}

impl ControlFn {
    /// `fval(p, dur, T)`: the control value at time `T` within a sound of
    /// total duration `dur`.
    ///
    /// A table with fewer than 2 points behaves as a constant (`fval`'s
    /// `p->n <= 1` early return). Otherwise: `T <= 0` clamps to the first
    /// point and `T >= dur` clamps to the last (the legacy binary-file
    /// path's boundary checks - used here instead of the ASCII path's
    /// `T < 0` / `T > dur`, since at `T == 0` both paths already produce
    /// the same value, and the ASCII path's `T > dur` handling has a
    /// sequential-file-scan quirk at exactly `T == dur` that doesn't
    /// apply once the table is just an in-memory array). Otherwise,
    /// linearly interpolates between the two bracketing points of a
    /// virtual index `(T / dur) * (n - 1)`.
    pub fn at(&self, t: f32, dur: f32) -> f32 {
        let table = match self {
            ControlFn::Const(v) => return *v,
            ControlFn::Table(table) => table,
        };
        let n = table.len();
        if n <= 1 {
            return table.first().copied().unwrap_or(0.0);
        }
        if t <= 0.0 {
            return table[0];
        }
        if t >= dur {
            return table[n - 1];
        }
        let v = (t / dur) * (n - 1) as f32;
        // Clamped defensively: for T approaching dur, floating-point
        // rounding of `t / dur` could otherwise round up to exactly
        // `n - 1`, pushing `ivhi` to `n` (out of bounds) - a real risk in
        // the original C's array access too, just not one that panics
        // there. Doesn't change the interpolated curve: legitimate `v`
        // for `t < dur` is always `< n - 1`.
        let ivlow = (v as usize).min(n - 2);
        let ivhi = ivlow + 1;
        let z = v - ivlow as f32;
        table[ivlow] + z * (table[ivhi] - table[ivlow])
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// The plan's explicit Task 2.6 acceptance test: replicate `fval` on a
    /// 3-point table at t=0, mid, and t=dur.
    #[test]
    fn three_point_table_at_start_middle_and_end() {
        let f = ControlFn::Table(vec![10.0, 20.0, 30.0]);
        let dur = 10.0;

        assert_eq!(f.at(0.0, dur), 10.0);
        // Virtual index at t=5: (5/10)*(3-1) = 1.0 -> exactly point 1.
        assert_eq!(f.at(5.0, dur), 20.0);
        assert_eq!(f.at(dur, dur), 30.0);
    }

    #[test]
    fn interpolates_linearly_between_points() {
        let f = ControlFn::Table(vec![0.0, 10.0]);
        let dur = 4.0;
        // Virtual index at t=1: (1/4)*(2-1) = 0.25 -> quarter of the way.
        assert!((f.at(1.0, dur) - 2.5).abs() < 1e-5);
        assert!((f.at(3.0, dur) - 7.5).abs() < 1e-5);
    }

    #[test]
    fn negative_and_past_duration_times_clamp() {
        let f = ControlFn::Table(vec![1.0, 2.0, 3.0]);
        let dur = 10.0;
        assert_eq!(f.at(-5.0, dur), 1.0);
        assert_eq!(f.at(dur + 5.0, dur), 3.0);
    }

    #[test]
    fn constant_ignores_time_and_duration() {
        let f = ControlFn::Const(42.0);
        assert_eq!(f.at(0.0, 1.0), 42.0);
        assert_eq!(f.at(100.0, 1.0), 42.0);
        assert_eq!(f.at(-100.0, 1.0), 42.0);
    }

    #[test]
    fn single_point_table_behaves_as_constant() {
        let f = ControlFn::Table(vec![7.5]);
        assert_eq!(f.at(0.0, 10.0), 7.5);
        assert_eq!(f.at(5.0, 10.0), 7.5);
        assert_eq!(f.at(20.0, 10.0), 7.5);
    }

    #[test]
    fn empty_table_returns_zero_rather_than_panicking() {
        let f = ControlFn::Table(vec![]);
        assert_eq!(f.at(5.0, 10.0), 0.0);
    }
}
