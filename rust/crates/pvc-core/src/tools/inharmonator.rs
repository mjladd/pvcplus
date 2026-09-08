//! Ports `inharmonator.c`: an "inharmonic partials remapper" - reads a
//! data table of discrete target partials (number, pitch-shift target,
//! decibels, time delay, feedback-decay time), builds a per-bin filter
//! array from windowed bands around each partial's own bin (half-Hann,
//! half-Welch, or rectangle), and resynthesizes the input through it via
//! the oscillator bank (`inharmonator.c` never overlap-adds: `P` is a
//! fixed `1.0` never touched by any flag, exactly like `tools::ring`'s
//! and `tools::harmonizer`'s own already-documented "always obank"
//! finding). Bins outside every partial's own band ("non-targets", the
//! residual spectrum) get a separate, uniform pitch/frequency-shift/gain
//! treatment instead. Both paths are then fed through a genuine spectral
//! feedback delay line, and can optionally be mixed with a delayed,
//! shifted, gain-adjusted copy of the raw source spectrum.
//!
//! **Real, severe bug: seven data-modifier scalers are used
//! uninitialized unless their own flag is passed.** `data_partial_number_scaler`/
//! `_shifter`, `data_decibel_scaler`, `data_time_delay_scaler`/`_shifter`,
//! `data_time_decay_scaler`/`_shifter` are declared as plain `float`
//! locals in `main()` with **no initializer at all** - confirmed by
//! reading the declaration line directly, not inferred from `usage()`.
//! Unlike this project's every other "`usage()` and the initializer
//! disagree" finding so far (`delayfilter`'s `-T`, `filtdeviator`'s
//! `-q`/`-B`, `tvfiltdeviator`'s `-O`, `ratechanger`'s `-O` - each of
//! which has a *real*, if undocumented, initializer value), these seven
//! have no real value to reproduce at all: whichever of `-z`/`-R`/`-y`/
//! `-o`/`-O`/`-g`/`-k` isn't passed leaves its variable holding whatever
//! garbage was already on the stack, silently corrupting every partial's
//! own number/decibels/delay/decay by an unpredictable amount. Since
//! there is no deterministic C behavior here to match, this port uses
//! `usage()`'s own documented defaults (`z:[1] R:[0] y:[1] o:[1] O:[0]
//! g:[0] k:[0]`) instead - the tool's own stated intent, not its actual
//! undefined behavior.
//!
//! **Real finding: "master gain" (`-A`) only ever affects the *source*
//! signal, never the resynthesized partials/non-targets.** `gain =
//! dB_to_amp(dBgain.A[0])` is computed every frame but consumed at
//! exactly one call site, `source_channel_out[i] *= gain` - confirmed by
//! grepping every reference to `gain` in the file. Despite `usage()`
//! calling it "master gain in decibels" with no qualification, it is a
//! source-only gain in the actual, compiled behavior. Reproduced as
//! read: [`process_channel`] applies `master_gain_db` only to the source
//! path.
//!
//! **`-Y` (`INHARM_freqsmooth`) is entirely dead.** Its value is computed
//! and fed to `smooth_setup` every frame, but the line that would
//! actually apply the resulting coefficients to `harmony`'s frequency
//! slots is commented out in the C source itself (`//harmony[i] =
//! (ifreqsmoothc * previous_harmony[i]) + ...`), not merely unreachable -
//! confirmed by reading the literal comment. Not exposed as a `pvc
//! inharmonator` flag at all, matching this project's established
//! convention of not exposing a provably-dead flag (see
//! `tools::compander`'s own `-L`/release precedent).
//!
//! **Randomized delay times (`-H`/`-K`, `TARGETS_randomizationFlag`/
//! `NON_TARGETS_randomizationFlag`) are not ported**, matching
//! `tools::ring`'s established `randf()` precedent - both flags' off
//! state (the default) takes the deterministic "use the data file's own
//! delay time directly" path this port always takes. Their own smoothing
//! control functions (`-c`/`-n`, `TARGETS_timedelayresponsetime`/
//! `NON_TARGETS_timedelayresponsetime`) have no other consumer in the C
//! (confirmed by grepping every reference), so they are dead in this
//! port too and not exposed.
//!
//! **The spectral feedback delay line** reuses this project's established
//! `unconvert2`+`unconvert3`+`convert3` pattern (see
//! `tools::filtdeviator`'s own doc comment for the general technique):
//! attenuate a raw copy of the delay line's own fetched input by
//! `dB_to_amp(-60 / (decayFrames / delayFrames))` per bin, `unconvert_only`
//! it alongside the delay ring's own current-slot contents (two
//! independent phase-tracking `Synthesizer`s), sum the two raw spectra
//! directly, and `convert` the sum back into amplitude/frequency via a
//! third, independent `PhaseTracker` before writing it back into the
//! ring - never an inverse FFT in between. The *source* path has no such
//! feedback accumulator - only a single-frame delay fetch, matching
//! `tools::filtdeviator`'s own source path.
//!
//! **`source_enabled` reads `-t`'s own oscillator threshold, not a fixed
//! `-96dB`** - confirmed by reading the exact condition
//! (`SOURCE_dB.A[0] > threshfacdB`), unlike `tools::filtdeviator`'s own
//! `source_enabled` helper (hardcoded `-96.0`), which ports a genuinely
//! different tool's own genuinely different condition - not assumed to
//! match on the strength of the similar name.
//!
//! **Half-Hann/half-Welch band windows** (`halfHannWindow`/
//! `halfWelchWindow`) are each a 1024-entry lookup table quantized at
//! parse time - the same "reproduce the table, not the closed form"
//! convention `pvc_core::units::DbToAmp`/`SemitonesToMult` already
//! established - defensively clamped to the table's own bounds here
//! (see `tools::ratechanger`'s own doc comment on why every fixed-size
//! lookup table in this project now gets that treatment).
//!
//! **Dead `crack()` flags**, confirmed by cross-referencing the parser's
//! own accept string (which lists both cases of every letter it takes)
//! against every `case` in the `switch`: lowercase `d` and lowercase `s`
//! are both accepted but have no handler (`D`/`S` are real, distinct
//! flags - frames-per-second and the target frequency-interpolation
//! control, respectively). Not exposed as `pvc inharmonator` flags.

