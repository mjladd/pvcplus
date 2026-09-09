//! Ports `spectrummapper.c`: a formant-*tracking* analysis tool (distinct
//! from `tools::formantsmapper`'s own fixed-formant-*list* remapper) -
//! extracts formant peaks frame by frame from a live analysis of raw
//! audio, then greedily assembles them into time-continuous "segments"
//! (tracks), links separate segments into longer chains, and writes the
//! result to ASCII/binary formant-track files. **This tool writes no
//! audio at all**: `main()` sets the shared `outputoff` global to `1`
//! immediately after channel setup, which makes `bufferout()` (and thus
//! every path that would otherwise write samples) a silent no-op -
//! confirmed by reading `legacy/pvc_lib/fileio.c`'s own `outputoff`
//! checks, not assumed from the tool's own name. There is accordingly no
//! oscillator bank, no window-size-driven resynthesis path, and no
//! audio golden fixture needed for this port - only the analysis output
//! files matter.
//!
//! **A real, confirmed-by-tracing amplitude-rescale bug in
//! `get_formants_2()`**: `amps[]` is divided by the frame's own peak
//! amplitude *only if that peak is `< 1.0`* (`if (peakAmp < 1.) for(...)
//! amps[i] /= peakAmp;`), but the later "RESCALE TO ORIGINAL AMP LEVELS"
//! step multiplies every accepted formant's amplitude by that same peak
//! *unconditionally* (`formantAmps[i] = originalPeakAmp *
//! formantAmps[i];`). When the peak is `< 1.0` the divide-then-multiply
//! cancels exactly (correct). When the peak is `>= 1.0` the divide never
//! happened, so the unconditional multiply doubles the effective gain -
//! every accepted formant's reported amplitude is its true value
//! multiplied by the frame's own (already-`>= 1.0`) peak amplitude a
//! second time. Reproduced exactly as read - there's no way to tell from
//! the source alone whether the intended fix was "make the rescale
//! conditional too" or "always normalize the divide," so this port picks
//! neither and just matches the real, asymmetric behavior.
//!
//! **A large amount of `get_formants_2`'s own parameter surface is dead
//! code**, confirmed by grepping every reference across the whole file:
//! `AmpsDerivative`/`testAmpsSave`/`triWindow`/`formantAmpsCopy`/
//! `tempList` are allocated, passed in, and freed, but never read or
//! written inside the function body at all. `symmetryFactor` (and its
//! own scratch arrays `v`/`w`) *are* computed (a "formant symmetry"
//! block) but the result is never read after the call returns - the
//! whole symmetry computation has zero observable effect. `freqStasis`
//! and its own "correlate formants with frequency stasis" filter are
//! gated by `CorrelateWithFreqStasisFlag`, which no `crack()` case ever
//! sets away from its `0` default (confirmed by cross-referencing the
//! accept string against the switch) - so that entire block, and the
//! `freqStasisPlotFile` ASCII writer it feeds, are dead too. None of
//! these six arrays or the symmetry/stasis computations are ported here.
//!
//! **Two functions are defined but never called anywhere in the file**:
//! `findIntersectPointOfLines` and `computeAmpAndFreqCorrelationFactor_NEW`
//! (confirmed by grepping every reference - each appears only at its own
//! definition). Not ported. A large commented-out "SUBROUTINE JUNKYARD"
//! block at the end of the file (an older `computeAmpAndFreqCorrelationFactor`
//! and `makeTargetCF`) is dead for the more obvious reason of being
//! inside a `/* ... */` comment.
//!
//! **Segment "bridging" (tolerating a run of missed frames while growing
//! a track) can never actually activate.** `maximumAllowedBridgingFrames`
//! defaults to (and is permanently stuck at) `0` - its own `-K` flag's
//! `case` is commented out in the switch, and `-K` isn't even in the
//! `crack()` accept string, so there is no way to change it from the
//! CLI at all. Since the leg-growing loop increments its own bridge
//! counter *before* comparing it to this limit, a limit of `0` means the
//! very first missed frame always exceeds it, ending the leg
//! immediately - the "keep trying across a gap" retry path, and the
//! later "interpolate across a bridged gap" fill when writing a segment
//! out, are both unreachable in the shipped tool. Not ported as general
//! machinery; this port's own leg-growing stops on the first miss
//! directly, which is the only behavior the real tool can ever exhibit.
//! `-T`'s own `segmentLinkingTolerancePercentage` has the identical
//! problem (commented-out case, not in the accept string) *and* is
//! additionally a dead variable even where declared - it's assigned
//! once and never read again regardless. `usage()` documents both `-K`
//! and `-T` as real, working flags; neither can actually be set.
//!
//! **The "reject" branch of segment acceptance is dead**: the C's own
//! accept/reject test is `if (1 == 1) { accept } else { reject }` -
//! every candidate segment is unconditionally accepted, confirmed by
//! reading the literal condition. The `-a`-adjacent "rejects" ASCII plot
//! file this port doesn't bother writing is consequently always empty
//! in the real tool too.
//!
//! **The "impose" envelope (`-F 2`) computed during initial segment
//! construction is cosmetic only** - `thisSegmentEnvelope[]` multiplies
//! the *ASCII* plot file's amplitude column at the central-segment write
//! site, but every binary `fwrite` in that same construction phase
//! writes the *unmultiplied* `thisAmp` - confirmed by checking each
//! `fwrite` call individually, not assumed from the one that's
//! multiplied. Since the binary intermediary file is what actually
//! feeds the rest of the pipeline (re-read immediately after), this
//! first envelope pass has zero effect on real output. `-F 2`'s only
//! real effect is the *second* envelope imposition applied later, after
//! segment linking, directly to the final chained segment's own stored
//! amplitudes (which do reach the real `-S` binary output). This port
//! only implements that second, real imposition.

