//! Ports `pitchtracker.c`: detects a fundamental frequency per analysis
//! frame over three possible methods, then a second pass note-stabilizes
//! (`-j`/`-J`), gates/holds through silence, smooths, and outputs the
//! result in one of six frequency-unit formats. Like
//! `pvc-core::tools::{envelope,centroid,fluxoid}`, this never
//! resynthesizes audio - it outputs a time-series of scalar values.
//!
//! Doc corrections found by reading the source directly (both
//! `usage()`'s printed banner and this project's own pre-existing
//! parameter-inventory doc have these backwards):
//! - `--band-low` really defaults to `0` Hz - `pitchtracker.c`'s own
//!   `usage()` prints `[13]`, but the actual initializer is
//!   `lowfreq.A[0] = 0.` (a stray copy-pasted comment mislabels it "HIGH
//!   FREQUENCY BOUND").
//! - The ASCII/float output type really defaults to **ASCII**, not
//!   float - `usage()` prints `[1]` (float), but the actual initializer
//!   is `int outtype = 0` (ASCII), confirmed independently by this
//!   tool's own golden case (`tests/golden/cases/pitchtracker/
//!   basic_ascii.toml` invokes the tool with no `-g` flag at all and
//!   expects ASCII text output).
//!
//! `--band-low`/`--band-high`/`--reference` all use pitchtracker's own
//! octave.pitchclass threshold, `<= 12.0` (not the `< 3.0`/`< 15.0`
//! thresholds `envelope`/`centroid`/`fluxoid` use) - a real, tool-
//! specific convention, not a shared one, confirmed by reading every
//! site that resolves one of these three values.
//!
//! **Real, deliberately faithful bug**: multi-channel amplitude
//! combination under `--channel-method average` (the default) is
//! completely broken in the C - it computes a genuine running average
//! into a variable (`averageAmp`) that is then never used, and instead
//! writes each channel's own unmodified value, so the final combined
//! amplitude ends up as simply the *last* channel's own value, not an
//! average of all of them. The parallel frequency-combination code one
//! block down does *not* have this bug (it correctly reuses its own
//! `freqnow` variable) - a real, asymmetric divergence between two
//! near-identical-looking blocks, reproduced faithfully in
//! [`combine_channels_amp`] (distinct from [`combine_channels_freq`]).
//! Peak mode (`--channel-method peak`) is unaffected - both blocks
//! combine correctly there.
//!
//! **Real, deliberately faithful bug**: pass 1's frequency output has a
//! duplicate-write mechanism (`backlogOutputFreqs`) that writes an
//! ever-growing *count* of copies of `-1.0` (the "no candidate found"
//! sentinel) for each consecutive frame that fails to detect a pitch,
//! but the amplitude output always writes exactly one value per frame -
//! so pass 1's frequency and amplitude series can end up different
//! lengths, and pass 2 (which reads one value from each, in lockstep)
//! ends when *either* series runs out. Reproduced faithfully by building
//! two independently-lengthed sequences in [`analyze_channel`] rather
//! than one aligned per-frame pair.
//!
//! **Real quirk, reproduced faithfully**: `--band-low`/`--band-high`'s
//! per-frame control-function evaluation only happens in pass 1; pass 2
//! reuses whatever the *last pass-1 frame* evaluated them to, with no
//! re-evaluation of its own - invisible under this tool's own constant-
//! control-function golden case, but a real quirk for a time-varying
//! `--band-low`/`--band-high` table (same general "state outlives its
//! own pass" pattern already found in `envelope`/`fluxoid`/
//! `harmonizer`).
//!
//! Not ported: the `ANALYSIS_DATA_FILE` alternate input mode (reading a
//! pre-computed analysis file instead of doing its own FFT - a secondary
//! input path this tool's own golden case doesn't exercise, matching
//! this project's established practice of deferring unexercised
//! secondary input modes).

use crate::pvoc::Analyzer;
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

pub use crate::tools::envelope::ChannelMethod;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DetectMethod {
    OptimalComb,
    Strongest,
    Centroid,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OutputFormat {
    Freq,
    OctaveDecimal,
    SemitonesDeviation,
    NegSemitonesDeviation,
    Midi,
    OctavePitchclass,
}

#[derive(Debug, Clone)]
pub struct PitchtrackerParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub band_low: ControlFn,
    /// `<= 12` means octave.pitchclass, resolved via
    /// [`crate::response::oppc_to_hz`]; `< 0` means Nyquist.
    pub band_high: ControlFn,
    pub method: DetectMethod,
    /// `-j`: beginning note-stabilization buffer size in seconds.
    pub window_min_secs: f32,
    /// `-J`: maximum note-stabilization buffer size in seconds.
    pub window_max_secs: f32,
    /// `-d`: per-frame candidate threshold, relative to that frame's own
    /// peak bin, and later reused as the voiced/unvoiced gate on the
    /// normalized envelope.
    pub detect_threshold_db: f32,
    /// `-H`: temporal mode-filter window, in seconds (`0` disables it).
    pub mode_filter_window_secs: f32,
    /// `-E`: amplitude-weighted oversampling factor (`0` disables it).
    pub oversample_factor: f32,
    /// `-a`: one-pole lowpass on the output frequency alone (no attack/
    /// release asymmetry).
    pub smooth_response_secs: f32,
    pub channel_method: ChannelMethod,
    pub compress_threshold_db: ControlFn,
    pub compress_amount_db: ControlFn,
    pub gate_threshold_db: ControlFn,
    pub warp: ControlFn,
    pub attack: ControlFn,
    pub release: ControlFn,
    pub output_rate: f32,
    pub output_format: OutputFormat,
    pub reference_pitch: ControlFn,
}

