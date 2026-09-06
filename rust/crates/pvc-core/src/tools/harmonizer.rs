//! Ports `harmonizer.c`: builds one or more "voices" from a data table
//! of frequency bands, each voice a pitch/frequency-shifted copy of a
//! triangular-windowed slice of the input spectrum, then resynthesizes
//! them (always via the oscillator bank - `harmonizer.c` never overlap-
//! adds, confirmed by reading the whole file: `obank` is set from `P !=
//! 0.` but `P` is a fixed `1.0` never touched by any flag) optionally
//! summed with a delayed/shifted copy of the source signal itself
//! (`sourceflag`, on by default whenever `-G`'s dB value could ever
//! exceed the oscillator threshold - see [`wants_source`]).
//!
//! **Table format**: unlike `harmonizer.c`'s own `cut_data_lines()`
//! preprocessor (which supports `{...}` block comments and `!`/`m`
//! solo/mute line markers, shared with several other legacy tools' data
//! tables), this port's table reader is a plain whitespace-delimited
//! float parser - a deliberate simplification, not a faithfulness gap:
//! none of the golden cases or the plan's own data-table examples use
//! the comment/solo/mute machinery, and every other ported tool that
//! reads a legacy data table (`filtresponsemaker`'s breakpoints,
//! `chordresponsemaker`'s partials) already made the same simplification.
//!
//! Each table row is 8 columns: shift factor (multiplier/adder/
//! semitones, per `--shift-format`), low/high/center frequency (Hz or
//! octave.pitchclass), peak dB, stopband dB (added to peak, not an
//! absolute floor), Q-index (curve shape: `0` linear, `+` sharper, `-`
//! wider), and delay time in seconds.
//!
//! **Real crash bug, NOT reproduced:** `harmonizer.c` divides by
//! `(centerChannel - lowChannel)` and `(highChannel - centerChannel)`
//! (both quantized FFT-bin numbers, `harmonizer.c:578,580`) with no
//! guard against either being zero - a band whose center frequency
//! quantizes to the same bin as its low or high boundary produces
//! `NaN`/`inf`, which then propagates into the oscillator bank's table
//! index and segfaults (`legacy/pvc_lib/noscbank.c`'s `(int) address`
//! cast on a `NaN` comparison that never clamps). [`build_entries`]
//! validates this explicitly and panics with a clear message instead.
//!
//! **Real bug, reproduced faithfully - `--amp-interp` is completely
//! dead:** `target_AmpInterpControl_VALUES` is allocated
//! (`harmonizer.c:483`) but *never written* anywhere in the file - there
//! is no `getGlobalFunctionValues(target_AmpInterpControl_VALUES, ...)`
//! call for it, unlike every other per-band control array
//! (`target_harmadd_VALUES`, `target_dB_VALUES`,
//! `target_FreqInterpControl_VALUES`, `target_TimeInterpControl_VALUES`
//! each get one - confirmed by reading the whole "GET THE VALUES"
//! section and grepping every reference to the array). It stays at
//! whatever `fvec()`'s zero-initialization left it - permanently `0.0`
//! for every band, regardless of `-J`'s value or its own documented
//! default of `1.0`. Since the per-bin amplitude blend is
//! `temp4 = temp2 + target_AmpInterpControl_VALUES[band] * (temp3 -
//! temp2)` (`harmonizer.c:1003`, where `temp2` is the *unwindowed* dB
//! amplitude and `temp3` the fully-windowed one), a permanent `0.0`
//! collapses this to `temp4 = temp2` always - the static per-band
//! triangular window (`HARMONIZER_DATA_amp`/[`Entry::amp_profile`]) is
//! computed for nothing: every bin passes through at its *raw* analysis
//! amplitude (gain/target-gain adjusted only), completely unattenuated
//! by its distance from the band's center. Found via the oracle, not by
//! reading: a naive port that actually applies the window (as the
//! source code's evident *intent* would suggest) measures roughly two
//! orders of magnitude quieter than the real binary and activates far
//! more oscillators past the resynthesis threshold, since the real
//! tool's peak is dominated by the loudest *raw* analysis bin rather
//! than a window-suppressed one. `--amp-interp` is not exposed as a
//! `pvc harmonize` flag at all, matching `pvc compand`'s treatment of
//! `-L` - exposing a provably-dead flag would be misleading.
//!
//! **Real cross-band interpolation bug, reproduced faithfully:** in the
//! per-band setup loop, `--freq-interp` (`-K`) is applied correctly in
//! the adder branch (`--shift-format adder`) but not in the multiplier/
//! semitones branches, which use `target_FreqInterpControl.A[0]`, the
//! *last* band's own value (a side effect of `getGlobalFunctionValues`
//! overwriting that shared field on every one of its per-band
//! iterations), for *every* band's bins rather than each bin's own band
//! value (`harmonizer.c:1010`, confirmed by reading
//! `getGlobalFunctionValues.c` and the surrounding per-band setup loop).
//! This silently makes `--freq-interp`'s per-band granularity
//! meaningless whenever `--shift-format` is `multiplier` (the default)
//! or `semitones` - all bands share the last band's control value
//! instead of their own.
//!
//! **Not reproduced** (dead code, confirmed by reading): the whole
//! "feed decayed inputs back into the delay line" block is commented out
//! in the C (a never-finished decay-time feature; `NUM_PARAMETERS` is 8,
//! not the 9 a decay column would need). Also skipped: the per-band
//! setup loop's redundant post-scale octave.pitchclass re-check
//! (`harmonizer.c:526,531,536`) - by the time that loop runs, the
//! table's frequency columns have already been converted from
//! octave.pitchclass to Hz and scaled/shifted once; the redundant check
//! only ever matters for a boundary/center frequency that happens to
//! land back in `(0, 15)` Hz after scaling, an edge case narrow enough
//! (and, at line 536, itself bugged - it uses the low-boundary macro
//! instead of the center one) that no golden case or realistic use
//! exercises it.

