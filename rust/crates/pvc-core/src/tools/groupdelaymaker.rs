//! Ports `groupdelaymaker.c`: synthesizes a response file of `(amp,
//! delay-time)` pairs - the same binary layout as a `.fr` file, but the
//! "frequency" slot holds a per-bin *delay time in seconds* instead,
//! consumed elsewhere by a delay-line tool (not yet ported) rather than
//! `pvc filter`'s own frequency-shaped gain. Its own `usage()` text
//! describes it as `chordresponsemaker` plus a time-delay column, but
//! the two tools' dB-rolloff and edge-falloff formulas are genuinely
//! different - confirmed by reading both files side by side, not
//! assumed from that description - so this port writes its own
//! synthesis loop rather than reusing `tools::chordresponsemaker`'s.
//!
//! **The dB rolloff is linear in partial index, not in frequency
//! ratio.** `chordresponsemaker.c` computes each partial's own rolloff
//! fresh from `log2(partial_freq / fundamental_freq)`; `groupdelaymaker.c`
//! instead computes one `dBrolloff = db_rolloff_total / (num_partials -
//! 1)` per tone (or `db_rolloff_total` itself, for a single partial), then
//! applies `dBrolloff * (k - 1)` for partial `k` - a straight-line
//! decibel ramp across partial *count*, unrelated to how far apart in
//! Hz the partials actually land (relevant when `partial_spacing != 1.0`,
//! i.e. non-harmonic partials).
//!
//! **The edge falloff (`-D`) is a plain relative offset, not a fixed
//! floor.** `chordresponsemaker.c`'s own edge taper interpolates down to
//! a fixed `-96dB` floor at the band's outer edge; `groupdelaymaker.c`'s
//! `-D` (`db_edge`) is added *relative to the partial's own dB*, split
//! evenly per bin across the band's half-width - `0` (the default) makes
//! the band perfectly flat (no taper at all, effectively rectangular),
//! not silent at the edges.
//!
//! Not ported: the commented-out band-reject/invert block (`invert_flag`
//! is declared nowhere else in the file - dead code, confirmed by
//! grepping) and `-v` (a stderr spectrum printout).

use crate::response::{normalize_spectrum, oppc_to_hz};

/// `spectmethod`: how overlapping partial windows resolve their `(amp,
/// delay)` pair at a shared bin.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OverlapMethod {
    ShortestDelay,
    LongestDelay,
    Average,
    Loudest,
    Softest,
    LoudestIfShortest,
    LoudestIfLongest,
}

/// One tone (`legacy`: a septuple in the data file).
#[derive(Debug, Clone, Copy)]
pub struct GroupDelayTone {
    /// `<= 12.0` is octave.pitchclass (see [`oppc_to_hz`]), else Hz.
    pub pitch_or_hz: f32,
    /// `<= 0` means "as many as fit below Nyquist, at `partial_spacing`
    /// apart" - resolved before use.
    pub num_partials: i32,
    /// `> 1.0` is Hz directly; otherwise a proportion of the
    /// fundamental frequency.
    pub bandwidth: f32,
    pub db: f32,
    /// Spacing between successive partials, as a proportion of the
    /// fundamental (harmonic partials use `1.0`).
    pub partial_spacing: f32,
    /// Total dB drop from the first partial to the last one.
    pub db_rolloff_total: f32,
    pub delay_secs: f32,
}

/// Resolves one bin's `(amp, delay)` pair against a candidate value from
/// a new tone/partial, per `method` - ports `value_from_methods()`.
/// `touched` is an explicit "was this bin ever written before" flag,
/// standing in for the C's own `-1.0` sentinel value in both `slot`
/// fields (an explicit flag is exact regardless of what value a real
/// tone might legitimately produce, where `-1.0` - never actually
/// reachable for either an amplitude or a delay time here - is exact
/// only by the data's own good behavior).
fn resolve_overlap(
    slot: &mut (f32, f32),
    touched: &mut bool,
    method: OverlapMethod,
    amp: f32,
    delay: f32,
) {
    if !*touched {
        *slot = (amp, delay);
        *touched = true;
        return;
    }
    let (old_amp, old_delay) = *slot;
    let use_new = match method {
        OverlapMethod::ShortestDelay => delay < old_delay,
        OverlapMethod::LongestDelay => delay > old_delay,
        OverlapMethod::Average => {
            *slot = ((amp + old_amp) * 0.5, (delay + old_delay) * 0.5);
            return;
        }
        OverlapMethod::Loudest => amp > old_amp,
        OverlapMethod::Softest => amp < old_amp,
        OverlapMethod::LoudestIfShortest => amp > old_amp && delay < old_delay,
        OverlapMethod::LoudestIfLongest => amp > old_amp && delay > old_delay,
    };
    if use_new {
        *slot = (amp, delay);
    }
}

