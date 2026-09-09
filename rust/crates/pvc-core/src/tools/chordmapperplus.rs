//! Ports `chordmapperplus.c` (6071 lines - the largest tool in this
//! project's Phase 5 port): a data-file-driven multi-tone additive
//! chord/harmony synthesizer. An arbitrary number of independently
//! configured "tones" each pick a source point (a fundamental frequency
//! or octave.pitchclass) out of an input `.pva` analysis file, build a
//! set of partials around it (spacing/count/bandwidth), and resynthesize
//! them through an oscillator bank - blended, per bin, between the
//! *live* per-frame analysis value and a *static* spectral-morph value
//! sampled from 80 pre-averaged "loudness bucket" snapshots of the whole
//! file (see [`compute_static_freq_response`]).
//!
//! # Phase 1 scope (this module, as it stands)
//!
//! This is the first of several checkpointed phases on
//! `feat/pvc-chordmapperplus`. Implemented here: the tone-data-file
//! parser ([`parse_tone_data_file`]), per-tone partial/band setup
//! ([`setup_bands`]), the static frequency-response averaging that
//! feeds the live/static blend ([`compute_static_freq_response`]), and
//! the core per-frame synthesis loop ([`process_channel`]) - time
//! navigation, per-band pitch "tuning" toward each band's own
//! amp-weighted average frequency, the live/static amplitude blend
//! (`force_factor`/`stasis_median_harmony_db`-driven), per-tone gain/
//! transpose/spectral-stretch, the per-tone bandpass tone filter, and
//! oscillator-bank output for a single output channel.
//!
//! **Explicitly deferred to later phases** (each call site below is
//! marked `TODO(phaseN)`):
//! - **Phase 2 - noise bands**: the "residue" bins left over once every
//!   tone's own harmony bins are excluded, `noiseBandDecibelLimiterThreshold`/
//!   gap-based noise-band segmentation, and the noise half of
//!   [`compute_static_freq_response`]'s static-snapshot lookup
//!   (`static_noise`).
//! - **Phase 2 - vibrato/tuning**: natural-vibrato-period detection and
//!   per-tone source-point auto-tuning (`getVibratoValuesAndIncrement`,
//!   the two `system("pitchtracker ...")` call sites in the real C) -
//!   reuse [`crate::tools::pitchtracker`]'s in-process API instead of
//!   shelling out, once this phase lands. Also: `Rate_Correlated_*`
//!   dynamics (`-H`/`-B`, both default off) and onset/release segment
//!   mode (`-@`, default off) - both are real, but this phase only
//!   implements each feature's own *default-off* path (confirmed
//!   degenerate: `Rate_Correlated_Tone_Control_in_dB`'s own `0.0` default
//!   makes its `dB_to_amp(0.0 * temp) == 1.0` regardless of `temp`, so
//!   omitting the whole rate-tracking machinery changes nothing at the
//!   real default).
//! - **Phase 3 - delay lines and per-tone output-channel routing**: the
//!   real C runs every tone's own contribution through a per-tone delay
//!   ring buffer (`channel_delay`/`delayTime`) before mixing, and can
//!   route each partial band or noise bank to one of several output
//!   channels. This phase assumes every tone's delay is `0` (in which
//!   case the ring buffer degenerates to reading the *current* frame
//!   directly - the simplification this phase relies on) and produces
//!   exactly one output channel.
//! - **Phase 4 - CLI wiring, golden tests, docs**: no `pvc-cli` surface
//!   yet: [`ChordmapperplusParams`] is complete enough to synthesize a
//!   real tone-data file's *Phase 1* feature subset directly, but no
//!   flag maps to it yet.
//!
//! Also not reproduced (both auto-detected via `funcStats`-equivalent
//! range checks over the whole run, not per-frame): `auto_adjust_cf_and_bw`
//! (`-n`, default off - the C's own condition `(auto_adjust_cf_and_bw == 0)
//! || (peakdB > threshold)` short-circuits to "always include" when off,
//! so [`setup_bands`] only implements that always-true default path).
//!
//! # `makeInterpolatedFilterFrame` call-site audit
//!
//! Per this project's own `analysis_N`-vs-`analysis_Nplus2` stride-bug
//! precedent (found in `convolver.c`/`filtdeviator.c`/`tvfiltdeviator.c`),
//! every one of this file's 10 call sites needed checking. The 3 in
//! Phase 1's own scope (`main()`'s pre-loop average-response scan at line
//! 2189, the per-frame fetch at line 3446, and
//! `makeStaticFreqResponseAveragesFromDataFile`'s own scan at line 5149)
//! all correctly pass `analysis_N + 2` / `analysis_Nplus2` - no stride
//! bug. The remaining 7 (lines 5308, 5349 - both still inside
//! `makeStaticFreqResponseAveragesFromDataFile`, so already covered by
//! this phase's own port - plus 5957, 5969, 6042, 6050, 6058, all inside
//! `findJumpPointGainScales`/`normalizeLoopAmplitudesForChordmapperplus`,
//! which belong to the vibrato-release-jump and loop-normalization
//! machinery deferred to a later phase) still need verification when
//! their own phase comes.
//!
//! In any case, this project's own [`crate::timenav::interpolate_frame`]
//! already carries the *other*, much larger `makeInterpolatedFilterFrame`
//! finding (from `tools::twarp`'s own port): despite its name, the C
//! never actually interpolates between frames at all (an `int`-typed
//! fractional-position variable truncates to exactly `0` every time) -
//! every call site, including every one of this file's 10, always reads
//! back its floor frame verbatim.

use crate::control::ControlFn;
use crate::pvoc::{getthresh, Frame, OscBank};
use crate::response::oppc_to_hz;
use crate::smooth::{smooth_setup, Smoother};
use crate::timenav::{
    interpolate_frame, make_loop_smooth_time, LoopMode, TimeNavConfig, TimeNavigator,
};
use crate::units::{amp_to_db, DbToAmp, SemitonesToMult};
use crate::warp::curve;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::twarp`'s doc
/// comment. Applied to oscillator-bank output only, matching every other
/// obank-resynthesis tool in this project.
const OSCILBANKGAIN: f32 = 1.7782794;

/// `NUM_PARAMETERS` in the C: fields per tone-data-file record.
pub const NUM_PARAMETERS: usize = 23;

/// `NUMBER_OF_STATIC_FREQRESPONSE_AVERAGES` in the C.
pub const NUM_STATIC_LEVELS: usize = 80;

// ---------------------------------------------------------------------
// Tone data file
// ---------------------------------------------------------------------

/// One tone-data-file record (`NUM_PARAMETERS` fields). Field order and
/// meaning match the C's own `#define`d field indices exactly.
#[derive(Debug, Clone)]
pub struct ToneParams {
    /// Field 0: a source point, either an Hz value or (if `0 <`
    /// `source_point < 13`) an octave.pitchclass.
    pub source_point: f32,
    /// Field 1.
    pub transpose_point_pitch: ControlFn,
    /// Field 2: `>= 1`.
    pub low_partial: f32,
    /// Field 3: `>= 1` selects "passmode" spacing; `<= -2` selects
    /// "rejectmode" (drop every `|spacing|`th partial instead).
    pub partial_spacing: f32,
    /// Field 4: `>= 1` overrides the auto-computed partial count.
    pub number_of_partials: f32,
    /// Field 5: `> 0`, a proportion of the source frequency.
    pub bandwidth: f32,
    /// Field 6.
    pub partial_shift: ControlFn,
    /// Field 7.
    pub spectral_stretch_compress: ControlFn,
    /// Field 8.
    pub tone_db: ControlFn,
    /// Field 9.
    pub stasis_median_harmony_db: ControlFn,
    /// Field 10.
    pub noise_db: ControlFn,
    /// Field 11.
    pub stasis_median_noise_db: ControlFn,
    /// Field 12: `0` or `1`. TODO(phase2 - noise bands): unused so far.
    pub noise_switch: f32,
    /// Field 13.
    pub transpose_point_noise: ControlFn,
    /// Field 14.
    pub force_factor: ControlFn,
    /// Field 15: `1` enables master transposition/freq-shift for this
    /// tone.
    pub master_transposition_switch: f32,
    /// Field 16: `0` or `1`. TODO(phase2 - vibrato): unused so far.
    pub synthetic_vibrato_switch: f32,
    /// Field 17.
    pub bandpass_cf: ControlFn,
    /// Field 18.
    pub bandpass_rolloff_db_per_octave: ControlFn,
    /// Field 19: encodes both the tone and noise bandpass-filter
    /// enable/type (`(raw*0.1) as int` selects on/off, the fractional
    /// remainder times 10 selects filter type 0-3).
    pub tone_filter_switch_raw: f32,
    /// Field 20. TODO(phase3 - delay lines): unused so far.
    pub delay_time_switch_scaler: f32,
    /// Field 21. TODO(phase3 - delay lines): unused so far.
    pub delay_time: ControlFn,
    /// Field 22: `0` means "all output channels". TODO(phase3 -
    /// per-tone output-channel routing): unused so far.
    pub tone_channel_output_number: f32,
}