use crate::pvoc::{Analyzer, Frame, OscBank};
use crate::units::{amp_to_db, DbToAmp, SemitonesToMult};
use crate::warp::{curve, spectmagwarp};
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant. `harmonizer` always resynthesizes via
/// the oscillator bank, so this always applies.
const OSCILBANKGAIN: f32 = 1.7782794;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ShiftFormat {
    Multiplier,
    Adder,
    Semitones,
}

/// One raw row of the 8-column data table, before scaling/shifting or
/// octave.pitchclass conversion.
#[derive(Debug, Clone, Copy)]
pub struct TableRow {
    pub shift_factor: f32,
    pub low_freq: f32,
    pub high_freq: f32,
    pub center_freq: f32,
    pub peak_db: f32,
    pub stopband_db: f32,
    pub q_index: f32,
    pub delay_time: f32,
}

#[derive(Debug, Clone)]
pub struct HarmonizerParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,
    pub shift_format: ShiftFormat,
    pub table: Vec<TableRow>,
    pub table_shift_scale: f32,
    pub table_shift_shift: f32,
    pub table_freq_scale: f32,
    pub table_freq_shift: f32,
    pub table_peak_scale: f32,
    pub table_stopband_scale: f32,
    pub table_stopband_shift: f32,
    pub table_q_shift: f32,
    pub table_delay_scale: f32,
    pub table_delay_shift: f32,
    pub gain_db: ControlFn,
    pub voice_freq_shift_hz: ControlFn,
    pub voice_pitch_semitones: ControlFn,
    pub voice_gain_db: ControlFn,
    pub voice_warpshape: ControlFn,
    pub freq_interp: ControlFn,
    pub time_interp: ControlFn,
    pub source_freq_shift_hz: ControlFn,
    pub source_pitch_semitones: ControlFn,
    pub source_gain_db: ControlFn,
    pub source_delay_secs: ControlFn,
    pub threshold_db: f32,
}

