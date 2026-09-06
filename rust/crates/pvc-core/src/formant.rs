//! Formant (spectral peak) detection and formant-band gain shaping,
//! ported from `legacy/pvc_lib/getFormants.c` and
//! `NormalizeToPeaksOfSpectrumInBand.c` - used by `freqresponse` to
//! smooth its accumulated spectrum, find its formant peaks, and
//! optionally normalize/compand the gain between them.
//!
//! Not ported from `getFormants.c` (confirmed dead for every actual
//! caller - `freqresponse.c` never sets the one flag that would make
//! them matter):
//! - The "FREQ STASIS" block (`freqStasis[]`, its std-dev threshold, the
//!   `-12`-curved distribution) - only ever consumed by
//!   `freqStasisPlotFile` (write) and `CorrelateWithFreqStasisFlag`
//!   (filter), and `freqresponse.c` never sets either (no flag exists
//!   for them in its `crack()` string).
//! - The "FIND SYMMETRIES" block (`symmetryFactor[]`) - computed but
//!   never read again afterward anywhere in the function.
//! - A commented-out earlier formant-finding algorithm, superseded by
//!   the one actually used.
//! - Six `fopen("/tmp/...", "w+")` "secret plot" debug writes to fixed
//!   paths with no CLI flag controlling them at all.

use crate::pvoc::Frame;
use crate::units::{amp_to_db, DbToAmp};

const HAMMING_WINDOW_SIZE: usize = 1024;
const SMOOTHING_WINDOW_AS_PROPORTION_OF_CENTROID: f32 = 1.0 / 50.0;
const FORMANT_OVERLAP_TOLERANCE_PROP: f32 = 0.5;

fn hamming_window(size: usize) -> Vec<f32> {
    let twopi = std::f32::consts::TAU;
    (0..size)
        .map(|i| 0.54 - 0.46 * (twopi * i as f32 / (size as f32 - 1.0)).cos())
        .collect()
}

#[derive(Debug, Clone, Copy)]
pub struct FormantParams {
    pub low_freq_limit: f32,
    pub high_freq_limit: f32,
    pub minimum_formant_db: f32,
    pub formant_selection_threshold: f32,
}

#[derive(Debug, Clone, Copy)]
pub struct Formant {
    pub center_freq: f32,
    pub amp: f32,
    pub bw: f32,
    pub q: f32,
    pub index: usize,
    pub low_stopband_index: usize,
    pub high_stopband_index: usize,
}