/// Ports `cut_data_lines()`: strips `{...}` comments, then applies either
/// "solo" mode (keep only records marked with a leading `!`) or "mute"
/// mode (drop records marked with a leading `m`) - solo mode wins if any
/// `!` marker exists anywhere in the file, matching the C's own
/// file-wide pre-scan. A marker character only counts when the
/// *previous* character isn't alphabetic and isn't `/` (the C's own
/// guard against misreading a stray `!`/`m` inside other content).
///
/// Deliberately not reproduced: the real `cut_data_lines()` shells out
/// through actual temp files on disk (`/tmp/<user>.<pid>.temp_data_file`).
/// This operates on the string directly instead, which is behaviorally
/// identical for any file that doesn't depend on that temp-file identity
/// (none do).
pub fn cut_data_lines(text: &str) -> Result<String, String> {
    let mut stage1 = String::with_capacity(text.len());
    let mut paren_open = false;
    for c in text.chars() {
        match c {
            '{' => {
                if paren_open {
                    return Err("two open braces in a row".to_string());
                }
                paren_open = true;
            }
            '}' => {
                if !paren_open {
                    return Err("brace not open".to_string());
                }
                paren_open = false;
            }
            _ => {
                if !paren_open {
                    stage1.push(c);
                }
            }
        }
    }
    if paren_open {
        return Err("open curly brace at end of file".to_string());
    }

    let chars: Vec<char> = stage1.chars().collect();

    let is_marker = |chars: &[char], i: usize, marker: char| -> bool {
        if chars[i] != marker {
            return false;
        }
        let last_c = if i == 0 { ' ' } else { chars[i - 1] };
        !last_c.is_alphabetic() && last_c != '/'
    };

    let solo_on = (0..chars.len()).any(|i| is_marker(&chars, i, '!'));

    let mut out = String::with_capacity(stage1.len());
    let mut i = 0usize;
    if solo_on {
        while i < chars.len() {
            if is_marker(&chars, i, '!') {
                i += 1;
                for field in 0..NUM_PARAMETERS {
                    let (tok, next) = take_token(&chars, i);
                    i = next;
                    if field > 0 {
                        out.push(' ');
                    }
                    out.push_str(&tok);
                }
                out.push(' ');
            } else {
                i += 1;
            }
        }
    } else {
        while i < chars.len() {
            if is_marker(&chars, i, 'm') {
                i += 1;
                for _ in 0..NUM_PARAMETERS {
                    let (_, next) = take_token(&chars, i);
                    i = next;
                }
            } else {
                out.push(chars[i]);
                i += 1;
            }
        }
    }
    Ok(out)
}

/// Skips leading whitespace, then collects one whitespace-delimited
/// token (matching `fscanf(f, "%s", ...)`). Returns the token and the
/// index just past it.
fn take_token(chars: &[char], mut i: usize) -> (String, usize) {
    while i < chars.len() && chars[i].is_whitespace() {
        i += 1;
    }
    let mut tok = String::new();
    while i < chars.len() && !chars[i].is_whitespace() {
        tok.push(chars[i]);
        i += 1;
    }
    (tok, i)
}

/// Ports the tone-data-file parsing in `main()` (after `cut_data_lines`):
/// splits the filtered text into whitespace tokens, `NUM_PARAMETERS` per
/// tone, and parses each field per its own real type (a plain float for
/// most fields, a control function - either a bare number or a
/// breakpoint-table file - for the twelve `struct func` fields).
///
/// `resolve_table` loads a breakpoint-table file's raw values given its
/// path (as named by a tone-data-file token that doesn't parse as a
/// plain number) - injected rather than done here directly, since
/// `pvc-core` does no I/O of its own (`pvc-io`/`pvc-cli` own that).
pub fn parse_tone_data_file(
    text: &str,
    mut resolve_table: impl FnMut(&str) -> Result<Vec<f32>, String>,
) -> Result<Vec<ToneParams>, String> {
    let filtered = cut_data_lines(text)?;
    let tokens: Vec<&str> = filtered.split_whitespace().collect();
    if tokens.len() < NUM_PARAMETERS - 1 {
        return Err("insufficient tones data file".to_string());
    }
    let num_tones = tokens.len() / NUM_PARAMETERS;

    let mut tones = Vec::with_capacity(num_tones);
    for t in 0..num_tones {
        let base = t * NUM_PARAMETERS;
        let f = |i: usize| -> Result<f32, String> {
            tokens[base + i]
                .parse::<f32>()
                .map_err(|_| format!("tone {t}: bad numeric field {i}: {:?}", tokens[base + i]))
        };
        let mut cf = |i: usize| -> Result<ControlFn, String> {
            let tok = tokens[base + i];
            if let Ok(v) = tok.parse::<f32>() {
                Ok(ControlFn::Const(v))
            } else {
                resolve_table(tok).map(ControlFn::Table)
            }
        };
        tones.push(ToneParams {
            source_point: f(0)?,
            transpose_point_pitch: cf(1)?,
            low_partial: f(2)?,
            partial_spacing: f(3)?,
            number_of_partials: f(4)?,
            bandwidth: f(5)?,
            partial_shift: cf(6)?,
            spectral_stretch_compress: cf(7)?,
            tone_db: cf(8)?,
            stasis_median_harmony_db: cf(9)?,
            noise_db: cf(10)?,
            stasis_median_noise_db: cf(11)?,
            noise_switch: f(12)?,
            transpose_point_noise: cf(13)?,
            force_factor: cf(14)?,
            master_transposition_switch: f(15)?,
            synthetic_vibrato_switch: f(16)?,
            bandpass_cf: cf(17)?,
            bandpass_rolloff_db_per_octave: cf(18)?,
            tone_filter_switch_raw: f(19)?,
            delay_time_switch_scaler: f(20)?,
            delay_time: cf(21)?,
            tone_channel_output_number: f(22)?,
        });
    }
    Ok(tones)
}

/// The C's `funcStats()` low/high range over a control function's own
/// values (used only to *classify* a function, e.g. "is this whole
/// function always below 13, i.e. an octave.pitchclass" - never to
/// resolve a per-frame value).
fn control_fn_range(cf: &ControlFn) -> (f32, f32) {
    match cf {
        ControlFn::Const(v) => (*v, *v),
        ControlFn::Table(t) if t.is_empty() => (0.0, 0.0),
        ControlFn::Table(t) => (
            t.iter().copied().fold(f32::INFINITY, f32::min),
            t.iter().copied().fold(f32::NEG_INFINITY, f32::max),
        ),
    }
}

/// The C's `funcStats()` average (a plain mean of the function's own
/// breakpoint values, not time-integrated) - used only for the
/// bandpass-rolloff-average-is-zero filter-disable check below.
fn control_fn_average(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(t) if t.is_empty() => 0.0,
        ControlFn::Table(t) => t.iter().sum::<f32>() / t.len() as f32,
    }
}

// ---------------------------------------------------------------------
// Band/partial setup
// ---------------------------------------------------------------------

