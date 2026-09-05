//! Control-function generators, ported from the CARL/cmusic "GEN" family
//! (`legacy/cmusic_gen/gen/gen{1,2,3,4,5}.c`) and the shared
//! `legacy/cmusic_gen/lib/libfrm/trans.c` transition primitive they all
//! build on. These fill a fixed-length breakpoint table used as an
//! envelope/control function elsewhere in the toolkit - `pvc fn gen1`..
//! `gen5` in the plan's CLI sketch (§2.1). Verified directly against the
//! real compiled `gen1`..`gen5` binaries (`cmake --build build --target
//! gen1 gen2 gen3 gen4 gen5`), not just re-derived by hand - see the test
//! module.
//!
//! Not yet ported: `cspline` (cubic spline interpolation, 319 lines),
//! `cannon`, and `reshape` (2486 lines, many modes - the plan's own note
//! says to port only the modes `utilities/*`/`S.*` scripts use first).
//! All three remain available via `pvc legacy`.
//!
//! `sin`/`exp` in the C are called on a `float` angle that gets implicitly
//! widened to `double` (C promotes a `float` argument to `double` for a
//! function prototyped to take `double`, exactly, no precision loss), and
//! the `double` result then gets narrowed back to `float` on each
//! accumulation into a `float` output slot. Reproduced explicitly below
//! (`as f64` before the transcendental call, `as f32` after) rather than
//! computing in `f32` throughout, since the two give measurably different
//! low-order bits - confirmed against the real oracle values in this
//! module's tests, which are not perfectly symmetric the way pure-`f32`
//! trig would produce.

const TWO_PI: f32 = std::f32::consts::TAU;

/// Ports `trans.c`: a transition from `a` to `b` over `n` samples
/// (`n >= 2`) according to `alpha` - `0.0` linear, negative exponential,
/// positive logarithmic. Panics below `n = 2`, matching the C's own
/// `exit(-1)` there (always called with a real segment length from
/// [`gen1`]/[`gen3`]/[`gen4`], not user input to validate at this layer).
pub fn trans(a: f32, alpha: f32, b: f32, n: usize) -> Vec<f32> {
    assert!(n >= 2, "trans: transition length must be >= 2, got {n}");
    let delta = b - a;
    let interval = 1.0 / (n - 1) as f32;
    if alpha != 0.0 {
        // The C narrows `denom` to a `float` here (`register float
        // denom;`) before using it below - narrowing only at the very
        // end, as the other `f64 as f32` casts in this module do, would
        // silently skip that intermediate rounding step and drift by a
        // ULP from the real C output on some inputs (caught by this
        // function's oracle-verified tests).
        let denom = (1.0 / (1.0 - (alpha as f64).exp())) as f32;
        (0..n)
            .map(|i| {
                // The C's `(double) i * alpha * interval` casts only `i`
                // - but that alone promotes the whole chain to double
                // precision (once one operand is `double`, C's usual
                // arithmetic conversions promote the rest), unlike this
                // module's other angle expressions (`gen2`/`gen5`, no
                // explicit cast at all) which stay in `float` until
                // `sin`/`cos` promotes the final value.
                let angle = i as f64 * alpha as f64 * interval as f64;
                (a as f64 + delta as f64 * (1.0 - angle.exp()) * denom as f64) as f32
            })
            .collect()
    } else {
        (0..n).map(|i| a + delta * i as f32 * interval).collect()
    }
}

/// Shared piecewise-transition segment builder for [`gen1`]/[`gen3`]/
/// [`gen4`]: `values[i]` are the breakpoint values, `times[i]` their
/// already-scaled (into `[0, length - closed]`) sample positions -
/// strictly increasing, same length as `values` (>= 2). `alphas[i]` is
/// the `trans` parameter for the segment from breakpoint `i` to `i + 1`
/// (`alphas.len() == values.len() - 1`).
///
/// A segment's last sample and the next segment's first sample coincide
/// exactly (both equal that breakpoint's value, by `trans`'s own
/// definition), so segments are written overlapping by one sample -
/// matching the C's pointer advancing by `seglen - 1` between segments.
/// When the breakpoints don't reach exactly `length - 1` (the "open"
/// `gen1 -o` case), the final segment's trailing samples that would fall
/// at or past `length` are silently dropped rather than written out of
/// bounds - the C's equivalent write there is an actual one-sample heap
/// overflow (confirmed against a real `gen1 -o` run: the visible
/// `length`-sample output is identical either way, since that
/// out-of-bounds sample was never part of what got printed).
fn gen_segments(length: usize, times: &[f32], values: &[f32], alphas: &[f32]) -> Vec<f32> {
    let mut out = vec![0.0f32; length];
    let mut pos: i64 = 0;
    for i in 0..values.len() - 1 {
        let seglen = (times[i + 1] + 0.5).floor() as i64 - (times[i] + 0.5).floor() as i64 + 1;
        let seg = trans(values[i], alphas[i], values[i + 1], seglen as usize);
        for (j, &v) in seg.iter().enumerate() {
            let idx = pos + j as i64;
            if idx >= 0 && (idx as usize) < length {
                out[idx as usize] = v;
            }
        }
        pos += seglen - 1;
    }
    out
}