/// Ports `get_formants()`. `nyquist` is `R/2`; the FFT size is recovered
/// from `frame.bins.len()` (`N2 + 1` bins).
pub fn get_formants(frame: &Frame, nyquist: f32, params: &FormantParams) -> Vec<Formant> {
    let n2 = frame.bins.len();
    let hamming = hamming_window(HAMMING_WINDOW_SIZE);

    let mut amps: Vec<f32> = frame.bins.iter().map(|&(m, _)| m).collect();
    let freqs: Vec<f32> = frame.bins.iter().map(|&(_, f)| f).collect();

    let peak_amp = amps.iter().copied().fold(f32::MIN, f32::max);
    // Only normalizes when the peak is quiet - a real conditional in the
    // C, not an edge case: a peak >= 1.0 is left completely alone.
    if peak_amp < 1.0 {
        for a in &mut amps {
            *a /= peak_amp;
        }
    }
    let mut new_amps = amps.clone();

    let mut centroid_freq = 0.0f32;
    let mut sum_of_amps = 0.0f32;
    for i in 0..n2 {
        centroid_freq += freqs[i] * new_amps[i];
        sum_of_amps += new_amps[i];
    }
    centroid_freq /= sum_of_amps;

    let window_bw = centroid_freq * SMOOTHING_WINDOW_AS_PROPORTION_OF_CENTROID;

    let mut freqs_diff_sum = 0.0f32;
    for i in 1..n2 {
        freqs_diff_sum += freqs[i] - freqs[i - 1];
    }
    let avg_bin_freq_diff = freqs_diff_sum / (n2 - 1) as f32;

    let mut avg_win_size = (window_bw / avg_bin_freq_diff) as i64;
    if avg_win_size % 2 == 0 {
        avg_win_size += 1;
    }
    if avg_win_size < 3 {
        avg_win_size = 3;
    }
    let avg_win_size = avg_win_size as usize;

    // Hamming-weighted local moving average, edge-clamped.
    let mut win_array = vec![0.0f32; avg_win_size];
    let mut peak_amp_smoothed = f32::MIN;
    for i in 0..n2 {
        for (n, w) in win_array.iter_mut().enumerate() {
            let k = (n as i64 + i as i64 - (avg_win_size / 2) as i64).clamp(0, n2 as i64 - 1);
            *w = new_amps[k as usize];
        }
        let step = (HAMMING_WINDOW_SIZE - 1) as f32 / (avg_win_size - 1) as f32;
        let mut hamming_float_index = 0.0f32;
        for w in win_array.iter_mut() {
            *w *= hamming[(hamming_float_index + 0.5) as usize];
            hamming_float_index += step;
        }
        let vv: f32 = win_array.iter().sum::<f32>() / avg_win_size as f32;
        new_amps[i] = vv;
        if vv > peak_amp_smoothed {
            peak_amp_smoothed = vv;
        }
    }
    for a in &mut new_amps {
        *a /= peak_amp_smoothed;
    }

    // Per-bin peak/average amplitude within a `centroidFreq*0.75`-wide
    // Hamming-weighted band, used only for `diffdBs`'s formant-threshold
    // test below.
    //
    // `fundamental = (nyquist * 2.) / (float) N` in the C - and `N` here
    // is get_formants' *own* parameter, which every real caller passes
    // as `Nplus2` (`N_actual + 2`), not `N_actual`. So this is really
    // `R / (N_actual + 2)`, not the true fundamental `R / N_actual` -
    // presumably a bug (confusing which "N" convention applies, the
    // exact kind of mixup this whole codebase's various N/N+2 quirks
    // keep producing), reproduced faithfully rather than corrected. In
    // terms of `n2` (`N_actual/2 + 1`, this module's bin count),
    // `N_actual + 2 = 2 * n2`, so this is `nyquist / n2`.
    let fundamental = nyquist / n2 as f32;
    let half_band = centroid_freq * 0.75;
    let mut diff_dbs = vec![0.0f32; n2];
    for (i, diff_db) in diff_dbs.iter_mut().enumerate() {
        let cf = fundamental * i as f32;
        let low_index = (0.5 + (cf - half_band) / fundamental) as i64;
        let high_index = (0.5 + (cf + half_band) / fundamental) as i64;
        let mut amp_sum = 0.0f32;
        let mut hamming_sum = 0.0f32;
        let mut peak = f32::MIN;
        let range = (high_index - low_index + 1) as f32;
        for j in low_index..=high_index {
            if j >= 0 && j < n2 as i64 - 1 {
                let hamming_index = ((HAMMING_WINDOW_SIZE as f32 - 0.001)
                    * ((j - low_index) as f32 / range))
                    as usize;
                let h = hamming[hamming_index];
                hamming_sum += h;
                let this_amp = new_amps[j as usize] * h;
                amp_sum += this_amp;
                if this_amp > peak {
                    peak = this_amp;
                }
            }
        }
        let avg = amp_sum / hamming_sum;
        *diff_db = amp_to_db(peak) - amp_to_db(avg);
    }

    // Formant detection: local maxima of `new_amps` within
    // `[low_freq_limit, high_freq_limit]` and above `minimum_formant_db`,
    // accepted once the drop to their surrounding decline points clears
    // a threshold derived from `diff_dbs`.
    let db_to_amp = DbToAmp::new();
    let min_formant_amp = db_to_amp.convert(params.minimum_formant_db);
    let mut formants = Vec::new();
    for i in 1..=(n2 - 2) {
        if !(freqs[i] >= params.low_freq_limit
            && freqs[i] <= params.high_freq_limit
            && new_amps[i] > new_amps[i - 1]
            && new_amps[i] > new_amps[i + 1]
            && new_amps[i] >= min_formant_amp)
        {
            continue;
        }

        // Trace the decline on each side until amplitude stops falling.
        // Bounds-clamped rather than reading out-of-bounds like the C
        // does transiently (see this module's doc comment) - clamping
        // makes the decline stop exactly at the edge, the same outcome
        // a bounds-safe version of the same algorithm would produce.
        let trace = |sign: i64| -> (f32, usize) {
            let mut n: i64 = i as i64 + sign;
            loop {
                let next = (n + sign).clamp(0, n2 as i64 - 1);
                if new_amps[n as usize] - new_amps[next as usize] > 0.0 {
                    n += sign;
                    if n < 0 || n > n2 as i64 - 1 {
                        n = n.clamp(0, n2 as i64 - 1);
                        break;
                    }
                } else {
                    break;
                }
            }
            (amp_to_db(new_amps[n as usize]), n as usize)
        };
        let (low_db, this_low_index) = trace(-1);
        let (high_db, this_high_index) = trace(1);

        let this_cf_db = amp_to_db(new_amps[i]);
        let threshold = params.formant_selection_threshold * diff_dbs[i];
        let accept = ((this_cf_db - low_db) > threshold
            && (this_cf_db - high_db) > threshold * (1.0 - FORMANT_OVERLAP_TOLERANCE_PROP))
            || ((this_cf_db - low_db) > threshold * (1.0 - FORMANT_OVERLAP_TOLERANCE_PROP)
                && (this_cf_db - high_db) > threshold);
        if !accept {
            continue;
        }

        formants.push(Formant {
            center_freq: freqs[i],
            amp: amps[i],
            bw: 0.0,
            q: 0.0,
            index: i,
            low_stopband_index: this_low_index,
            high_stopband_index: this_high_index,
        });
    }

    // Provisional Q and bandwidth from each formant's original (not
    // smoothed) amplitude relative to its stop-band points.
    for f in &mut formants {
        let this_cf = freqs[f.index];
        let this_cf_db = amp_to_db(amps[f.index]);
        let low_freq = freqs[f.low_stopband_index];
        let high_freq = freqs[f.high_stopband_index];
        let low_db = amp_to_db(amps[f.low_stopband_index]);
        let high_db = amp_to_db(amps[f.high_stopband_index]);
        let base_db = low_db + (high_db - low_db) * ((this_cf - low_freq) / (high_freq - low_freq));
        let db_diff = this_cf_db - base_db;
        f.bw = (3.0 / db_diff) * (high_freq - low_freq);
        f.q = this_cf / f.bw;
    }

    formants
}