/// One harmony bin: a single (amplitude, frequency) slot in the analysis
/// channel array assigned to one tone's one partial.
#[derive(Debug, Clone)]
pub struct HarmonyBin {
    pub tone: usize,
    pub band: usize,
    /// Index of this bin's amplitude slot in the `analysis_n + 2`-long
    /// channel array (`indexInChannelForHarmony[mm]` in the C); the
    /// frequency slot is always one past it.
    pub channel_amp_index: usize,
    /// The half-cosine cross-fade window applied at a partial's own
    /// bin-range edges (`1.0` at the partial's own center bin).
    pub coswindow: f32,
    /// The tone's own unshifted partial frequency this bin belongs to
    /// (`sourcefreq[mm]` in the C) - constant for the whole run, used
    /// both as the static-harmony frequency and as the "null phase"
    /// tuning target.
    pub source_freq: f32,
}

#[derive(Debug, Clone)]
pub struct BandSetup {
    pub bins: Vec<HarmonyBin>,
    /// Length `num_bands + 1`; band `b`'s own bins are
    /// `bins[band_begin[b]..band_begin[b + 1]]` (the last entry is a
    /// sentinel equal to `bins.len()`, matching the C's own
    /// `partial_Band_Begin[numberOfBands] = NC`).
    pub band_begin: Vec<usize>,
    pub source_pt_fund_freq: Vec<f32>,
    /// `true` selects `transposePoint_PITCH`'s own resolved value being
    /// treated as an octave.pitchclass (needs `oppc_to_hz`), `false`
    /// means it's already Hz - decided once from the whole control
    /// function's own range, per tone.
    pub transpose_shift_method_pitch_is_oppc: Vec<bool>,
    pub trans_switch: Vec<bool>,
    pub tone_filter_switch: Vec<bool>,
    /// `0`: bandpass, `2`: highpass, `3`: lowpass (`1` behaves as
    /// bandpass too - the C only special-cases `!= 2`/`!= 3`).
    pub tone_filter_type: Vec<i32>,
}

impl BandSetup {
    pub fn num_bands(&self) -> usize {
        self.band_begin.len().saturating_sub(1)
    }
}

/// `0.5 * (cos(v * PI) + 1.0)`: the C's `halfcoscurve()`, a cosine ramp
/// from `1` (at `v = 0`) to `0` (at `v = 1`). The real C looks this up
/// from a 1024-entry table (`(int)((v * 1023.) + .5)`); computed exactly
/// here instead - the table's own quantization error (~1e-4 across a
/// smooth cosine) is well under every golden tolerance this project
/// uses, so this is a deliberate simplification, not a faithfully-
/// reproduced LUT (unlike `units::DbToAmp`/`SemitonesToMult`, whose own
/// LUT quantization is coarse enough to matter and is reproduced
/// exactly).
fn halfcoscurve(v: f32) -> f32 {
    0.5 * ((v * std::f32::consts::PI).cos() + 1.0)
}

/// Ports the per-tone band/partial setup in `main()` (the `for(l=0;l<2;
/// l++)` two-pass loop in the C, done here in one pass since `Vec`
/// grows - the C's own two passes exist only to size fixed C arrays
/// before filling them).
///
/// Not reproduced: `auto_adjust_cf_and_bw` (`-n`, default off) - see
/// this module's own top doc comment.
pub fn setup_bands(
    tones: &[ToneParams],
    nyquist: f32,
    fundamental: f32,
    n: usize,
) -> Result<BandSetup, String> {
    let mut bins = Vec::new();
    let mut band_begin = Vec::new();
    let mut source_pt_fund_freq = vec![0.0f32; tones.len()];
    let mut transpose_shift_method_pitch_is_oppc = vec![false; tones.len()];
    let mut trans_switch = vec![false; tones.len()];
    let mut tone_filter_switch = vec![false; tones.len()];
    let mut tone_filter_type = vec![0i32; tones.len()];

    for (tone_now, tone) in tones.iter().enumerate() {
        let this_source_freq = if tone.source_point > 0.0 && tone.source_point < 13.0 {
            oppc_to_hz(tone.source_point)
        } else {
            tone.source_point
        };
        source_pt_fund_freq[tone_now] = this_source_freq;

        let (lo, hi) = control_fn_range(&tone.transpose_point_pitch);
        if (lo < 13.0 && hi > 13.0) || (hi < 13.0 && lo > 13.0) {
            return Err(format!(
                "tone {tone_now}: transpose point function crosses the unit range from frequency to octave.pitchclass"
            ));
        }
        transpose_shift_method_pitch_is_oppc[tone_now] = lo < 13.0;

        if tone.bandwidth <= 0.0 {
            return Err(format!("tone {tone_now}: bandwidth must be > 0"));
        }
        let bw = this_source_freq * tone.bandwidth * 0.5;

        if tone.low_partial < 1.0 {
            return Err(format!("tone {tone_now}: lowest partial must be >= 1"));
        }

        let mut numpasspartials = ((nyquist - this_source_freq * tone.low_partial)
            / (this_source_freq * tone.partial_spacing.abs()))
            as i64;
        if tone.number_of_partials as i64 >= 1 {
            numpasspartials = tone.number_of_partials as i64;
        }

        if tone.partial_spacing < 1.0 {
            let temp7 = ((nyquist - this_source_freq) / this_source_freq) as i64;
            numpasspartials = temp7 - numpasspartials;
            if numpasspartials <= 0 {
                return Err(format!(
                    "tone {tone_now}: no partials in reject mode - check spacing/number of partials/first partial"
                ));
            }
        }

        let mut part_count: i64 = 1;
        let mut rejected_part_count: i64 = 0;
        let mut current_rejected_part = tone.low_partial as i64;

        for k in 1..=numpasspartials {
            let unshifted_partial_freq = if tone.partial_spacing >= 1.0 {
                this_source_freq * tone.low_partial
                    + this_source_freq * tone.partial_spacing * (k - 1) as f32
            } else {
                if part_count == current_rejected_part {
                    part_count += 1;
                    rejected_part_count += 1;
                    if tone.number_of_partials == 0.0
                        || (rejected_part_count as f32) < tone.number_of_partials
                    {
                        current_rejected_part += tone.partial_spacing.abs() as i64;
                    }
                }
                let freq = this_source_freq * part_count as f32;
                part_count += 1;
                freq
            };

            let this_partial_number = unshifted_partial_freq / this_source_freq;
            let mut this_partial_bw = bw * (1.0 + (this_partial_number - 1.0) * 0.25);
            if this_partial_bw > this_source_freq * 0.5 {
                this_partial_bw = this_source_freq * 0.5;
            }

            let i1 =
                1 + 2 * ((unshifted_partial_freq - this_partial_bw) / fundamental).round() as i64;
            let i2 =
                1 + 2 * ((unshifted_partial_freq + this_partial_bw) / fundamental).round() as i64;
            let ipartial = 1 + 2 * (unshifted_partial_freq / fundamental).round() as i64;

            let band_index = band_begin.len();
            let bin_start = bins.len();

            let mut i = i1;
            while i < ipartial {
                if i > 0 && i < n as i64 {
                    let temp6 = (ipartial - i) as f32 / (ipartial - i1) as f32;
                    bins.push(HarmonyBin {
                        tone: tone_now,
                        band: band_index,
                        channel_amp_index: (i - 1) as usize,
                        coswindow: halfcoscurve(temp6),
                        source_freq: unshifted_partial_freq,
                    });
                }
                i += 2;
            }
            if ipartial > 0 && ipartial < n as i64 {
                bins.push(HarmonyBin {
                    tone: tone_now,
                    band: band_index,
                    channel_amp_index: (ipartial - 1) as usize,
                    coswindow: 1.0,
                    source_freq: unshifted_partial_freq,
                });
            }
            let mut i = i2;
            while i > ipartial {
                if i > 0 && i < n as i64 {
                    let temp6 = (i - ipartial) as f32 / (i2 - ipartial) as f32;
                    bins.push(HarmonyBin {
                        tone: tone_now,
                        band: band_index,
                        channel_amp_index: (i - 1) as usize,
                        coswindow: halfcoscurve(temp6),
                        source_freq: unshifted_partial_freq,
                    });
                }
                i -= 2;
            }

            band_begin.push(bin_start);
        }

        let sw = (0.1 * tone.tone_filter_switch_raw) as i64;
        tone_filter_switch[tone_now] = sw == 1 || sw == 2;
        let scaled = tone.tone_filter_switch_raw * 0.1;
        let filt_type = (10.0 * (scaled - scaled.floor())) as i64;
        if !(0..=3).contains(&filt_type) {
            return Err(format!(
                "tone {tone_now}: {} is not an accepted filter switch",
                tone.tone_filter_switch_raw
            ));
        }
        tone_filter_type[tone_now] = filt_type as i32;

        trans_switch[tone_now] = tone.master_transposition_switch == 1.0;
    }

    band_begin.push(bins.len());

    // Post-pass: force the filter off when the bandpass rolloff's own
    // average is exactly zero, regardless of the encoded switch value
    // (`main()`'s own post-setup loop).
    for (tone_now, tone) in tones.iter().enumerate() {
        if control_fn_average(&tone.bandpass_rolloff_db_per_octave) == 0.0 {
            tone_filter_switch[tone_now] = false;
        }
    }

    Ok(BandSetup {
        bins,
        band_begin,
        source_pt_fund_freq,
        transpose_shift_method_pitch_is_oppc,
        trans_switch,
        tone_filter_switch,
        tone_filter_type,
    })
}

