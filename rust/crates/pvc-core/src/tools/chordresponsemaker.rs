//! Ports `chordresponsemaker.c`: synthesizes a `.fr` frequency-response
//! spectrum as a stack of harmonic-partial tones, each with a
//! triangular- or rectangular-windowed dB rolloff around its center
//! frequency, from unordered sextuples `(pitch-or-Hz, num_partials,
//! bandwidth, decibels, partial_spacing, db_rolloff_per_octave)`.
//!
//! Not ported: `-a`/`-v` (decibels-spectrum-plot-file / stderr
//! printout) - pure reporting.

use crate::pvoc::Frame;
use crate::response::{normalize_spectrum, oppc_to_hz};

/// `spectmethod`: how overlapping partial windows combine at a bin.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Accumulation {
    Peak,
    Sum,
}

/// `window_t`: the dB rolloff shape around each partial.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BandWindow {
    Triangle,
    Rectangle,
}

/// One chord tone (`legacy`: a sextuple in the data file).
#[derive(Debug, Clone, Copy)]
pub struct ChordTone {
    /// `<= 12.0` is octave.pitchclass (see [`oppc_to_hz`]), else Hz.
    pub pitch_or_hz: f32,
    /// `0` means "as many as fit below Nyquist, at `partial_spacing`
    /// apart" - resolved before use.
    pub num_partials: i32,
    /// `> 1.0` is Hz directly; otherwise a proportion of the
    /// fundamental frequency.
    pub bandwidth: f32,
    pub db: f32,
    /// Spacing between successive partials, as a proportion of the
    /// fundamental (harmonic partials use `1.0`).
    pub partial_spacing: f32,
    pub db_rolloff_per_octave: f32,
}

/// Synthesizes the frequency-response [`Frame`] for FFT size `fft_size`
/// at sample rate `sample_rate`.
pub fn synthesize(
    tones: &[ChordTone],
    fft_size: usize,
    sample_rate: u32,
    accumulation: Accumulation,
    band_window: BandWindow,
    invert: bool,
) -> Frame {
    let n = fft_size;
    let r = sample_rate as f32;
    let freqdiff = r / n as f32;
    let nyquist = r / 2.0;
    let db_edge = -96.0f32;

    let n_plus_2 = n + 2;
    let mut amp = vec![0.0f32; n_plus_2 / 2]; // amp[j] for bin j (freq j*freqdiff)

    for tone in tones {
        let fundfreq = if tone.pitch_or_hz <= 12.0 {
            oppc_to_hz(tone.pitch_or_hz)
        } else {
            tone.pitch_or_hz
        };
        let bw = if tone.bandwidth > 1.0 {
            tone.bandwidth
        } else {
            fundfreq * tone.bandwidth * 0.5
        };
        let num_partials = if tone.num_partials <= 0 {
            ((nyquist - fundfreq) / (fundfreq * tone.partial_spacing)) as i32
        } else {
            tone.num_partials
        };
        let db_rolloff = tone.db_rolloff_per_octave;

        for k in 1..=num_partials {
            let partfreq = fundfreq + fundfreq * tone.partial_spacing * (k - 1) as f32;
            // Legacy `i1`/`i2`/`ipartial` are `1 + 2*bin`, i.e. odd
            // "frequency slot" indices into the flat N+2 array; dividing
            // by 2 here recovers the plain bin index this port's `amp[]`
            // uses instead.
            let i1 = ((partfreq - bw) / freqdiff).round() as i64;
            let i2 = ((partfreq + bw) / freqdiff).round() as i64;
            let ipartial = (partfreq / freqdiff).round() as i64;

            let octroll = ((partfreq / fundfreq) as f64).log10() / (2.0f64).log10();
            let octroll = octroll as f32;
            let partdb = (tone.db + octroll * db_rolloff).max(-96.0);

            let below_bins = (ipartial - i1) as f32;
            let dbdown1 = if below_bins == 0.0 || band_window == BandWindow::Rectangle {
                0.0
            } else {
                (db_edge - partdb) / below_bins
            };
            let above_bins = (i2 - ipartial) as f32;
            let dbdown2 = if above_bins == 0.0 || band_window == BandWindow::Rectangle {
                0.0
            } else {
                (db_edge - partdb) / above_bins
            };

            // The C's bound is `(flat_index > 0) && (flat_index < N)` on
            // the `1 + 2*bin` flat index; translated to a plain bin
            // index that's `bin >= 0 && bin < N/2` (`flat_index > 0`
            // holds for every `bin >= 0`, not just `bin > 0`).
            let mut accumulate = |bin: i64, db: f32| {
                if bin >= 0 && bin < (n / 2) as i64 {
                    let amp_val = 10.0f64.powf(db as f64 / 20.0) as f32;
                    let slot = &mut amp[bin as usize];
                    match accumulation {
                        Accumulation::Peak => {
                            if amp_val > *slot {
                                *slot = amp_val;
                            }
                        }
                        Accumulation::Sum => *slot += amp_val,
                    }
                }
            };

            accumulate(ipartial, partdb);
            let mut i = i1;
            while i < ipartial {
                let db =
                    (tone.db + octroll * db_rolloff + (ipartial - i) as f32 * dbdown1).max(-96.0);
                accumulate(i, db);
                i += 1;
            }
            let mut i = i2;
            while i > ipartial {
                let db =
                    (tone.db + octroll * db_rolloff + (i - ipartial) as f32 * dbdown2).max(-96.0);
                accumulate(i, db);
                i -= 1;
            }
        }
    }

    let mut bins: Vec<(f32, f32)> = amp
        .iter()
        .enumerate()
        .map(|(j, &a)| (a, j as f32 * freqdiff))
        .collect();

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
    fn single_tone_produces_a_peak_at_its_fundamental() {
        let tones = [ChordTone {
            pitch_or_hz: 220.0,
            num_partials: 8,
            bandwidth: 0.05,
            db: 0.0,
            partial_spacing: 1.0,
            db_rolloff_per_octave: -3.0,
        }];
        let frame = synthesize(
            &tones,
            1024,
            44100,
            Accumulation::Peak,
            BandWindow::Triangle,
            false,
        );
        assert_eq!(frame.bins.len(), 513);
        let peak = frame.bins.iter().map(|&(m, _)| m).fold(0.0f32, f32::max);
        assert!((peak - 1.0).abs() < 1e-6);
    }
}