const FREQUENCY_PROPORTION_THRESHOLD: f32 = 0.99;

fn mid_c() -> f32 {
    (220.0 * 2.0f64.powf(3.0 / 12.0)) as f32
}

fn hz_to_midi(hz: f32) -> f32 {
    60.0 + 12.0 * (hz / mid_c()).log10() / 2.0f32.log10()
}

fn midi_to_hz(midi: f32) -> f32 {
    mid_c() * 2.0f32.powf((midi - 60.0) / 12.0)
}

/// Resolves `--band-low`/`--band-high`/`--reference`'s own
/// octave.pitchclass convention: `<= 12.0` is octave.pitchclass
/// (converted via [`crate::response::oppc_to_hz`]), otherwise a literal
/// Hz value. A negative value (this project's own established CLI
/// sentinel, not present in the C) resolves to Nyquist.
fn resolve_freq(value: f32, nyquist: f32) -> f32 {
    if value < 0.0 {
        nyquist
    } else if value <= 12.0 {
        crate::response::oppc_to_hz(value)
    } else {
        value
    }
}

/// Ports `findMode()` (`legacy/pvc_lib/findMode.c`): a shrinking
/// histogram-mode finder - starts at 100 bins, merges adjacent bins
/// within `merge_width`, and accepts the top (by count) bin as the mode
/// only if it's at least 1.2x more populous than the second-place bin
/// ("modal prominence"); otherwise shrinks the bin count and retries,
/// down to a minimum of 3 bins (below which it accepts unconditionally).
/// A real, deliberately faithful quirk: when the second-place bin's
/// count is `0`, the prominence ratio is `+inf` in both C and Rust
/// `f32`, which trivially passes the `> 1.2` test on the very first
/// (100-bin) pass - not special-cased, matching the C exactly.
fn find_mode(values: &[f32], merge_width: f32) -> f32 {
    let low = values.iter().copied().fold(f32::MAX, f32::min);
    let high = values.iter().copied().fold(f32::MIN, f32::max);

    let mut numbins = 100usize;
    loop {
        let binwidth = (high - low) / numbins as f32;
        let mut counts = vec![0i32; numbins];
        let mut sums = vec![0.0f32; numbins];

        for &v in values {
            for k in 0..numbins {
                let lowdiv = low + binwidth * k as f32;
                let highdiv = lowdiv + binwidth;
                let in_bin = if k == 0 {
                    v >= lowdiv && v <= highdiv
                } else {
                    v > lowdiv && v <= highdiv
                };
                if in_bin {
                    counts[k] += 1;
                    sums[k] += v;
                }
            }
        }

        let mut modes = vec![0.0f32; numbins];
        for k in 0..numbins {
            if sums[k] != 0.0 {
                modes[k] = sums[k] / counts[k] as f32;
            }
        }

        if merge_width > 0.0 {
            for i in (1..numbins).rev() {
                if (modes[i] - modes[i - 1]).abs() <= merge_width {
                    counts[i - 1] += counts[i];
                    counts[i] = 0;
                    sums[i - 1] += sums[i];
                    sums[i] = 0.0;
                    modes[i - 1] = sums[i - 1] / counts[i - 1] as f32;
                    modes[i] = 0.0;
                }
            }
        }

        // Ascending bubble sort by count (matches the C's own odd/even
        // interleaved bubble sort in effect - a plain stable ascending
        // sort produces the same last-two-elements-are-largest result).
        let mut idx: Vec<usize> = (0..numbins).collect();
        idx.sort_by_key(|&i| counts[i]);
        let last = counts[idx[numbins - 1]];
        let second_last = counts[idx[numbins - 2]];
        let ratio = last as f32 / second_last as f32;

        if ratio > 1.2 || numbins <= 2 {
            return sums[idx[numbins - 1]] / last as f32;
        }
        numbins -= 1;
    }
}

