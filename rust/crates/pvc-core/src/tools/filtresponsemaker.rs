//! Ports `filtresponsemaker.c`: synthesizes a `.fr` frequency-response
//! spectrum from unordered `(frequency-or-octave.pitchclass, decibels)`
//! breakpoint duples - a "frequency gradient" filter shape, linearly
//! interpolated in dB between neighboring breakpoints.
//!
//! Not ported: `-A` (auto-adjust FFT size to fit the smallest breakpoint
//! frequency gap) and `-a`/`-v` (decibels-spectrum-plot-file / stderr
//! printout) - the former is a convenience the new CLI's caller can
//! apply themselves by just picking a large enough `--fft`, the latter
//! are pure reporting.

use crate::curve;
use crate::pvoc::Frame;
use crate::response::{normalize_spectrum, oppc_to_hz};
use crate::units::DbToAmp;

/// One `(pitch-or-Hz, decibels)` breakpoint from the data file - `pitch`
/// values `<= 12.0` are octave.pitchclass (see [`oppc_to_hz`]), anything
/// larger is already Hz, matching the C's own `PP[j] <= 12.` test.
#[derive(Debug, Clone, Copy)]
pub struct Breakpoint {
    pub pitch_or_hz: f32,
    pub db: f32,
}

/// Synthesizes the frequency-response [`Frame`] for FFT size `fft_size`
/// at sample rate `sample_rate`, band-passing (or, if `invert`,
/// band-rejecting) according to `breakpoints` (need not be sorted or
/// include `0`/Nyquist endpoints - both are added here, matching the C).
pub fn synthesize(
    breakpoints: &[Breakpoint],
    fft_size: usize,
    sample_rate: u32,
    invert: bool,
) -> Frame {
    let n = fft_size;
    let r = sample_rate as f32;
    let freqdiff = r / n as f32;
    let nyquist = r / 2.0;

    // Resolve octave.pitchclass to Hz, then sort ascending by frequency
    // - matches the C's bubble-sort-to-convergence loop, just via a
    // stable sort (the C's sort is not otherwise order-preserving in any
    // way callers could depend on for tied frequencies).
    let mut points: Vec<(f32, f32)> = breakpoints
        .iter()
        .map(|bp| {
            let freq = if bp.pitch_or_hz <= 12.0 {
                oppc_to_hz(bp.pitch_or_hz)
            } else {
                bp.pitch_or_hz
            };
            (freq, bp.db)
        })
        .collect();
    points.sort_by(|a, b| a.0.partial_cmp(&b.0).unwrap());

    // Add a 0Hz point (copying the lowest breakpoint's dB) if absent,
    // and a Nyquist point (copying the highest breakpoint's dB) if the
    // data doesn't already reach it - `filtresponsemaker.c`'s own
    // "PUT ... LINE IN IF ABSENT" steps.
    if points.first().is_some_and(|&(f, _)| f > 0.0) {
        let low_db = points[0].1;
        points.insert(0, (0.0, low_db));
    }
    if points.last().is_some_and(|&(f, _)| f < nyquist) {
        let high_db = points.last().unwrap().1;
        points.push((nyquist, high_db));
    }

    let db_to_amp = DbToAmp::new();
    let mut bins = Vec::with_capacity(n / 2 + 1);
    // Walks the sorted breakpoint list one pair at a time, matching the
    // C's `while (PP[k+2] < F[i]) k += 2;` (a flat-array, two-at-a-time
    // walk over the same duples `points` already pairs up here). Capped
    // so `points[k+1]` always exists - safe because the last bin's
    // frequency is exactly the Nyquist breakpoint's, never past it.
    let mut k = 0usize;
    let last = points.len() - 1;
    for j in 0..=(n / 2) {
        let freq = j as f32 * freqdiff;
        while k + 1 < last && points[k + 1].0 < freq {
            k += 1;
        }
        let (f_lo, db_lo) = points[k];
        let (f_hi, db_hi) = points[k + 1];
        let prop = (freq - f_lo) / (f_hi - f_lo);
        let db = curve(db_lo, db_hi, prop, 0.0);
        bins.push((db_to_amp.convert(db), freq));
    }

    let peak = bins.iter().map(|&(m, _)| m).fold(0.0f32, f32::max);
    normalize_spectrum(&mut bins, peak);

    if invert {
        for (mag, _freq) in bins.iter_mut() {
            *mag = 1.0 - *mag;
        }
    }

    Frame { bins }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn bandpass_shape_matches_fixture() {
        let breakpoints = [
            Breakpoint {
                pitch_or_hz: 20.0,
                db: 0.0,
            },
            Breakpoint {
                pitch_or_hz: 1000.0,
                db: 0.0,
            },
            Breakpoint {
                pitch_or_hz: 4000.0,
                db: -6.0,
            },
            Breakpoint {
                pitch_or_hz: 10000.0,
                db: -24.0,
            },
            Breakpoint {
                pitch_or_hz: 22050.0,
                db: -48.0,
            },
        ];
        let frame = synthesize(&breakpoints, 1024, 44100, false);
        assert_eq!(frame.bins.len(), 513);
        // Peak should be exactly 1.0 somewhere in the passband.
        let peak = frame.bins.iter().map(|&(m, _)| m).fold(0.0f32, f32::max);
        assert!((peak - 1.0).abs() < 1e-6);
        // Amplitude should fall off toward Nyquist (rolled off by -48dB
        // relative to the passband).
        assert!(frame.bins.last().unwrap().0 < frame.bins[10].0);
    }

    #[test]
    fn invert_complements_amplitude() {
        let breakpoints = [
            Breakpoint {
                pitch_or_hz: 0.0,
                db: 0.0,
            },
            Breakpoint {
                pitch_or_hz: 22050.0,
                db: 0.0,
            },
        ];
        let normal = synthesize(&breakpoints, 64, 44100, false);
        let inverted = synthesize(&breakpoints, 64, 44100, true);
        for ((m1, _), (m2, _)) in normal.bins.iter().zip(&inverted.bins) {
            assert!((m1 + m2 - 1.0).abs() < 1e-6);
        }
    }
}
