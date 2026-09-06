//! Ports `freqresponse.c`: an analysis-driven `.fr` response - unlike
//! `filtresponsemaker`/`chordresponsemaker` (which synthesize a response
//! from a breakpoint/partial table), this analyzes a sound file and
//! accumulates its spectrum (by average or peak amplitude, optionally
//! weighted toward louder frames) into a single response, shared across
//! *all* channels: `AmplitudeSpectrum`/`binAmpSumAndFreqSum`/
//! `buffer_count` are reset once, not per channel - confirmed by reading
//! the per-channel "REINITS" block, which resets `frame_count`/`eof`/
//! `t`/`samps` but not those - so multi-channel input produces one
//! combined response, not one per channel.
//!
//! No `phaselock`, no resynthesis - straight `fold`/`rfft`/`convert`
//! accumulation, then [`crate::eq::eq`] (always normalizing, unless
//! bypassed), [`crate::formant::get_formants`], then
//! [`crate::formant::normalize_to_peaks_of_spectrum_in_band`] - note
//! that last step is not a no-op just because `--formant-normalize` is
//! off: its gain-ramp *shape* (`curve(0, 1, x, expansion_index)`) still
//! applies whenever `expansion_index != 0`, companding the dynamic range
//! between formants regardless (see that function's own doc comment).
//!
//! Not ported (pure reporting/plotting, no CLI flags exposed for them):
//! `-a`/`-o`/`-i`/`-F` (decibels-spectrum/formants-bar-plot/formants-
//! ASCII/formants-binary output files) and `-P` (stderr spectrum
//! printout). `-b`/`-e` (begin/end time) aren't ported either - like
//! `plainpv`/`pvanalysis`, they only ever feed a `dur` that's printed
//! and never otherwise read (freqresponse has no control functions to
//! normalize against it).

use crate::eq::eq;
use crate::formant::{get_formants, normalize_to_peaks_of_spectrum_in_band, FormantParams};
use crate::pvoc::{Analyzer, Frame};
use crate::units::DbToAmp;
use crate::window::{make_windows, Window};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Method {
    Average,
    Peak,
}

#[derive(Debug, Clone)]
pub struct FreqresponseParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub method: Method,
    pub weight_average: bool,
    pub shelf_low_db: f32,
    pub shelf_high_db: f32,
    pub shelf_low_freq: f32,
    pub shelf_high_freq: f32,
    /// `-B`, EQ-with-normalization bypass: `false` (the C's `0`, the
    /// default) means EQ *with* normalization to peak `1.0`; `true`
    /// skips both the EQ and the normalization entirely.
    pub eq_normalize_bypass: bool,
    pub normalize_to_peaks: bool,
    pub companding_index: f32,
    pub low_freq_limit: f32,
    /// `0.0` means Nyquist (the C's own `if (highFreqLimit==0.)
    /// highFreqLimit = nyquist;` default).
    pub high_freq_limit: f32,
    pub minimum_formant_db: f32,
    pub formant_selection_threshold: f32,
}