/// Ports `find_common_freq()`'s `-j`/`-J` plateau-seeking, amplitude-
/// gated note stabilizer. `fbuff`/`abuff` are the sliding buffers of
/// recent (frequency, amplitude) pairs, index `0` most recent.
///
/// **Real out-of-bounds read, not reproduced**: the C's own symmetric-
/// pair index (`beginbufferindexpoint + begin_buffsize - i`) reaches
/// exactly `max_buffsize` - one past `fbuff`/`abuff`'s real allocated
/// size (`fvec(fbuff, max_buffsize)`, no padding) - whenever `i` is at
/// its smallest value in the inner loop and `begin_buffsize` has been
/// shrunk to `max_buffsize - beginbufferindexpoint` (the "shrink toward
/// end" phase). Confirmed by this port's own unit test panicking on
/// first run; clamped to the last valid index here instead, matching
/// this project's established treatment of this bug class (e.g.
/// `filter_response::smooth_response`, `tools::spectwarper`'s sliding
/// window).
fn find_common_freq(
    fbuff: &[f32],
    abuff: &[f32],
    mut begin_buffsize: usize,
    ampthresh: f32,
) -> f32 {
    let max_buffsize = fbuff.len();
    let begin_buffsize_save = begin_buffsize;
    let mut tfreqprop = FREQUENCY_PROPORTION_THRESHOLD;

    loop {
        let mut notefreq = -1.0f32;
        let mut zeroflag = false;
        let mut beginbufferindexpoint = 0usize;

        while notefreq == -1.0 && begin_buffsize > 5 {
            let ampsum: f32 = abuff[beginbufferindexpoint..beginbufferindexpoint + begin_buffsize]
                .iter()
                .sum();

            if ampsum != 0.0 {
                zeroflag = true;
                let min_num_vals_for_average = (begin_buffsize as f32 * 0.25) as usize;
                let mut ampweightedfreqsum = 0.0f32;
                let mut ampsum2 = 0.0f32;
                let mut n_found = 0usize;

                for i in beginbufferindexpoint..beginbufferindexpoint + begin_buffsize / 2 {
                    let mirror = (beginbufferindexpoint + begin_buffsize - i).min(max_buffsize - 1);
                    let v1 = fbuff[i];
                    let v2 = fbuff[mirror];
                    let a1 = abuff[i];
                    let a2 = abuff[mirror];
                    if a1 > ampthresh && a2 > ampthresh {
                        let mut temp1 = if v2 == 0.0 {
                            (v1 / 0.001).abs()
                        } else {
                            (v1 / v2).abs()
                        };
                        if temp1 > 1.0 {
                            temp1 = 1.0 / temp1;
                        }
                        if temp1 > tfreqprop {
                            ampweightedfreqsum += a1 * v1 + a2 * v2;
                            ampsum2 += a1 + a2;
                            n_found += 2;
                        }
                    }
                }

                if n_found >= min_num_vals_for_average {
                    notefreq = ampweightedfreqsum / ampsum2;
                }
            }

            if notefreq == -1.0 {
                if beginbufferindexpoint == 0 {
                    begin_buffsize += 1;
                    if begin_buffsize > max_buffsize {
                        beginbufferindexpoint = 1;
                        if !zeroflag {
                            return notefreq;
                        }
                        begin_buffsize = max_buffsize - beginbufferindexpoint;
                    }
                } else {
                    beginbufferindexpoint += 1;
                    begin_buffsize = max_buffsize - beginbufferindexpoint;
                }
            }
        }

        if notefreq != -1.0 {
            return notefreq;
        }

        tfreqprop *= tfreqprop;
        if tfreqprop < 0.75 {
            let mut collected: Vec<f32> = Vec::new();
            for i in 0..max_buffsize {
                if abuff[i] >= ampthresh {
                    collected.push(fbuff[i]);
                }
            }
            if collected.is_empty() {
                return -1.0;
            }
            collected.sort_by(|a, b| a.partial_cmp(b).unwrap());
            let n = collected.len();
            let median = if n % 2 == 1 {
                collected[n / 2]
            } else {
                0.5 * (collected[n / 2 - 1] + collected[n / 2])
            };
            return median;
        }

        begin_buffsize = begin_buffsize_save;
    }
}

/// One `optimal_comb()`/`strongest` detector instance. `mode_filter_len`
/// `0` genuinely disables the mode filter (matches the C's own
/// `numberInBuffers` clip-to-window-size-of-`0`, which keeps
/// `numberInBuffers > 5` permanently false) - allocated with a minimum
/// capacity of `1` regardless, so index-`0` access into the output
/// buffers (used as the "last output" fallback whenever no candidate is
/// found at all) is always valid, unlike the C's own zero-length
/// `calloc` (a real out-of-bounds write the C tolerates only because
/// `calloc(0, ...)` happens to round up to a nonzero allocation on
/// glibc - not reproduced here).
struct OptimalComb {
    mode_filter_len: usize,
    frames_buffer: Vec<f32>,
    number_in_buffers: usize,
    freq_output_buffer: Vec<f32>,
    amp_output_buffer: Vec<f32>,
}

impl OptimalComb {
    fn new(mode_filter_len: usize) -> Self {
        let cap = mode_filter_len.max(1);
        OptimalComb {
            mode_filter_len,
            frames_buffer: vec![0.0; cap],
            number_in_buffers: 0,
            freq_output_buffer: vec![0.0; cap],
            amp_output_buffer: vec![0.0; cap],
        }
    }