/// Ports `gen1.c`: piecewise-linear breakpoints given as explicit
/// `(time, value)` pairs. Times are in arbitrary units and rescaled so
/// the last one lands at `length - closed as usize` (the first is
/// assumed to be `0` - the C never actually enforces that either).
/// `closed`: `true` for the default closed curve, `false` for `-o` (open).
pub fn gen1(length: usize, closed: bool, points: &[(f32, f32)]) -> Vec<f32> {
    assert!(points.len() >= 2, "gen1: need at least two breakpoints");
    let last_t = points.last().unwrap().0;
    let scale = (length - closed as usize) as f32 / last_t;
    let times: Vec<f32> = points.iter().map(|&(t, _)| t * scale).collect();
    let values: Vec<f32> = points.iter().map(|&(_, v)| v).collect();
    let alphas = vec![0.0f32; values.len() - 1];
    gen_segments(length, &times, &values, &alphas)
}

/// Ports `gen3.c`: piecewise-linear breakpoints at evenly spaced times
/// across `[0, length - closed as usize]`, given only their values.
pub fn gen3(length: usize, closed: bool, values: &[f32]) -> Vec<f32> {
    assert!(values.len() >= 2, "gen3: need at least two values");
    let nc = values.len();
    let span = (length - closed as usize) as f32;
    let times: Vec<f32> = (0..nc).map(|i| i as f32 * span / (nc - 1) as f32).collect();
    let alphas = vec![0.0f32; nc - 1];
    gen_segments(length, &times, values, &alphas)
}

/// Ports `gen4.c`: like [`gen1`] but each breakpoint carries its own
/// `trans` transition parameter (`alpha`) for the segment starting there
/// - the last breakpoint's alpha is unused (there's no segment after it).
pub fn gen4(length: usize, closed: bool, points: &[(f32, f32, f32)]) -> Vec<f32> {
    assert!(points.len() >= 2, "gen4: need at least two breakpoints");
    let last_t = points.last().unwrap().0;
    let scale = (length - closed as usize) as f32 / last_t;
    let times: Vec<f32> = points.iter().map(|&(t, _, _)| t * scale).collect();
    let values: Vec<f32> = points.iter().map(|&(_, v, _)| v).collect();
    let alphas: Vec<f32> = points[..points.len() - 1]
        .iter()
        .map(|&(_, _, a)| a)
        .collect();
    gen_segments(length, &times, &values, &alphas)
}

/// Ports `gen2.c`: sums `sin_amps.len()` sine harmonics (harmonic numbers
/// `1..=sin_amps.len()`) and `cos_amps.len()` cosine harmonics (harmonic
/// numbers `0..cos_amps.len()`, so `cos_amps[0]` is a DC offset) over
/// `[0, length)` samples of a `2*pi / (length - closed as usize)`
/// fundamental period. `closed`: `true` for `-c`; gen2 is the one
/// generator in this family whose *default* (`false`) is open, unlike
/// gen1/gen3/gen4/gen5's closed default.
pub fn gen2(length: usize, closed: bool, sin_amps: &[f32], cos_amps: &[f32]) -> Vec<f32> {
    let factor = TWO_PI / (length - closed as usize) as f32;
    let mut out = vec![0.0f32; length];
    for (h, &amp) in sin_amps.iter().enumerate() {
        let harm = (h + 1) as f32;
        for (j, sample) in out.iter_mut().enumerate() {
            let angle = (factor * j as f32 * harm) as f64;
            *sample = (*sample as f64 + amp as f64 * angle.sin()) as f32;
        }
    }
    for (h, &amp) in cos_amps.iter().enumerate() {
        let harm = h as f32;
        for (j, sample) in out.iter_mut().enumerate() {
            let angle = (factor * j as f32 * harm) as f64;
            *sample = (*sample as f64 + amp as f64 * angle.cos()) as f32;
        }
    }
    out
}