// ---------------------------------------------------------------------
// Static frequency-response averaging
// ---------------------------------------------------------------------

#[derive(Debug, Clone)]
pub struct StaticFreqResponse {
    /// `[level * n_plus_2 + bin]`, `level` in `0..NUM_STATIC_LEVELS`.
    pub averages: Vec<f32>,
    pub peak_sum_db: f32,
    /// Per band: which bin (an index into `BandSetup::bins`) is that
    /// band's own strongest bin, summed across all 80 levels.
    pub band_strongest_bin: Vec<usize>,
    /// `[level * num_bands + band]`: the strongest bin's own peak
    /// amplitude at that level (all other bins in the band read `0` at
    /// every level - the C zeroes them explicitly; this just never
    /// stores anything for them).
    pub band_peak_amp: Vec<f32>,
    /// Per bin: `1.0` for a band's own strongest bin, `5.0` otherwise
    /// (`harmony_force_curve_indeces` in the C - a `curve()` warp index,
    /// not a literal exponent).
    pub force_curve_index: Vec<f32>,
    pub band_average_freq: Vec<f32>,
}

/// Ports `makeStaticFreqResponseAveragesFromDataFile()`, called once per
/// output channel with `midTimePoint = analysis_dur * 0.5` and
/// `timeWindowSize = analysis_dur` (`main()`'s own "STATIC FRAME" call) -
/// both fixed here to their real call-site values rather than exposed as
/// parameters, since neither varies for chordmapperplus's own actual
/// usage: with those values the "shrink window near either end" branches
/// in the C are always exactly on the boundary (`halfTimeWindowSize ==
/// midTimePoint == analysis_dur - midTimePoint`) and never fire, so the
/// scan always covers the whole file, `[0, analysis_dur)`.
///
/// `UseHammingWindowFlag` is always passed as literal `0` at that same
/// call site (the C's own per-frame weighting is dead in practice - only
/// the *smoothing* pass below still uses the Hamming table
/// unconditionally), so this omits the flag and always weights `1.0`.
///
/// Not reproduced: if the very first level (`0`, the loudest bucket)
/// ends up with zero frames while some later level has frames, the C's
/// own gap-fill interpolation reads `static_freqresponse_averages[-N-2 +
/// ...]` - a negative array index, genuine undefined behavior, not a
/// deterministic value. Leading gaps like that are left at `0.0` here
/// instead (matching this project's established "don't reproduce
/// undefined behavior" policy - see `tools::ratechanger`'s own doc
/// comment). In practice level `0` is the frame(s) at the file's own
/// overall peak amplitude, so this almost never has zero frames for real
/// audio.
pub fn compute_static_freq_response(
    analysis: &[Vec<f32>],
    iframes_per_sec: f32,
    n_plus_2: usize,
    analysis_fundamental: f32,
    band_setup: &BandSetup,
) -> StaticFreqResponse {
    let analysis_dur = analysis.len() as f32 / iframes_per_sec;
    let num_bands = band_setup.num_bands();

    const HAMMING_SIZE: usize = 1024;
    let hamming: Vec<f32> = (0..HAMMING_SIZE)
        .map(|i| {
            0.54 - 0.46
                * (2.0 * std::f32::consts::PI * i as f32 / (HAMMING_SIZE as f32 - 1.0)).cos()
        })
        .collect();

    let time_incr = 1.0f32 / 25.0;
    let total_frames = (analysis_dur / time_incr) as usize;

    let db_to_amp = DbToAmp::new();
    // `AMP_REDUCTION dB_to_amp( -7 )` in the C.
    let amp_reduction = db_to_amp.convert(-7.0);

    let mut peak_db_sum = f32::NEG_INFINITY;
    for frame in 0..total_frames {
        let this_time = frame as f32 * time_incr;
        let flat = interpolate_frame(analysis, iframes_per_sec, this_time);
        let mut sum_of_amps = 0.0f32;
        for i in (0..n_plus_2).step_by(2) {
            sum_of_amps += flat[i] * amp_reduction;
        }
        // `(int)(0.5 + amp_to_dB(sumOfAmps))`: NOT the same as rounding
        // for negative values (the common case) - C's `(int)` cast
        // truncates toward zero, so e.g. `0.5 + -40.3 = -39.8` truncates
        // to `-39`, not `round(-40.3) == -40`. Reproduced exactly via
        // `as i64` (Rust's float-to-int cast also truncates toward
        // zero), not `.round()`.
        let db_sum = ((0.5 + amp_to_db(sum_of_amps)) as i64) as f32;
        if db_sum > peak_db_sum {
            peak_db_sum = db_sum;
        }
    }

    let mut averages = vec![0.0f32; NUM_STATIC_LEVELS * n_plus_2];
    let mut amp_weight_sums = vec![0.0f32; NUM_STATIC_LEVELS * n_plus_2];
    let mut counts = vec![0u32; NUM_STATIC_LEVELS];

    for frame in 0..total_frames {
        let this_time = frame as f32 * time_incr;
        let flat = interpolate_frame(analysis, iframes_per_sec, this_time);
        let mut sum_of_amps = 0.0f32;
        for i in (0..n_plus_2).step_by(2) {
            sum_of_amps += flat[i] * amp_reduction;
        }
        let db_sum = ((0.5 + amp_to_db(sum_of_amps)) as i64) as f32;
        let db_diff_sum = (peak_db_sum - db_sum + 0.5) as i64;
        let level = db_diff_sum.clamp(0, NUM_STATIC_LEVELS as i64 - 1) as usize;
        let base = level * n_plus_2;

        for i in (1..n_plus_2).step_by(2) {
            let amp_weight = flat[i - 1] * amp_reduction;
            averages[base + i] += flat[i] * amp_weight;
            amp_weight_sums[base + i - 1] += amp_weight;
        }
        for i in (0..n_plus_2).step_by(2) {
            averages[base + i] += flat[i] * amp_reduction;
        }
        counts[level] += 1;
    }

    #[allow(clippy::needless_range_loop)]
    for level in 0..NUM_STATIC_LEVELS {
        if counts[level] == 0 {
            continue;
        }
        let base = level * n_plus_2;
        for k in (0..n_plus_2).step_by(2) {
            averages[base + k] /= counts[level] as f32;
            let l = k + 1;
            if amp_weight_sums[base + k] > 0.0 {
                averages[base + l] /= amp_weight_sums[base + k];
            } else {
                // `analysis_fundamental * (float)(dBlevelFreqresponse / 2)`:
                // integer division in the C (`dBlevelFreqresponse` is
                // `int`), not `level as f32 / 2.0`.
                averages[base + l] = analysis_fundamental * (level / 2) as f32;
            }
        }
    }

    // Gap-fill: linearly interpolate any run of zero-count levels
    // between two populated ones.
    let mut i_flag = false;
    let mut start: i64 = 0;
    #[allow(clippy::needless_range_loop)]
    for level in 0..NUM_STATIC_LEVELS {
        if counts[level] == 0 {
            if !i_flag {
                start = level as i64 - 1;
                i_flag = true;
            }
        } else if i_flag {
            let end = level as i64;
            i_flag = false;
            if start >= 0 {
                for k in (start + 1)..end {
                    let l = k - start;
                    let mult = l as f32 / (end - start) as f32;
                    for amp_idx in (0..n_plus_2).step_by(2) {
                        let freq_idx = amp_idx + 1;
                        averages[(k as usize) * n_plus_2 + amp_idx] = (1.0 - mult)
                            * averages[(start as usize) * n_plus_2 + amp_idx]
                            + mult * averages[(end as usize) * n_plus_2 + amp_idx];
                        averages[(k as usize) * n_plus_2 + freq_idx] = (1.0 - mult)
                            * averages[(start as usize) * n_plus_2 + freq_idx]
                            + mult * averages[(end as usize) * n_plus_2 + freq_idx];
                    }
                }
            }
        }
    }

    // Smooth across the dB-level axis with an 11-tap Hamming window,
    // levels `0..NUM_STATIC_LEVELS - 1` only (the last level, `79`, is
    // left unsmoothed - a real quirk of the C's own loop bound,
    // reproduced faithfully). Overwritten **in place**, level by
    // increasing level: by the time a later level's own window reads an
    // earlier level, that earlier level has already been overwritten
    // with its own smoothed value - a genuine cascading effect in the
    // real C (no double-buffering there), reproduced here by not using
    // a second buffer either.
    for level in 0..(NUM_STATIC_LEVELS - 1) {
        for l in 0..n_plus_2 {
            let mut value_sum = 0.0f32;
            let mut window_amp_sum = 0.0f32;
            let mut n = 0usize;
            let mut k = level as i64 - 5;
            while k <= level as i64 + 5 {
                if k >= 0 && k < NUM_STATIC_LEVELS as i64 {
                    let idx = ((n as f32 / 11.0) * (HAMMING_SIZE as f32 - 1.0)) as usize;
                    let temp = hamming[idx];
                    value_sum += temp * averages[(k as usize) * n_plus_2 + l];
                    window_amp_sum += temp;
                }
                k += 1;
                n += 1;
            }
            averages[level * n_plus_2 + l] = value_sum / window_amp_sum;
        }
    }

    let mut band_strongest_bin = vec![0usize; num_bands];
    let mut force_curve_index = vec![0.0f32; band_setup.bins.len()];
    let mut band_peak_amp = vec![0.0f32; NUM_STATIC_LEVELS * num_bands];
    let mut band_average_freq = vec![0.0f32; num_bands];

    for band in 0..num_bands {
        let bin_lo = band_setup.band_begin[band];
        let bin_hi = band_setup.band_begin[band + 1];

        let mut strongest_sum = f32::NEG_INFINITY;
        let mut strongest_bin = bin_lo;
        for bin in bin_lo..bin_hi {
            let idx = band_setup.bins[bin].channel_amp_index;
            let mut this_sum = 0.0f32;
            for level in 0..(NUM_STATIC_LEVELS - 1) {
                this_sum += averages[level * n_plus_2 + idx];
            }
            if this_sum > strongest_sum {
                strongest_sum = this_sum;
                strongest_bin = bin;
            }
        }
        band_strongest_bin[band] = strongest_bin;

        let mut average_freq_for_band = 0.0f32;
        let mut this_amp_weight_sum = 0.0f32;

        for level in 0..(NUM_STATIC_LEVELS - 1) {
            let mut peak_amp = f32::NEG_INFINITY;
            let mut this_freq = 0.0f32;
            for bin in bin_lo..bin_hi {
                let idx = band_setup.bins[bin].channel_amp_index;
                let temp = averages[level * n_plus_2 + idx];
                if temp > peak_amp {
                    peak_amp = temp;
                    this_freq = averages[level * n_plus_2 + idx + 1];
                }
            }
            average_freq_for_band += this_freq * peak_amp;
            this_amp_weight_sum += peak_amp;

            if level == 0 {
                #[allow(clippy::needless_range_loop)]
                for bin in bin_lo..bin_hi {
                    force_curve_index[bin] = if bin == strongest_bin { 1.0 } else { 5.0 };
                }
            }
            band_peak_amp[level * num_bands + band] = peak_amp;
        }
        band_average_freq[band] = average_freq_for_band / this_amp_weight_sum;
    }

    StaticFreqResponse {
        averages,
        peak_sum_db: peak_db_sum,
        band_strongest_bin,
        band_peak_amp,
        force_curve_index,
        band_average_freq,
    }
}