    /// `strongest_only`: `true` for `-m strongest` (skip harmonic
    /// reinforcement and octave-subharmonic correction), `false` for
    /// `-m optimal-comb`.
    fn detect(
        &mut self,
        bins: &[(f32, f32)],
        detect_threshold_db: f32,
        low_freq: f32,
        high_freq: f32,
        strongest_only: bool,
    ) -> (f32, f32) {
        const MAX_VALS: usize = 20;
        let db_to_amp = crate::units::DbToAmp::new();
        let amp_thresh = db_to_amp.convert(detect_threshold_db);

        let peak_amp = bins.iter().map(|&(a, _)| a).fold(0.0f32, f32::max);
        let peak_adjusted_thresh = amp_thresh * peak_amp;

        let mut strongest_freqs = [0.0f32; MAX_VALS];
        let mut strongest_sums = [0.0f32; MAX_VALS];

        for &(amp, freq) in bins {
            if freq >= low_freq && freq <= high_freq && amp >= peak_adjusted_thresh {
                let mut is_unique = true;
                let mut break_loop = false;
                let mut i = 0;
                while i < MAX_VALS && !break_loop {
                    if strongest_freqs[i] > 0.0 {
                        let diff = (hz_to_midi(freq) - hz_to_midi(strongest_freqs[i])).abs();
                        if diff <= 0.25 {
                            is_unique = false;
                            if amp > strongest_sums[i] {
                                strongest_freqs[i] = freq;
                                strongest_sums[i] = amp;
                                sort_by_amp(&mut strongest_freqs, &mut strongest_sums, MAX_VALS);
                            }
                            break_loop = true;
                        }
                    }
                    i += 1;
                }

                if is_unique {
                    let mut k = 0;
                    let mut insert_flag = false;
                    while !insert_flag && k < MAX_VALS {
                        if amp > strongest_sums[k] {
                            insert_flag = true;
                            for j in (k + 1..MAX_VALS).rev() {
                                strongest_sums[j] = strongest_sums[j - 1];
                                strongest_freqs[j] = strongest_freqs[j - 1];
                            }
                            strongest_sums[k] = amp;
                            strongest_freqs[k] = freq;
                        }
                        k += 1;
                    }
                }
            }
        }

        if !strongest_only {
            let mut k = 0;
            while strongest_freqs[k] > 0.0 {
                let mut previously_in_band = false;
                let mut this_partial_peak = 0.0f32;
                for &(amp, freq) in bins {
                    let float_partial = freq / strongest_freqs[k];
                    if float_partial > 1.5 && float_partial < 7.5 {
                        let partial_fraction =
                            (float_partial - (float_partial + 0.5).trunc()).abs();
                        if partial_fraction < 0.05 {
                            if !previously_in_band {
                                previously_in_band = true;
                                this_partial_peak = amp;
                            } else if amp > this_partial_peak {
                                this_partial_peak = amp;
                            }
                        } else if previously_in_band {
                            previously_in_band = false;
                            strongest_sums[k] += this_partial_peak;
                            this_partial_peak = 0.0;
                        }
                    } else if previously_in_band {
                        previously_in_band = false;
                        strongest_sums[k] += this_partial_peak;
                        this_partial_peak = 0.0;
                    }
                }
                k += 1;
                if k >= MAX_VALS {
                    break;
                }
            }
            sort_by_amp(&mut strongest_freqs, &mut strongest_sums, MAX_VALS);
        }

        let (freqnow, freqampnow) = if strongest_sums[0] != 0.0 {
            // `alternateAmp` in the C is computed alongside
            // `alternateFreq` here but never actually read by anything
            // (dead, confirmed by reading the whole function) - only
            // `alt_freq` (feeding `insert` below) matters.
            let mut alt_freq = strongest_freqs[0];
            if !strongest_only {
                for i in 1..MAX_VALS {
                    if strongest_freqs[i] < strongest_freqs[0] && strongest_freqs[i] > 0.0 {
                        let ratio = strongest_freqs[0] / strongest_freqs[i];
                        let frac = (ratio - (ratio + 0.5).trunc()).abs();
                        if frac < 0.05 && strongest_freqs[i] < alt_freq {
                            alt_freq = strongest_freqs[i];
                        }
                    }
                }
            }

            let insert = if strongest_only {
                strongest_freqs[0]
            } else {
                alt_freq
            };
            if self.mode_filter_len > 0 {
                self.frames_buffer
                    .copy_within(0..self.mode_filter_len - 1, 1);
            }
            self.frames_buffer[0] = insert;
            self.number_in_buffers = (self.number_in_buffers + 1).min(self.mode_filter_len);

            if self.number_in_buffers > 5 {
                let midi: Vec<f32> = self.frames_buffer[..self.number_in_buffers]
                    .iter()
                    .map(|&f| hz_to_midi(f))
                    .collect();
                let mode = midi_to_hz(find_mode(&midi, 0.25));

                let mut best_diff = (strongest_freqs[0] - mode).abs();
                let mut out_freq = strongest_freqs[0];
                let mut out_amp = strongest_sums[0];
                for i in 1..MAX_VALS {
                    let diff = (strongest_freqs[i] - mode).abs();
                    if diff < best_diff {
                        best_diff = diff;
                        out_freq = strongest_freqs[i];
                        out_amp = strongest_sums[i];
                    }
                }

                if (hz_to_midi(out_freq) - hz_to_midi(mode)).abs() <= 1.0 {
                    (out_freq, out_amp)
                } else {
                    (
                        self.freq_output_buffer[0]
                            + (self.freq_output_buffer[0]
                                - self.freq_output_buffer
                                    [1.min(self.freq_output_buffer.len() - 1)]),
                        self.amp_output_buffer[0]
                            + (self.amp_output_buffer[0]
                                - self.amp_output_buffer[1.min(self.amp_output_buffer.len() - 1)]),
                    )
                }
            } else {
                // "Not enough in buffer": the C's fallback here uses
                // `strongestFundamentals[0]`/`strongestFundamentalSums[0]`
                // directly, NOT `alternateFreq`/`alternateAmp` - the
                // octave-subharmonic correction above only ever feeds
                // the mode-filter's own history buffer (`insert`
                // above), never this direct return path. Confirmed by
                // reading `pitchtracker.c`'s exact fallback line
                // (`*freqnow = strongestFundamentals[0]`) after this
                // port's own oracle comparison showed a real divergence
                // here - a naive read of "the correction exists, so
                // surely it's used for the output" is wrong for this
                // specific (default, mode-filter-disabled) branch.
                (strongest_freqs[0], strongest_sums[0])
            }
        } else {
            (self.freq_output_buffer[0], self.amp_output_buffer[0])
        };

        if self.mode_filter_len > 0 {
            self.freq_output_buffer
                .copy_within(0..self.mode_filter_len - 1, 1);
            self.amp_output_buffer
                .copy_within(0..self.mode_filter_len - 1, 1);
        }
        self.freq_output_buffer[0] = freqnow;
        self.amp_output_buffer[0] = freqampnow;

        (freqnow, freqampnow)
    }
}