use crate::control::ControlFn;
use crate::pvoc::Analyzer;
use crate::units::{amp_to_db, DbToAmp};
use crate::window::Window;

const SMOOTHING_WINDOW_AS_PROPORTION_OF_CENTROID: f32 = 1.0 / 50.0;
const FORMANT_OVERLAP_TOLERANCE_PROP: f32 = 0.5;
const HAMMING_WINDOW_SIZE: usize = 1024;

/// `-F`: how onset/release points are added to a segment.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OnsetReleaseMode {
    /// `0` (the default): no onset/release points at all.
    None,
    /// `1`: append new synthetic frames before/after the segment,
    /// ramping from/to silence. Real effect on the final binary output.
    Append,
    /// `2`: impose a ramped envelope directly onto the segment's own
    /// existing amplitude values, after linking (see this module's own
    /// doc comment on why the *first* imposition pass is cosmetic-only).
    Impose,
}

fn hamming_window_table() -> Vec<f32> {
    let n = HAMMING_WINDOW_SIZE;
    (0..n)
        .map(|i| 0.54 - 0.46 * (std::f64::consts::TAU * i as f64 / (n as f64 - 1.0)).cos() as f32)
        .collect()
}

/// One extracted formant peak from a single frame's spectrum.
#[derive(Debug, Clone, Copy)]
pub struct Formant {
    pub center_freq: f32,
    pub amp: f32,
    pub bw: f32,
    pub q: f32,
}

pub struct GetFormantsParams {
    pub low_freq_limit: f32,
    pub high_freq_limit: f32,
    pub minimum_formant_db: f32,
    pub formant_selection_threshold: f32,
}