// ---------------------------------------------------------------------
// Per-frame synthesis
// ---------------------------------------------------------------------

/// Phase 1's own parameter surface: every control this phase's math
/// actually uses. See this module's own top doc comment for what's
/// still missing.
#[derive(Debug, Clone)]
pub struct ChordmapperplusParams {
    pub window_size: usize,
    pub frames_per_sec: f32,
    /// `<= 0.0` means "use the analysis file's own duration".
    pub duration: f32,
    pub time_origin: ControlFn,
    pub rate: ControlFn,
    pub window_low: ControlFn,
    /// `< 0.0` means "use the analysis file's own duration".
    pub window_high: ControlFn,
    pub loop_smooth: ControlFn,
    pub autostop: bool,
    pub loop_mode: LoopMode,
    pub master_gain_db: ControlFn,
    pub tones_master_gain_db: ControlFn,
    pub tones_master_freq_shift_hz: ControlFn,
    pub tones_macro_pitch_semitones: ControlFn,
    pub threshold_db: f32,
}

/// Resynthesizes one output channel. `analysis`/`analysis_n`/
/// `analysis_d`/`analysis_sample_rate` are the same `.pva`-derived
/// inputs every other tool in this project takes
/// (`pvc_io::PvaData::channels[ch]` etc.); `tones`/`band_setup`/
/// `static_freq` come from [`parse_tone_data_file`]/[`setup_bands`]/
/// [`compute_static_freq_response`] above.
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    analysis: &[Vec<f32>],
    analysis_n: usize,
    analysis_d: u32,
    analysis_sample_rate: u32,
    tones: &[ToneParams],
    band_setup: &BandSetup,
    static_freq: &StaticFreqResponse,
    params: &ChordmapperplusParams,
) -> Vec<f32> {
    let r = analysis_sample_rate as f32;
    let n = analysis_n;
    let n_plus_2 = n + 2;
    let nyquist = r / 2.0;

    let iframes_per_sec = analysis_sample_rate as f32 / analysis_d as f32;
    let analysis_dur = analysis.len() as f32 / iframes_per_sec;
    let mut dur = if params.duration <= 0.0 {
        analysis_dur
    } else {
        params.duration
    };

    let d = (r / params.frames_per_sec) as usize;
    let i_factor = d;
    let filttinc = d as f32 / r;

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

    let threshfac = 10.0f64.powf(params.threshold_db as f64 / 20.0) as f32;

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();

    let nav_cfg = TimeNavConfig {
        onset_release: false, // TODO(phase2): onset/release mode (`-@`).
        autostop: params.autostop,
        loop_mode: params.loop_mode,
    };
    let initial_time_origin = params.time_origin.at(0.0, dur);
    let mut nav = TimeNavigator::new(nav_cfg, initial_time_origin);

    let mut osc = OscBank::new(
        band_setup.bins.len().max(1),
        nw,
        analysis_sample_rate,
        i_factor,
        1.0,
    );

    let mut channel_smoother = Smoother::new(n_plus_2);
    let mut previous_freq = vec![0.0f32; n_plus_2];
    let mut first_frame = true;

    let mut on: i64 = -(nw as i64) * i_factor as i64 / d as i64;

    let mut output = Vec::new();
    let mut samps_written: usize = 0;
    let mut t_for_check = 0.0f32;
    let mut autostop_from_prev = false;

    loop {
        if t_for_check >= dur || autostop_from_prev {
            break;
        }
        let t = samps_written as f32 / r;

        let time_origin_val = params.time_origin.at(t, dur);
        let rate_val = params.rate.at(t, dur);
        let win_low_val = params.window_low.at(t, dur);
        let win_hi_raw = params.window_high.at(t, dur);
        let win_hi_val = if win_hi_raw < 0.0 {
            analysis_dur
        } else {
            win_hi_raw
        };

        let step = nav.advance(
            filttinc,
            time_origin_val,
            rate_val,
            win_low_val,
            win_hi_val,
            analysis_dur,
            t,
            &mut dur,
        );

        let peak_loop_smooth_time = params.loop_smooth.at(t, dur);
        let loop_smooth_time = make_loop_smooth_time(
            step.filttnow,
            params.loop_mode,
            params.autostop,
            win_low_val,
            win_hi_val,
            peak_loop_smooth_time,
        );

        let mut channel = interpolate_frame(analysis, iframes_per_sec, step.filttnow);

        if first_frame {
            previous_freq.copy_from_slice(&channel[..n_plus_2.min(channel.len())]);
            first_frame = false;
        }

        let (loop_c, loop_minusc) = smooth_setup(loop_smooth_time, i_factor as f32 / r);
        channel_smoother.smooth(&mut channel, loop_c, loop_minusc, loop_c, loop_minusc);
        smoothfreqs(&mut channel, &mut previous_freq, loop_c, loop_minusc);

        // ---- per-tone control values for this frame ----
        // TODO(phase2): Rate_Correlated_Randomization_Switch/
        // rateCorrelatedForceSuppressionSwitch (both default off) would
        // modify `this_stasis_median_harmony`/`this_force_factor` here;
        // this phase always takes the real C's own default-off branch.
        let stasis_median_harmony: Vec<f32> = tones
            .iter()
            .map(|tn| tn.stasis_median_harmony_db.at(t, dur))
            .collect();
        let force_factor: Vec<f32> = tones
            .iter()
            .map(|tn| tn.force_factor.at(t, dur).min(1.0))
            .collect();
        let tone_db: Vec<f32> = tones.iter().map(|tn| tn.tone_db.at(t, dur)).collect();
        let partial_shift: Vec<f32> = tones.iter().map(|tn| tn.partial_shift.at(t, dur)).collect();
        let spectral_stretch: Vec<f32> = tones
            .iter()
            .map(|tn| tn.spectral_stretch_compress.at(t, dur))
            .collect();
        let bandpass_cf: Vec<f32> = tones.iter().map(|tn| tn.bandpass_cf.at(t, dur)).collect();
        let bandpass_rolloff: Vec<f32> = tones
            .iter()
            .map(|tn| tn.bandpass_rolloff_db_per_octave.at(t, dur))
            .collect();
        let transpose_point_pitch_hz: Vec<f32> = tones
            .iter()
            .enumerate()
            .map(|(i, tn)| {
                let v = tn.transpose_point_pitch.at(t, dur);
                if band_setup.transpose_shift_method_pitch_is_oppc[i] {
                    oppc_to_hz(v)
                } else {
                    v
                }
            })
            .collect();

        let master_gain_amp = db_to_amp.convert(params.master_gain_db.at(t, dur));
        let tones_master_gain_amp = db_to_amp.convert(params.tones_master_gain_db.at(t, dur));
        let tones_master_freq_shift = params.tones_master_freq_shift_hz.at(t, dur);
        let pmt = semitones_to_mult.convert(params.tones_macro_pitch_semitones.at(t, dur));

        // ---- static harmony (spectral-morph) amplitude, per bin ----
        let mut static_harmony_amp = vec![0.0f32; band_setup.bins.len()];
        let mut sum_non_suppressed = 0.0f32;
        let mut sum_static = 0.0f32;
        for (bin_idx, bin) in band_setup.bins.iter().enumerate() {
            let this_stasis_median = stasis_median_harmony[bin.tone];
            let real_median_index =
                (-this_stasis_median).clamp(0.0, (NUM_STATIC_LEVELS - 2) as f32);
            let lower_level = real_median_index as usize;
            let upper_level = lower_level + 1;
            let upper_prop = real_median_index - lower_level as f32;
            let lower_prop = 1.0 - upper_prop;

            let num_bands = band_setup.num_bands();
            let amp = if bin_idx == static_freq.band_strongest_bin[bin.band] {
                let lo = static_freq.band_peak_amp[lower_level * num_bands + bin.band];
                let hi = static_freq.band_peak_amp[upper_level * num_bands + bin.band];
                lower_prop * lo + upper_prop * hi
            } else {
                0.0
            };
            static_harmony_amp[bin_idx] = amp;

            let lo_avg = static_freq.averages[lower_level * n_plus_2 + bin.channel_amp_index];
            let hi_avg = static_freq.averages[upper_level * n_plus_2 + bin.channel_amp_index];
            sum_non_suppressed += lower_prop * lo_avg + upper_prop * hi_avg;
            sum_static += amp;
        }
        if sum_static > 0.0 {
            let rescale = sum_non_suppressed / sum_static;
            for a in static_harmony_amp.iter_mut() {
                *a *= rescale;
            }
        }

        // ---- live harmony, per bin (delay time assumed 0 - see
        // this module's own top doc comment on Phase 3) ----
        let mut harmony_amp: Vec<f32> = band_setup
            .bins
            .iter()
            .map(|b| channel[b.channel_amp_index])
            .collect();
        let mut harmony_freq: Vec<f32> = band_setup
            .bins
            .iter()
            .map(|b| channel[b.channel_amp_index + 1])
            .collect();

        // ---- per-band pitch "tuning" toward the band's own
        // amp-weighted average frequency ----
        for band in 0..band_setup.num_bands() {
            let lo = band_setup.band_begin[band];
            let hi = band_setup.band_begin[band + 1];
            let mut asum = 0.0f32;
            let mut fsum = 0.0f32;
            for mm in lo..hi {
                let amp_sq = harmony_amp[mm] * harmony_amp[mm];
                fsum += harmony_freq[mm] * amp_sq;
                asum += amp_sq;
            }
            if asum <= 0.0 {
                continue;
            }
            let tone_now = band_setup.bins[lo].tone;
            let this_tune_factor = force_factor[tone_now];
            let this_avg_freq = fsum / asum;
            let band_avg_freq = static_freq.band_average_freq[band];

            if this_tune_factor <= 1.0 {
                let tune_band_prop_point = 0.33;
                let this_prop = if this_tune_factor >= tune_band_prop_point {
                    let temp = curve(
                        1.0,
                        0.0,
                        (this_tune_factor - tune_band_prop_point)
                            * (1.0 / (1.0 - tune_band_prop_point)),
                        1.0,
                    );
                    temp * ((band_avg_freq / this_avg_freq) - 1.0) + 1.0
                } else {
                    (band_avg_freq / this_avg_freq) - 1.0 + 1.0
                };
                #[allow(clippy::needless_range_loop)]
                for mm in lo..hi {
                    harmony_freq[mm] *= this_prop;
                }
            } else {
                #[allow(clippy::needless_range_loop)]
                for mm in lo..hi {
                    harmony_freq[mm] *=
                        (this_avg_freq / band_avg_freq).powf(this_tune_factor - 1.0);
                }
            }

            if this_tune_factor < 1.0 {
                let tune_null_prop_point = 0.5;
                let this_prop = if this_tune_factor > tune_null_prop_point {
                    curve(
                        0.0,
                        1.0,
                        (1.0 / (1.0 - tune_null_prop_point))
                            * (this_tune_factor - tune_null_prop_point),
                        2.0,
                    )
                } else {
                    0.0
                };
                #[allow(clippy::needless_range_loop)]
                for mm in lo..hi {
                    let sf = band_setup.bins[mm].source_freq;
                    harmony_freq[mm] = sf + this_prop * (harmony_freq[mm] - sf);
                }
            }
        }

        // ---- blend static/live amplitude, apply tone gain, transpose ----
        for (bin_idx, bin) in band_setup.bins.iter().enumerate() {
            let tone_now = bin.tone;
            let this_amp_force_factor = curve(
                0.0,
                1.0,
                force_factor[tone_now],
                static_freq.force_curve_index[bin_idx],
            );

            harmony_amp[bin_idx] = static_harmony_amp[bin_idx]
                + this_amp_force_factor * (harmony_amp[bin_idx] - static_harmony_amp[bin_idx]);
            harmony_amp[bin_idx] *= db_to_amp.convert(tone_db[tone_now])
                * bin.coswindow
                * tones_master_gain_amp
                * master_gain_amp;

            let source_freq = band_setup.source_pt_fund_freq[tone_now];
            let mut newfreq = harmony_freq[bin_idx] + source_freq * partial_shift[tone_now];
            newfreq += source_freq * spectral_stretch[tone_now] * ((newfreq / source_freq) - 1.0);
            newfreq *= transpose_point_pitch_hz[tone_now] / source_freq;

            harmony_freq[bin_idx] = if band_setup.trans_switch[tone_now] {
                (newfreq + tones_master_freq_shift) * pmt
            } else {
                newfreq
            };
        }

        // ---- per-tone bandpass tone filter ----
        for band in 0..band_setup.num_bands() {
            let lo = band_setup.band_begin[band];
            let hi = band_setup.band_begin[band + 1];
            let tone_now = band_setup.bins[lo].tone;
            if !band_setup.tone_filter_switch[tone_now] {
                continue;
            }
            let rolloff_amp = db_to_amp.convert(bandpass_rolloff[tone_now]);
            if rolloff_amp == 1.0 {
                continue;
            }
            let cf = bandpass_cf[tone_now];
            let filter_type = band_setup.tone_filter_type[tone_now];
            for mm in lo..hi {
                if harmony_freq[mm] <= 0.0 {
                    continue;
                }
                // `log10(x) * log2` in the C, where `log2 = 1. /
                // log10(2.)` - i.e. exactly `log2(x)`, computed directly
                // here rather than via that double-log10 detour.
                if harmony_freq[mm] > cf {
                    if filter_type != 2 {
                        let temp2 = (harmony_freq[mm] / cf).log2();
                        harmony_amp[mm] *= rolloff_amp.powf(temp2);
                    }
                } else if filter_type != 3 {
                    let temp2 = (cf / harmony_freq[mm]).log2();
                    harmony_amp[mm] *= rolloff_amp.powf(temp2);
                }
            }
        }

        // ---- DC cut, bounds clamp ----
        {
            let mut interleaved = vec![0.0f32; band_setup.bins.len() * 2];
            for i in 0..band_setup.bins.len() {
                interleaved[2 * i] = harmony_amp[i];
                interleaved[2 * i + 1] = harmony_freq[i];
            }
            cut_dc(&mut interleaved, 20.0);
            for i in 0..band_setup.bins.len() {
                harmony_amp[i] = interleaved[2 * i];
                harmony_freq[i] = interleaved[2 * i + 1];
            }
        }
        for i in 0..band_setup.bins.len() {
            if harmony_freq[i] <= 0.0 || harmony_freq[i] >= nyquist {
                harmony_amp[i] = 0.0;
            }
        }

        let frame = Frame {
            bins: harmony_amp
                .iter()
                .zip(&harmony_freq)
                .map(|(&a, &f)| (a, f))
                .collect(),
        };
        let synt = getthresh(&frame.bins, threshfac);
        let hop_out = osc.synthesize(&frame, synt);
        on += i_factor as i64;
        if on + nw as i64 - i_factor as i64 >= 0 {
            output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
            samps_written += i_factor;
        }

        autostop_from_prev = step.autostop;
        t_for_check = t;
    }

    output
}