fn sort_by_amp(freqs: &mut [f32], sums: &mut [f32], n: usize) {
    let mut sorted = false;
    while !sorted {
        sorted = true;
        for k in 1..n {
            if sums[k] > sums[k - 1] {
                sums.swap(k - 1, k);
                freqs.swap(k - 1, k);
                sorted = false;
            }
        }
    }
}

/// Ports `find_centroid()` for detect-method `centroid` - same formula
/// as `pvc-core::tools::centroid`'s own copy (kept as a small separate
/// duplicate here rather than sharing code across modules, matching
/// this codebase's convention).
fn find_centroid_freq(bins: &[(f32, f32)], lowf: f32, hif: f32, old_value: f32) -> f32 {
    let mut sum = 0.0f32;
    let mut weighted = 0.0f32;
    for &(amp, freq) in bins {
        if freq >= lowf && freq <= hif {
            let amp2 = amp * amp;
            sum += amp2;
            weighted += freq * amp2;
        }
    }
    let value = if sum > 0.0 { weighted / sum } else { old_value };
    value.clamp(lowf, hif)
}

fn norm_value(
    avalue: f32,
    peak_env_amp: f32,
    compression: f32,
    ampthresh: f32,
    ampgatethresh: f32,
    warp: f32,
) -> f32 {
    let short_norm = 1.0 - 1.0 / 32760.0;
    let mut avalue = avalue / peak_env_amp;
    if avalue > ampthresh {
        avalue = ampthresh + (avalue - ampthresh) * compression;
    }
    avalue = if avalue > ampgatethresh {
        avalue - ampgatethresh
    } else {
        0.0
    };
    let temp3 = 1.0 / ((ampthresh + (1.0 - ampthresh) * compression) - ampgatethresh);
    avalue = avalue * temp3 * short_norm;
    curve(0.0, 1.0, avalue, warp)
}

struct Pass1Channel {
    /// One entry per analysis frame - the smoothed band-amplitude sum.
    amps: Vec<f32>,
    /// Independently-lengthed - see this module's doc comment on the
    /// real `backlogOutputFreqs` desync bug.
    freqs: Vec<f32>,
    peak_amp: f32,
    last_lowf: f32,
    last_hif: f32,
}