/// Analyzes every channel in `channels` into one combined response
/// [`Frame`] - see this module's doc comment for why it's one response,
/// not one per channel.
pub fn process(channels: &[Vec<f32>], sample_rate: u32, params: &FreqresponseParams) -> Frame {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
    let d = (r / params.frames_per_sec) as usize;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;

    let mut nw = params.window_size;
    if nw == 0 {
        nw = 2 * n;
    }

    let n_plus_2 = n + 2;
    let mut amplitude_spectrum = vec![0.0f32; n_plus_2];
    let mut bin_amp_sum_and_freq_sum = vec![0.0f32; n_plus_2];
    let mut total_frame_sum_for_weighting = 0.0f32;
    let mut buffer_count: usize = 0;

    for channel in channels {
        let window_pair = make_windows(params.window, nw, n, 0);
        let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);

        let mut valid: i64 = nw as i64;
        let mut pos = 0usize;

        loop {
            let mut hop = vec![0.0f32; d];
            if valid == nw as i64 {
                let available = d.min(channel.len().saturating_sub(pos));
                hop[..available].copy_from_slice(&channel[pos..pos + available]);
                pos += available;
                if available < d {
                    valid = nw as i64 - d as i64 + available as i64;
                }
            }
            if valid < nw as i64 {
                valid -= d as i64;
            }
            let eof_after_this_hop = valid <= 0;

            let frame = analyzer.push(&hop).expect("hop is exactly d samples");
            let flat = frame.to_pva_floats();

            // `framesumForWeightingAmplitudeSpectrum += pow(channel[i],
            // 5.)`: `pow()` returns `double`; a `float +=` with a
            // `double` RHS computes the add in double precision, then
            // narrows to `float` for storage - repeated exactly here
            // rather than summing in one precision throughout, since the
            // per-step narrowing is real (if usually negligible) lost
            // precision.
            let mut framesum_for_weighting = 0.0f32;
            if params.weight_average {
                for &a in flat.iter().step_by(2) {
                    let term = (a as f64).powf(5.0);
                    framesum_for_weighting = (framesum_for_weighting as f64 + term) as f32;
                }
            }

            for j in 0..=n2 {
                let i = 1 + 2 * j;
                match params.method {
                    Method::Average => {
                        if params.weight_average {
                            amplitude_spectrum[i - 1] += framesum_for_weighting * flat[i - 1];
                        } else {
                            amplitude_spectrum[i - 1] += flat[i - 1];
                        }
                        amplitude_spectrum[i] += flat[i];
                    }
                    Method::Peak => {
                        if flat[i - 1] > amplitude_spectrum[i - 1] {
                            amplitude_spectrum[i - 1] = flat[i - 1];
                        }
                        amplitude_spectrum[i] += flat[i];
                    }
                }
                bin_amp_sum_and_freq_sum[i - 1] += flat[i - 1];
                bin_amp_sum_and_freq_sum[i] += flat[i] * flat[i - 1];
            }

            total_frame_sum_for_weighting += framesum_for_weighting;
            buffer_count += 1;

            if eof_after_this_hop {
                break;
            }
        }
    }

    // Average frequency (amplitude-weighted), regardless of method.
    for j in 0..=n2 {
        let i = 1 + 2 * j;
        amplitude_spectrum[i] = bin_amp_sum_and_freq_sum[i] / bin_amp_sum_and_freq_sum[i - 1];
    }

    if params.method == Method::Average {
        for j in 0..=n2 {
            let i = 1 + 2 * j;
            if params.weight_average {
                amplitude_spectrum[i - 1] /= total_frame_sum_for_weighting;
            } else {
                amplitude_spectrum[i - 1] /= buffer_count as f32;
            }
        }
    }

    let db_to_amp = DbToAmp::new();
    if !params.eq_normalize_bypass {
        eq(
            &mut amplitude_spectrum,
            params.shelf_low_db,
            params.shelf_high_db,
            params.shelf_low_freq,
            params.shelf_high_freq,
            fundamental,
            1.0,
            0.0,
            true,
            &db_to_amp,
        );
    }

    let mut frame = Frame::from_pva_floats(&amplitude_spectrum);

    let high_freq_limit = if params.high_freq_limit == 0.0 {
        nyquist
    } else {
        params.high_freq_limit
    };
    let formant_params = FormantParams {
        low_freq_limit: params.low_freq_limit,
        high_freq_limit,
        minimum_formant_db: params.minimum_formant_db,
        formant_selection_threshold: params.formant_selection_threshold,
    };
    let formants = get_formants(&frame, nyquist, &formant_params);
    normalize_to_peaks_of_spectrum_in_band(
        &mut frame,
        &formants,
        params.normalize_to_peaks,
        params.companding_index,
    );

    frame
}