/// Ports `smoothfreqs()`: symmetric attack/release-free smoothing of the
/// odd-indexed (frequency) slots of a mag/freq-interleaved array -
/// distinct from [`Smoother`] (amplitude slots, asymmetric
/// attack/release). No first-call priming of its own here (unlike
/// `Smoother`) since `process_channel` primes `previous_freq` from the
/// very first frame's own channel data before the first call, matching
/// the C's own explicit `if(frame_count==0) for(...) previous_channel[i]
/// = channel[i];` priming right before `smooth()`/`smoothfreqs()` are
/// ever called.
fn smoothfreqs(a: &mut [f32], old_a: &mut [f32], coef: f32, minuscoef: f32) {
    if coef != 0.0 {
        for i in (1..a.len()).step_by(2) {
            a[i] = coef * old_a[i] + minuscoef * a[i];
        }
    }
    for i in (1..a.len()).step_by(2) {
        old_a[i] = a[i];
    }
}

/// Ports `cutDC()`: bins below `cutoff_freq` get their amplitude scaled
/// by `(freq / cutoff_freq)^4` (a steep, DC-blocking taper).
fn cut_dc(channel: &mut [f32], cutoff_freq: f32) {
    for i in (0..channel.len()).step_by(2) {
        if channel[i + 1] < cutoff_freq {
            let freq = channel[i + 1].max(0.0);
            channel[i + 1] = freq;
            let prop = freq / cutoff_freq;
            channel[i] *= prop * prop * prop * prop;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn no_tables(_: &str) -> Result<Vec<f32>, String> {
        Err("no tables in this test".to_string())
    }

    #[test]
    fn cut_data_lines_strips_brace_comments() {
        let text = "1 2 3 {this is a comment} 4 5";
        let out = cut_data_lines(text).unwrap();
        assert_eq!(
            out.split_whitespace().collect::<Vec<_>>(),
            ["1", "2", "3", "4", "5"]
        );
    }

    #[test]
    fn cut_data_lines_mismatched_brace_errors() {
        assert!(cut_data_lines("1 2 {3 4").is_err());
        assert!(cut_data_lines("1 2 } 3 4").is_err());
    }

    #[test]
    fn cut_data_lines_mute_mode_drops_marked_record() {
        let text = "1 2 3\nm 9 9 9\n4 5 6\n";
        let out = cut_data_lines_n(text, 3);
        // The muted record's own 3 tokens are gone; the other two remain.
        assert_eq!(
            out.split_whitespace().collect::<Vec<_>>(),
            ["1", "2", "3", "4", "5", "6"]
        );
    }

    #[test]
    fn cut_data_lines_solo_mode_keeps_only_marked_records() {
        let text = "1 2 3\n!4 5 6\n7 8 9\n";
        let out = cut_data_lines_n(text, 3);
        assert_eq!(out.split_whitespace().collect::<Vec<_>>(), ["4", "5", "6"]);
    }

    /// `cut_data_lines` with a caller-chosen record size (this module's
    /// own `NUM_PARAMETERS` is 23, too large for a compact unit test) -
    /// exercised via a tiny local re-implementation sharing the same
    /// tokenizer, since `NUM_PARAMETERS` is a `pub const`, not a
    /// parameter, in the real port.
    fn cut_data_lines_n(text: &str, record_size: usize) -> String {
        // Mirrors `cut_data_lines` exactly but with a caller-chosen
        // record size - kept in the test module only.
        let mut stage1 = String::with_capacity(text.len());
        let mut paren_open = false;
        for c in text.chars() {
            match c {
                '{' => paren_open = true,
                '}' => paren_open = false,
                _ => {
                    if !paren_open {
                        stage1.push(c);
                    }
                }
            }
        }
        let chars: Vec<char> = stage1.chars().collect();
        let is_marker = |chars: &[char], i: usize, marker: char| -> bool {
            if chars[i] != marker {
                return false;
            }
            let last_c = if i == 0 { ' ' } else { chars[i - 1] };
            !last_c.is_alphabetic() && last_c != '/'
        };
        let solo_on = (0..chars.len()).any(|i| is_marker(&chars, i, '!'));
        let mut out = String::new();
        let mut i = 0usize;
        if solo_on {
            while i < chars.len() {
                if is_marker(&chars, i, '!') {
                    i += 1;
                    for field in 0..record_size {
                        let (tok, next) = take_token(&chars, i);
                        i = next;
                        if field > 0 {
                            out.push(' ');
                        }
                        out.push_str(&tok);
                    }
                    out.push(' ');
                } else {
                    i += 1;
                }
            }
        } else {
            while i < chars.len() {
                if is_marker(&chars, i, 'm') {
                    i += 1;
                    for _ in 0..record_size {
                        let (_, next) = take_token(&chars, i);
                        i = next;
                    }
                } else {
                    out.push(chars[i]);
                    i += 1;
                }
            }
        }
        out
    }

    #[test]
    fn parse_tone_data_file_reads_all_23_fields() {
        let fields: Vec<String> = (0..NUM_PARAMETERS)
            .map(|i| (i as f32 + 1.0).to_string())
            .collect();
        let text = fields.join(" ");
        let tones = parse_tone_data_file(&text, no_tables).unwrap();
        assert_eq!(tones.len(), 1);
        let t = &tones[0];
        assert_eq!(t.source_point, 1.0);
        assert_eq!(t.low_partial, 3.0);
        assert_eq!(t.partial_spacing, 4.0);
        assert_eq!(t.number_of_partials, 5.0);
        assert_eq!(t.bandwidth, 6.0);
        assert_eq!(t.noise_switch, 13.0);
        assert_eq!(t.master_transposition_switch, 16.0);
        assert_eq!(t.tone_channel_output_number, 23.0);
        assert_eq!(t.transpose_point_pitch, ControlFn::Const(2.0));
        assert_eq!(t.delay_time, ControlFn::Const(22.0));
    }

    #[test]
    fn parse_tone_data_file_too_short_errors() {
        assert!(parse_tone_data_file("1 2 3", no_tables).is_err());
    }

    fn simple_tone(source_hz: f32) -> ToneParams {
        ToneParams {
            source_point: source_hz,
            transpose_point_pitch: ControlFn::Const(source_hz),
            low_partial: 1.0,
            partial_spacing: 1.0,
            number_of_partials: 3.0,
            bandwidth: 0.5,
            partial_shift: ControlFn::Const(0.0),
            spectral_stretch_compress: ControlFn::Const(0.0),
            tone_db: ControlFn::Const(0.0),
            stasis_median_harmony_db: ControlFn::Const(0.0),
            noise_db: ControlFn::Const(-96.0),
            stasis_median_noise_db: ControlFn::Const(-96.0),
            noise_switch: 0.0,
            transpose_point_noise: ControlFn::Const(source_hz),
            force_factor: ControlFn::Const(1.0),
            master_transposition_switch: 0.0,
            synthetic_vibrato_switch: 0.0,
            bandpass_cf: ControlFn::Const(20000.0),
            bandpass_rolloff_db_per_octave: ControlFn::Const(0.0),
            tone_filter_switch_raw: 0.0,
            delay_time_switch_scaler: 1.0,
            delay_time: ControlFn::Const(0.0),
            tone_channel_output_number: 0.0,
        }
    }

    #[test]
    fn setup_bands_single_tone_produces_expected_partial_count() {
        let tones = vec![simple_tone(440.0)];
        let n = 1024;
        let r = 44100.0;
        let nyquist = r / 2.0;
        let fundamental = r / n as f32;
        let setup = setup_bands(&tones, nyquist, fundamental, n).unwrap();
        // 3 partials requested explicitly (`number_of_partials = 3`).
        assert_eq!(setup.num_bands(), 3);
        assert!(!setup.bins.is_empty());
        for bin in &setup.bins {
            assert_eq!(bin.tone, 0);
            assert!(bin.channel_amp_index < n + 2);
        }
    }

    #[test]
    fn setup_bands_rejects_bad_bandwidth() {
        let mut tone = simple_tone(440.0);
        tone.bandwidth = 0.0;
        let err = setup_bands(&[tone], 22050.0, 43.0, 1024);
        assert!(err.is_err());
    }

    fn silence_analysis(n: usize, frames: usize) -> Vec<Vec<f32>> {
        vec![vec![0.0f32; n + 2]; frames]
    }

    #[test]
    fn silence_in_silence_out() {
        let n = 512;
        let r = 44100u32;
        let d = 128u32;
        let tones = vec![simple_tone(440.0)];
        let setup = setup_bands(&tones, r as f32 / 2.0, r as f32 / n as f32, n).unwrap();
        let analysis = silence_analysis(n, 100);
        let static_freq = compute_static_freq_response(
            &analysis,
            r as f32 / d as f32,
            n + 2,
            r as f32 / n as f32,
            &setup,
        );

        let params = ChordmapperplusParams {
            window_size: 0,
            frames_per_sec: 200.0,
            duration: 0.0,
            time_origin: ControlFn::Const(0.0),
            rate: ControlFn::Const(1.0),
            window_low: ControlFn::Const(0.0),
            window_high: ControlFn::Const(-1.0),
            loop_smooth: ControlFn::Const(0.0),
            autostop: true,
            loop_mode: LoopMode::Wrap,
            master_gain_db: ControlFn::Const(0.0),
            tones_master_gain_db: ControlFn::Const(0.0),
            tones_master_freq_shift_hz: ControlFn::Const(0.0),
            tones_macro_pitch_semitones: ControlFn::Const(1.0),
            threshold_db: -96.0,
        };

        let out = process_channel(&analysis, n, d, r, &tones, &setup, &static_freq, &params);
        assert!(
            out.iter().all(|&s| s == 0.0),
            "silence in must stay silence out"
        );
    }

    #[test]
    fn sine_tone_produces_bounded_nonzero_output() {
        let n = 512;
        let r = 44100u32;
        let d = 128u32;
        let source_hz = 440.0f32;
        let tones = vec![simple_tone(source_hz)];
        let setup = setup_bands(&tones, r as f32 / 2.0, r as f32 / n as f32, n).unwrap();

        // A single analysis frame repeated: one strong bin at the tone's
        // own fundamental. Every bin covering a given partial (including
        // its cosine-windowed edge bins) shares the same `source_freq`,
        // so pick the one with `coswindow == 1.0` (the exact-center bin,
        // not just any bin matching by frequency) to avoid landing on an
        // edge bin whose window tapers toward zero.
        let fundamental_bin = setup
            .bins
            .iter()
            .filter(|b| (b.source_freq - source_hz).abs() < 1.0)
            .max_by(|a, b| a.coswindow.total_cmp(&b.coswindow))
            .expect("fundamental bin should exist");
        let mut frame = vec![0.0f32; n + 2];
        frame[fundamental_bin.channel_amp_index] = 1.0;
        frame[fundamental_bin.channel_amp_index + 1] = source_hz;
        let analysis = vec![frame; 100];

        let static_freq = compute_static_freq_response(
            &analysis,
            r as f32 / d as f32,
            n + 2,
            r as f32 / n as f32,
            &setup,
        );

        let params = ChordmapperplusParams {
            window_size: 0,
            frames_per_sec: 200.0,
            duration: 0.0,
            time_origin: ControlFn::Const(0.0),
            rate: ControlFn::Const(1.0),
            window_low: ControlFn::Const(0.0),
            window_high: ControlFn::Const(-1.0),
            loop_smooth: ControlFn::Const(0.0),
            autostop: true,
            loop_mode: LoopMode::Wrap,
            master_gain_db: ControlFn::Const(0.0),
            tones_master_gain_db: ControlFn::Const(0.0),
            tones_master_freq_shift_hz: ControlFn::Const(0.0),
            tones_macro_pitch_semitones: ControlFn::Const(1.0),
            threshold_db: -96.0,
        };

        let out = process_channel(&analysis, n, d, r, &tones, &setup, &static_freq, &params);
        assert!(!out.is_empty(), "expected some resynthesized output");
        assert!(
            out.iter().any(|&s| s != 0.0),
            "expected nonzero output for a real tone"
        );
        assert!(
            out.iter().all(|&s| s.is_finite() && s.abs() < 100.0),
            "output should stay bounded"
        );
    }

    #[test]
    fn cut_dc_scales_low_frequency_bins_toward_zero() {
        let mut channel = [1.0f32, 5.0, 1.0, 30.0];
        cut_dc(&mut channel, 20.0);
        assert!(channel[0] < 1.0);
        assert_eq!(channel[2], 1.0, "bins at/above cutoff are untouched");
    }
}