use crate::control::ControlFn;
use crate::pvoc::{getthresh, Analyzer, Frame, OscBank, PhaseTracker, Synthesizer};
use crate::response::oppc_to_hz;
use crate::smooth::{smooth_setup, Smoother};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::curve;
use crate::window::{make_windows, Window};

const OSCILBANKGAIN: f32 = 1.7782794;

/// `-Z`: how a partial's own "shift data" column (`PP[j+1]`) is
/// interpreted.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ShiftMethod {
    /// `0`: a plain frequency multiplier.
    Multiplier,
    /// `1`: an absolute target frequency in Hz.
    FreqPoint,
    /// `2`: an absolute target pitch as `octave.pitchclass`.
    OctavePitchClass,
    /// `3`: a multiplier of the tool's own fundamental (not the
    /// partial's own nominal frequency).
    PartialShiftPoint,
}

/// `-B`: how the bins between a partial's own bin and its band edges
/// taper.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PartialBandWindow {
    Rectangle,
    Hann,
    Welch,
}

/// One row of the partials data table (`NUM_PARAMETERS = 5` columns).
#[derive(Debug, Clone, Copy)]
pub struct Partial {
    pub number: f32,
    pub shift_data: f32,
    pub decibels: f32,
    pub time_delay: f32,
    pub feedback_decay_time: f32,
}

/// The seven `-z`/`-R`/`-y`/`-o`/`-O`/`-g`/`-k` data-modifier scalers -
/// see this module's doc comment on why their defaults come from
/// `usage()` rather than the (nonexistent) C initializer.
#[derive(Debug, Clone, Copy)]
pub struct DataModifiers {
    pub number_scaler: f32,
    pub number_shifter: f32,
    pub decibel_scaler: f32,
    pub delay_scaler: f32,
    pub delay_shifter: f32,
    pub decay_scaler: f32,
    pub decay_shifter: f32,
}

impl Default for DataModifiers {
    fn default() -> Self {
        DataModifiers {
            number_scaler: 1.0,
            number_shifter: 0.0,
            decibel_scaler: 1.0,
            delay_scaler: 1.0,
            delay_shifter: 0.0,
            decay_scaler: 0.0,
            decay_shifter: 0.0,
        }
    }
}

/// Applies the data-modifier scalers to a freshly-parsed partials table -
/// `main()`'s own per-column scale/shift loop, including the `PP[k] <
/// 1.` clamp on the (post-scale) partial number.
pub fn resolve_partials(raw: &[Partial], modifiers: &DataModifiers) -> Vec<Partial> {
    raw.iter()
        .map(|p| {
            let mut number =
                modifiers.number_scaler * (p.number - 1.0) + 1.0 + modifiers.number_shifter;
            if number < 1.0 {
                number = 1.0;
            }
            Partial {
                number,
                shift_data: p.shift_data,
                decibels: p.decibels * modifiers.decibel_scaler,
                time_delay: p.time_delay * modifiers.delay_scaler + modifiers.delay_shifter,
                feedback_decay_time: p.feedback_decay_time * modifiers.decay_scaler
                    + modifiers.decay_shifter,
            }
        })
        .collect()
}

pub struct InharmonatorParams {
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,

    pub method: ShiftMethod,
    pub partial_band_window: PartialBandWindow,
    pub partial_bandwidth: ControlFn,

    /// `-A`. Source-only - see this module's doc comment.
    pub master_gain_db: ControlFn,