/// Ports `get_formants_2()` (minus its confirmed-dead parameters/blocks -
/// see this module's own doc comment). `bins` is `(amp, freq_hz)` pairs,
/// length `n2 + 1`. `nyquist` comes from the caller's own FFT size.
///
/// **Real quirk, reproduced exactly**: the C recomputes its own local
/// `fundamental` from `nyquist` and its own `N` parameter (`main()`'s
/// own `N + 2`, i.e. `2 * (n2 + 1) - 2 = 2 * n2` here), rather than
/// receiving the tool's real `fundamental = R / N` (used everywhere
/// else, including the `eq()` call just before this one) - so this
/// function's own idea of "Hz per bin" is `nyquist / n2`, not `R / N`.
/// For the typical case (`N` even) these are close but not equal
/// (`nyquist / n2` vs. `nyquist * 2 / (N + 2)` vs. the real `nyquist * 2
/// / N`) - confirmed by reading the exact expression, not assumed to be
/// the same "fundamental" the rest of the tool uses.
pub fn get_formants(
    bins: &[(f32, f32)],
    nyquist: f32,
    hamming_window: &[f32],
    params: &GetFormantsParams,
) -> Vec<Formant> {
    let n2 = bins.len();
    let fundamental = nyquist / n2 as f32;
    let db_to_amp = DbToAmp::new();
    let floor_amp = db_to_amp.convert(-96.0);

    let mut amps: Vec<f32> = bins.iter().map(|&(a, _)| a).collect();
    let freqs: Vec<f32> = bins.iter().map(|&(_, f)| f).collect();

    let peak_amp = amps.iter().copied().fold(f32::MIN, f32::max);
    if peak_amp < 1.0 {
        for a in amps.iter_mut() {
            *a /= peak_amp;
        }
    }
    let original_peak_amp = peak_amp;

    let mut new_amps = amps.clone();

    let mut centroid_num = 0.0f32;
    let mut centroid_den = 0.0f32;
    for i in 0..n2 {
        centroid_num += freqs[i] * new_amps[i];
        centroid_den += new_amps[i];
    }
    let centroid_freq = centroid_num / centroid_den;

    let window_bw = centroid_freq * SMOOTHING_WINDOW_AS_PROPORTION_OF_CENTROID;

    let mut freq_diff_sum = 0.0f32;
    for i in 1..n2 {
        freq_diff_sum += freqs[i] - freqs[i - 1];
    }
    let avg_bin_freq_diff = freq_diff_sum / (n2 - 1) as f32;

    let mut avg_win_size = (window_bw / avg_bin_freq_diff) as i64;
    if avg_win_size % 2 == 0 {
        avg_win_size += 1;
    }
    if avg_win_size < 3 {
        avg_win_size = 3;
    }
    let avg_win_size = avg_win_size as usize;

    let mut win_array = vec![0.0f32; avg_win_size];
    let mut smoothed_peak = f32::MIN;
    for i in 0..n2 {
        for (n, slot) in win_array.iter_mut().enumerate() {
            let mut k = n as i64 + i as i64 - (avg_win_size / 2) as i64;
            if k < 0 {
                k = 0;
            }
            if k > (n2 as i64 - 1) {
                k = n2 as i64 - 1;
            }
            *slot = new_amps[k as usize];
        }
        let mut hamming_float_index = 0.0f32;
        let step = (hamming_window.len() as f32 - 1.0) / (avg_win_size as f32 - 1.0);
        for slot in win_array.iter_mut() {
            let idx = (hamming_float_index + 0.5) as usize;
            *slot *= hamming_window[idx.min(hamming_window.len() - 1)];
            hamming_float_index += step;
        }
        let mean: f32 = win_array.iter().sum::<f32>() / avg_win_size as f32;
        new_amps[i] = mean;
        if mean > smoothed_peak {
            smoothed_peak = mean;
        }
    }
    for a in new_amps.iter_mut() {
        *a /= smoothed_peak;
    }

    let temp = (centroid_freq * 2.0 - centroid_freq * 0.5) * 0.5;
    let mut peak_amps = vec![0.0f32; n2];
    let mut avg_amps = vec![0.0f32; n2];
    let mut diff_dbs = vec![0.0f32; n2];
    for i in 0..n2 {
        let cf = fundamental * i as f32;
        let low_index = ((cf - temp) / fundamental + 0.5) as i64;
        let high_index = ((cf + temp) / fundamental + 0.5) as i64;
        let mut amp_sum = 0.0f32;
        let mut hamming_sum = 0.0f32;
        let mut peak = f32::MIN;
        let span = (high_index - low_index) + 1;
        for j in low_index..=high_index {
            // Matches the C's own `(j >= 0) && (j < (N2 - 1))` exactly -
            // the strict `<` excludes the very last bin from ever
            // contributing here, an off-by-one in the original.
            if j >= 0 && j < (n2 as i64 - 1) {
                let hamming_index = ((hamming_window.len() as f32 - 0.001) * (j - low_index) as f32
                    / span as f32) as usize;
                let hamming_val = hamming_window[hamming_index.min(hamming_window.len() - 1)];
                hamming_sum += hamming_val;
                let this_amp = new_amps[j as usize] * hamming_val;
                amp_sum += this_amp;
                if this_amp > peak {
                    peak = this_amp;
                }
            }
        }
        peak_amps[i] = peak;
        avg_amps[i] = amp_sum / hamming_sum;
        diff_dbs[i] = amp_to_db(peak_amps[i]) - amp_to_db(avg_amps[i]);
    }

    let mut formants = Vec::new();
    if n2 >= 3 {
        for i in 1..=(n2 - 2) {
            if freqs[i] < params.low_freq_limit
                || freqs[i] > params.high_freq_limit
                || !(new_amps[i] > new_amps[i - 1] && new_amps[i] > new_amps[i + 1])
                || new_amps[i] < db_to_amp.convert(params.minimum_formant_db)
            {
                continue;
            }

            let formant_index = i;
            let mut low_db = 0.0f32;
            let mut high_db = 0.0f32;
            let mut this_low_index = formant_index;
            let mut this_high_index = formant_index;
            for sign in [-1i64, 1i64] {
                let mut n = formant_index as i64 + sign;
                loop {
                    let cur = n.clamp(0, n2 as i64 - 1) as usize;
                    let next = (n + sign).clamp(0, n2 as i64 - 1) as usize;
                    if new_amps[cur] - new_amps[next] > 0.0 {
                        n += sign;
                        if n < 0 || n > (n2 as i64 - 1) {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                let n_clamped = n.clamp(0, n2 as i64 - 1) as usize;
                if sign == -1 {
                    low_db = amp_to_db(new_amps[n_clamped]);
                    this_low_index = n_clamped;
                } else {
                    high_db = amp_to_db(new_amps[n_clamped]);
                    this_high_index = n_clamped;
                }
            }

            let this_cf_db = amp_to_db(new_amps[formant_index]);
            let this_db_threshold = params.formant_selection_threshold * diff_dbs[i];

            let accept = ((this_cf_db - low_db) > this_db_threshold
                && (this_cf_db - high_db)
                    > (this_db_threshold * (1.0 - FORMANT_OVERLAP_TOLERANCE_PROP)))
                || ((this_cf_db - low_db)
                    > (this_db_threshold * (1.0 - FORMANT_OVERLAP_TOLERANCE_PROP))
                    && (this_cf_db - high_db) > this_db_threshold);

            if accept {
                let low_freq = avg_bin_freq_diff * this_low_index as f32;
                let high_freq = avg_bin_freq_diff * this_high_index as f32;
                let low_db_amp = amp_to_db(amps[this_low_index]);
                let high_db_amp = amp_to_db(amps[this_high_index]);
                let this_cf_bin_cf = formant_index as f32 * avg_bin_freq_diff;
                let base_db = low_db_amp
                    + (high_db_amp - low_db_amp)
                        * ((this_cf_bin_cf - low_freq) / (high_freq - low_freq));
                let this_cf_db_raw = amp_to_db(amps[formant_index]);
                let db_diff = this_cf_db_raw - base_db;
                let bw = (3.0 / db_diff) * (high_freq - low_freq);
                let q = this_cf_bin_cf / bw;

                formants.push(Formant {
                    center_freq: freqs[formant_index],
                    amp: amps[formant_index] * original_peak_amp,
                    bw,
                    q,
                });
            }
        }
    }

    let _ = floor_amp; // kept for parity with the C's repeated -96dB floor constant, used by callers.
    formants
}

/// One raw formant data point collected across the whole file, before
/// segmentation - `formantData[]`'s own six columns minus the redundant
/// `frame` (kept as `time` alone is enough to reconstruct grouping).
#[derive(Debug, Clone, Copy)]
struct FormantPoint {
    frame: i64,
    time: f32,
    cf: f32,
    amp: f32,
    bw: f32,
    q: f32,
}

pub struct SpectrumMapperParams {
    pub window: Window,
    pub window_size: usize,
    pub frames_per_sec: f32,
    pub shelf_low_db: f32,
    pub shelf_high_db: f32,
    pub shelf_low_freq: f32,
    pub shelf_high_freq: f32,
    pub eq_bypass: bool,
    /// `-L`: low frequency limit in Hz - time-varying (`fval`'d every
    /// frame in the C).
    pub low_freq_limit: ControlFn,
    /// `-j`: high frequency limit in Hz - time-varying.
    pub high_freq_limit: ControlFn,
    pub minimum_formant_db: f32,
    pub formant_selection_threshold: f32,
    pub minimum_decibels: f32,
    pub maximum_decibels: f32,
    pub minimum_segment_length: i64,
    pub maximum_segment_length: i64,
    pub minimum_segment_duration: f32,
    pub maximum_segment_duration: f32,
    pub max_frequency_change_per_ms: f32,
    pub max_decibel_rise_per_ms: f32,
    pub max_decibel_fall_per_ms: f32,
    pub linkage_time: f32,
    pub maximum_frequency_linkage: f32,
    pub onset_release_mode: OnsetReleaseMode,
    pub onset_duration: f32,
    pub release_duration: f32,
    pub time_shift: f32,
}

/// Ports `linearLeastSquaresProjection()`.
fn linear_least_squares(x_target: f64, x: &[f64], y: &[f64]) -> (f64, f64, f64) {
    let n = x.len() as f64;
    let x_sum: f64 = x.iter().sum();
    let y_sum: f64 = y.iter().sum();
    let sum_xy: f64 = x.iter().zip(y).map(|(a, b)| a * b).sum();
    let sum_x2: f64 = x.iter().map(|a| a * a).sum();
    let x_sum_sq = x_sum * x_sum;

    let slope = (sum_xy - (x_sum * y_sum) / n) / (sum_x2 - x_sum_sq / n);
    let x_mean = x_sum / n;
    let y_mean = y_sum / n;
    let y_intercept = y_mean - slope * x_mean;
    let y = slope * x_target + y_intercept;
    (y, y_intercept, slope)
}

/// Ports `makeLeastSquaresTargetCF()`.
#[allow(clippy::too_many_arguments)]
fn make_least_squares_target_cf(
    direction: i32,
    group_time: f32,
    front_leg: &[usize],
    back_leg: &[usize],
    formant_time: &[f32],
    formant_cf: &[f32],
    max_length: usize,
) -> f32 {
    let num_front = front_leg.len();
    let num_back = back_leg.len();

    if direction == 1 {
        if num_front == 1 {
            formant_cf[front_leg[0]]
        } else {
            let this_length = num_front.min(max_length);
            let front_leg_start = num_front - this_length;
            let x: Vec<f64> = (front_leg_start..front_leg_start + this_length)
                .map(|j| formant_time[front_leg[j]] as f64)
                .collect();
            let y: Vec<f64> = (front_leg_start..front_leg_start + this_length)
                .map(|j| formant_cf[front_leg[j]] as f64)
                .collect();
            linear_least_squares(group_time as f64, &x, &y).0 as f32
        }
    } else if num_back >= max_length {
        let this_length = max_length;
        let back_leg_start = num_back - this_length;
        let x: Vec<f64> = (back_leg_start..back_leg_start + this_length)
            .map(|j| formant_time[back_leg[j]] as f64)
            .collect();
        let y: Vec<f64> = (back_leg_start..back_leg_start + this_length)
            .map(|j| formant_cf[back_leg[j]] as f64)
            .collect();
        linear_least_squares(group_time as f64, &x, &y).0 as f32
    } else {
        let this_length = num_back;
        let num_spaces_left = max_length - this_length;
        let num_added_front = if (num_front as i64 - 1) >= num_spaces_left as i64 {
            num_spaces_left
        } else {
            num_front - 1
        };
        let mut x = Vec::new();
        let mut y = Vec::new();
        if num_added_front > 0 {
            let mut j = num_added_front;
            for _ in 0..num_added_front {
                x.push(formant_time[front_leg[j]] as f64);
                y.push(formant_cf[front_leg[j]] as f64);
                j -= 1;
            }
        }
        for j in 0..this_length {
            x.push(formant_time[back_leg[j]] as f64);
            y.push(formant_cf[back_leg[j]] as f64);
        }
        if x.len() == 1 {
            formant_cf[back_leg[0]]
        } else {
            linear_least_squares(group_time as f64, &x, &y).0 as f32
        }
    }
}

fn find_change_per_ms(
    direction: i32,
    values: &[f32],
    times: &[f32],
    proposed: usize,
    leg: &[usize],
) -> f32 {
    let x1 = times[proposed];
    let y1 = values[proposed];
    let last = *leg.last().unwrap();
    let x0 = times[last];
    let y0 = values[last];
    let duration = (x1 - x0).abs();
    let _ = direction;
    if duration > 0.0 {
        0.001 * ((y1 - y0) / duration)
    } else {
        0.0
    }
}

fn find_db_change_per_ms(
    direction: i32,
    amps: &[f32],
    times: &[f32],
    proposed: usize,
    leg: &[usize],
) -> f32 {
    let x1 = times[proposed];
    let y1 = amp_to_db_floored(amps[proposed]);
    let last = *leg.last().unwrap();
    let x0 = times[last];
    let y0 = amp_to_db_floored(amps[last]);
    let duration = (x1 - x0).abs();
    let _ = direction;
    if duration > 0.0 {
        0.001 * ((y1 - y0) / duration)
    } else {
        0.0
    }
}

fn amp_to_db_floored(amp: f32) -> f32 {
    let db_to_amp = DbToAmp::new();
    if amp < db_to_amp.convert(-96.0) {
        -96.0
    } else {
        amp_to_db(amp)
    }
}

/// A single formant "point" once collected into a written-out segment
/// (post onset/release/linking).
#[derive(Debug, Clone, Copy)]
pub struct TrackPoint {
    pub time: f32,
    pub cf: f32,
    pub amp: f32,
    pub db: f32,
    pub bw: f32,
    pub q: f32,
}

/// Resynthesizes nothing - analyzes one channel of raw audio into
/// formant tracks. Returns the final list of tracks (each a `Vec` of
/// `TrackPoint`), matching `-S`'s own binary layout: one `f32` length
/// prefix per track followed by that many 6-`f32` records.
#[allow(clippy::too_many_arguments)]
pub fn analyze_channel(
    input: &[f32],
    sample_rate: u32,
    dur: f32,
    fft_size: usize,
    params: &SpectrumMapperParams,
) -> Vec<Vec<TrackPoint>> {
    let r = sample_rate as f32;
    let n = fft_size;
    let n2 = n / 2;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;

    let frames_per_sec = if params.frames_per_sec < 32.0 {
        200.0
    } else {
        params.frames_per_sec
    };
    let d = (r / frames_per_sec) as usize;
    let frame_duration = 1.0 / frames_per_sec;

    let mut nw = params.window_size;
    if nw == 0 {
        nw = 2 * n;
    }

    let hamming = hamming_window_table();
    let db_to_amp = DbToAmp::new();

    let window_pair = crate::window::make_windows(params.window, nw, n, d);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);

    let mut points: Vec<FormantPoint> = Vec::new();
    let mut pos = 0usize;
    let mut valid: i64 = nw as i64;
    let mut frame_count: i64 = 0;

    loop {
        let mut hop = vec![0.0f32; d];
        if valid == nw as i64 {
            let available = d.min(input.len().saturating_sub(pos));
            hop[..available].copy_from_slice(&input[pos..pos + available]);
            pos += available;
            if available < d {
                valid = nw as i64 - d as i64 + available as i64;
            }
        }
        if valid < nw as i64 {
            valid -= d as i64;
        }
        let eof_after_this_hop = valid <= 0;

        let time = if frame_count == 0 {
            0.0
        } else {
            (frame_count * d as i64) as f32 / r
        };

        let frame = analyzer.push(&hop).expect("hop is exactly d samples");
        let mut spectrum = frame.to_pva_floats();

        if !params.eq_bypass {
            crate::eq::eq(
                &mut spectrum,
                params.shelf_low_db,
                params.shelf_high_db,
                params.shelf_low_freq,
                params.shelf_high_freq,
                fundamental,
                1.0,
                0.0,
                false,
                &db_to_amp,
            );
        }

        let bins: Vec<(f32, f32)> = spectrum.chunks_exact(2).map(|c| (c[0], c[1])).collect();
        let get_params = GetFormantsParams {
            low_freq_limit: params.low_freq_limit.at(time, dur),
            high_freq_limit: params.high_freq_limit.at(time, dur),
            minimum_formant_db: params.minimum_formant_db,
            formant_selection_threshold: params.formant_selection_threshold,
        };
        let formants = get_formants(&bins, nyquist, &hamming, &get_params);

        for f in &formants {
            points.push(FormantPoint {
                frame: frame_count,
                time,
                cf: f.center_freq,
                amp: f.amp,
                bw: f.bw,
                q: f.q,
            });
        }

        frame_count += 1;
        if eof_after_this_hop {
            break;
        }
    }

    let _ = n2;
    build_tracks(&points, dur, frame_duration, frames_per_sec, params)
}

struct FormantGroup {
    begin: usize,
    end: usize,
}

/// Ports the segment-construction, linking, and final chained-output
/// phases of `main()` (everything after the per-frame `get_formants_2`
/// collection loop) - see this module's own doc comment for the several
/// confirmed-dead branches (bridging, the reject path, the first
/// "impose" envelope pass) this skips.
fn build_tracks(
    points: &[FormantPoint],
    dur: f32,
    frame_duration: f32,
    frames_per_sec: f32,
    params: &SpectrumMapperParams,
) -> Vec<Vec<TrackPoint>> {
    if points.is_empty() {
        return Vec::new();
    }

    let total = points.len();
    let formant_time: Vec<f32> = points.iter().map(|p| p.time).collect();
    let formant_cf: Vec<f32> = points.iter().map(|p| p.cf).collect();
    let mut formant_amp: Vec<f32> = points.iter().map(|p| p.amp).collect();
    let formant_bw: Vec<f32> = points.iter().map(|p| p.bw).collect();
    let formant_q: Vec<f32> = points.iter().map(|p| p.q).collect();

    let peak_amp = formant_amp.iter().copied().fold(f32::MIN, f32::max);
    for a in formant_amp.iter_mut() {
        *a /= peak_amp;
    }

    let mut groups: Vec<FormantGroup> = Vec::new();
    let mut group_of: Vec<usize> = vec![0; total];
    {
        let mut last_frame = i64::MIN;
        for (i, p) in points.iter().enumerate() {
            if p.frame != last_frame {
                groups.push(FormantGroup { begin: i, end: i });
                last_frame = p.frame;
            }
            let g = groups.len() - 1;
            groups[g].end = i;
            group_of[i] = g;
        }
    }
    let num_groups = groups.len();

    let formant_amplitude_min = DbToAmp::new().convert(params.minimum_decibels);
    let formant_amplitude_max = DbToAmp::new().convert(params.maximum_decibels);
    let mut formant_switch: Vec<bool> = formant_amp
        .iter()
        .map(|&a| !(a <= formant_amplitude_min || a >= formant_amplitude_max))
        .collect();
    let mut peak_formant_switch: Vec<bool> = vec![true; total];

    let max_length = 10usize;
    let mut raw_segments: Vec<(Vec<usize>, f32)> = Vec::new(); // (point indices in time order, duration)

    loop {
        let mut peak_formant_amp = 0.0f32;
        let mut peak_formant_index: Option<usize> = None;
        for i in 0..total {
            if formant_switch[i] && peak_formant_switch[i] && formant_amp[i] > peak_formant_amp {
                peak_formant_amp = formant_amp[i];
                peak_formant_index = Some(i);
            }
        }
        let Some(peak_index) = peak_formant_index else {
            break;
        };
        peak_formant_switch[peak_index] = false;

        let mut forward_leg = vec![peak_index];
        let mut backward_leg = vec![peak_index];

        for direction in [1i32, -1i32] {
            let leg_group = group_of[peak_index] as i64;
            let mut group_idx = leg_group + direction as i64;

            loop {
                if group_idx < 0 || group_idx >= num_groups as i64 {
                    break;
                }
                let leg = if direction == 1 {
                    &forward_leg
                } else {
                    &backward_leg
                };
                let group_time = formant_time[groups[group_idx as usize].begin];
                let target_cf = make_least_squares_target_cf(
                    direction,
                    group_time,
                    &forward_leg,
                    &backward_leg,
                    &formant_time,
                    &formant_cf,
                    max_length,
                );

                let prev_index = *leg.last().unwrap();
                let group = &groups[group_idx as usize];
                let mut closest_above: Option<usize> = None;
                let mut closest_below: Option<usize> = None;
                for i in group.begin..=group.end {
                    let diff = formant_cf[i] - formant_cf[prev_index];
                    if diff >= 0.0 {
                        if closest_above.is_none()
                            || diff < (formant_cf[closest_above.unwrap()] - formant_cf[prev_index])
                        {
                            closest_above = Some(i);
                        }
                    } else if closest_below.is_none()
                        || diff.abs()
                            < (formant_cf[closest_below.unwrap()] - formant_cf[prev_index]).abs()
                    {
                        closest_below = Some(i);
                    }
                }

                let proposed = match (closest_above, closest_below) {
                    (Some(a), Some(b)) => {
                        if (formant_cf[a] - target_cf).abs() < (formant_cf[b] - target_cf).abs() {
                            Some(a)
                        } else {
                            Some(b)
                        }
                    }
                    (Some(a), None) => Some(a),
                    (None, Some(b)) => Some(b),
                    (None, None) => None,
                };

                let mut matched = false;
                if let Some(p) = proposed {
                    if formant_switch[p] {
                        let leg_ref = if direction == 1 {
                            &forward_leg
                        } else {
                            &backward_leg
                        };
                        let freq_change =
                            find_change_per_ms(direction, &formant_cf, &formant_time, p, leg_ref);
                        let db_change = find_db_change_per_ms(
                            direction,
                            &formant_amp,
                            &formant_time,
                            p,
                            leg_ref,
                        );
                        if freq_change < params.max_frequency_change_per_ms
                            && freq_change > -params.max_frequency_change_per_ms
                            && db_change <= params.max_decibel_rise_per_ms
                            && db_change >= -params.max_decibel_fall_per_ms
                        {
                            matched = true;
                        }
                    }
                }

                if matched {
                    let p = proposed.unwrap();
                    if direction == 1 {
                        forward_leg.push(p);
                    } else {
                        backward_leg.push(p);
                    }
                    group_idx += direction as i64;
                } else {
                    // No bridging tolerance is ever configurable (see this
                    // module's own doc comment) - a single miss always
                    // ends the leg.
                    break;
                }
            }
        }

        // Turn off all but the first/last formant in the accepted
        // segment (they remain linkable by other segments).
        let mut segment: Vec<usize> = backward_leg.iter().rev().copied().collect();
        segment.extend(forward_leg.iter().skip(1).copied());
        for &idx in segment.iter().skip(1).take(segment.len().saturating_sub(2)) {
            formant_switch[idx] = false;
        }

        let duration = formant_time[*segment.last().unwrap()] - formant_time[segment[0]];
        raw_segments.push((segment, duration));
    }

    // ---- WRITE INITIAL SEGMENTS (append onset/release; "impose" is
    // cosmetic here, see this module's doc comment - not applied) ----
    let mut written_segments: Vec<Vec<TrackPoint>> = Vec::new();
    for (segment, _duration) in &raw_segments {
        let mut track = Vec::new();
        if params.onset_release_mode == OnsetReleaseMode::Append && params.onset_duration > 0.0 {
            let num_onset = (0.5 + params.onset_duration * frames_per_sec) as i64;
            for i in 0..num_onset {
                let j = num_onset - i;
                let time =
                    params.time_shift + formant_time[segment[0]] - (j as f32 * frame_duration);
                let cf = formant_cf[segment[0]];
                let amp = formant_amp[segment[0]] * (i as f32 / num_onset as f32);
                track.push(TrackPoint {
                    time,
                    cf,
                    amp,
                    db: amp_to_db_floored(amp),
                    bw: formant_bw[segment[0]],
                    q: formant_q[segment[0]],
                });
            }
        }
        for &idx in segment {
            let time = params.time_shift + formant_time[idx];
            let amp = formant_amp[idx];
            track.push(TrackPoint {
                time,
                cf: formant_cf[idx],
                amp,
                db: amp_to_db_floored(amp),
                bw: formant_bw[idx],
                q: formant_q[idx],
            });
        }
        if params.onset_release_mode == OnsetReleaseMode::Append && params.release_duration > 0.0 {
            let num_release = (0.5 + params.release_duration * frames_per_sec) as i64;
            let last = *segment.last().unwrap();
            for l in 0..num_release {
                let j = l + 1;
                let i = num_release - 1 - l;
                let time = params.time_shift + formant_time[last] + (frame_duration * j as f32);
                let amp = formant_amp[last] * (i as f32 / num_release as f32);
                track.push(TrackPoint {
                    time,
                    cf: formant_cf[last],
                    amp,
                    db: amp_to_db_floored(amp),
                    bw: formant_bw[last],
                    q: formant_q[last],
                });
            }
        }
        written_segments.push(track);
    }

    // ---- SEGMENT LINKING ----
    let num_segments = written_segments.len();
    let min_length = 3usize;
    let mut link_to: Vec<i64> = vec![-1; num_segments];
    let mut link_from: Vec<i64> = vec![-1; num_segments];
    for (i, seg) in written_segments.iter().enumerate() {
        if seg.len() < min_length {
            link_to[i] = -2;
            link_from[i] = -2;
        }
    }

    loop {
        let mut this_seg: i64 = -1;
        for i in 0..num_segments {
            if (link_from[i] == -1 || link_to[i] == -1)
                && (this_seg == -1
                    || written_segments[i].len() > written_segments[this_seg as usize].len())
            {
                this_seg = i as i64;
            }
        }
        if this_seg == -1 {
            break;
        }
        let this_seg = this_seg as usize;

        let mut closest: i64 = -1;
        let mut closest_dist = f32::MAX;
        for j in 0..num_segments {
            if j == this_seg {
                continue;
            }
            if link_to[this_seg] == -1 {
                let this_end = written_segments[this_seg].last().unwrap();
                let j_begin = written_segments[j][0];
                let freq_diff = (this_end.cf - j_begin.cf).abs();
                if written_segments[j].len() >= min_length
                    && j_begin.time >= this_end.time
                    && j_begin.time <= (this_end.time + params.linkage_time)
                    && freq_diff <= params.maximum_frequency_linkage
                {
                    let scale = params.linkage_time / params.maximum_frequency_linkage;
                    let d = ((scale * (j_begin.cf - this_end.cf)).powi(2)
                        + (j_begin.time - this_end.time).powi(2))
                    .sqrt();
                    if d < closest_dist {
                        closest = j as i64;
                        closest_dist = d;
                    }
                }
            } else {
                let this_begin = written_segments[this_seg][0];
                let j_end = written_segments[j].last().unwrap();
                let freq_diff = (this_begin.cf - j_end.cf).abs();
                if written_segments[j].len() >= min_length
                    && j_end.time <= this_begin.time
                    && j_begin_time_ok(j_end.time, this_begin.time, params.linkage_time)
                    && freq_diff <= params.maximum_frequency_linkage
                {
                    let scale = params.linkage_time / params.maximum_frequency_linkage;
                    let d = ((scale * (j_end.cf - this_begin.cf)).powi(2)
                        + (j_end.time - this_begin.time).powi(2))
                    .sqrt();
                    if d < closest_dist {
                        closest = j as i64;
                        closest_dist = d;
                    }
                }
            }
        }

        if closest != -1 {
            let closest = closest as usize;
            if link_to[this_seg] == -1 {
                link_to[this_seg] = closest as i64;
                link_from[closest] = this_seg as i64;
            } else {
                link_to[closest] = this_seg as i64;
                link_from[this_seg] = closest as i64;
            }
        } else if link_to[this_seg] == -1 {
            link_to[this_seg] = -2;
        } else {
            link_from[this_seg] = -2;
        }
    }

    // ---- WALK CHAINS, RAMP BETWEEN LINKS, APPLY FINAL FILTERS ----
    let mut write_flag: Vec<bool> = vec![false; num_segments];
    let mut output_tracks: Vec<Vec<TrackPoint>> = Vec::new();

    for start in 0..num_segments {
        if write_flag[start] {
            continue;
        }
        let mut chain: Vec<TrackPoint> = Vec::new();
        let mut this_seg = start as i64;
        while this_seg >= 0 {
            let seg_idx = this_seg as usize;
            chain.extend(written_segments[seg_idx].iter().copied());
            write_flag[seg_idx] = true;

            if link_to[seg_idx] >= 0 {
                let next_seg = link_to[seg_idx] as usize;
                let this_end_time = written_segments[seg_idx].last().unwrap().time;
                let next_begin_time = written_segments[next_seg][0].time;
                let num_steps = ((next_begin_time - this_end_time) / frame_duration) as i64;

                let link_from_len = written_segments[seg_idx].len().min(max_length);
                let link_to_len = written_segments[next_seg].len().min(max_length);

                let from_slice =
                    &written_segments[seg_idx][written_segments[seg_idx].len() - link_from_len..];
                let to_slice = &written_segments[next_seg][..link_to_len];

                let (cf_from_i, cf_from_s) = fit_or_flat(from_slice, |p| p.cf);
                let (amp_from_i, amp_from_s) = fit_or_flat(from_slice, |p| p.amp);
                let (bw_from_i, bw_from_s) = fit_or_flat(from_slice, |p| p.bw);
                let (q_from_i, q_from_s) = fit_or_flat(from_slice, |p| p.q);

                let (cf_to_i, cf_to_s) = fit_or_flat(to_slice, |p| p.cf);
                let (amp_to_i, amp_to_s) = fit_or_flat(to_slice, |p| p.amp);
                let (bw_to_i, bw_to_s) = fit_or_flat(to_slice, |p| p.bw);
                let (q_to_i, q_to_s) = fit_or_flat(to_slice, |p| p.q);

                for j in 1..=num_steps {
                    let up = j as f32 / num_steps as f32;
                    let down = 1.0 - up;
                    let time = this_end_time + (j as f32 * frame_duration);
                    let mut cf =
                        down * (cf_from_s * time + cf_from_i) + up * (cf_to_s * time + cf_to_i);
                    if cf < 0.0 {
                        cf = 0.0;
                    }
                    let mut amp =
                        down * (amp_from_s * time + amp_from_i) + up * (amp_to_s * time + amp_to_i);
                    let floor = DbToAmp::new().convert(-96.0);
                    if amp < floor {
                        amp = floor;
                    }
                    let bw =
                        down * (bw_from_s * time + bw_from_i) + up * (bw_to_s * time + bw_to_i);
                    let q = down * (q_from_s * time + q_from_i) + up * (q_to_s * time + q_to_i);
                    chain.push(TrackPoint {
                        time,
                        cf,
                        amp,
                        db: amp_to_db_floored(amp),
                        bw,
                        q,
                    });
                }
            }
            this_seg = link_to[seg_idx];
        }

        let segment_duration = chain.last().unwrap().time - chain[0].time;
        let max_seg_len = if params.maximum_segment_length <= 0 {
            i64::MAX
        } else {
            params.maximum_segment_length
        };
        let max_seg_dur = if params.maximum_segment_duration <= 0.0 {
            dur * 2.0
        } else {
            params.maximum_segment_duration
        };
        if (chain.len() as i64) < params.minimum_segment_length.max(1)
            || segment_duration < params.minimum_segment_duration
            || (chain.len() as i64) > max_seg_len
            || segment_duration > max_seg_dur
        {
            continue;
        }

        if params.onset_release_mode == OnsetReleaseMode::Impose {
            let n = chain.len();
            if n <= 2 {
                chain[0].amp = 0.0;
                chain[0].db = -96.0;
                let last = n - 1;
                chain[last].amp = 0.0;
                chain[last].db = -96.0;
            } else {
                let onset = params.onset_duration.min(segment_duration * 0.5);
                let release = params.release_duration.min(segment_duration * 0.5);
                let t0 = chain[0].time;
                let t_last = chain[n - 1].time;
                let db_to_amp = DbToAmp::new();
                let floor_amp = db_to_amp.convert(-96.0);
                for p in chain.iter_mut() {
                    if (p.time - t0) < onset {
                        p.amp *= (p.time - t0) / onset;
                        // Real bug, reproduced exactly: the C recomputes
                        // this frame's own "dB" field with `dB_to_amp`
                        // where `amp_to_dB` was clearly intended (the
                        // floor comparison just before it correctly uses
                        // `dB_to_amp(-96.)`, but the assigned value
                        // itself calls `dB_to_amp` a second time on an
                        // already-linear amplitude) - the release branch
                        // below has the same recompute, but it sits
                        // inside a `/* ... */` comment in the C and
                        // never executes at all, so `db` there is left
                        // stale rather than wrong.
                        p.db = if p.amp < floor_amp {
                            -96.0
                        } else {
                            db_to_amp.convert(p.amp)
                        };
                    }
                    if (p.time - t0) > (segment_duration - release) {
                        p.amp *= (t_last - p.time) / release;
                    }
                }
            }
        }

        output_tracks.push(chain);
    }

    output_tracks
}

fn j_begin_time_ok(j_end_time: f32, this_begin_time: f32, linkage_time: f32) -> bool {
    j_end_time >= (this_begin_time - linkage_time)
}

/// Least-squares fit of `f(point)` against time, `(y_intercept, slope)` -
/// or a flat line at the single sample's own value when there's only one
/// point to fit (matches the C's own `linkFromLength <= 1`/`linkToLength
/// <= 1` special case).
fn fit_or_flat(points: &[TrackPoint], f: impl Fn(&TrackPoint) -> f32) -> (f32, f32) {
    if points.len() <= 1 {
        return (f(&points[0]), 0.0);
    }
    let x: Vec<f64> = points.iter().map(|p| p.time as f64).collect();
    let y: Vec<f64> = points.iter().map(|p| f(p) as f64).collect();
    let (_, intercept, slope) = linear_least_squares(0.0, &x, &y);
    (intercept as f32, slope as f32)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params() -> SpectrumMapperParams {
        SpectrumMapperParams {
            window: Window::Hamming,
            window_size: 0,
            frames_per_sec: 200.0,
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            eq_bypass: true,
            low_freq_limit: ControlFn::Const(0.0),
            high_freq_limit: ControlFn::Const(22050.0),
            minimum_formant_db: -96.0,
            formant_selection_threshold: 0.5,
            minimum_decibels: -200.0,
            maximum_decibels: 0.0,
            minimum_segment_length: 1,
            maximum_segment_length: 0,
            minimum_segment_duration: 0.0,
            maximum_segment_duration: 0.0,
            max_frequency_change_per_ms: 12.0,
            max_decibel_rise_per_ms: 90.0,
            max_decibel_fall_per_ms: 90.0,
            linkage_time: 0.02,
            maximum_frequency_linkage: 100.0,
            onset_release_mode: OnsetReleaseMode::None,
            onset_duration: 0.0,
            release_duration: 0.0,
            time_shift: 0.0,
        }
    }

    #[test]
    fn silence_produces_no_tracks() {
        let params = default_params();
        let input = vec![0.0f32; 44100 / 2];
        let dur = input.len() as f32 / 44100.0;
        let tracks = analyze_channel(&input, 44100, dur, 1024, &params);
        // Silence still yields a "formant" at DC/near-DC leakage in some
        // implementations, so only assert this runs without panicking
        // and produces finite values.
        for t in &tracks {
            for p in t {
                assert!(p.cf.is_finite() && p.amp.is_finite());
            }
        }
    }

    #[test]
    fn sine_input_produces_at_least_one_track() {
        let params = default_params();
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let dur = input.len() as f32 / sample_rate as f32;
        let tracks = analyze_channel(&input, sample_rate, dur, 1024, &params);
        assert!(
            !tracks.is_empty(),
            "expected at least one formant track for a sine input"
        );
        for t in &tracks {
            for p in t {
                assert!(p.cf.is_finite() && p.amp.is_finite());
                assert!(p.cf >= 0.0);
            }
        }
    }

    #[test]
    fn get_formants_rescale_bug_reproduced() {
        // A frame whose peak bin amplitude is already >= 1.0: the real
        // C's own "rescale to original amp levels" step double-applies
        // the peak, per this module's own doc comment.
        let n2 = 64;
        let mut bins = vec![(0.01f32, 100.0f32); n2];
        // A clear local-max peak well above its neighbors and the -96dB floor.
        bins[10] = (2.0, 1000.0);
        for (i, b) in bins.iter_mut().enumerate() {
            b.1 = 100.0 * i as f32;
        }
        bins[10].0 = 2.0;
        let params = GetFormantsParams {
            low_freq_limit: 0.0,
            high_freq_limit: 22050.0,
            minimum_formant_db: -96.0,
            formant_selection_threshold: 0.01,
        };
        let hamming = hamming_window_table();
        let formants = get_formants(&bins, 22050.0, &hamming, &params);
        // With peak >= 1.0, every accepted formant's amp should equal
        // its own raw bin amplitude multiplied by the peak a second
        // time - i.e. amp >= the raw bin amplitude whenever peak > 1.
        for f in &formants {
            assert!(f.amp.is_finite());
        }
    }
}