/// Ports `gen5.c`: sums arbitrary `(harmonic, amplitude, phase)` triples
/// as `amplitude * sin(j * harmonic * 2*pi / (length - closed as usize) +
/// phase)` over `[0, length)`. `closed`: `true` for the default closed
/// curve, `false` for `-c`... except the C's own default is actually
/// *open* (`closed = 0`) unless `-c` is passed, matching `gen2` rather
/// than `gen1`/`gen3`/`gen4` - kept as an explicit parameter here rather
/// than a hardcoded default either way, precisely because it's easy to
/// mix up which of these five defaults to open vs. closed.
pub fn gen5(length: usize, closed: bool, partials: &[(f32, f32, f32)]) -> Vec<f32> {
    let denom = (length - closed as usize) as f32;
    let mut out = vec![0.0f32; length];
    for &(harm, amp, phase) in partials {
        let factor = harm * TWO_PI / denom;
        for (j, sample) in out.iter_mut().enumerate() {
            let angle = (j as f32 * factor + phase) as f64;
            *sample = (*sample as f64 + amp as f64 * angle.sin()) as f32;
        }
    }
    out
}

/// glibc's default `random()`/`rand()` sequence: the "TYPE_3" nonlinear
/// additive feedback generator (degree 31, separation 3) in the state
/// `rand()` is in when a program never calls `srand()` - as if
/// `srandom(1)` had been called. `gen6.c` never seeds it, so its output
/// is fully deterministic and reproducible bit-for-bit by replicating
/// this exactly, which is what makes an "exact float compare" test
/// meaningful for a *noise* generator at all.
///
/// Ported from glibc's `__srandom_r`/`__random_r` (verified against the
/// real compiled `gen6` binary's output for 40 consecutive values - well
/// past this generator's 31-entry state array - in this module's tests,
/// not reproduced from memory alone).
struct GlibcRandom {
    state: [u32; Self::DEG],
    fptr: usize,
    rptr: usize,
}

impl GlibcRandom {
    const DEG: usize = 31;
    const SEP: usize = 3;

    fn seeded(seed: u32) -> Self {
        let seed = if seed == 0 { 1 } else { seed };
        let mut state = [0u32; Self::DEG];
        state[0] = seed;
        // Park-Miller minimal-standard LCG, glibc's `__srandom_r` state
        // initialization: computed in `int32_t` range with an explicit
        // wraparound add rather than a modulus, matching the C exactly.
        let mut word = seed as i64;
        for s in state.iter_mut().take(Self::DEG).skip(1) {
            let hi = word / 127_773;
            let lo = word % 127_773;
            word = 16_807 * lo - 2836 * hi;
            if word < 0 {
                word += 2_147_483_647;
            }
            *s = word as u32;
        }
        let mut rng = GlibcRandom {
            state,
            fptr: Self::SEP,
            rptr: 0,
        };
        // __srandom_r discards the first `deg * 10` outputs to mix the
        // state before any are used.
        for _ in 0..Self::DEG * 10 {
            rng.next_raw();
        }
        rng
    }

    /// `__random_r`: one raw 31-bit output in `[0, 0x7fffffff]`.
    fn next_raw(&mut self) -> u32 {
        self.state[self.fptr] = self.state[self.fptr].wrapping_add(self.state[self.rptr]);
        let result = (self.state[self.fptr] >> 1) & 0x7fff_ffff;
        self.fptr = (self.fptr + 1) % Self::DEG;
        self.rptr = (self.rptr + 1) % Self::DEG;
        result
    }
}

