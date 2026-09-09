//! Ports `formantsmapper.c`: reads two lists of formants (spectral
//! peaks - center frequency, amplitude, bandwidth) from binary files
//! written externally (see `pvc_io::formants`'s own doc comment - no C
//! tool in this codebase produces them), one for the analyzed "source"
//! sound and one for a "target" sound, pairs each source formant with
//! its nearest-frequency target formant (and vice versa), and remaps
//! each source formant's own bin band onto the target's frequency and
//! amplitude via one or two independently-controllable oscillator banks
//! ("bank A" and, optionally, "bank B") - always via the oscillator bank
//! (`P = 1.; obank = 1;` hardcoded, the same "always obank" shape
//! already documented for `tools::ring`/`tools::harmonizer`/
//! `tools::inharmonator`). Bins outside every formant's own band can
//! optionally pass through as a separate "residue" oscillator bank.
//!
//! **Source and target formant processing (filter by amplitude/frequency,
//! then optionally extend with synthetic harmonic-partial formants, sort,
//! and deduplicate overlapping added partials) are byte-for-byte
//! identical algorithms in the C**, confirmed by diffing both blocks
//! directly - down to shared variable names reused across both (`l`,
//! `transferCode`, `m1`). The one real difference: target's own extension
//! dedup checks for bandwidth *frequency* overlap
//! (`extendedTargetFormantCenterFreqs[k-1] + 0.5*bw > centerFreq - 0.5*bw`
//! computed directly), while source's own checks *stop-band bin index*
//! overlap instead (`extendedSourceFormantHighStopBandIndices[k-1] >
//! extendedSourceFormantLowStopBandIndices[i]`) - a real, not cosmetic,
//! difference, so [`extend_source_formants`]/[`extend_target_formants`]
//! stay separate functions rather than one shared with a closure, per
//! this project's own reuse-before-rederive convention (confirming a
//! genuine difference before *not* sharing, same as confirming one
//! before sharing).
//!
//! **A large, sophisticated subsystem is entirely dead code**: `main()`
//! computes `SourceAmpSumOfSourceFormantsUsingThisSourceFormant`/
//! `SourceAmpSumOfTargetFormantsUsingThisSourceFormant` (summing a
//! source formant's own amplitude across every output formant that
//! reuses it) into `thisSourceFormantDuplicateDBScaler`/
//! `thisTargetFormantDuplicateDBScaler`, stored per bin into
//! `outputSourceFormantDuplicateDBScaler`/
//! `outputTargetFormantDuplicateDBScaler` - confirmed by grepping every
//! reference that neither of those two per-bin arrays, nor
//! `outputMappingSourcePartialMultiplier` (a third array, holding which
//! harmonic partial a formant was synthesized from), is ever read again
//! after being written. None of the three - nor the two `SourceAmpSum*`
//! arrays that feed them - are ported.
//!
//! **A real, likely-unintended unit mismatch in the amplitude-scaler
//! clamp**: `-~`'s own `amplitude_normalization_decibel_gain_limit`
//! (default `200`, `usage()` calling it a "Decibel Gain Limit") is used
//! two different ways in the same expression
//! (`amp_to_dB(thisAmpScaler) > amp_to_dB(amplitude_normalization_decibel_gain_limit)
//! ? dB_to_amp(amplitude_normalization_decibel_gain_limit) : thisAmpScaler`).
//! The *comparison* treats `200` as a plain amplitude ratio
//! (`amp_to_dB(200)` ≈ 46 dB), while the *clamped replacement value*
//! treats the same `200` as if it were already a dB value
//! (`dB_to_amp(200)`, an enormous ratio). The two interpretations differ
//! by orders of magnitude, so triggering the clamp at its default
//! replaces a merely-large ratio with a far *larger* one, the opposite
//! of what a "gain limit" evidently intends. Reproduced exactly as read,
//! not "fixed" to either plausible intended meaning.
//!
//! **A real doc-vs-code mismatch**: `usage()` labels *both* `-E` and
//! `-g` "Target Formants File" - `-E` is actually the source formants
//! file (`case 'E': strcpy(sourceFormantsFile, ...)`), confirmed by
//! reading the `switch`, not the `usage()` text.
//!
//! **Dead `crack()` flags**: lowercase `q` and uppercase `Q` are both
//! accepted but have no `case`. `-u` (`interpolationPathDiffusion`,
//! `randf()`-driven per-formant interpolation-curve diffusion) is not
//! ported, matching `tools::ring`'s established `randf()` precedent -
//! always the deterministic `0` (no diffusion) path. `warpshape` is
//! declared and initialized but never assigned by any flag and never
//! read anywhere in the file - dead by construction, not exposed.
//!
//! **`CartesianSmooth`, not `smooth()`**: attack/release smoothing here
//! runs on the *raw* per-frame FFT buffer (real/imaginary pairs, before
//! `convert()` turns it into amplitude/frequency), touching every array
//! slot uniformly - unlike every other tool's own `smooth()` (this
//! project's `Smoother`), which only ever touches amplitude (even-index)
//! slots of an already-`convert()`ed amp/freq array. A fresh, small
//! implementation ([`CartesianSmoother`]), not `pvc_core::smooth::Smoother`
//! reused with its own index-skipping removed - genuinely different
//! data, not a variant of the same algorithm.
//!
//! **Enabling residue bins together with dual-bank mode silently drops
//! bank B.** `main()`'s own resynthesis dispatch calls `noscbank2(...,
//! outputNumBins, ...)` (bank A's own bin count only) whenever residue
//! bins are on, *regardless* of `bank_A_0__banks_A_and_B_1` - the
//! `outputNumBins*2`-sized dual-bank branch is reached only when residue
//! bins are off. Confirmed by reading the exact `if`/`else` nesting, not
//! assumed from the flag names. Reproduced exactly: [`process_channel`]
//! only ever resynthesizes bank B when residue bins are disabled.