#[allow(clippy::too_many_arguments)]
fn analyze_channel(
    input: &[f32],
    sample_rate: u32,
    params: &PitchtrackerParams,
    dur: f32,
) -> Pass1Channel {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
    let d = (r / params.frames_per_sec) as usize;
    let nyquist = r / 2.0;
    let freqdiff = r / n as f32;
    let ir = d as f32 / r;
    let ar_db = 10.0f64.powf(-60.0 / 20.0);

    let mut nw = params.window_size;
    if nw == 0 {
        nw = 2 * n;
    }

    let window_pair = make_windows(params.window, nw, n, 0);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);
    let mode_filter_frames =
        (params.mode_filter_window_secs * params.frames_per_sec).round() as usize;
    let mut comb = OptimalComb::new(mode_filter_frames);

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;
    let mut samples_seen: usize = 0;

    let mut old_bin_amps_sum = 0.0f32;
    let mut old_freqnow = -1.0f32;
    let mut peak_amp = 0.0f32;
    let mut amps = Vec::new();
    let mut freqs = Vec::new();
    let mut backlog_output_freqs: usize = 1;
    let (mut last_lowf, mut last_hif);

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

        let frame = analyzer.push(&hop).expect("hop is exactly d samples");

        let t = samples_seen as f32 / r;
        let lowf = resolve_freq(params.band_low.at(t, dur), nyquist).max(0.0);
        let hif = resolve_freq(params.band_high.at(t, dur), nyquist).min(nyquist);
        last_lowf = lowf;
        last_hif = hif;

        let release = params.release.at(t, dur);
        let (releasec, minusreleasec) = if release <= 0.0 {
            (0.0, 1.0)
        } else {
            let c = ar_db.powf(ir as f64 / release as f64) as f32;
            (c, 1.0 - c)
        };
        let attack = params.attack.at(t, dur);
        let (attackc, minusattackc) = if attack <= 0.0 {
            (0.0, 1.0)
        } else {
            let c = ar_db.powf(ir as f64 / attack as f64) as f32;
            (c, 1.0 - c)
        };

        let (freqnow, _freqampnow) = match params.method {
            DetectMethod::OptimalComb => {
                comb.detect(&frame.bins, params.detect_threshold_db, lowf, hif, false)
            }
            DetectMethod::Strongest => {
                comb.detect(&frame.bins, params.detect_threshold_db, lowf, hif, true)
            }
            DetectMethod::Centroid => {
                let f = find_centroid_freq(&frame.bins, lowf, hif, old_freqnow.max(0.0));
                (f, 0.0)
            }
        };
        old_freqnow = freqnow;

        if freqnow == -1.0 {
            backlog_output_freqs += 1;
        } else {
            backlog_output_freqs = 1;
        }

        let k1 = ((lowf / freqdiff) as usize).min(n2);
        let k2 = ((hif / freqdiff) as usize).min(n2);
        let mut bin_amps_sum: f32 = frame.bins[k1..=k2].iter().map(|&(a, _)| a).sum();
        bin_amps_sum = if bin_amps_sum > old_bin_amps_sum {
            attackc * old_bin_amps_sum + minusattackc * bin_amps_sum
        } else {
            releasec * old_bin_amps_sum + minusreleasec * bin_amps_sum
        };
        old_bin_amps_sum = bin_amps_sum;
        if bin_amps_sum > peak_amp {
            peak_amp = bin_amps_sum;
        }

        amps.push(bin_amps_sum);
        for _ in 0..backlog_output_freqs {
            freqs.push(freqnow);
        }

        samples_seen += d;
        if eof_after_this_hop {
            break;
        }
    }

    Pass1Channel {
        amps,
        freqs,
        peak_amp,
        last_lowf,
        last_hif,
    }
}

/// Real, deliberately faithful bug: the average-mode combine for
/// amplitude is broken in the C - see this module's doc comment.
/// Reproduced here as "last channel wins" for `Average`.
fn combine_channels_amp(channels: &[Pass1Channel], method: ChannelMethod) -> Vec<f32> {
    let n = channels.iter().map(|c| c.amps.len()).min().unwrap_or(0);
    (0..n)
        .map(|i| match method {
            ChannelMethod::Average => channels.last().map(|c| c.amps[i]).unwrap_or(0.0),
            ChannelMethod::Peak => channels.iter().map(|c| c.amps[i]).fold(f32::MIN, f32::max),
        })
        .collect()
}

fn combine_channels_freq(channels: &[Pass1Channel], method: ChannelMethod) -> Vec<f32> {
    let n = channels.iter().map(|c| c.freqs.len()).min().unwrap_or(0);
    (0..n)
        .map(|i| match method {
            ChannelMethod::Average => {
                // Channel 0 is written as-is in the C (`if(channow==0)
                // write; else combine`) - only channels 1.. run through
                // the running-average formula.
                let mut avg = channels[0].freqs[i];
                for (channow, c) in channels.iter().enumerate().skip(1) {
                    avg = (c.freqs[i] * channow as f32 + avg) / (channow + 1) as f32;
                }
                avg
            }
            ChannelMethod::Peak => channels.iter().map(|c| c.freqs[i]).fold(f32::MIN, f32::max),
        })
        .collect()
}