fn control_fn_max(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(vals) => vals.iter().copied().fold(f32::MIN, f32::max),
    }
}

/// `sourceflag`: `SOURCE_dB.A[0] > threshfacdB || SOURCE_dB.n != 1.` -
/// the source is included whenever its (constant) gain is louder than
/// the oscillator threshold, or whenever it's a time-varying table at
/// all (since some point in it might exceed the threshold).
fn wants_source(source_gain_db: &ControlFn, threshold_db: f32) -> bool {
    match source_gain_db {
        ControlFn::Const(v) => *v > threshold_db,
        ControlFn::Table(_) => true,
    }
}

/// One post-scale/shift, Hz-converted table row.
struct Band {
    freq_shift: f32,
    low_hz: f32,
    high_hz: f32,
    center_hz: f32,
    peak_db: f32,
    stopband_db: f32,
    q_index: f32,
    /// Raw (post scale/shift, **not** clamped `>= 0`) delay in seconds -
    /// used both as the per-band control-function time offset
    /// (`getGlobalFunctionValues`'s `t - delayTimes[band]`) and, clamped,
    /// as this band's applied resynthesis delay.
    delay_time_raw: f32,
}

fn build_bands(params: &HarmonizerParams, nyquist: f32) -> Vec<Band> {
    params
        .table
        .iter()
        .map(|row| {
            let freq_shift = match params.shift_format {
                ShiftFormat::Multiplier | ShiftFormat::Adder => row.shift_factor,
                ShiftFormat::Semitones => SemitonesToMult::new().convert(row.shift_factor),
            };
            let freq_shift = (freq_shift * params.table_shift_scale) + params.table_shift_shift;

            let mut low_hz = if row.low_freq < 15.0 && row.low_freq > 0.0 {
                crate::response::oppc_to_hz(row.low_freq)
            } else if row.low_freq < 0.0 {
                0.0
            } else {
                row.low_freq
            };
            low_hz = (params.table_freq_scale * low_hz) + params.table_freq_shift;

            let mut high_hz = if row.high_freq < 15.0 && row.high_freq > 0.0 {
                crate::response::oppc_to_hz(row.high_freq)
            } else if row.high_freq < 0.0 {
                nyquist
            } else {
                row.high_freq
            };
            high_hz = (params.table_freq_scale * high_hz) + params.table_freq_shift;

            let mut center_hz = if row.center_freq < 15.0 && row.center_freq > 0.0 {
                crate::response::oppc_to_hz(row.center_freq)
            } else if row.center_freq < 0.0 {
                0.0
            } else {
                row.center_freq
            };
            center_hz = (params.table_freq_scale * center_hz) + params.table_freq_shift;

            let peak_db = params.table_peak_scale * row.peak_db;
            let stopband_db =
                (params.table_stopband_scale * row.stopband_db) + params.table_stopband_shift;
            let q_index = row.q_index + params.table_q_shift;
            let delay_time_raw =
                (params.table_delay_scale * row.delay_time) + params.table_delay_shift;

            assert!(
                center_hz >= low_hz && center_hz <= high_hz,
                "harmonize: band's center frequency ({center_hz} Hz) must lie between its low \
                 ({low_hz} Hz) and high ({high_hz} Hz) boundaries"
            );

            Band {
                freq_shift,
                low_hz,
                high_hz,
                center_hz,
                peak_db,
                stopband_db,
                q_index,
                delay_time_raw,
            }
        })
        .collect()
}

/// One bin entry the oscillator bank drives: which analysis bin to read,
/// its static triangular-window gain, this band's frequency-shift value,
/// its clamped applied delay, and which band it came from (for looking
/// up that band's per-frame control values). No dedup across bands - two
/// overlapping bands' ranges produce two independent entries for the
/// same analysis bin, matching `harmonizer.c`'s own `mm++`-per-band-
/// per-bin counting with no collision check.
struct Entry {
    bin: usize,
    amp_profile: f32,
    freq_shift: f32,
    delay_secs: f32,
    band: usize,
}