/// Synthesizes the `(amp, delay-seconds)` response for FFT size
/// `fft_size` at sample rate `sample_rate`, `n2 + 1` pairs long.
/// `db_edge` is `-D` (relative dB offset at each partial band's outer
/// edge; `0.0` is a flat/rectangular band). `default_db`/
/// `default_delay_secs` are `-i`/`-I`, applied to every bin no tone ever
/// touches.
pub fn synthesize(
    tones: &[GroupDelayTone],
    fft_size: usize,
    sample_rate: u32,
    db_edge: f32,
    default_db: f32,
    default_delay_secs: f32,
    method: OverlapMethod,
) -> Vec<(f32, f32)> {
    let n = fft_size;
    let n2 = n / 2;
    let r = sample_rate as f32;
    let freqdiff = r / n as f32;
    let nyquist = r / 2.0;

    let mut bins = vec![(0.0f32, 0.0f32); n2 + 1];
    let mut touched = vec![false; n2 + 1];

    for tone in tones {
        let fundfreq = if tone.pitch_or_hz <= 12.0 {
            oppc_to_hz(tone.pitch_or_hz)
        } else {
            tone.pitch_or_hz
        };
        let bw = if tone.bandwidth <= 1.0 {
            fundfreq * tone.bandwidth * 0.5
        } else {
            tone.bandwidth * 0.5
        };
        let num_partials = if tone.num_partials <= 0 {
            ((nyquist - fundfreq) / (fundfreq * tone.partial_spacing)) as i32
        } else {
            tone.num_partials
        };
        let db_rolloff = if num_partials > 1 {
            tone.db_rolloff_total / (num_partials - 1) as f32
        } else {
            tone.db_rolloff_total
        };

        for k in 1..=num_partials {
            let partfreq = fundfreq + fundfreq * tone.partial_spacing * (k - 1) as f32;
            let i1_bin = ((partfreq - bw) / freqdiff).round() as i64;
            let i2_bin = ((partfreq + bw) / freqdiff).round() as i64;
            let ipartial_bin = (partfreq / freqdiff).round() as i64;

            let below_bins = (ipartial_bin - i1_bin) as f32;
            let dbdown1 = if below_bins == 0.0 {
                0.0
            } else {
                db_edge / below_bins
            };
            let above_bins = (i2_bin - ipartial_bin) as f32;
            let dbdown2 = if above_bins == 0.0 {
                0.0
            } else {
                db_edge / above_bins
            };

            let base_db = tone.db + db_rolloff * (k - 1) as f32;

            // The C's bound is `(flat_index > 0) && (flat_index < N)` on
            // the `1 + 2*bin` flat index - equivalent to `0 <= bin <
            // N/2`, excluding the Nyquist bin (see this module's doc
            // comment / `tools::chordresponsemaker`'s own precedent for
            // the same translation).
            let mut accumulate = |bin: i64, db: f32| {
                if (0..n2 as i64).contains(&bin) {
                    let amp = 10.0f64.powf(db as f64 / 20.0) as f32;
                    let idx = bin as usize;
                    resolve_overlap(
                        &mut bins[idx],
                        &mut touched[idx],
                        method,
                        amp,
                        tone.delay_secs,
                    );
                }
            };

            accumulate(ipartial_bin, base_db);
            let mut i = i1_bin;
            while i < ipartial_bin {
                let db = base_db + (ipartial_bin - i) as f32 * dbdown1;
                accumulate(i, db);
                i += 1;
            }
            let mut i = i2_bin;
            while i > ipartial_bin {
                let db = base_db + (i - ipartial_bin) as f32 * dbdown2;
                accumulate(i, db);
                i -= 1;
            }
        }
    }

    let default_amp = 10.0f64.powf(default_db as f64 / 20.0) as f32;
    for (i, slot) in bins.iter_mut().enumerate() {
        if !touched[i] {
            *slot = (default_amp, default_delay_secs);
        }
    }

    let peak = bins.iter().map(|&(a, _)| a).fold(0.0f32, f32::max);
    normalize_spectrum(&mut bins, peak);
    bins
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn untouched_bins_get_the_default_amp_and_delay() {
        // With no tones at all, every bin is untouched and gets the same
        // default amp - normalization then rescales that single shared
        // value to exactly 1.0 (a flat, full-scale response).
        let bins = synthesize(&[], 1024, 44100, 0.0, -6.0, 0.25, OverlapMethod::Average);
        for &(amp, delay) in &bins {
            assert!(
                (amp - 1.0).abs() < 1e-6,
                "amp {amp} should normalize to 1.0"
            );
            assert!((delay - 0.25).abs() < 1e-6);
        }
    }

    #[test]
    fn a_single_tone_writes_its_own_delay_at_its_bin() {
        let tones = [GroupDelayTone {
            pitch_or_hz: 440.0,
            num_partials: 1,
            bandwidth: 0.0,
            db: 0.0,
            partial_spacing: 1.0,
            db_rolloff_total: 0.0,
            delay_secs: 0.5,
        }];
        let bins = synthesize(
            &tones,
            1024,
            44100,
            0.0,
            -96.0,
            0.0,
            OverlapMethod::ShortestDelay,
        );
        let freqdiff = 44100.0 / 1024.0;
        let bin = (440.0f32 / freqdiff).round() as usize;
        assert!((bins[bin].1 - 0.5).abs() < 1e-6);
        assert!(bins[bin].0 > 0.9); // this tone's own peak bin
    }

    #[test]
    fn shortest_delay_method_prefers_the_smaller_delay_on_overlap() {
        let tones = [
            GroupDelayTone {
                pitch_or_hz: 440.0,
                num_partials: 1,
                bandwidth: 0.2,
                db: 0.0,
                partial_spacing: 1.0,
                db_rolloff_total: 0.0,
                delay_secs: 1.0,
            },
            GroupDelayTone {
                pitch_or_hz: 440.0,
                num_partials: 1,
                bandwidth: 0.2,
                db: -3.0,
                partial_spacing: 1.0,
                db_rolloff_total: 0.0,
                delay_secs: 0.1,
            },
        ];
        let bins = synthesize(
            &tones,
            1024,
            44100,
            0.0,
            -96.0,
            0.0,
            OverlapMethod::ShortestDelay,
        );
        let freqdiff = 44100.0 / 1024.0;
        let bin = (440.0f32 / freqdiff).round() as usize;
        assert!((bins[bin].1 - 0.1).abs() < 1e-6);
    }
}