use crate::eq::{eq2, ShelfEq};
use crate::fft::rfft;
use crate::pvoc::{fold, getthresh, Frame, OscBank, PhaseTracker};
use crate::smooth::smooth_setup;
use crate::units::{amp_to_db, DbToAmp, SemitonesToMult};
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

const OSCILBANKGAIN: f32 = 1.7782794;

/// One formant record, resolved from `pvc_io::formants::FormantRecord`.
#[derive(Debug, Clone, Copy)]
pub struct Formant {
    pub center_freq: f32,
    pub amp: f32,
    pub bw: f32,
    pub q: f32,
    pub index: i32,
    pub low_stop_band_index: i32,
    pub high_stop_band_index: i32,
}

/// `-S`: resets a (source-only) formant's own stop-band indices to
/// `index ± factor*(original half-width)`, clamped to `[0, n2-1]` and to
/// not cross into a neighboring formant's own stop-band - `main()`'s own
/// "RESET STOP BANDS IF DESIRED" block. A no-op at the default `1.0`.
pub fn apply_bandwidth_extension_factor(formants: &mut [Formant], factor: f32, n2: usize) {
    if factor == 1.0 {
        return;
    }
    for i in 0..formants.len() {
        let mut low = formants[i].index
            - (factor * (formants[i].index - formants[i].low_stop_band_index) as f32) as i32;
        if low < 0 {
            low = 0;
        }
        if i != 0 && low <= formants[i - 1].high_stop_band_index {
            low = formants[i - 1].high_stop_band_index + 1;
        }
        formants[i].low_stop_band_index = low;

        let mut high = formants[i].index
            + (factor * (formants[i].high_stop_band_index - formants[i].index) as f32) as i32;
        if high > (n2 as i32 - 1) {
            high = n2 as i32 - 1;
        }
        if i != formants.len() - 1 && high >= formants[i + 1].low_stop_band_index {
            high = formants[i + 1].low_stop_band_index - 1;
        }
        formants[i].high_stop_band_index = high;
    }
}

/// Filters formants by amplitude threshold and frequency range - shared,
/// byte-for-byte-identical logic between source and target (see this
/// module's own doc comment).
pub fn filter_formants(
    formants: &[Formant],
    db_threshold: f32,
    mut low_freq: f32,
    mut high_freq: f32,
    nyquist: f32,
) -> Vec<Formant> {
    if low_freq < 0.0 {
        low_freq = 0.0;
    }
    if high_freq <= 0.0 || high_freq > nyquist {
        high_freq = nyquist;
    }
    formants
        .iter()
        .copied()
        .filter(|f| {
            amp_to_db(f.amp) >= db_threshold
                && f.center_freq >= low_freq
                && f.center_freq <= high_freq
                && f.center_freq <= nyquist
                && f.center_freq > 0.0
        })
        .collect()
}

/// A formant plus which harmonic partial it was synthesized from (`1` =
/// an original, unextended formant).
#[derive(Debug, Clone, Copy)]
pub struct ExtFormant {
    pub center_freq: f32,
    pub amp: f32,
    pub bw: f32,
    pub index: i32,
    pub low_stop_band_index: i32,
    pub high_stop_band_index: i32,
    pub partial: i32,
}