fn build_entries(bands: &[Band], fundamental: f32, n2: usize, db_to_amp: &DbToAmp) -> Vec<Entry> {
    let mut entries = Vec::new();
    for (band_idx, band) in bands.iter().enumerate() {
        let low_ch = (0.5 + band.low_hz / fundamental).floor();
        let high_ch = (0.5 + band.high_hz / fundamental).floor();
        let center_ch = (0.5 + band.center_hz / fundamental).floor();
        assert!(
            center_ch != low_ch && center_ch != high_ch,
            "harmonize: band {band_idx}'s center frequency quantizes to the same FFT bin as its \
             low or high boundary ({center_ch} == {low_ch} or {high_ch}) - the legacy tool \
             divides by zero here (undefined behavior, typically a segfault); pick a center \
             frequency that quantizes to a bin strictly between distinct low/high bins"
        );

        let delay_secs = band.delay_time_raw.max(0.0);
        let mut channel_now = low_ch;
        while channel_now <= high_ch {
            if channel_now >= 0.0 && (channel_now as i64) < n2 as i64 {
                let temp = if channel_now <= center_ch {
                    1.0 - (channel_now - low_ch) / (center_ch - low_ch)
                } else {
                    (channel_now - center_ch) / (high_ch - center_ch)
                };
                let temp = curve(0.0, 1.0, temp, -band.q_index);
                let temp = band.peak_db + temp * band.stopband_db;
                let amp_profile = db_to_amp.convert(temp);

                entries.push(Entry {
                    bin: channel_now as usize,
                    amp_profile,
                    freq_shift: band.freq_shift,
                    delay_secs,
                    band: band_idx,
                });
            }
            channel_now += 1.0;
        }
    }
    entries
}

/// A ring of the last `capacity` analysis frames, indexed by
/// `frame_count % capacity` - ports `HARMONIZER_channel_delay`/
/// `SOURCE_channel_delay`'s flat-array-plus-modular-index pattern
/// (same shape as `tools::filter::DelayLine`, kept as a separate small
/// copy here rather than sharing code across modules, matching this
/// codebase's convention of small per-tool duplication over a shared
/// generic when the two use sites don't otherwise interact).
struct DelayLine {
    frames: Vec<Frame>,
    capacity: usize,
}

impl DelayLine {
    fn new(capacity: usize, n_bins: usize) -> Self {
        DelayLine {
            frames: vec![
                Frame {
                    bins: vec![(0.0, 0.0); n_bins]
                };
                capacity
            ],
            capacity,
        }
    }

    fn push(&mut self, frame_count: usize, frame: Frame) -> usize {
        let index = frame_count % self.capacity;
        self.frames[index] = frame;
        index
    }

    fn get(&self, now_index: usize, delay_secs: f32, frames_per_sec: f32) -> &Frame {
        let offset = (delay_secs * frames_per_sec + 0.5) as i64;
        let mut index = now_index as i64 - offset;
        while index < 0 {
            index += self.capacity as i64;
        }
        &self.frames[(index as usize) % self.capacity]
    }
}