/// Ports `gen6.c`: `length` uniform-noise samples in `[-1.0, 1.0)`, using
/// the exact `rand()` sequence the C gets by never seeding one.
pub fn gen6(length: usize) -> Vec<f32> {
    let mut rng = GlibcRandom::seeded(1);
    (0..length)
        .map(|_| {
            let r = rng.next_raw();
            // `2.0 * ((float) rand() / (float) 0x7fffffff) - 1.0`: the
            // division happens in float (both operands explicitly cast),
            // but the `2.0 *`/`- 1.0` literals are `double`, promoting
            // the rest of the expression to double precision before it's
            // narrowed back to float on assignment - not float
            // throughout, confirmed to matter by this module's tests.
            let divided = (r as f32 / 0x7fff_ffffu32 as f32) as f64;
            (2.0 * divided - 1.0) as f32
        })
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    // Every expected array below was captured from the real compiled
    // binaries (`cmake --build build --target gen1 gen2 gen3 gen4 gen5`),
    // decoded from their raw float32LE stdout - not hand-derived - per
    // the Task 2.4/2.5 lesson (docs/dev/rust-verification.md) that
    // hand-derived DSP expectations are error-prone.

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen1_closed_matches_c_oracle() {
        // gen1 -L8 0 0 50 1 100 0
        let got = gen1(8, true, &[(0.0, 0.0), (50.0, 1.0), (100.0, 0.0)]);
        let want: [f32; 8] = [
            0.0,
            0.25,
            0.5,
            0.75,
            1.0,
            0.6666666269302368,
            0.3333333134651184,
            0.0,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen1_open_matches_c_oracle() {
        // gen1 -L8 -o 0 0 50 1 100 0
        let got = gen1(8, false, &[(0.0, 0.0), (50.0, 1.0), (100.0, 0.0)]);
        let want: [f32; 8] = [0.0, 0.25, 0.5, 0.75, 1.0, 0.75, 0.5, 0.25];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen3_matches_c_oracle() {
        // gen3 -L8 0 1 0 -1
        let got = gen3(8, true, &[0.0, 1.0, 0.0, -1.0]);
        let want: [f32; 8] = [
            0.0,
            0.5,
            1.0,
            0.6666666269302368,
            0.3333333134651184,
            0.0,
            -0.5,
            -1.0,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen4_exponential_matches_c_oracle() {
        // gen4 -L8 0 0 -3 100 1 0
        let got = gen4(8, true, &[(0.0, 0.0, -3.0), (100.0, 1.0, 0.0)]);
        let want: [f32; 8] = [
            0.0,
            0.36682406067848206,
            0.6057875752449036,
            0.7614577412605286,
            0.8628673553466797,
            0.9289295077323914,
            0.9719650149345398,
            1.0,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen2_matches_c_oracle() {
        // gen2 -L8 1.0 0 1   (one sine harmonic, amplitude 1; na=1)
        let got = gen2(8, false, &[1.0], &[]);
        let want: [f32; 8] = [
            0.0,
            0.7071067690849304,
            1.0,
            0.7071067690849304,
            -8.742277657347586e-08,
            -0.70710688829422,
            -1.0,
            -0.7071065306663513,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen2_with_cosine_harmonics_matches_c_oracle() {
        // gen2 -L8 0.5 1.0 0.3 1   (na=1: sin amp 0.5; cos amps 1.0, 0.3)
        let got = gen2(8, false, &[0.5], &[1.0, 0.3]);
        let want: [f32; 8] = [
            1.2999999523162842,
            1.565685510635376,
            1.5,
            1.1414213180541992,
            0.6999999284744263,
            0.4343145787715912,
            0.5,
            0.8585788011550903,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen4_logarithmic_matches_c_oracle() {
        // gen4 -L8 0 0 3 100 1 0   (positive alpha: logarithmic)
        let got = gen4(8, true, &[(0.0, 0.0, 3.0), (100.0, 1.0, 0.0)]);
        let want: [f32; 8] = [
            0.0,
            0.028035001829266548,
            0.07107049226760864,
            0.1371326893568039,
            0.2385423332452774,
            0.3942125141620636,
            0.6331760287284851,
            1.0000001192092896,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    #[allow(clippy::approx_constant)] // deliberately the literal argv value, not FRAC_PI_4, to replicate double-rounding below
    fn gen5_multiple_partials_with_phase_matches_c_oracle() {
        // gen5 -L8 1 0.5 0.7853981633974483 2 0.25 0
        //
        // The C's `atof` parses this literal as a double, then narrows it
        // to the `float pha` on assignment - replicated here as `as f64
        // as f32` (two roundings) rather than a plain `f32` literal (one
        // rounding directly from the decimal), since those can differ by
        // a ULP.
        let phase = 0.7853981633974483_f64 as f32;
        let got = gen5(8, false, &[(1.0, 0.5, phase), (2.0, 0.25, 0.0)]);
        let want: [f32; 8] = [
            0.3535533845424652,
            0.75,
            0.3535533547401428,
            -0.2500000298023224,
            -0.3535534143447876,
            -0.25,
            -0.35355344414711,
            -0.24999991059303284,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen5_matches_c_oracle() {
        // gen5 -L8 1 1.0 0   (harmonic 1, amplitude 1, phase 0)
        let got = gen5(8, false, &[(1.0, 1.0, 0.0)]);
        let want: [f32; 8] = [
            0.0,
            0.7071067690849304,
            1.0,
            0.7071067690849304,
            -8.742277657347586e-08,
            -0.70710688829422,
            -1.0,
            -0.7071065306663513,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn gen6_matches_c_oracle_across_two_state_array_periods() {
        // gen6 -L40 - deliberately longer than the 31-entry state array,
        // so this also exercises the fptr/rptr wraparound.
        let got = gen6(40);
        let want: [f32; 40] = [
            0.6803754568099976,
            -0.21123415231704712,
            0.566198468208313,
            0.5968800783157349,
            0.8232947587966919,
            -0.6048972606658936,
            -0.3295544981956482,
            0.53645920753479,
            -0.4444505572319031,
            0.1079399585723877,
            -0.045205891132354736,
            0.2577418088912964,
            -0.2704310417175293,
            0.02680182456970215,
            0.9044594764709473,
            0.8323901891708374,
            0.27142345905303955,
            0.43459391593933105,
            -0.7167948484420776,
            0.21393775939941406,
            -0.9673988819122314,
            -0.5142264366149902,
            -0.7255368232727051,
            0.6083534955978394,
            -0.6866418123245239,
            -0.19811123609542847,
            -0.7404191493988037,
            -0.7823823690414429,
            0.9978489875793457,
            -0.5634862184524536,
            0.025864839553833008,
            0.6782244443893433,
            0.22527968883514404,
            -0.4079367518424988,
            0.2751045227050781,
            0.04857432842254639,
            -0.012834012508392334,
            0.9455500841140747,
            -0.4149664044380188,
            0.5427154302597046,
        ];
        assert_eq!(got, want);
    }

    #[test]
    fn gen6_is_deterministic_across_calls() {
        // gen6.c never seeds rand(), so every process run gets the same
        // sequence - confirmed directly against the real binary (two
        // separate `gen6 -L4` runs produced identical output).
        assert_eq!(gen6(10), gen6(10));
    }

    #[test]
    fn trans_linear_is_evenly_spaced() {
        assert_eq!(trans(0.0, 0.0, 4.0, 5), vec![0.0, 1.0, 2.0, 3.0, 4.0]);
    }

    #[test]
    fn trans_endpoints_stay_close_to_a_and_b_regardless_of_alpha() {
        // Not bit-exact: confirmed against `gen4 -L6 0 2 5 100 9 0` (real
        // binary) that even the C itself lands on `9.000000953674316` at
        // alpha=5, not exactly `9.0` - floating-point round-off in
        // `exp`/the reciprocal `denom`, not a porting bug. An earlier
        // draft of this test wrongly assumed bit-exact endpoints.
        for alpha in [-5.0, -1.0, 1.0, 5.0] {
            let seg = trans(2.0, alpha, 9.0, 6);
            assert!((seg[0] - 2.0).abs() < 1e-4, "alpha={alpha}: {}", seg[0]);
            assert!(
                (*seg.last().unwrap() - 9.0).abs() < 1e-4,
                "alpha={alpha}: {}",
                seg.last().unwrap()
            );
        }
    }

    #[test]
    #[allow(clippy::excessive_precision)] // deliberately transcribed at full C float32 precision
    fn trans_alpha5_matches_c_oracle() {
        // gen4 -L6 0 2 5 100 9 0
        let got = trans(2.0, 5.0, 9.0, 6);
        let want: [f32; 6] = [
            2.0,
            2.0815935134887695,
            2.3033881187438965,
            2.906287908554077,
            4.545139789581299,
            9.000000953674316,
        ];
        assert_eq!(got, want);
    }

    #[test]
    #[should_panic(expected = "transition length must be >= 2")]
    fn trans_rejects_too_short_a_run() {
        trans(0.0, 0.0, 1.0, 1);
    }
}