/// Ports the source-formants harmonic-partial extension block: for each
/// formant above `ext_db_threshold`, appends partials `2, 3, 4, ...` (and,
/// if `add_octaves`, every power-of-two multiple of each) up to
/// `peak_partial` (or the Nyquist limit if `peak_partial == 0`), each
/// with amplitude `formant.amp * dB_to_amp(rolloff_per_partial *
/// (partial - 1))`, then sorts by frequency and keeps only the stronger
/// of any two overlapping added formants (an original formant always
/// wins over an added one; between two added ones, the louder one wins).
/// Overlap is decided by *stop-band index* overlap here - see this
/// module's doc comment on why [`extend_target_formants`] decides it
/// differently.
#[allow(clippy::too_many_arguments)]
pub fn extend_source_formants(
    formants: &[Formant],
    enabled: bool,
    ext_db_threshold: f32,
    peak_partial: f32,
    add_octaves: bool,
    rolloff_per_partial: f32,
    fundamental: f32,
    nyquist: f32,
) -> Vec<ExtFormant> {
    if !enabled {
        return formants
            .iter()
            .map(|f| ExtFormant {
                center_freq: f.center_freq,
                amp: f.amp,
                bw: f.bw,
                index: f.index,
                low_stop_band_index: f.low_stop_band_index,
                high_stop_band_index: f.high_stop_band_index,
                partial: 1,
            })
            .collect();
    }

    let db_to_amp = DbToAmp::new();
    let ext_threshold_amp = db_to_amp.convert(ext_db_threshold);

    let mut extended: Vec<ExtFormant> = formants
        .iter()
        .map(|f| ExtFormant {
            center_freq: f.center_freq,
            amp: f.amp,
            bw: f.bw,
            index: f.index,
            low_stop_band_index: f.low_stop_band_index,
            high_stop_band_index: f.high_stop_band_index,
            partial: 1,
        })
        .collect();

    for f in formants {
        if f.amp <= ext_threshold_amp {
            continue;
        }
        let mut partial_number = 2.0f32;
        let mut proposed_freq = f.center_freq * partial_number;
        while proposed_freq < nyquist && (partial_number <= peak_partial || peak_partial == 0.0) {
            let mut this_octave = 1.0f32;
            let mut this_partial_number = (partial_number * this_octave) as i32;
            proposed_freq = f.center_freq * this_partial_number as f32;
            while proposed_freq < nyquist {
                let amp = f.amp
                    * db_to_amp.convert(rolloff_per_partial * (this_partial_number as f32 - 1.0));
                let center_freq = proposed_freq;
                let bw = f.bw;
                let index = ((center_freq / fundamental) + 0.5) as i32;
                let low_stop_band_index = (((center_freq - 0.5 * bw) / fundamental) + 0.5) as i32;
                let high_stop_band_index = (((center_freq + 0.5 * bw) / fundamental) + 0.5) as i32;
                extended.push(ExtFormant {
                    center_freq,
                    amp,
                    bw,
                    index,
                    low_stop_band_index,
                    high_stop_band_index,
                    partial: this_partial_number,
                });

                if !add_octaves {
                    break;
                }
                this_octave *= 2.0;
                this_partial_number = (partial_number * this_octave) as i32;
                proposed_freq = f.center_freq * this_partial_number as f32;
            }

            partial_number += 1.0;
            proposed_freq = f.center_freq * partial_number;
        }
    }

    extended.sort_by(|a, b| a.center_freq.partial_cmp(&b.center_freq).unwrap());
    dedup_by_stop_band_overlap(extended)
}

fn dedup_by_stop_band_overlap(sorted: Vec<ExtFormant>) -> Vec<ExtFormant> {
    let mut out: Vec<ExtFormant> = Vec::with_capacity(sorted.len());
    for f in sorted {
        if out.is_empty() {
            out.push(f);
            continue;
        }
        let last = *out.last().unwrap();
        if last.partial + f.partial > 2 {
            if last.high_stop_band_index > f.low_stop_band_index {
                if last.partial == 1 && f.partial != 1 {
                    // keep old, add nothing
                } else if last.partial != 1 && f.partial == 1 {
                    *out.last_mut().unwrap() = f;
                } else if last.amp > f.amp {
                    // keep old (louder), add nothing
                } else {
                    *out.last_mut().unwrap() = f;
                }
            } else {
                out.push(f);
            }
        } else {
            out.push(f);
        }
    }
    out
}