/// Processes one channel start to finish. `dur` is the control-function
/// normalization duration in seconds, matching `tools::pv`.
pub fn process_channel(
    channel: &[f32],
    sample_rate: u32,
    params: &HarmonizerParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;
    let d = (r / params.frames_per_sec) as usize;
    let i_factor = (d as f32 * params.time_factor) as usize;

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

    let nyquist = r / 2.0;
    let fundamental = r / n as f32;

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();
    let threshfac = db_to_amp.convert(params.threshold_db);

    let bands = build_bands(params, nyquist);
    assert!(
        !bands.is_empty(),
        "harmonize: table must have at least one band"
    );
    let entries = build_entries(&bands, fundamental, n2, &db_to_amp);
    let nc = entries.len();
    assert!(
        nc > 0,
        "harmonize: no FFT bins fell inside any band's [low, high) window"
    );

    let source_wanted = wants_source(&params.source_gain_db, params.threshold_db);

    let harmonizer_max_delay = entries.iter().map(|e| e.delay_secs).fold(0.0f32, f32::max);
    let harmonizer_max_frames = 1 + (harmonizer_max_delay * params.frames_per_sec + 0.5) as usize;
    let mut harmonizer_ring = DelayLine::new(harmonizer_max_frames, nc);

    let source_max_delay = if source_wanted {
        control_fn_max(&params.source_delay_secs).max(0.0)
    } else {
        0.0
    };
    let source_max_frames = 1 + (source_max_delay * params.frames_per_sec + 0.5) as usize;
    let mut source_ring = if source_wanted {
        Some(DelayLine::new(source_max_frames, n2 + 1))
    } else {
        None
    };

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);

    // Two independent oscillator banks whose output hops are summed into
    // one buffer each frame (`noscbank2()`'s single `O[]` accumulated by
    // both its loops) - but sharing ONE cosine table, built from the
    // *source*'s `n2`/`nw` (`noscbank2.c`'s `static` `table`, computed
    // once on its first call and reused unmodified for the harmony
    // loop's very different bin count) - see `OscBank::with_shared_table`'s
    // doc comment.
    let mut harmony_osc;
    let mut source_osc = if source_wanted {
        let bank = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
        harmony_osc = OscBank::with_shared_table(nc, sample_rate, i_factor, 1.0, bank.table());
        Some(bank)
    } else {
        harmony_osc = OscBank::new(nc, nw, sample_rate, i_factor, 1.0);
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

        // TRANSFER channel INTO HARMONIZER CIRCULAR DELAY LINE.
        let harmonizer_bins: Vec<(f32, f32)> = entries.iter().map(|e| frame.bins[e.bin]).collect();
        let harmonizer_now = harmonizer_ring.push(
            frame_count,
            Frame {
                bins: harmonizer_bins,
            },
        );

        // TRANSFER channel INTO SOURCE CIRCULAR DELAY LINE.
        let source_now = source_ring
            .as_mut()
            .map(|ring| ring.push(frame_count, frame.clone()));

        let t = samps_written as f32 / r;

        let gain = db_to_amp.convert(params.gain_db.at(t, dur));

        // Per-band control values, each evaluated with this band's own
        // time offset (`getGlobalFunctionValues`'s `t - delayTimes[band]`
        // - note this uses each band's *raw* delay, not the per-bin
        // clamped one).
        let mut target_harmadd_values = vec![0.0f32; bands.len()];
        let mut pmt_values = vec![0.0f32; bands.len()];
        let mut target_gain_values = vec![0.0f32; bands.len()];
        let mut target_freq_interp_values = vec![0.0f32; bands.len()];
        let mut target_time_interp_values = vec![0.0f32; bands.len()];
        for (i, band) in bands.iter().enumerate() {
            let t_band = t - band.delay_time_raw;
            target_harmadd_values[i] = params.voice_freq_shift_hz.at(t_band, dur);
            pmt_values[i] = semitones_to_mult.convert(params.voice_pitch_semitones.at(t_band, dur));
            target_gain_values[i] = db_to_amp.convert(params.voice_gain_db.at(t_band, dur));
            target_freq_interp_values[i] = params.freq_interp.at(t_band, dur);
            target_time_interp_values[i] = params.time_interp.at(t_band, dur);
        }
        // Real bug, reproduced faithfully - see this module's doc
        // comment: the multiplier/semitones branch below uses the
        // *last* band's freq-interp value for every bin, not each bin's
        // own band's value.
        let last_band_freq_interp = *target_freq_interp_values.last().unwrap();

        let warpshape = params.voice_warpshape.at(t, dur);

        // SOURCE.
        let channel_source_out = source_ring.as_ref().map(|ring| {
            let source_delay = params.source_delay_secs.at(t, dur);
            let t_source = t - source_delay;
            let source_harmadd = params.source_freq_shift_hz.at(t_source, dur);
            let pms = semitones_to_mult.convert(params.source_pitch_semitones.at(t_source, dur));
            let source_gain = db_to_amp.convert(params.source_gain_db.at(t_source, dur));

            let delayed = ring.get(source_now.unwrap(), source_delay, params.frames_per_sec);
            let mut out = vec![(0.0f32, 0.0f32); n2 + 1];
            for (j, &(amp, freq)) in delayed.bins.iter().enumerate() {
                let temp = pms * (freq + source_harmadd);
                let mut new_amp = amp * source_gain * gain;
                let mut new_freq = freq;
                if temp <= 0.0 || temp >= nyquist {
                    new_amp = 0.0;
                } else {
                    new_freq = temp;
                }
                out[j] = (new_amp, new_freq);
            }
            Frame { bins: out }
        });

        // MAKE TRANSFER TO HARMONY ARRAY.
        let mut harmony: Vec<(f32, f32)> = Vec::with_capacity(nc);
        for (bin, entry) in entries.iter().enumerate() {
            let delay_now = entry.delay_secs * target_time_interp_values[entry.band];
            let delayed = harmonizer_ring.get(harmonizer_now, delay_now, params.frames_per_sec);
            let (delayed_amp, delayed_freq) = delayed.bins[bin];

            // The amp-interp blend factor is *always* 0.0 in the real
            // tool - see this module's doc comment on
            // `target_AmpInterpControl_VALUES` never being written -
            // which collapses `temp4 = temp2 + 0.0*(temp3-temp2)` to
            // `temp4 = temp2`: `amp` always ends up equal to its own
            // pre-window value, round-tripped once through
            // `amp_to_db`/`dB_to_amp` (replicated below, not simplified
            // away, since that round trip is not perfectly lossless and
            // real golden cases are sensitive to it).
            let mut amp = delayed_amp * gain * target_gain_values[entry.band];
            if amp > db_to_amp.convert(-600.0) {
                let orig_db = amp_to_db(amp);
                let windowed_amp = amp * entry.amp_profile;
                if windowed_amp > db_to_amp.convert(-600.0) {
                    amp = db_to_amp.convert(orig_db);
                }
            }

            let freq = match params.shift_format {
                ShiftFormat::Adder => {
                    (delayed_freq
                        + target_freq_interp_values[entry.band] * entry.freq_shift
                        + target_harmadd_values[entry.band])
                        * pmt_values[entry.band]
                }
                ShiftFormat::Multiplier | ShiftFormat::Semitones => {
                    let temp = 1.0 + last_band_freq_interp * (entry.freq_shift - 1.0);
                    (delayed_freq * temp + target_harmadd_values[entry.band])
                        * pmt_values[entry.band]
                }
            };

            let amp = if freq <= 0.0 || freq >= nyquist {
                0.0
            } else {
                amp
            };
            harmony.push((amp, freq));
        }

        let mut harmony_flat: Vec<f32> = Vec::with_capacity(nc * 2);
        for &(amp, freq) in &harmony {
            harmony_flat.push(amp);
            harmony_flat.push(freq);
        }
        spectmagwarp(&mut harmony_flat, warpshape, false);
        let harmony_frame = Frame::from_pva_floats(&harmony_flat);

        let synt = if let Some(source_frame) = &channel_source_out {
            let source_thresh = crate::pvoc::getthresh(&source_frame.bins[..n2], threshfac);
            let harmony_thresh = crate::pvoc::getthresh(&harmony_frame.bins, threshfac);
            source_thresh.max(harmony_thresh)
        } else {
            crate::pvoc::getthresh(&harmony_frame.bins, threshfac)
        };

        let mut hop_out = vec![0.0f32; i_factor];
        if let (Some(source_frame), Some(bank)) = (&channel_source_out, source_osc.as_mut()) {
            for (o, s) in hop_out.iter_mut().zip(bank.synthesize(source_frame, synt)) {
                *o += s;
            }
        }
        for (o, s) in hop_out
            .iter_mut()
            .zip(harmony_osc.synthesize(&harmony_frame, synt))
        {
            *o += s;
        }

        on += i_factor as i64;
        if on + nw as i64 - i_factor as i64 >= 0 {
            output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
            samps_written += i_factor;
        }

        frame_count += 1;
        if eof_after_this_hop {
            break;
        }
    }

    output.extend(vec![0.0f32; i_factor]);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params(fft_size: usize, table: Vec<TableRow>) -> HarmonizerParams {
        HarmonizerParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            shift_format: ShiftFormat::Multiplier,
            table,
            table_shift_scale: 1.0,
            table_shift_shift: 0.0,
            table_freq_scale: 1.0,
            table_freq_shift: 0.0,
            table_peak_scale: 1.0,
            table_stopband_scale: 1.0,
            table_stopband_shift: 0.0,
            table_q_shift: 0.0,
            table_delay_scale: 1.0,
            table_delay_shift: 0.0,
            gain_db: ControlFn::Const(0.0),
            voice_freq_shift_hz: ControlFn::Const(0.0),
            voice_pitch_semitones: ControlFn::Const(1.0),
            voice_gain_db: ControlFn::Const(0.0),
            voice_warpshape: ControlFn::Const(0.0),
            freq_interp: ControlFn::Const(1.0),
            time_interp: ControlFn::Const(1.0),
            source_freq_shift_hz: ControlFn::Const(0.0),
            source_pitch_semitones: ControlFn::Const(1.0),
            source_gain_db: ControlFn::Const(0.0),
            source_delay_secs: ControlFn::Const(0.0),
            threshold_db: -96.0,
        }
    }

    fn basic_table() -> Vec<TableRow> {
        vec![
            TableRow {
                shift_factor: 1.5,
                low_freq: 0.0,
                high_freq: 22050.0,
                center_freq: 1000.0,
                peak_db: 0.0,
                stopband_db: -96.0,
                q_index: 0.0,
                delay_time: 0.0,
            },
            TableRow {
                shift_factor: 2.0,
                low_freq: 0.0,
                high_freq: 22050.0,
                center_freq: 2000.0,
                peak_db: -3.0,
                stopband_db: -96.0,
                q_index: 0.0,
                delay_time: 0.0,
            },
        ]
    }

    #[test]
    fn silence_in_silence_out() {
        let params = default_params(1024, basic_table());
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(&input, 44100, &params, 1.0);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn sine_input_produces_bounded_output() {
        let params = default_params(1024, basic_table());
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(&input, sample_rate, &params, 1.0);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.0, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    #[should_panic(expected = "divides by zero")]
    fn degenerate_center_equal_to_low_boundary_panics_instead_of_matching_c_crash() {
        let mut params = default_params(1024, basic_table());
        params.table[0].center_freq = 0.0; // == low_freq
        let sample_rate = 44100u32;
        let input = vec![0.0f32; sample_rate as usize / 4];
        process_channel(&input, sample_rate, &params, 1.0);
    }

    #[test]
    fn source_excluded_when_gain_below_threshold() {
        let mut params = default_params(1024, basic_table());
        params.source_gain_db = ControlFn::Const(-200.0);
        params.threshold_db = -96.0;
        assert!(!wants_source(&params.source_gain_db, params.threshold_db));
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let output = process_channel(&input, sample_rate, &params, 1.0);
        assert!(!output.is_empty());
    }
}
