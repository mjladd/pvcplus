//! Amplitude envelope attack/release smoothing, ported from
//! `legacy/pvc_lib/smooth.c` and `smooth_setup()` in `miscellania.c`.
//! Verified against real compiled output via `legacy/tools/dumputils.c`
//! (see `docs/dev/rust-verification.md`).

/// Ports `smooth_setup(t, c, minusc, IR)`: the exponential smoothing
/// coefficient for an attack/release time `t` (seconds) at a given hop
/// duration `ir` (seconds per resynthesis frame, legacy `I / R`).
/// Returns `(c, minusc)` where `minusc = 1.0 - c`.
pub fn smooth_setup(t: f32, ir: f32) -> (f32, f32) {
    if t <= 0.0 {
        return (0.0, 1.0);
    }
    // `ar_dB = pow(10.0, -60./20.)`: computed once as a `static float` in
    // the C (an exact `-60dB` time constant, not the lookup-table
    // approximation `dB_to_amp` uses) - recomputed here each call since
    // it's cheap and this port avoids hidden global/static state.
    let ar_db = 10.0f64.powf(-60.0 / 20.0) as f32;
    // `pow((double) ar_dB, (double) (IR / t))`: `IR / t` is float
    // division (no double literal involved), only then cast to double
    // for `pow`.
    let c = (ar_db as f64).powf((ir / t) as f64) as f32;
    let minusc = (1.0 - c as f64) as f32;
    (c, minusc)
}

/// Ports `smooth_one_value(A, old_A, att, matt, rel, mrel)`
/// (`legacy/pvc_lib/miscellania.c`): attack/release-smooths a single
/// scalar (`spectwarper.c`'s per-frame or per-bin peak-follower) -
/// distinct from [`Smoother`] (which smooths a whole spectrum array via
/// [`smooth_setup`]'s coefficients, applied unconditionally); this picks
/// between the release pair (`rel`/`mrel`) and the attack pair (`att`/
/// `matt`) based on whether the new value is quieter or louder than the
/// previous one.
pub fn smooth_one_value(a: f32, old_a: f32, att: f32, matt: f32, rel: f32, mrel: f32) -> f32 {
    if a < old_a {
        rel * old_a + mrel * a
    } else {
        att * old_a + matt * a
    }
}

/// Ports `smooth()`: attack/release-smooths the amplitude (even-indexed)
/// slots of a mag/freq-interleaved spectrum array, one instance per
/// channel (the C's `old_A` "previous channel" buffer plus its
/// `frame_count == 0` first-frame special case, both held here instead
/// of relying on a global frame counter).
pub struct Smoother {
    old_a: Vec<f32>,
    first_frame: bool,
}

impl Smoother {
    pub fn new(n_plus_2: usize) -> Self {
        Smoother {
            old_a: vec![0.0; n_plus_2],
            first_frame: true,
        }
    }

    /// On the first call, primes `old_a` from `a` (matching the C's
    /// `frame_count == 0` full-array copy) - this makes the first frame a
    /// no-op regardless of `att`/`rel`, confirmed against real output
    /// (see the oracle test below).
    pub fn smooth(&mut self, a: &mut [f32], att: f32, matt: f32, rel: f32, mrel: f32) {
        if self.first_frame {
            self.old_a.copy_from_slice(a);
            self.first_frame = false;
        }
        if rel != 0.0 || att != 0.0 {
            for i in (0..a.len()).step_by(2) {
                a[i] = if a[i] < self.old_a[i] {
                    rel * self.old_a[i] + mrel * a[i]
                } else {
                    att * self.old_a[i] + matt * a[i]
                };
            }
        }
        for i in (0..a.len()).step_by(2) {
            self.old_a[i] = a[i];
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn smooth_setup_matches_c_oracle() {
        let (c, minusc) = smooth_setup(0.05, 0.005);
        assert_eq!(c, 0.501187265);
        assert_eq!(minusc, 0.498812735);

        let (c, minusc) = smooth_setup(0.0, 0.005);
        assert_eq!(c, 0.0);
        assert_eq!(minusc, 1.0);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn smooth_across_three_frames_matches_c_oracle() {
        let (att, matt, rel, mrel) = (0.3f32, 0.7f32, 0.6f32, 0.4f32);
        let mut smoother = Smoother::new(6);

        let mut a1 = [1.0f32, 0.0, 2.0, 0.0, 3.0, 0.0];
        smoother.smooth(&mut a1, att, matt, rel, mrel);
        assert_eq!([a1[0], a1[2], a1[4]], [1.0, 2.0, 3.0]);

        let mut a2 = [0.5f32, 0.0, 5.0, 0.0, 1.0, 0.0];
        smoother.smooth(&mut a2, att, matt, rel, mrel);
        assert_eq!([a2[0], a2[2], a2[4]], [0.800000012, 4.0999999, 2.20000005]);

        let mut a3 = [0.8f32, 0.0, 4.0, 0.0, 2.0, 0.0];
        smoother.smooth(&mut a3, att, matt, rel, mrel);
        assert_eq!([a3[0], a3[2], a3[4]], [0.800000012, 4.05999994, 2.12000012]);
    }
}