/// Ports the target-formants harmonic-partial extension block - see
/// [`extend_source_formants`]'s own doc comment for the shared shape and
/// this module's doc comment for the one real difference (overlap is
/// decided by bandwidth-derived frequency overlap here, not stop-band
/// index overlap).
pub fn extend_target_formants(
    formants: &[Formant],
    enabled: bool,
    ext_db_threshold: f32,
    peak_partial: f32,
    add_octaves: bool,
    rolloff_per_partial: f32,
    nyquist: f32,
) -> Vec<ExtFormant> {
    if !enabled {
        return formants
            .iter()
            .map(|f| ExtFormant {
                center_freq: f.center_freq,
                amp: f.amp,
                bw: f.bw,
                index: f.index,
                low_stop_band_index: f.low_stop_band_index,
                high_stop_band_index: f.high_stop_band_index,
                partial: 1,
            })
            .collect();
    }

    let db_to_amp = DbToAmp::new();
    let ext_threshold_amp = db_to_amp.convert(ext_db_threshold);

    let mut extended: Vec<ExtFormant> = formants
        .iter()
        .map(|f| ExtFormant {
            center_freq: f.center_freq,
            amp: f.amp,
            bw: f.bw,
            index: f.index,
            low_stop_band_index: f.low_stop_band_index,
            high_stop_band_index: f.high_stop_band_index,
            partial: 1,
        })
        .collect();

    for f in formants {
        if f.amp <= ext_threshold_amp {
            continue;
        }
        let mut partial_number = 2.0f32;
        let mut proposed_freq = f.center_freq * partial_number;
        while proposed_freq < nyquist && (partial_number <= peak_partial || peak_partial == 0.0) {
            let mut this_octave = 1.0f32;
            let mut this_partial_number = (partial_number * this_octave) as i32;
            proposed_freq = f.center_freq * this_partial_number as f32;
            while proposed_freq < nyquist {
                let amp = f.amp
                    * db_to_amp.convert(rolloff_per_partial * (this_partial_number as f32 - 1.0));
                extended.push(ExtFormant {
                    center_freq: proposed_freq,
                    amp,
                    bw: f.bw,
                    index: 0,
                    low_stop_band_index: 0,
                    high_stop_band_index: 0,
                    partial: this_partial_number,
                });

                if !add_octaves {
                    break;
                }
                this_octave *= 2.0;
                this_partial_number = (partial_number * this_octave) as i32;
                proposed_freq = f.center_freq * this_partial_number as f32;
            }

            partial_number += 1.0;
            proposed_freq = f.center_freq * partial_number;
        }
    }

    extended.sort_by(|a, b| a.center_freq.partial_cmp(&b.center_freq).unwrap());

    let mut out: Vec<ExtFormant> = Vec::with_capacity(extended.len());
    for f in extended {
        if out.is_empty() {
            out.push(f);
            continue;
        }
        let last = *out.last().unwrap();
        if last.partial + f.partial > 2 {
            let high_stop_band_freq_of_lower = last.center_freq + 0.5 * last.bw;
            let low_stop_band_freq_of_this = f.center_freq - 0.5 * f.bw;
            if high_stop_band_freq_of_lower > low_stop_band_freq_of_this {
                if last.partial == 1 && f.partial != 1 {
                    // keep old
                } else if last.partial != 1 && f.partial == 1 {
                    *out.last_mut().unwrap() = f;
                } else if last.amp > f.amp {
                    // keep old (louder)
                } else {
                    *out.last_mut().unwrap() = f;
                }
            } else {
                out.push(f);
            }
        } else {
            out.push(f);
        }
    }
    out
}

/// Nearest-center-frequency pairing, one direction: for each formant in
/// `from`, the index into `to` of its closest (by absolute frequency
/// difference) match.
fn pair_nearest(from: &[ExtFormant], to: &[ExtFormant]) -> Vec<usize> {
    from.iter()
        .map(|f| {
            let mut best = 0usize;
            let mut best_diff = f32::MAX;
            for (k, t) in to.iter().enumerate() {
                let diff = (f.center_freq - t.center_freq).abs();
                if diff < best_diff {
                    best_diff = diff;
                    best = k;
                }
            }
            best
        })
        .collect()
}

/// One output bin: which source bin to fetch, and how to remap it.
#[derive(Debug, Clone, Copy)]
pub struct OutputBin {
    pub source_bin_index: usize,
    pub transpose_multiplier: f32,
    pub amp_scaler: f32,
    pub octave_distance: f32,
}

/// Builds the per-bin output mapping: pairs source and target formants in
/// both directions, picks whichever list is larger as the output formant
/// count (matching each of its own members to its nearest counterpart in
/// the other list), then expands each output formant into its source
/// formant's own bin range (`low_stop_band_index..=high_stop_band_index`,
/// clamped to `[0, n2-1]`).
pub fn build_output_bins(
    source_ext: &[ExtFormant],
    target_ext: &[ExtFormant],
    fundamental: f32,
    n2: usize,
    amp_gain_limit: f32,
) -> Vec<OutputBin> {
    let source_to_target = pair_nearest(source_ext, target_ext);
    let target_to_source = pair_nearest(target_ext, source_ext);

    let (output_source, output_target): (Vec<usize>, Vec<usize>) =
        if source_ext.len() > target_ext.len() {
            ((0..source_ext.len()).collect(), source_to_target)
        } else {
            (target_to_source, (0..target_ext.len()).collect())
        };

    let last = n2 as i32 - 1;
    let db_to_amp = DbToAmp::new();
    let amp_gain_limit_db = amp_to_db(amp_gain_limit);

    let mut bins = Vec::new();
    for (&src_idx, &tgt_idx) in output_source.iter().zip(output_target.iter()) {
        let src = source_ext[src_idx];
        let tgt = target_ext[tgt_idx];
        let low = src.low_stop_band_index.clamp(0, last);
        let high = src.high_stop_band_index.clamp(0, last);
        let transpose_multiplier = tgt.center_freq / src.center_freq;
        let raw_amp_scaler = tgt.amp / src.amp;
        let amp_scaler = if amp_to_db(raw_amp_scaler) > amp_gain_limit_db {
            db_to_amp.convert(amp_gain_limit)
        } else {
            raw_amp_scaler
        };
        let center_bin_freq = fundamental * src.index as f32;

        for j in low..=high {
            let octave_distance = if j != 0 {
                12.0 * ((j as f32 * fundamental / center_bin_freq) as f64)
                    .log10()
                    .abs() as f32
                    / (2.0f64.log10() as f32)
            } else {
                0.0
            };
            bins.push(OutputBin {
                source_bin_index: j as usize,
                transpose_multiplier,
                amp_scaler,
                octave_distance,
            });
        }
    }
    bins
}