/// Analyzes every channel and returns the final output sample series.
pub fn process(
    channels: &[Vec<f32>],
    sample_rate: u32,
    params: &PitchtrackerParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let d = (r / params.frames_per_sec) as usize;
    let ir = d as f32 / r;
    let ar_db = 10.0f64.powf(-60.0 / 20.0);

    let per_channel: Vec<Pass1Channel> = channels
        .iter()
        .map(|c| analyze_channel(c, sample_rate, params, dur))
        .collect();
    let peak_env_amp = per_channel
        .iter()
        .map(|c| c.peak_amp)
        .fold(0.0f32, f32::max);
    let (lowf, hif) = per_channel
        .last()
        .map(|c| (c.last_lowf, c.last_hif))
        .unwrap_or((0.0, r / 2.0));

    let mut amps = combine_channels_amp(&per_channel, params.channel_method);
    let mut freqs = combine_channels_freq(&per_channel, params.channel_method);

    // Whole-track mode of the raw (pre-normalization) frequency series -
    // used only as the very first good-frequency frame's `old_freqnow`
    // fallback (`findFuncStats`'s own `*mode` output, reusing the same
    // `findMode` primitive with no bin-merging, on raw Hz values - not
    // the MIDI-converted values `optimal_comb`'s own internal mode
    // filter uses).
    let track_mode = if freqs.len() > 1 {
        let mut sorted = freqs.clone();
        sorted.sort_by(|a, b| a.partial_cmp(b).unwrap());
        find_mode(&sorted, 0.0)
    } else {
        freqs.first().copied().unwrap_or(0.0)
    };

    // `-E`: amplitude-weighted oversampling - duplicate each (freq, amp)
    // pair `round(factor * amp + 0.5)` times, skipping non-positive amps
    // entirely.
    if params.oversample_factor >= 1.0 {
        let mut new_freqs = Vec::new();
        let mut new_amps = Vec::new();
        for i in 0..freqs.len().min(amps.len()) {
            let n = if amps[i] > 0.0 {
                (params.oversample_factor * amps[i] + 0.5) as usize
            } else {
                0
            };
            for _ in 0..n {
                new_freqs.push(freqs[i]);
                new_amps.push(amps[i]);
            }
        }
        freqs = new_freqs;
        amps = new_amps;
    }

    let (asmooth, minusasmooth) = if params.smooth_response_secs <= 0.0 {
        (0.0, 1.0)
    } else {
        let c = ar_db.powf(ir as f64 / params.smooth_response_secs as f64) as f32;
        (c, 1.0 - c)
    };

    let begin_buffsize_init =
        ((0.5 + params.window_min_secs * params.frames_per_sec) as usize).max(6);
    let max_buffsize_init =
        ((0.5 + params.window_max_secs * params.frames_per_sec) as usize).max(12);

    let mut fbuff = vec![freqs.first().copied().unwrap_or(0.0); max_buffsize_init];
    let mut abuff = vec![amps.first().copied().unwrap_or(0.0); max_buffsize_init];
    let mut max_buffsize = max_buffsize_init;
    let mut begin_buffsize = begin_buffsize_init;

    let mut tpinc = params.output_rate;
    if tpinc < params.frames_per_sec {
        tpinc = 1.0;
    } else {
        tpinc = 1.0 / (tpinc / params.frames_per_sec);
    }
    assert!(
        tpinc <= 1.0,
        "pitchtracker: output rate must be >= frames-per-sec"
    );

    let db_to_amp = crate::units::DbToAmp::new();
    let note_ampthresh = db_to_amp.convert(params.detect_threshold_db);

    let mut notestate_flag = false;
    let mut firstnote = true;
    let mut goodfreq = false;
    let mut old_freqnow = -1.0f32;
    let mut backlogcount: usize = 0;
    let mut tp = 0.0f32;
    // The pre-fill above already "consumed" index 0 (matching the C's
    // own `fread(&fbuff[0],...)` before its main loop starts) - the
    // main loop's own first read starts at index 1.
    let mut freq_cursor = 1usize;
    let mut amp_cursor = 1usize;
    let mut frame_count: usize = 0;
    let mut eof = false;
    let mut out = Vec::new();

    while max_buffsize > 6 {
        let t = frame_count as f32 * ir;
        let compression_db = params.compress_amount_db.at(t, dur);
        let compression = if compression_db < 0.0 {
            10.0f64.powf(compression_db as f64 / 20.0) as f32
        } else {
            1.0
        };
        let ampthresh = 10.0f64.powf(params.compress_threshold_db.at(t, dur) as f64 / 20.0) as f32;
        let ampgatethresh = 10.0f64.powf(params.gate_threshold_db.at(t, dur) as f64 / 20.0) as f32;
        assert!(
            ampthresh > ampgatethresh,
            "pitchtracker: gate threshold must be below compression threshold"
        );
        let warp = params.warp.at(t, dur);
        let reference = params.reference_pitch.at(t, dur);
        let outfreqorigin = resolve_freq(reference, r / 2.0);

        if frame_count == 0 {
            for a in abuff.iter_mut() {
                *a = norm_value(
                    *a,
                    peak_env_amp,
                    compression,
                    ampthresh,
                    ampgatethresh,
                    warp,
                );
            }
        }

        if !eof {
            let fvalue = freqs.get(freq_cursor).copied();
            freq_cursor += 1;
            let avalue_raw = amps.get(amp_cursor).copied();
            amp_cursor += 1;
            if fvalue.is_none() || avalue_raw.is_none() {
                eof = true;
            }
            if let (Some(fv), Some(av)) = (fvalue, avalue_raw) {
                let av = norm_value(
                    av,
                    peak_env_amp,
                    compression,
                    ampthresh,
                    ampgatethresh,
                    warp,
                );
                if !eof {
                    fbuff.copy_within(0..max_buffsize - 1, 1);
                    abuff.copy_within(0..max_buffsize - 1, 1);
                    fbuff[0] = fv;
                    abuff[0] = av;
                }
            }
        }
        if eof {
            max_buffsize -= 1;
            if begin_buffsize > max_buffsize {
                begin_buffsize = max_buffsize;
            }
        }

        let ampnow = abuff[0];

        if frame_count == 0 && old_freqnow == -1.0 {
            old_freqnow = track_mode;
        }

        let mut freqnow = find_common_freq(&fbuff, &abuff, begin_buffsize, note_ampthresh);

        if ampnow < note_ampthresh {
            if firstnote {
                goodfreq = false;
            } else {
                freqnow = old_freqnow;
                goodfreq = true;
            }
        } else if !(lowf..=hif).contains(&freqnow) {
            if notestate_flag && goodfreq {
                freqnow = old_freqnow;
                goodfreq = true;
            } else {
                goodfreq = false;
            }
        } else {
            goodfreq = true;
        }

        notestate_flag = ampnow >= note_ampthresh;
        backlogcount += 1;

        if goodfreq {
            if firstnote {
                old_freqnow = freqnow;
                firstnote = false;
            }

            // Held ("not good") frames are only ever silently dropped
            // from the amplitude side (dead, see this module's doc
            // comment on `temp4`) - but each one still gets its own full
            // pass through the frequency interpolation/output loop
            // below, replayed `bsize` times here to match the real
            // tool's output *count* exactly. `freqnow` itself doesn't
            // change between replays (only `old_freqnow` catches up to
            // it via the assignment at the end of each pass), so with
            // the default `asmooth=0` every replay after the first
            // outputs a flat `freqnow` - only the very first backlog
            // pass actually ramps from the pre-gap value.
            while backlogcount > 0 {
                freqnow = asmooth * old_freqnow + minusasmooth * freqnow;

                while tp < 1.0 {
                    let ftemp4 = curve(old_freqnow, freqnow, tp, 0.0);

                    let outval = match params.output_format {
                        OutputFormat::Freq => ftemp4,
                        OutputFormat::OctaveDecimal => {
                            8.0 + (ftemp4 / 261.625).log10() / 2.0f32.log10()
                        }
                        OutputFormat::SemitonesDeviation => {
                            12.0 * (ftemp4 / outfreqorigin).log10() / 2.0f32.log10()
                        }
                        OutputFormat::NegSemitonesDeviation => {
                            -(12.0 * (ftemp4 / outfreqorigin).log10() / 2.0f32.log10())
                        }
                        OutputFormat::Midi => hz_to_midi(ftemp4),
                        OutputFormat::OctavePitchclass => crate::response::hz_to_oppc(ftemp4),
                    };
                    out.push(outval);
                    tp += tpinc;
                }
                old_freqnow = freqnow;
                tp -= tp.floor();
                backlogcount -= 1;
            }
        }

        frame_count += 1;
    }

    out
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params(fft_size: usize) -> PitchtrackerParams {
        PitchtrackerParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            band_low: ControlFn::Const(0.0),
            band_high: ControlFn::Const(-1.0),
            method: DetectMethod::OptimalComb,
            window_min_secs: 0.05,
            window_max_secs: 0.3,
            detect_threshold_db: -40.0,
            mode_filter_window_secs: 0.0,
            oversample_factor: 0.0,
            smooth_response_secs: 0.0,
            channel_method: ChannelMethod::Average,
            compress_threshold_db: ControlFn::Const(0.0),
            compress_amount_db: ControlFn::Const(0.0),
            gate_threshold_db: ControlFn::Const(-96.0),
            warp: ControlFn::Const(0.0),
            attack: ControlFn::Const(0.0),
            release: ControlFn::Const(0.0),
            output_rate: 500.0,
            output_format: OutputFormat::Freq,
            reference_pitch: ControlFn::Const(440.0),
        }
    }

    #[test]
    fn sine_input_tracks_near_440hz() {
        let params = default_params(1024);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let out = process(&[input], sample_rate, &params, 1.0);
        assert!(!out.is_empty());
        let mid = out[out.len() / 2];
        assert!(
            (mid - 440.0).abs() < 20.0,
            "tracked freq {mid} not near 440Hz"
        );
    }

    #[test]
    fn hz_midi_round_trip() {
        let hz = 440.0f32;
        let midi = hz_to_midi(hz);
        let back = midi_to_hz(midi);
        assert!((back - hz).abs() < 0.01);
    }
}