    pub target_harmadd: ControlFn,
    pub target_ptrans: ControlFn,
    pub target_db: ControlFn,
    pub target_amp_interp: ControlFn,
    pub target_freq_interp: ControlFn,
    /// `-T`. Evaluated at plain `t`, not per-partial-delay-shifted.
    pub target_time_interp: ControlFn,

    pub fundamental_freq_or_oppc: ControlFn,

    pub non_target_harmadd: ControlFn,
    pub non_target_ptrans: ControlFn,
    pub non_target_db: ControlFn,
    /// Evaluated at plain `t`.
    pub non_target_delay_t: ControlFn,
    pub non_target_decay_t: ControlFn,

    pub inharm_attack: ControlFn,
    pub inharm_release: ControlFn,

    pub warpshape: ControlFn,

    pub source_harmadd: ControlFn,
    pub source_ptrans: ControlFn,
    pub source_db: ControlFn,
    pub source_release: ControlFn,
    pub source_attack: ControlFn,
    /// Evaluated at plain `t`.
    pub source_delay_t: ControlFn,

    pub threshold_db: f32,
}

/// `SOURCE_dB.A[0] > threshfacdB || SOURCE_dB.n != 1.` - see this
/// module's doc comment on why this reads `threshold_db` rather than a
/// fixed `-96.0`.
pub fn source_enabled(source_db: &ControlFn, threshold_db: f32) -> bool {
    match source_db {
        ControlFn::Table(_) => true,
        ControlFn::Const(v) => *v > threshold_db,
    }
}

fn control_fn_max(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(vals) => vals.iter().copied().fold(f32::MIN, f32::max),
    }
}

fn half_hann_table() -> Vec<f32> {
    let twopi = std::f64::consts::PI * 2.0;
    (0..1024)
        .map(|i| {
            let x = (i as f64 / 1024.0) * (twopi * 0.5);
            (0.5 * (x.cos() + 1.0)) as f32
        })
        .collect()
}

fn half_welch_table() -> Vec<f32> {
    (0..1024)
        .map(|i| {
            let x = i as f64 / 1023.0;
            (1.0 - x * x) as f32
        })
        .collect()
}

fn table_lookup(table: &[f32], v: f32) -> f32 {
    let i = ((v * 1023.0 + 0.5) as i64).clamp(0, table.len() as i64 - 1);
    table[i as usize]
}

/// Ports `cutDC()`: bins below `cutoff_freq` get their amplitude scaled
/// by `(freq / cutoff_freq)^4` - a soft DC-blocking taper, not a hard
/// zero.
fn cut_dc(channel: &mut [f32], cutoff_freq: f32) {
    for pair in channel.chunks_exact_mut(2) {
        if pair[1] < cutoff_freq {
            if pair[1] < 0.0 {
                pair[1] = 0.0;
            }
            let prop = pair[1] / cutoff_freq;
            pair[0] *= prop * prop * prop * prop;
        }
    }
}

/// Ports `spectmagwarp_inharm()`: like `crate::warp::spectmagwarp`'s own
/// `normalize=false` branch, but the peak search and the warp itself
/// only ever consider bins belonging to a target partial's own band
/// (`f[2*bin] != -1.0`) - non-target bins are left completely untouched.
/// A genuinely distinct function from `spectmagwarp`, not a variant
/// worth sharing (see this module's doc comment on why this tool's
/// machinery isn't reused from a sibling without confirming first).
fn spectmagwarp_inharm(sp: &mut [f32], f: &[f32], warpshape: f32) {
    if warpshape == 0.0 {
        return;
    }
    let peak = sp
        .chunks_exact(2)
        .zip(f.chunks_exact(2))
        .filter(|(_, fp)| fp[0] != -1.0)
        .map(|(s, _)| s[0])
        .fold(f32::MIN, f32::max);
    if peak <= 0.0 {
        return;
    }
    for (s, fp) in sp.chunks_exact_mut(2).zip(f.chunks_exact(2)) {
        if fp[0] != -1.0 {
            s[0] = curve(0.0, peak, s[0] / peak, warpshape);
        }
    }
}

/// `getGlobalFunctionValues()`: evaluates `cf` once per partial, each at
/// that partial's own `t - delay_times[i]` - the `timeRateScalers`
/// multiply the C also applies is always `1.0` in `inharmonator.c` (set
/// once, never touched again), so it's omitted here rather than modeled
/// as a dead no-op parameter.
fn per_partial_values(cf: &ControlFn, dur: f32, t: f32, delay_times: &[f32]) -> Vec<f32> {
    delay_times.iter().map(|&dt| cf.at(t - dt, dur)).collect()
}

/// A ring of the last `capacity` analysis frames (flat `n_plus_2`-float
/// layout) - see `tools::filtdeviator::DelayRing`'s identical shape
/// (kept as an independent copy per this project's convention of not
/// sharing tool-local machinery across ports).
struct DelayRing {
    frames: Vec<Vec<f32>>,
    capacity: usize,
}

impl DelayRing {
    fn new(capacity: usize, n_plus_2: usize) -> Self {
        DelayRing {
            frames: vec![vec![0.0; n_plus_2]; capacity],
            capacity,
        }
    }