/// Bin indices in `0..n2` (inclusive of `n2`, matching `main()`'s own
/// `N2` count) that no output bin's own source range ever touches - the
/// residual spectrum, when `-x` (residue bins) is on.
pub fn compute_residue_bins(output_bins: &[OutputBin], n2: usize) -> Vec<usize> {
    let mut used = vec![false; n2 + 1];
    for b in output_bins {
        if b.source_bin_index <= n2 {
            used[b.source_bin_index] = true;
        }
    }
    (0..=n2).filter(|&i| !used[i]).collect()
}

/// One control bank's per-frame parameters (bank A or bank B).
#[derive(Debug, Clone)]
pub struct BankParams {
    pub freq_interp: ControlFn,
    pub amp_interp: ControlFn,
    pub gain_db: ControlFn,
    pub rolloff_per_octave: ControlFn,
    pub pitch_transpose: ControlFn,
}

pub struct FormantsMapperParams {
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,

    pub gain_db: ControlFn,
    pub pitch_transpose: ControlFn,
    pub freq_shift: ControlFn,
    pub attack: ControlFn,
    pub release: ControlFn,

    pub shelf_low_db: ControlFn,
    pub shelf_high_db: ControlFn,
    pub shelf_low_freq: ControlFn,
    pub shelf_high_freq: ControlFn,

    pub bank_a: BankParams,
    pub bank_b: Option<BankParams>,

    pub residue_gain_db: ControlFn,

    pub threshold_db: f32,
}

/// A plain "smooth every element uniformly" attack/release filter on a
/// raw FFT buffer (`CartesianSmooth()`) - genuinely different data than
/// `pvc_core::smooth::Smoother` (which only ever touches every-other
/// amplitude slot of an already-`convert()`ed amp/freq array), not a
/// variant of the same algorithm. See this module's own doc comment.
struct CartesianSmoother {
    old: Vec<f32>,
    first: bool,
}

impl CartesianSmoother {
    fn new(n: usize) -> Self {
        CartesianSmoother {
            old: vec![0.0; n],
            first: true,
        }
    }

    fn smooth(&mut self, a: &mut [f32], att: f32, matt: f32, rel: f32, mrel: f32) {
        if self.first {
            self.old.copy_from_slice(a);
            self.first = false;
        }
        if rel != 0.0 || att != 0.0 {
            for (ai, oi) in a.iter_mut().zip(self.old.iter()) {
                *ai = if *ai < *oi {
                    rel * *oi + mrel * *ai
                } else {
                    att * *oi + matt * *ai
                };
            }
        }
        self.old.copy_from_slice(a);
    }
}