/// Ports `NormalizeToPeaksOfSpectrumInBand()`: normalizes the spectrum's
/// peak to `1.0`, then applies a gain ramp between each pair of
/// consecutive formant boundaries (and from `0`/Nyquist to the first/
/// last formant). When `normalize_to_peaks` is off, the ramp endpoints
/// are unity gain - but the ramp *shape* itself (`curve(0, 1, x,
/// expansion_index)`) still applies whenever `expansion_index != 0`, so
/// this is not a no-op even with normalization off (a real, if
/// non-obvious, companding effect - see `tools::freqresponse`'s doc
/// comment).
///
/// No-ops (matching the C's own unreachable/undefined edge case rather
/// than guessing at it) when `formants` is empty, since the C indexes
/// `formantIndices[0]` regardless of `numFormants` in that case, which
/// would read uninitialized memory there.
pub fn normalize_to_peaks_of_spectrum_in_band(
    frame: &mut Frame,
    formants: &[Formant],
    normalize_to_peaks: bool,
    expansion_index: f32,
) {
    if formants.is_empty() {
        return;
    }
    let n2 = frame.bins.len();

    let input_peak_amp = frame.bins.iter().map(|&(m, _)| m).fold(f32::MIN, f32::max);
    if input_peak_amp != 1.0 {
        for (mag, _) in frame.bins.iter_mut() {
            *mag /= input_peak_amp;
        }
    }

    // Ported using the C's own *flat* (`2 * bin`) index arithmetic
    // throughout - `lowIndex`/`highIndex`/`k` are all flat indices there
    // (`formantIndices[formant] * 2`, `N - 2`, `(highIndex-lowIndex+1)/4`
    // with C integer division), and re-deriving equivalent bin-unit
    // formulas independently is an easy place to introduce an off-by-one
    // from the flat/2 relationship - so this mirrors the flat-index
    // formulas exactly and only converts to a bin index (`/2`) at the
    // point of indexing into `frame.bins`.
    let num_formants = formants.len();
    for formant in 0..=num_formants {
        let (low_flat, high_flat, low_gain, high_gain, low_amp, high_amp) = if formant == 0 {
            let low_flat = 0i64;
            let high_flat = formants[0].index as i64 * 2;
            if normalize_to_peaks {
                let k = (high_flat - low_flat + 1) / 4;
                let mut peak = f32::MIN;
                let mut j = low_flat;
                while j < k {
                    let bin = (j / 2) as usize;
                    if frame.bins[bin].0 > peak {
                        peak = frame.bins[bin].0;
                    }
                    j += 2;
                }
                let (low_gain, low_amp) = if peak > 0.0 {
                    (1.0 / peak, 0.0)
                } else {
                    (1.0, 1.0)
                };
                let high_gain = 1.0 / frame.bins[(high_flat / 2) as usize].0;
                (low_flat, high_flat, low_gain, high_gain, low_amp, 0.0)
            } else {
                (low_flat, high_flat, 1.0, 1.0, 0.0, 0.0)
            }
        } else if formant == num_formants {
            let low_flat = formants[formant - 1].index as i64 * 2;
            let high_flat = n2 as i64 * 2 - 2;
            if normalize_to_peaks {
                let low_gain = 1.0 / frame.bins[(low_flat / 2) as usize].0;
                let k = high_flat - (high_flat - low_flat + 1) / 4;
                let mut peak = f32::MIN;
                let mut j = k;
                while j < high_flat {
                    let bin = (j / 2) as usize;
                    if frame.bins[bin].0 > peak {
                        peak = frame.bins[bin].0;
                    }
                    j += 2;
                }
                let (high_gain, high_amp) = if peak > 0.0 {
                    (1.0 / peak, 0.0)
                } else {
                    (1.0, 1.0)
                };
                (low_flat, high_flat, low_gain, high_gain, 0.0, high_amp)
            } else {
                (low_flat, high_flat, 1.0, 1.0, 0.0, 0.0)
            }
        } else {
            let low_flat = formants[formant - 1].index as i64 * 2;
            let high_flat = formants[formant].index as i64 * 2;
            if normalize_to_peaks {
                let low_gain = 1.0 / frame.bins[(low_flat / 2) as usize].0;
                let high_gain = 1.0 / frame.bins[(high_flat / 2) as usize].0;
                (low_flat, high_flat, low_gain, high_gain, 0.0, 0.0)
            } else {
                (low_flat, high_flat, 1.0, 1.0, 0.0, 0.0)
            }
        };

        let index_range = ((high_flat - low_flat + 2) / 2) as f32;
        let mut j = low_flat;
        let mut k = 0.0f32;
        while j < high_flat {
            let up_ramp_raw = k / index_range;
            let down_ramp_raw = 1.0 - up_ramp_raw;
            let up_ramp = crate::curve(0.0, 1.0, up_ramp_raw, expansion_index);
            let down_ramp = crate::curve(0.0, 1.0, down_ramp_raw, expansion_index);
            let mag = &mut frame.bins[(j / 2) as usize].0;
            *mag = (*mag * (up_ramp * high_gain + down_ramp * low_gain))
                + (low_amp * down_ramp)
                + (high_amp * up_ramp);
            if *mag > 1.0 {
                *mag = 1.0;
            }
            j += 2;
            k += 1.0;
        }
    }

    if input_peak_amp > 0.0 && input_peak_amp != 1.0 {
        for (mag, _) in frame.bins.iter_mut() {
            *mag *= input_peak_amp;
        }
    }
}
