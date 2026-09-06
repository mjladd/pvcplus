//! Shared primitives for the `.fr` frequency-response-file family
//! (`freqresponse`, `filtresponsemaker`, `chordresponsemaker`, consumed
//! by `filter`): `legacy/pvc_lib/OPPC_to_Hz.c` and `normalize.c`.
//!
//! A response is represented the same way as an analysis frame
//! ([`crate::pvoc::Frame`]): `n2 + 1` (magnitude, frequency-in-Hz) pairs.
//! The legacy `.fr` file is just that array's `n + 2` floats written
//! raw, with no header at all - not even `N` - so a reader must already
//! know the FFT size from context (matching how every legacy tool that
//! reads one takes an explicit `-N` alongside `-F<response file>`).

/// Ports `OPPC_to_Hz()`: converts an "octave.pitchclass" pitch
/// (`8.00` = middle C, `.01` per pitch class, i.e. semitone) to Hz.
/// Panics on an invalid pitch-class fraction (`> .12`), matching the
/// C's own `exit(0)` there - this is a data-file parsing error, not a
/// runtime condition callers should recover from.
pub fn oppc_to_hz(octave_point_pitch_class: f32) -> f32 {
    // `midC = (220. * pow(2., (3./12.)))`: computed and narrowed to
    // `float` once, in the C's `static float midC` - double precision
    // throughout the RHS, narrowed only at the final assignment.
    let mid_c = (220.0 * 2.0f64.powf(3.0 / 12.0)) as f32;
    let integer = octave_point_pitch_class.trunc();
    let fraction = octave_point_pitch_class - integer;
    assert!(
        fraction <= 0.12,
        "invalid pitchclass in octave.pitchclass: {octave_point_pitch_class}"
    );
    let temp = (12.0 * (integer - 8.0) + 100.0 * fraction) / 12.0;
    mid_c * 2.0f64.powf(temp as f64) as f32
}

/// Ports `Hz_to_OPPC()` (`legacy/pvc_lib/Hz_to_OPPC.c`), `oppc_to_hz`'s
/// inverse: Hz to an "octave.pitchclass" pitch. Unlike `oppc_to_hz`, the
/// fractional part isn't `.01` per pitch class - it's `.12 *` the raw
/// fractional octave, so a value landing just under an integer octave
/// (a float-precision artifact, not a real semitone) reads as `.11xxxx`
/// pitch classes into the *previous* octave rather than rounding up -
/// confirmed against the real compiled `Hztopitch` binary below, not
/// just algebraically: `Hz_to_OPPC(261.625)` (a hair below middle C's
/// exact 261.62558) is `7.119999`, not `8.00`. Reproduced exactly, not
/// smoothed over - `pvc pitchtrack`'s `octave-pitchclass` output format
/// depends on this same quirk.
pub fn hz_to_oppc(hz: f32) -> f32 {
    let mid_c = (220.0 * 2.0f64.powf(3.0 / 12.0)) as f32;
    let temp2 = 8.0 + (hz / mid_c).log10() / 2.0f32.log10();
    let temp1 = temp2.trunc();
    temp1 + 0.12 * (temp2 - temp1)
}

/// Ports `normalize()`: scales every bin's amplitude so the peak becomes
/// `1.0`, clamping anything that would exceed `1.0` after scaling
/// (possible when `peakamp` is itself less than the true peak, e.g. a
/// stale value from before some later modification). Returns whether any
/// bin was clamped.
pub fn normalize_spectrum(bins: &mut [(f32, f32)], peakamp: f32) -> bool {
    let mut clamped = false;
    for (mag, _freq) in bins.iter_mut() {
        *mag /= peakamp;
        if *mag > 1.0 {
            *mag = 1.0;
            clamped = true;
        }
    }
    clamped
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[allow(clippy::excessive_precision)]
    fn oppc_to_hz_matches_c_oracle() {
        // 8.00 = middle C.
        assert_eq!(oppc_to_hz(8.00), 261.62558);
        assert_eq!(oppc_to_hz(8.09), 440.000397);
        assert_eq!(oppc_to_hz(7.00), 130.81279);
        assert_eq!(oppc_to_hz(9.00), 523.25116);
        assert_eq!(oppc_to_hz(8.03), 311.126526);
        assert_eq!(oppc_to_hz(0.00), 1.02197492);
    }

    #[test]
    #[allow(clippy::excessive_precision)]
    fn hz_to_oppc_matches_c_oracle() {
        // `/tmp/pvcbuild/pvc_src/Hztopitch 261.625 440.0 130.81`
        assert_eq!(hz_to_oppc(261.625), 7.1199994);
        assert_eq!(hz_to_oppc(440.0), 8.09);
        assert_eq!(hz_to_oppc(130.81), 6.1199965);
        // Exact middle C - lands squarely on the octave boundary, unlike
        // the case above.
        assert_eq!(hz_to_oppc(261.62558), 8.0);
    }

    #[test]
    fn normalize_spectrum_scales_and_clamps() {
        let mut bins = vec![(0.5, 100.0), (2.0, 200.0), (1.0, 300.0)];
        let clamped = normalize_spectrum(&mut bins, 1.0);
        assert!(clamped);
        assert_eq!(bins, vec![(0.5, 100.0), (1.0, 200.0), (1.0, 300.0)]);
    }
}