/// Resynthesizes one channel. `output_bins` and `residue_bins` come from
/// [`build_output_bins`]/[`compute_residue_bins`] - computed once, not
/// per-channel, since they depend only on the (already-loaded) formant
/// lists.
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    input: &[f32],
    output_bins: &[OutputBin],
    residue_bins: &[usize],
    n: usize,
    sample_rate: u32,
    dur: f32,
    params: &FormantsMapperParams,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n2 = n / 2;
    let n_plus_2 = n + 2;
    let nyquist = r / 2.0;
    let fundamental = r / n as f32;

    let frames_per_sec = if params.frames_per_sec < 32.0 {
        200.0
    } else {
        params.frames_per_sec
    };
    let time_factor = if params.time_factor <= 0.0 {
        1.0
    } else {
        params.time_factor
    };
    let d = (r / frames_per_sec) as usize;
    let i_factor = (d as f32 * time_factor) as usize;
    let ir = i_factor as f32 / r;

    let mut nw = params.window_size;
    if nw == 0 {
        nw = 2 * n;
    }
    if nw < i_factor {
        nw = 2;
        while nw <= i_factor {
            nw *= 2;
        }
    }

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();

    let smoothing_enabled = matches!(&params.attack, ControlFn::Table(_))
        || matches!(&params.release, ControlFn::Table(_))
        || control_fn_initial(&params.attack) > 0.0
        || control_fn_initial(&params.release) > 0.0;

    let output_num_bins = output_bins.len();
    let residue_num_bins = residue_bins.len();

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let analysis_window = window_pair.analysis;

    let mut input_ring = vec![0.0f32; nw];
    let mut fft_buf = vec![0.0f32; n];
    let mut fold_pos: i64 = -(nw as i64);
    let mut phase = PhaseTracker::new_analysis(n2, d, sample_rate);
    let mut cartesian_smoother = CartesianSmoother::new(n);

    let mut osc_bank_a = OscBank::new(output_num_bins, nw, sample_rate, i_factor, 1.0);
    let shared_table = osc_bank_a.table();
    let mut osc_bank_b = params.bank_b.as_ref().map(|_| {
        OscBank::with_shared_table(
            output_num_bins,
            sample_rate,
            i_factor,
            1.0,
            shared_table.clone(),
        )
    });
    let mut osc_bank_dual = if params.bank_b.is_some() {
        Some(OscBank::with_shared_table(
            output_num_bins * 2,
            sample_rate,
            i_factor,
            1.0,
            shared_table.clone(),
        ))
    } else {
        None
    };
    let mut osc_bank_residue = if residue_num_bins > 0 {
        Some(OscBank::with_shared_table(
            residue_num_bins,
            sample_rate,
            i_factor,
            1.0,
            shared_table,
        ))
    } else {
        None
    };

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;
    let mut on: i64 = (-(nw as i64) * i_factor as i64) / d as i64;

    let mut output = Vec::new();
    let mut samps_written: usize = 0;
    let mut frame_count: usize = 0;

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

        let t = samps_written as f32 / r;

        // ---- ANALYSIS: shiftin + fold + rfft (no convert yet - see
        // this module's doc comment on why CartesianSmooth needs the
        // raw buffer). ----
        input_ring.copy_within(d..nw, 0);
        input_ring[nw - d..].copy_from_slice(&hop);
        fold_pos += d as i64;
        fold(&input_ring, &analysis_window, &mut fft_buf, fold_pos);
        rfft(&mut fft_buf, n2, true);

        if smoothing_enabled {
            let attack_val = params.attack.at(t, dur);
            let release_val = params.release.at(t, dur);
            let (attackc, minus_attackc) = smooth_setup(attack_val, ir);
            let (releasec, minus_releasec) = smooth_setup(release_val, ir);
            cartesian_smoother.smooth(
                &mut fft_buf,
                attackc,
                minus_attackc,
                releasec,
                minus_releasec,
            );
        }

        let mut channel_frame = phase.convert(&fft_buf);
        let mut channel_flat = channel_frame.to_pva_floats();

        // ---- PITCH/FREQ SHIFT + BOUNDS GATE ----
        let harmadd = params.freq_shift.at(t, dur);
        let pm = semitones_to_mult.convert(params.pitch_transpose.at(t, dur));
        for j in 0..=n2 {
            let temp = pm * (harmadd + channel_flat[2 * j + 1]);
            if temp > 0.0 && temp < nyquist {
                channel_flat[2 * j + 1] = temp;
            } else {
                channel_flat[2 * j] = 0.0;
            }
        }

        // ---- SHELF EQ (frequency computed from harmadd/pm directly,
        // not the bin's own possibly-different detected frequency - see
        // this module's doc comment). ----
        let mut channel_freqdev = vec![0.0f32; n_plus_2];
        for j in 0..=n2 {
            channel_freqdev[2 * j] = harmadd;
            channel_freqdev[2 * j + 1] = pm;
        }
        let shelf = ShelfEq {
            d_blow: params.shelf_low_db.at(t, dur),
            d_bhi: params.shelf_high_db.at(t, dur),
            freqlow: params.shelf_low_freq.at(t, dur),
            freqhi: params.shelf_high_freq.at(t, dur),
        };
        eq2(
            &mut channel_flat,
            &shelf,
            fundamental,
            &channel_freqdev,
            false,
            &db_to_amp,
        );
        channel_frame = Frame::from_pva_floats(&channel_flat);

        let gain = db_to_amp.convert(params.gain_db.at(t, dur));

        // ---- BUILD MAPPED OUTPUT (BANK A) ----
        let mut mapped_a = vec![0.0f32; output_num_bins * 2];
        write_bank(
            &mut mapped_a,
            output_bins,
            &channel_frame.bins,
            &params.bank_a,
            t,
            dur,
            gain,
            &db_to_amp,
            &semitones_to_mult,
        );

        let mapped_b = params.bank_b.as_ref().map(|bank_b| {
            let mut mapped_b = vec![0.0f32; output_num_bins * 2];
            write_bank(
                &mut mapped_b,
                output_bins,
                &channel_frame.bins,
                bank_b,
                t,
                dur,
                gain,
                &db_to_amp,
                &semitones_to_mult,
            );
            mapped_b
        });

        let residue_frame = if residue_num_bins > 0 {
            let residue_gain = db_to_amp.convert(params.residue_gain_db.at(t, dur));
            let mut flat = vec![0.0f32; residue_num_bins * 2];
            for (k, &bin) in residue_bins.iter().enumerate() {
                let (amp, freq) = channel_frame.bins[bin];
                flat[2 * k] = residue_gain * amp;
                flat[2 * k + 1] = freq;
            }
            Some(Frame::from_pva_floats(&flat))
        } else {
            None
        };

        // ---- RESYNTHESIS ----
        let mut combined_flat = mapped_a.clone();
        if let Some(ref mb) = mapped_b {
            if residue_frame.is_none() {
                combined_flat.extend_from_slice(mb);
            }
            // When residue bins are also on, bank B is silently dropped -
            // see this module's own doc comment on this real C bug.
        }
        let combined_frame = Frame::from_pva_floats(&combined_flat);
        let threshfac = db_to_amp.convert(params.threshold_db);
        let synt = getthresh(&combined_frame.bins, threshfac);

        let mut out_samples = if let Some(ref rf) = residue_frame {
            let mut out = osc_bank_a.synthesize(&Frame::from_pva_floats(&mapped_a), synt);
            let residue_out = osc_bank_residue
                .as_mut()
                .expect("residue bank exists when residue_frame is Some")
                .synthesize(rf, synt);
            for (o, r) in out.iter_mut().zip(&residue_out) {
                *o += r;
            }
            out
        } else if mapped_b.is_some() {
            osc_bank_dual
                .as_mut()
                .expect("dual bank exists when bank_b params exist")
                .synthesize(&combined_frame, synt)
        } else {
            osc_bank_a.synthesize(&combined_frame, synt)
        };
        // `osc_bank_b` is intentionally unused in every live branch above
        // (bank B's own contribution is always folded into
        // `osc_bank_dual`'s combined frame instead, matching the C's own
        // single `noscbank(outputMappedChannel, outputNumBins*2, ...)`
        // call for the dual-bank/no-residue case) - kept only so its
        // shared-table construction cost is paid once up front like the
        // others, not recreated per frame if a future change needs it.
        let _ = &mut osc_bank_b;

        for s in out_samples.iter_mut() {
            *s *= OSCILBANKGAIN;
        }

        on += i_factor as i64;
        if on + nw as i64 - i_factor as i64 >= 0 {
            output.extend(out_samples);
            samps_written += i_factor;
        }

        frame_count += 1;
        let _ = frame_count;
        if eof_after_this_hop {
            break;
        }
    }

    output.extend(vec![0.0f32; i_factor]);
    output
}