    fn push(&mut self, frame_count: usize, flat: Vec<f32>) -> usize {
        let index = frame_count % self.capacity;
        self.frames[index] = flat;
        index
    }

    fn frame(&self, now_index: usize, delay_frames: i64) -> &[f32] {
        let mut index = now_index as i64 - delay_frames;
        while index < 0 {
            index += self.capacity as i64;
        }
        &self.frames[(index as usize) % self.capacity]
    }

    fn bin(&self, now_index: usize, delay_frames: i64, bin: usize) -> (f32, f32) {
        let f = self.frame(now_index, delay_frames);
        (f[2 * bin], f[2 * bin + 1])
    }
}

/// Resynthesizes one channel. `partials` is already data-modifier-scaled
/// (see [`resolve_partials`]). `n` is `-N`'s own FFT size.
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    input: &[f32],
    partials: &[Partial],
    n: usize,
    sample_rate: u32,
    dur: f32,
    params: &InharmonatorParams,
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
    let half_hann = half_hann_table();
    let half_welch = half_welch_table();

    let source_enabled = source_enabled(&params.source_db, params.threshold_db);

    // ---- RING CAPACITY ----
    let harmony_max_delay_from_partials = partials
        .iter()
        .map(|p| p.time_delay + p.feedback_decay_time)
        .fold(0.0f32, f32::max);
    let non_target_max = control_fn_max(&params.non_target_delay_t).max(0.0)
        + control_fn_max(&params.non_target_decay_t).max(0.0);
    let harmony_max_delay_t = harmony_max_delay_from_partials.max(non_target_max);
    let harmony_max_num_delay_frames = 1 + (harmony_max_delay_t * frames_per_sec + 0.5) as usize;

    let source_max_delay_t = if source_enabled {
        control_fn_max(&params.source_delay_t).max(0.0)
    } else {
        0.0
    };
    let source_max_num_delay_frames = 1 + (source_max_delay_t * frames_per_sec + 0.5) as usize;

    // `ringTime` (a global in the C, set here and read back by
    // `fileio.c`'s own `shiftin`/`setupfiles` machinery once the real
    // input is exhausted) extends how long the input keeps reading as
    // silence past its own end, so a bin's own time delay/decay can ring
    // out into real output samples - see `tools::filtdeviator`'s own
    // identical finding and doc comment for the general mechanism.
    // Modeled here the same way: pad the input with that many zero
    // samples rather than reproducing the global/side-channel mechanism.
    let ring_time = harmony_max_delay_t.max(source_max_delay_t);
    let ring_time_samples = (ring_time * r) as usize;
    let padded_input;
    let input: &[f32] = if ring_time_samples > 0 {
        let mut v = input.to_vec();
        v.extend(std::iter::repeat_n(0.0f32, ring_time_samples));
        padded_input = v;
        &padded_input
    } else {
        input
    };

    let mut harmony_ring = DelayRing::new(harmony_max_num_delay_frames, n_plus_2);
    let mut source_ring = DelayRing::new(source_max_num_delay_frames, n_plus_2);

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);

    let mut osc_harmony = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let osc_table = osc_harmony.table();
    let mut osc_source = OscBank::with_shared_table(n2, sample_rate, i_factor, 1.0, osc_table);

    let mut decayed_input_synth = Synthesizer::new(n, vec![0.0; nw], i_factor, d, sample_rate);
    let mut delay_now_synth = Synthesizer::new(n, vec![0.0; nw], i_factor, d, sample_rate);
    let mut delay_feedback_phase = PhaseTracker::new_analysis(n2, d, sample_rate);

    let mut harmony_smoother = Smoother::new(n_plus_2);
    let mut source_smoother = Smoother::new(n_plus_2);

    // Persistent per-bin filter state, rebuilt only when the trigger
    // condition below holds - matches the C's own stack arrays declared
    // once in `main()` and conditionally rewritten.
    let mut f_amp = vec![-1.0f32; n2 + 1]; // -1.0 sentinel = "not a target bin".
    let mut f_shift_mult = vec![0.0f32; n2 + 1];
    let mut time_delay = vec![0.0f32; n2 + 1];
    let mut decay_time = vec![0.0f32; n2 + 1];
    let mut partial_data_line = vec![0usize; n2 + 1];

    let delay_times: Vec<f32> = partials.iter().map(|p| p.time_delay).collect();

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

        let channel_frame = analyzer.push(&hop).expect("hop is exactly d samples");
        let channel_flat = channel_frame.to_pva_floats();

        let harmony_now_index = harmony_ring.push(frame_count, channel_flat.clone());
        let source_now_index = source_ring.push(frame_count, channel_flat.clone());

        // ---- SOURCE FETCH + MASTER GAIN ----
        let source_delay_t = params.source_delay_t.at(t, dur);
        let source_delay_frames = (source_delay_t * frames_per_sec + 0.5) as i64;
        let mut source_channel_out = source_ring
            .frame(source_now_index, source_delay_frames)
            .to_vec();
        let master_gain = db_to_amp.convert(params.master_gain_db.at(t, dur));
        for a in source_channel_out.iter_mut().step_by(2) {
            *a *= master_gain;
        }

        // ---- ATTACK/RELEASE SMOOTHING SETUP ----
        let (iattackc, iattackm) = smooth_setup(params.inharm_attack.at(t, dur), ir);
        let (ireleasec, ireleasem) = smooth_setup(params.inharm_release.at(t, dur), ir);
        let warpshape = params.warpshape.at(t, dur);

        // ---- FUNDAMENTAL ----
        let fund_raw = params.fundamental_freq_or_oppc.at(t, dur);
        let fundamental_freq_now = if fund_raw <= 12.0 {
            oppc_to_hz(fund_raw)
        } else {
            fund_raw
        };

        // ---- PER-PARTIAL CONTROL VALUES ----
        let target_harmadd_values =
            per_partial_values(&params.target_harmadd, dur, t, &delay_times);
        let pmt_tone_values: Vec<f32> =
            per_partial_values(&params.target_ptrans, dur, t, &delay_times)
                .iter()
                .map(|&v| semitones_to_mult.convert(v))
                .collect();
        let target_gain_tone_values: Vec<f32> =
            per_partial_values(&params.target_db, dur, t, &delay_times)
                .iter()
                .map(|&v| db_to_amp.convert(v))
                .collect();
        let target_amp_interp_values =
            per_partial_values(&params.target_amp_interp, dur, t, &delay_times);
        let target_freq_interp_values =
            per_partial_values(&params.target_freq_interp, dur, t, &delay_times);
        let partial_bandwidth_values =
            per_partial_values(&params.partial_bandwidth, dur, t, &delay_times);
        let target_time_interp = params.target_time_interp.at(t, dur);

        // ---- NON-TARGET CONTROL VALUES ----
        let non_target_delay_t = params.non_target_delay_t.at(t, dur);
        let shifted_nt = t - non_target_delay_t;
        let non_target_harmadd = params.non_target_harmadd.at(shifted_nt, dur);
        let non_target_pm = semitones_to_mult.convert(params.non_target_ptrans.at(shifted_nt, dur));
        let non_target_gain = db_to_amp.convert(params.non_target_db.at(shifted_nt, dur));
        let non_target_decay_t = params.non_target_decay_t.at(shifted_nt, dur);

        // ---- SOURCE CONTROL VALUES ----
        let (source_gain, source_pm, source_harmadd, sattackc, sattackm, sreleasec, sreleasem) =
            if source_enabled {
                let shifted_s = t - source_delay_t;
                let gain = db_to_amp.convert(params.source_db.at(shifted_s, dur));
                let pm = semitones_to_mult.convert(params.source_ptrans.at(shifted_s, dur));
                let harmadd = params.source_harmadd.at(shifted_s, dur);
                let (rc, rm) = smooth_setup(params.source_release.at(shifted_s, dur), ir);
                let (ac, am) = smooth_setup(params.source_attack.at(shifted_s, dur), ir);
                (gain, pm, harmadd, ac, am, rc, rm)
            } else {
                (0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0)
            };

        // ---- REBUILD FILTER ARRAY IF TRIGGERED ----
        let rebuild = frame_count == 0
            || matches!(params.fundamental_freq_or_oppc, ControlFn::Table(_))
            || matches!(params.non_target_delay_t, ControlFn::Table(_))
            || matches!(params.non_target_decay_t, ControlFn::Table(_))
            || matches!(params.partial_bandwidth, ControlFn::Table(_));

        if rebuild {
            for bin in 0..=n2 {
                f_amp[bin] = -1.0;
                time_delay[bin] = -1.0;
                decay_time[bin] = 0.0;
            }

            for (data_line, p) in partials.iter().enumerate() {
                let part = fundamental_freq_now * p.number;
                let bandwidth = partial_bandwidth_values[data_line];
                let ipartial =
                    1 + 2 * ((p.number * fundamental_freq_now / fundamental).round() as i64);
                let i1 = 1 + 2
                    * (((p.number - bandwidth * 0.5) * fundamental_freq_now / fundamental).round()
                        as i64);
                let i2 = 1 + 2
                    * (((p.number + bandwidth * 0.5) * fundamental_freq_now / fundamental).round()
                        as i64);

                let shift_mult = |shift_data: f32| -> f32 {
                    match params.method {
                        ShiftMethod::Multiplier => shift_data,
                        ShiftMethod::FreqPoint => shift_data / part,
                        ShiftMethod::OctavePitchClass => oppc_to_hz(shift_data) / part,
                        ShiftMethod::PartialShiftPoint => shift_data * fundamental_freq_now / part,
                    }
                };

                let set_bin = |bin_index: i64,
                               bin: usize,
                               f_amp: &mut [f32],
                               f_shift_mult: &mut [f32],
                               time_delay: &mut [f32],
                               decay_time: &mut [f32],
                               partial_data_line: &mut [usize],
                               window: Option<f32>| {
                    time_delay[bin] = p.time_delay;
                    decay_time[bin] = p.feedback_decay_time;
                    partial_data_line[bin] = data_line;
                    if bin_index >= 0 && (bin_index as usize) < n {
                        f_shift_mult[bin] = shift_mult(p.shift_data);
                        // Real bug, reproduced faithfully: `window` *replaces*
                        // `dB_to_amp(decibels)` in the C rather than scaling
                        // it (`F[i-1] = halfWelchWindow(...)` is a plain
                        // assignment, not `*=`) - so a partial's own decibel
                        // level only actually reaches its exact center bin
                        // under Hann/Welch windowing; every other bin in its
                        // band gets the raw window value alone, `decibels`
                        // discarded entirely. Confirmed against the real
                        // binary's own per-bin F[] values, not just the
                        // source reading - see this module's doc comment.
                        f_amp[bin] = window.unwrap_or_else(|| db_to_amp.convert(p.decibels));
                    }
                };

                if ipartial >= 0 {
                    let bin = ((ipartial - 1) / 2) as usize;
                    if bin <= n2 {
                        set_bin(
                            ipartial,
                            bin,
                            &mut f_amp,
                            &mut f_shift_mult,
                            &mut time_delay,
                            &mut decay_time,
                            &mut partial_data_line,
                            None,
                        );
                    }
                }

                // Bins below the partial: i1 (inclusive) up to ipartial (exclusive).
                {
                    let mut i = i1;
                    let mut n_step = 2i64;
                    while i < ipartial {
                        if i >= 0 {
                            let bin = ((i - 1) / 2) as usize;
                            if bin <= n2 {
                                let window = match params.partial_band_window {
                                    PartialBandWindow::Rectangle => None,
                                    PartialBandWindow::Hann => Some(table_lookup(
                                        &half_hann,
                                        1.0 - (n_step as f32 / (ipartial - i1) as f32),
                                    )),
                                    PartialBandWindow::Welch => Some(table_lookup(
                                        &half_welch,
                                        1.0 - (n_step as f32 / (ipartial - i1) as f32),
                                    )),
                                };
                                set_bin(
                                    i,
                                    bin,
                                    &mut f_amp,
                                    &mut f_shift_mult,
                                    &mut time_delay,
                                    &mut decay_time,
                                    &mut partial_data_line,
                                    window,
                                );
                            }
                        }
                        i += 2;
                        n_step += 2;
                    }
                }

                // Bins above the partial: i2 (inclusive) down to ipartial (exclusive).
                {
                    let mut i = i2;
                    let mut n_step = 2i64;
                    while i > ipartial {
                        if i >= 0 {
                            let bin = ((i - 1) / 2) as usize;
                            if bin <= n2 {
                                let window = match params.partial_band_window {
                                    PartialBandWindow::Rectangle => None,
                                    PartialBandWindow::Hann => Some(table_lookup(
                                        &half_hann,
                                        1.0 - (n_step as f32 / (i2 - ipartial) as f32),
                                    )),
                                    PartialBandWindow::Welch => Some(table_lookup(
                                        &half_welch,
                                        1.0 - (n_step as f32 / (i2 - ipartial) as f32),
                                    )),
                                };
                                set_bin(
                                    i,
                                    bin,
                                    &mut f_amp,
                                    &mut f_shift_mult,
                                    &mut time_delay,
                                    &mut decay_time,
                                    &mut partial_data_line,
                                    window,
                                );
                            }
                        }
                        i -= 2;
                        n_step += 2;
                    }
                }
            }

            for bin in 0..=n2 {
                if time_delay[bin] == -1.0 {
                    time_delay[bin] = non_target_delay_t;
                    decay_time[bin] = non_target_decay_t;
                }
            }
        }

        let time_delay_in_frames: Vec<i64> = time_delay
            .iter()
            .map(|&td| (td * frames_per_sec + 0.5).floor() as i64)
            .collect();
        let decay_time_in_frames: Vec<i64> = decay_time
            .iter()
            .map(|&dt| (dt * frames_per_sec + 0.5).floor() as i64)
            .collect();

        // ---- PER-BIN HARMONY FETCH + TARGET/NON-TARGET PROCESSING ----
        let mut harmony = vec![0.0f32; n_plus_2];
        let mut harmony_delayed_inputs = vec![0.0f32; n_plus_2];

        for bin in 0..=n2 {
            let delay_frames = (target_time_interp * time_delay[bin] * frames_per_sec + 0.5) as i64;
            let (amp, freq) = harmony_ring.bin(harmony_now_index, delay_frames, bin);
            harmony[2 * bin] = amp;
            harmony[2 * bin + 1] = freq;
            harmony_delayed_inputs[2 * bin] = amp;
            harmony_delayed_inputs[2 * bin + 1] = freq;

            if f_amp[bin] != -1.0 {
                let data_line = partial_data_line[bin];
                let temp = pmt_tone_values[data_line]
                    * (harmony[2 * bin + 1] * f_shift_mult[bin] + target_harmadd_values[data_line]);
                harmony[2 * bin + 1] +=
                    target_freq_interp_values[data_line] * (temp - harmony[2 * bin + 1]);

                harmony[2 * bin] *= target_gain_tone_values[data_line];
                if harmony[2 * bin] >= db_to_amp.convert(-600.0) {
                    let temp3 = crate::units::amp_to_db(harmony[2 * bin]);
                    let tv3 = harmony[2 * bin] * f_amp[bin];
                    if tv3 >= db_to_amp.convert(-600.0) {
                        let tv0 = crate::units::amp_to_db(tv3);
                        let tv1 = temp3 + target_amp_interp_values[data_line] * (tv0 - temp3);
                        harmony[2 * bin] = db_to_amp.convert(tv1);
                    }
                }
            } else {
                harmony[2 * bin + 1] = non_target_pm * (harmony[2 * bin + 1] + non_target_harmadd);
                harmony[2 * bin] *= non_target_gain;
            }
        }

        spectmagwarp_inharm(&mut harmony, &f_amp_as_pairs(&f_amp), warpshape);
        harmony_smoother.smooth(&mut harmony, iattackc, iattackm, ireleasec, ireleasem);

        // ---- FEEDBACK ACCUMULATOR ----
        for bin in 0..=n2 {
            if decay_time_in_frames[bin] <= 0 || time_delay_in_frames[bin] <= 0 {
                harmony_delayed_inputs[2 * bin] = 0.0;
            } else {
                let ratio = decay_time_in_frames[bin] as f32 / time_delay_in_frames[bin] as f32;
                harmony_delayed_inputs[2 * bin] *= db_to_amp.convert(-60.0 / ratio);
            }
        }
        let decayed_frame = Frame::from_pva_floats(&harmony_delayed_inputs);
        let buffer_decayed = decayed_input_synth.unconvert_only(&decayed_frame);
        let harmony_delay_now = harmony_ring.frame(harmony_now_index, 0).to_vec();
        let now_frame = Frame::from_pva_floats(&harmony_delay_now);
        let mut buffer_sum = delay_now_synth.unconvert_only(&now_frame);
        for (s, dcy) in buffer_sum.iter_mut().zip(&buffer_decayed) {
            *s += dcy;
        }
        let refreshed = delay_feedback_phase.convert(&buffer_sum);
        harmony_ring.frames[harmony_now_index] = refreshed.to_pva_floats();

        // ---- SOURCE PROCESSING ----
        if source_enabled {
            source_smoother.smooth(
                &mut source_channel_out,
                sattackc,
                sattackm,
                sreleasec,
                sreleasem,
            );
            for bin in 0..=n2 {
                source_channel_out[2 * bin] *= source_gain;
                let temp = source_pm * (source_harmadd + source_channel_out[2 * bin + 1]);
                if temp <= 0.0 || temp >= nyquist {
                    source_channel_out[2 * bin] = 0.0;
                } else {
                    source_channel_out[2 * bin + 1] = temp;
                }
            }
        }

        // ---- NYQUIST/DC CLEANUP ----
        for bin in 0..=n2 {
            if harmony[2 * bin + 1] < 0.0 {
                harmony[2 * bin] = 0.0;
                harmony[2 * bin + 1] = 0.0;
            }
            if harmony[2 * bin + 1] >= nyquist {
                harmony[2 * bin] = 0.0;
                harmony[2 * bin + 1] = nyquist;
            }
        }
        cut_dc(&mut harmony, 5.0);

        // ---- RESYNTHESIS (always oscillator bank - see this module's doc comment) ----
        let threshfac = db_to_amp.convert(params.threshold_db);
        let harmony_frame = Frame::from_pva_floats(&harmony);
        let synt = if source_enabled {
            let source_frame = Frame::from_pva_floats(&source_channel_out);
            getthresh(&source_frame.bins[..n2], threshfac)
                .max(getthresh(&harmony_frame.bins[..n2], threshfac))
        } else {
            getthresh(&harmony_frame.bins[..n2], threshfac)
        };

        let mut combined = osc_harmony.synthesize(&harmony_frame, synt);
        if source_enabled {
            let source_frame = Frame::from_pva_floats(&source_channel_out);
            let source_out = osc_source.synthesize(&source_frame, synt);
            for (c, s) in combined.iter_mut().zip(&source_out) {
                *c += s;
            }
        }
        on += i_factor as i64;
        if on + nw as i64 - i_factor as i64 >= 0 {
            output.extend(combined.iter().map(|&s| s * OSCILBANKGAIN));
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

/// `spectmagwarp_inharm` needs the `f`/target-sentinel check in the same
/// interleaved layout as `sp` - `f_amp` here is stored as one float per
/// bin (the sentinel/dB value only; the shift multiplier lives in a
/// separate array), so this adapts it to a throwaway interleaved buffer
/// for that one call rather than changing `f_amp`'s own storage shape.
fn f_amp_as_pairs(f_amp: &[f32]) -> Vec<f32> {
    let mut out = vec![0.0f32; f_amp.len() * 2];
    for (bin, &v) in f_amp.iter().enumerate() {
        out[2 * bin] = v;
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params() -> InharmonatorParams {
        InharmonatorParams {
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            method: ShiftMethod::Multiplier,
            partial_band_window: PartialBandWindow::Welch,
            partial_bandwidth: ControlFn::Const(1.0),
            master_gain_db: ControlFn::Const(0.0),
            target_harmadd: ControlFn::Const(0.0),
            target_ptrans: ControlFn::Const(0.0),
            target_db: ControlFn::Const(0.0),
            target_amp_interp: ControlFn::Const(1.0),
            target_freq_interp: ControlFn::Const(1.0),
            target_time_interp: ControlFn::Const(1.0),
            fundamental_freq_or_oppc: ControlFn::Const(220.0),
            non_target_harmadd: ControlFn::Const(0.0),
            non_target_ptrans: ControlFn::Const(0.0),
            non_target_db: ControlFn::Const(0.0),
            non_target_delay_t: ControlFn::Const(0.0),
            non_target_decay_t: ControlFn::Const(0.0),
            inharm_attack: ControlFn::Const(0.0),
            inharm_release: ControlFn::Const(0.0),
            warpshape: ControlFn::Const(0.0),
            source_harmadd: ControlFn::Const(0.0),
            source_ptrans: ControlFn::Const(0.0),
            source_db: ControlFn::Const(0.0),
            source_release: ControlFn::Const(0.0),
            source_attack: ControlFn::Const(0.0),
            source_delay_t: ControlFn::Const(0.0),
            threshold_db: -96.0,
        }
    }

    fn one_partial() -> Vec<Partial> {
        vec![Partial {
            number: 1.0,
            shift_data: 1.5,
            decibels: 0.0,
            time_delay: 0.0,
            feedback_decay_time: 0.0,
        }]
    }

    #[test]
    fn silence_in_silence_out() {
        let fft = 1024;
        let params = default_params();
        let partials = one_partial();
        let input = vec![0.0f32; 44100 / 4];
        let dur = input.len() as f32 / 44100.0;
        let output = process_channel(&input, &partials, fft, 44100, dur, &params);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-4));
    }

    #[test]
    fn sine_input_produces_bounded_output() {
        let fft = 1024;
        let params = default_params();
        let partials = one_partial();
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let dur = input.len() as f32 / sample_rate as f32;
        let output = process_channel(&input, &partials, fft, sample_rate, dur, &params);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak.is_finite());
        assert!(peak < 10.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn source_mixing_runs_without_panicking() {
        let fft = 1024;
        let mut params = default_params();
        params.source_db = ControlFn::Const(0.0);
        let partials = one_partial();
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let dur = input.len() as f32 / sample_rate as f32;
        let output = process_channel(&input, &partials, fft, sample_rate, dur, &params);
        assert!(!output.is_empty());
        assert!(output.iter().all(|s| s.is_finite()));
    }

    #[test]
    fn nonzero_delay_and_decay_stay_bounded() {
        let fft = 1024;
        let mut params = default_params();
        let mut partials = one_partial();
        partials[0].time_delay = 0.05;
        partials[0].feedback_decay_time = 0.1;
        params.non_target_delay_t = ControlFn::Const(0.02);
        params.non_target_decay_t = ControlFn::Const(0.05);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let dur = input.len() as f32 / sample_rate as f32;
        let output = process_channel(&input, &partials, fft, sample_rate, dur, &params);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak.is_finite());
        assert!(
            peak < 20.0,
            "peak {peak} unexpectedly large - possible feedback blowup"
        );
    }

    #[test]
    fn resolve_partials_applies_scalers_and_clamps_number() {
        let raw = vec![Partial {
            number: 1.0,
            shift_data: 2.0,
            decibels: -10.0,
            time_delay: 1.0,
            feedback_decay_time: 1.0,
        }];
        let modifiers = DataModifiers {
            number_scaler: 2.0,
            number_shifter: -5.0,
            decibel_scaler: 2.0,
            delay_scaler: 0.5,
            delay_shifter: 0.1,
            decay_scaler: 0.5,
            decay_shifter: 0.0,
        };
        let resolved = resolve_partials(&raw, &modifiers);
        // (2*(1-1)+1) - 5 = -4 -> clamped to 1.
        assert_eq!(resolved[0].number, 1.0);
        assert_eq!(resolved[0].decibels, -20.0);
        assert_eq!(resolved[0].time_delay, 0.6);
        assert_eq!(resolved[0].feedback_decay_time, 0.5);
    }
}