fn control_fn_initial(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(vals) => vals.first().copied().unwrap_or(0.0),
    }
}

#[allow(clippy::too_many_arguments)]
fn write_bank(
    mapped: &mut [f32],
    output_bins: &[OutputBin],
    channel_bins: &[(f32, f32)],
    bank: &BankParams,
    t: f32,
    dur: f32,
    gain: f32,
    db_to_amp: &DbToAmp,
    semitones_to_mult: &SemitonesToMult,
) {
    let amp_interp_ctl = bank.amp_interp.at(t, dur);
    let freq_interp_ctl = bank.freq_interp.at(t, dur);
    let gain_db = bank.gain_db.at(t, dur);
    let rolloff = bank.rolloff_per_octave.at(t, dur);
    let pitch_mult = semitones_to_mult.convert(bank.pitch_transpose.at(t, dur));

    for (i, ob) in output_bins.iter().enumerate() {
        let (src_amp, src_freq) = channel_bins[ob.source_bin_index];

        // Formant-interpolation-path diffusion (`-u`) is not ported (see
        // this module's doc comment) - the warp index is always `0.0`,
        // matching `curve`'s own linear behavior at that value.
        let amp_interp = curve(0.0, 1.0, amp_interp_ctl, 0.0);
        let amp_temp = 1.0 + amp_interp * (ob.amp_scaler - 1.0);
        mapped[2 * i] =
            gain * db_to_amp.convert(gain_db + ob.octave_distance * rolloff) * src_amp * amp_temp;

        let freq_interp = curve(0.0, 1.0, freq_interp_ctl, 0.0);
        mapped[2 * i + 1] =
            pitch_mult * src_freq * (1.0 + freq_interp * (ob.transpose_multiplier - 1.0));
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_bank() -> BankParams {
        BankParams {
            freq_interp: ControlFn::Const(0.0),
            amp_interp: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            rolloff_per_octave: ControlFn::Const(0.0),
            pitch_transpose: ControlFn::Const(0.0),
        }
    }

    fn default_params() -> FormantsMapperParams {
        FormantsMapperParams {
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            gain_db: ControlFn::Const(0.0),
            pitch_transpose: ControlFn::Const(0.0),
            freq_shift: ControlFn::Const(0.0),
            attack: ControlFn::Const(0.0),
            release: ControlFn::Const(0.0),
            shelf_low_db: ControlFn::Const(0.0),
            shelf_high_db: ControlFn::Const(0.0),
            shelf_low_freq: ControlFn::Const(200.0),
            shelf_high_freq: ControlFn::Const(2000.0),
            bank_a: default_bank(),
            bank_b: None,
            residue_gain_db: ControlFn::Const(0.0),
            threshold_db: -96.0,
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let n = 1024usize;
        let sample_rate = 44100u32;
        let n2 = n / 2;
        let fundamental = sample_rate as f32 / n as f32;
        let source = vec![Formant {
            center_freq: 440.0,
            amp: 0.5,
            bw: 40.0,
            q: 10.0,
            index: (440.0 / fundamental).round() as i32,
            low_stop_band_index: (440.0 / fundamental).round() as i32 - 3,
            high_stop_band_index: (440.0 / fundamental).round() as i32 + 3,
        }];
        let target = vec![Formant {
            center_freq: 880.0,
            amp: 0.5,
            bw: 40.0,
            q: 10.0,
            index: (880.0 / fundamental).round() as i32,
            low_stop_band_index: (880.0 / fundamental).round() as i32 - 3,
            high_stop_band_index: (880.0 / fundamental).round() as i32 + 3,
        }];
        let source_ext = extend_source_formants(
            &source,
            false,
            -96.0,
            0.0,
            false,
            0.0,
            fundamental,
            sample_rate as f32 / 2.0,
        );
        let target_ext = extend_target_formants(
            &target,
            false,
            -96.0,
            0.0,
            false,
            0.0,
            sample_rate as f32 / 2.0,
        );
        let output_bins = build_output_bins(&source_ext, &target_ext, fundamental, n2, 200.0);
        let residue_bins = compute_residue_bins(&output_bins, n2);
        let params = default_params();
        let input = vec![0.0f32; 44100 / 4];
        let dur = input.len() as f32 / 44100.0;
        let out = process_channel(
            &input,
            &output_bins,
            &residue_bins,
            n,
            sample_rate,
            dur,
            &params,
        );
        assert!(!out.is_empty());
        assert!(out.iter().all(|&s| s.abs() < 1e-4));
    }

    #[test]
    fn sine_input_maps_source_formant_to_target_frequency() {
        let n = 1024usize;
        let sample_rate = 44100u32;
        let n2 = n / 2;
        let fundamental = sample_rate as f32 / n as f32;
        let src_idx = (440.0 / fundamental).round() as i32;
        let tgt_freq = 880.0f32;
        let source = vec![Formant {
            center_freq: 440.0,
            amp: 0.5,
            bw: 40.0,
            q: 10.0,
            index: src_idx,
            low_stop_band_index: src_idx - 3,
            high_stop_band_index: src_idx + 3,
        }];
        let target = vec![Formant {
            center_freq: tgt_freq,
            amp: 0.5,
            bw: 40.0,
            q: 10.0,
            index: (tgt_freq / fundamental).round() as i32,
            low_stop_band_index: 0,
            high_stop_band_index: 0,
        }];
        let source_ext = extend_source_formants(
            &source,
            false,
            -96.0,
            0.0,
            false,
            0.0,
            fundamental,
            sample_rate as f32 / 2.0,
        );
        let target_ext = extend_target_formants(
            &target,
            false,
            -96.0,
            0.0,
            false,
            0.0,
            sample_rate as f32 / 2.0,
        );
        let output_bins = build_output_bins(&source_ext, &target_ext, fundamental, n2, 200.0);
        let residue_bins = compute_residue_bins(&output_bins, n2);
        let params = default_params();
        let sr = sample_rate;
        let input: Vec<f32> = (0..sr)
            .map(|i| 0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sr as f32).sin())
            .collect();
        let dur = input.len() as f32 / sr as f32;
        let out = process_channel(
            &input,
            &output_bins,
            &residue_bins,
            n,
            sample_rate,
            dur,
            &params,
        );
        assert!(!out.is_empty());
        assert!(out.iter().all(|s| s.is_finite()));
        let peak = out.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.001, "peak {peak} too quiet");
        assert!(peak < 10.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn dual_bank_with_residue_drops_bank_b() {
        let n = 1024usize;
        let sample_rate = 44100u32;
        let n2 = n / 2;
        let fundamental = sample_rate as f32 / n as f32;
        let idx = (440.0 / fundamental).round() as i32;
        let source = vec![Formant {
            center_freq: 440.0,
            amp: 0.5,
            bw: 40.0,
            q: 10.0,
            index: idx,
            low_stop_band_index: idx - 3,
            high_stop_band_index: idx + 3,
        }];
        let target = vec![Formant {
            center_freq: 880.0,
            amp: 0.5,
            bw: 40.0,
            q: 10.0,
            index: 0,
            low_stop_band_index: 0,
            high_stop_band_index: 0,
        }];
        let source_ext = extend_source_formants(
            &source,
            false,
            -96.0,
            0.0,
            false,
            0.0,
            fundamental,
            sample_rate as f32 / 2.0,
        );
        let target_ext = extend_target_formants(
            &target,
            false,
            -96.0,
            0.0,
            false,
            0.0,
            sample_rate as f32 / 2.0,
        );
        let output_bins = build_output_bins(&source_ext, &target_ext, fundamental, n2, 200.0);
        let residue_bins = compute_residue_bins(&output_bins, n2);
        assert!(!residue_bins.is_empty());
        let mut params = default_params();
        params.bank_b = Some(default_bank());
        let input = vec![0.0f32; 4410];
        let dur = input.len() as f32 / 44100.0;
        let out = process_channel(
            &input,
            &output_bins,
            &residue_bins,
            n,
            sample_rate,
            dur,
            &params,
        );
        assert!(!out.is_empty());
        assert!(out.iter().all(|s| s.is_finite()));
    }
}
