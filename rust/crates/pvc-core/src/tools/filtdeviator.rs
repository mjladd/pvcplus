//! Ports `filtdeviator.c`: `tools::filter`'s own fixed-spectrum,
//! additive source+filter mixing (live FFT of raw audio, a static `.fr`
//! response, real sample-accurate `-b`/`-e` trimming via the same
//! `setupfiles()`/`openfiles()` pair `tools::spectralextractor`/
//! `tools::convolver` use) - plus three genuinely new subsystems
//! `filter.c` doesn't have: a per-bin, response-shaped *time delay* into
//! the filter's own delay line (not a single frame-wide delay like
//! `filter.c`'s), a self-referential *decay/feedback* accumulator that
//! blends each frame's fresh spectrum into that same delay line before
//! the next frame reads it back, and a per-bin *frequency deviation*
//! (the tool's own headline "response-correlated frequency deviation")
//! that reshapes each bin's frequency, not just its amplitude.
//!
//! **Reused directly, confirmed identical by reading both files**:
//! `tools::tvfilter::filter_lookup`'s exact shift-then-transpose-then-
//! interpolate index math (including its "second-to-last pair"/"first
//! pair" edge clamps) - `filtdeviator.c`'s own per-bin index calculation
//! (`legacy/pvc_src/filtdeviator.c:1014-1035`) reduces to the same
//! formula with `n_ratio` fixed at `1.0` (this tool has no independent
//! response FFT size - like `tools::filter`, `-N`'s value is forced to
//! the response file's own inferred size, not exposed as a separately
//! settable flag that would need to match it). Applied four times per
//! bin, once per independently-`spectmagwarp`ed copy of the loaded
//! response (`FampWarp`/`FfreqWarp`/`FtimeDelayWarp` - a fourth,
//! `FdecayTimeWarp`, is dead - see below).
//!
//! **The response's own frequency values are dead**, exactly like
//! `tools::filter`'s own finding: nothing past a one-time "convert to
//! phase differences" step ever reads an odd (frequency) slot of any of
//! the four response copies. This port represents all of them as plain
//! amplitude arrays, skipping that conversion.
//!
//! **A genuinely dead computation, confirmed by grepping the whole
//! file**: `interpDecayTFilterAmp` (interpolated from `FdecayTimeWarp`,
//! itself shaped by `-~`'s `decaytimewarpshape`) is computed fresh every
//! bin and never read again - the decay-time formula that reads
//! naturally like it should use it instead reuses `interpTdelayFilterAmp`
//! (the *time delay* response's own interpolated value). `-~` and
//! `FdecayTimeWarp` are consequently not ported at all; the decay-time
//! formula here uses the time-delay response's interpolation directly,
//! matching what the C's compiled behavior actually is.
//!
//! **Not ported, matching `tools::ring`'s established `randf()`
//! precedent** (each is off/response-mode by default): `-Y` (random time
//! delay - `thisFilterDelayT` always takes the deterministic "RESPONSE
//! ONLY MODE" formula here), `-c`/`-f` (random amplitude deviation - this
//! port always takes the "NON RANDOM MODE" amplitude formula; note the
//! real C has an unrelated real bug here too, a missing `break` after
//! `case 'c'` that falls through into `case 'd'`'s own `crackstring` call
//! using `-c`'s own argument string - not reproduced, since it depends on
//! an imperative flag-processing order this port's declarative CLI
//! parser has no equivalent of), and frequency-deviation "random mode"
//! (`-E`'s `freqdevmode` resolving to the constant `1.0` at parse time -
//! this port's [`FreqDevMode`] only distinguishes response/file modes;
//! see its own doc comment).
//!
//! **A global variable, not `filtdeviator.c`'s own `main()`, is what
//! actually extends the output past the trimmed input's own length.**
//! `ringTime` (`filter_max_delay_t` + `max_decay_time`, further raised to
//! the source's own max delay if source mixing is enabled) is computed
//! in `main()` but never added to `dur` there - reading only
//! `filtdeviator.c` suggests the output simply ends when the (trimmed)
//! input does, with no tail for a long decay/delay to ring out into. It
//! is a *global* (`legacy/pvc_src/globals.h`), though, and
//! `legacy/pvc_lib/fileio.c`'s own `shiftin`-adjacent code reads it back
//! to keep feeding zero-valued ("silent") hops for `ringTime` more
//! seconds after the real input is exhausted, before actually declaring
//! EOF - confirmed by grepping every reference to `ringTime` across
//! `pvc_lib`, not assumed from `main()` alone. Modeled here by literally
//! padding the trimmed input with that many extra zero samples rather
//! than reproducing the global/side-channel mechanism itself.
//!
//! **The self-referential decay/feedback accumulator** blends, every
//! frame, a decayed copy of what the filter's delay line already held
//! (`unconvert2`) with the *current* frame's fresh, undelayed spectrum
//! (`unconvert3`) by summing their rfft-packed spectra directly (no
//! inverse FFT in between) and re-deriving amplitude/frequency from that
//! sum (`convert3`) - four independent phase-tracking states in the C
//! (`unconvert`/`unconvert1`/`unconvert2`/`unconvert3`, each its own
//! hand-duplicated function with its own static phase memory), modeled
//! here the same way `tools::filter`'s own dual source/filter mix already
//! established: one throwaway [`Synthesizer`] (for its `unconvert_only`
//! alone) or bare [`PhaseTracker`] per independent phase memory needed,
//! never derived from a single shared instance.

use crate::eq::eq;
use crate::pvoc::{getthresh, Analyzer, Frame, OscBank, PhaseTracker, Synthesizer};
use crate::smooth::{smooth_setup, Smoother};
use crate::tools::tvfilter::filter_lookup;
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::twarp`'s doc
/// comment on the same constant.
const OSCILBANKGAIN: f32 = 1.7782794;

/// `-E`'s own three-way mode, resolved once at startup from the parsed
/// control function (matching the C's own one-time `ranfreqdevswitch`
/// derivation) - see this module's doc comment on why "random mode"
/// (the C's `ranfreqdevswitch == 1`) has no variant here.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FreqDevMode {
    /// `FILTER_OUTPUT_freqdevmode` is `Const(0.0)` - the C's default.
    Response,
    /// `FILTER_OUTPUT_freqdevmode` is a `Table` (`.n != 1` in the C).
    File,
}

impl FreqDevMode {
    /// Panics on anything else (the C's own "ILLEGAL FREQUENCY DEVIATION
    /// MODE" `exit(EXIT_FAILURE)`, which also covers the unsupported
    /// `Const(1.0)` random-mode case here).
    pub fn resolve(freq_dev_mode: &ControlFn) -> FreqDevMode {
        match freq_dev_mode {
            ControlFn::Table(_) => FreqDevMode::File,
            ControlFn::Const(v) if *v == 0.0 => FreqDevMode::Response,
            other => panic!(
                "filtdeviator: illegal frequency deviation mode {other:?} (only response mode \
                 [0] and file mode [a table] are supported - random mode [1] is not ported, see \
                 pvc-core::tools::filtdeviator's doc comment)"
            ),
        }
    }
}

pub struct FiltdeviatorParams {
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,

    /// `-h`. `> -96.0` or time-varying enables source mixing entirely -
    /// see [`source_enabled`].
    pub source_gain_db: ControlFn,
    /// `-r`.
    pub source_freq_shift: ControlFn,
    /// `-s`.
    pub source_delay_secs: ControlFn,
    /// `-y`.
    pub source_pitch_transpose: ControlFn,

    /// `-A`.
    pub filter_gain_db: ControlFn,
    /// `-P`.
    pub filter_pitch_transpose: ControlFn,
    /// `-a`.
    pub filter_freq_shift: ControlFn,

    /// `-T`.
    pub response_pitch_transpose: ControlFn,
    /// `-V`.
    pub response_freq_shift: ControlFn,
    /// `-S`.
    pub source_floor_db: ControlFn,

    /// `-l`.
    pub attack_secs: ControlFn,
    /// `-L`.
    pub release_secs: ControlFn,

    /// `-W`.
    pub amp_warpshape: ControlFn,
    /// `-v`.
    pub freq_warpshape: ControlFn,
    /// `-o`.
    pub time_delay_warpshape: ControlFn,

    /// `-G`.
    pub band_reject: bool,

    pub shelf_low_db: f32,
    pub shelf_high_db: f32,
    pub shelf_low_freq: f32,
    pub shelf_high_freq: f32,

    /// `-j`.
    pub time_delay_base: ControlFn,
    /// `-J`.
    pub time_delay_peak: ControlFn,
    /// `-q`.
    pub time_delay_dev_control: ControlFn,
    /// `-@`.
    pub delay_time_scaler: ControlFn,

    /// `-/`.
    pub peak_decay_time_secs: ControlFn,
    /// `-:`.
    pub base_decay_time_secs: ControlFn,
    /// `-B`.
    pub decay_time_dev_control: ControlFn,

    /// `-k`.
    pub freq_dev_base: ControlFn,
    /// `-K`.
    pub freq_dev_peak: ControlFn,
    /// `-u`.
    pub freq_shift_dev_base: ControlFn,
    /// `-U`.
    pub freq_shift_dev_peak: ControlFn,
    /// `-O`.
    pub freq_dev_control: ControlFn,
    /// `-E`. See [`FreqDevMode`].
    pub freq_dev_mode: ControlFn,

    /// `-n`.
    pub frame_norm_limit_db: ControlFn,
    /// `-x`.
    pub normalize_to_filter: bool,

    pub threshold_db: f32,
}

fn control_fn_max(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(vals) => vals.iter().copied().fold(f32::MIN, f32::max),
    }
}

fn is_const_zero(cf: &ControlFn) -> bool {
    matches!(cf, ControlFn::Const(v) if *v == 0.0)
}

/// `SOURCE_dB.A[0] > -96.0 || SOURCE_dB.n != 1.` - resolved once, from the
/// same parsed control function the CLI layer builds `source_gain_db`
/// from.
pub fn source_enabled(source_gain_db: &ControlFn) -> bool {
    match source_gain_db {
        ControlFn::Table(_) => true,
        ControlFn::Const(v) => *v > -96.0,
    }
}

/// A ring of the last `capacity` analysis frames (flat `n_plus_2`-float
/// layout), indexed by `frame_count % capacity` - ports the shared shape
/// of `SOURCE_channel_delay`/`FILTER_channel_delay`. Unlike
/// `tools::filter::DelayLine` (one delay value per whole frame), `bin_at`
/// resolves a *per-bin* delay, needed for `FILTER_channel_delay`'s own
/// per-bin time delay.
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

/// Resynthesizes one channel. `response_amps` is the loaded `.fr` file's
/// amplitudes (`pvc_io::read_fr_amplitudes`'s output; `analysis_n` is its
/// inferred FFT size, forced as this tool's own - see this module's doc
/// comment). `input` is this channel's raw samples, already trimmed to
/// `-b`/`-e` by the caller; `dur` is that trimmed span's own duration
/// (seconds) times `-I`'s time factor, matching `tools::convolver`'s own
/// convention for the same real trimming.
#[allow(clippy::too_many_arguments)]
pub fn process_channel(
    input: &[f32],
    response_amps: &[f32],
    analysis_n: usize,
    sample_rate: u32,
    dur: f32,
    params: &FiltdeviatorParams,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = analysis_n;
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

    let source_enabled = source_enabled(&params.source_gain_db);
    let freq_dev_mode = FreqDevMode::resolve(&params.freq_dev_mode);

    // ---- RING CAPACITY ----
    let filter_max_delay_t = control_fn_max(&params.time_delay_base)
        .max(control_fn_max(&params.time_delay_peak))
        .max(0.0);
    let max_decay_time = control_fn_max(&params.peak_decay_time_secs)
        .max(control_fn_max(&params.base_decay_time_secs))
        .max(0.0);
    let filter_max_delay_t = filter_max_delay_t + max_decay_time;
    let filter_max_num_delay_frames = 1 + (filter_max_delay_t * frames_per_sec + 0.5) as usize;

    let source_max_delay_t = if source_enabled {
        control_fn_max(&params.source_delay_secs).max(0.0)
    } else {
        0.0
    };
    let source_max_num_delay_frames = 1 + (source_max_delay_t * frames_per_sec + 0.5) as usize;

    // ---- RING-TIME INPUT PADDING ----
    // `ringTime` (a *global* in the C, set here and read back by
    // `fileio.c`'s own `shiftin`/`setupfiles` machinery once the real
    // input is exhausted) extends how long the input keeps reading as
    // silence past its own trimmed end - not by extending `dur` (the
    // control-function time base, computed from the untouched trimmed
    // span alone), but by continuing to feed zero-valued hops into the
    // analysis loop for `ringTime` more seconds, so a bin's own time
    // delay/decay can actually ring out into real output samples instead
    // of the file just ending mid-delay. Modeled here by literally
    // padding the (already `-b`/`-e`-trimmed) input with that many zero
    // samples, rather than modeling `fileio.c`'s own global/side-channel
    // mechanism - equivalent, since nothing else reads `ringTime`.
    let ring_time = filter_max_delay_t.max(source_max_delay_t);
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

    // ---- ONE-TIME RESPONSE LOAD ----
    let f_base = response_amps.to_vec();
    let filter_channel_amp_sum: f32 = f_base.iter().sum();

    // ---- STARTUP OBANK/OVERLAP-ADD SELECTION ----
    let obank = !(is_const_zero(&params.filter_pitch_transpose)
        && is_const_zero(&params.filter_freq_shift)
        && is_const_zero(&params.freq_dev_base)
        && is_const_zero(&params.freq_dev_peak)
        && is_const_zero(&params.freq_shift_dev_base)
        && is_const_zero(&params.freq_shift_dev_peak)
        && is_const_zero(&params.source_freq_shift)
        && is_const_zero(&params.source_pitch_transpose));

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);

    // Two independent phase-tracking `Synthesizer`s for the main
    // resynthesis mix (`unconvert1`/bare `unconvert`), only one of which
    // ever calls `finish`/holds the shared ring+window state.
    let mut synth = Synthesizer::new(n, window_pair.synthesis.clone(), i_factor, d, sample_rate);
    let mut source_synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, sample_rate);

    // Two independent oscillator banks sharing one cosine table (built
    // from the first bank's own `n2`/`nw`), matching `noscbank2`'s own
    // shared-table quirk (see `OscBank::with_shared_table`'s doc comment).
    let mut osc_filter = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let osc_table = osc_filter.table();
    let mut osc_source = OscBank::with_shared_table(n2, sample_rate, i_factor, 1.0, osc_table);

    // Independent phase memory for the decay/feedback accumulator's own
    // three legacy call sites (`unconvert2`, `unconvert3`, `convert3`) -
    // see this module's doc comment.
    let mut decayed_input_synth = Synthesizer::new(n, vec![0.0; nw], i_factor, d, sample_rate);
    let mut delay_now_synth = Synthesizer::new(n, vec![0.0; nw], i_factor, d, sample_rate);
    let mut delay_feedback_phase = PhaseTracker::new_analysis(n2, d, sample_rate);

    let mut source_ring = DelayRing::new(source_max_num_delay_frames, n_plus_2);
    let mut filter_ring = DelayRing::new(filter_max_num_delay_frames, n_plus_2);

    let mut previous_channel_filter = vec![0.0f32; n_plus_2];
    let mut smoother = Smoother::new(n_plus_2);

    let mut this_filter_delay_t = vec![0.0f32; n2 + 1];
    let mut this_filter_delay_t_in_frames = vec![0i64; n2 + 1];
    let mut this_filter_decay_t_in_frames = vec![0i64; n2 + 1];

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

        let source_now_index = source_ring.push(frame_count, channel_flat.clone());
        let filter_now_index = filter_ring.push(frame_count, channel_flat.clone());

        // ---- PER-FRAME CONTROL VALUES ----
        let attack_val = params.attack_secs.at(t, dur);
        let release_val = params.release_secs.at(t, dur);
        let (attackc, minus_attackc) = smooth_setup(attack_val, ir);
        let (releasec, minus_releasec) = smooth_setup(release_val, ir);

        let time_delay_base = params.time_delay_base.at(t, dur);
        let time_delay_peak = params.time_delay_peak.at(t, dur);
        let time_range = time_delay_peak - time_delay_base;
        let time_delay_dev_control = params.time_delay_dev_control.at(t, dur);

        let frame_norm_limit_db = params.frame_norm_limit_db.at(t, dur);
        let frame_norm_amp_limit = db_to_amp.convert(frame_norm_limit_db);

        let fs = params.response_freq_shift.at(t, dur);
        let fm = semitones_to_mult.convert(params.response_pitch_transpose.at(t, dur));

        let amp_warpshape = params.amp_warpshape.at(t, dur);
        let freq_warpshape = params.freq_warpshape.at(t, dur);
        let time_delay_warpshape = params.time_delay_warpshape.at(t, dur);

        // `eq()`/`spectmagwarp()` need the interleaved amp/freq layout;
        // adapt via a throwaway buffer (frequency slots dead, see this
        // module's doc comment) then copy the shaped amplitudes back out.
        let mut f_amp = f_base.clone();
        {
            let mut interleaved = interleave(&f_amp);
            eq(
                &mut interleaved,
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
            deinterleave_amps(&interleaved, &mut f_amp);
        }
        if params.band_reject {
            crate::filter_response::invert_response(&mut f_amp, false, &db_to_amp);
        }
        {
            let mut interleaved = interleave(&f_amp);
            spectmagwarp(&mut interleaved, amp_warpshape, true);
            deinterleave_amps(&interleaved, &mut f_amp);
        }

        let mut f_freq = f_base.clone();
        {
            let mut interleaved = interleave(&f_freq);
            spectmagwarp(&mut interleaved, freq_warpshape, true);
            deinterleave_amps(&interleaved, &mut f_freq);
        }

        let mut f_time_delay = f_base.clone();
        {
            let mut interleaved = interleave(&f_time_delay);
            spectmagwarp(&mut interleaved, time_delay_warpshape, true);
            deinterleave_amps(&interleaved, &mut f_time_delay);
        }

        // ---- FIRST PER-BIN PASS: delay fetch, decay setup, amp/freq deviation ----
        let mut channel_filter = vec![0.0f32; n_plus_2];
        let mut channel_filter_delayed_inputs = vec![0.0f32; n_plus_2];
        let mut channel_out_amp_sum = 0.0f32;

        for j in 0..=n2 {
            let interp_tdelay_amp = filter_lookup(&f_time_delay, j, 1.0, fundamental, fs, fm);

            let delay_t =
                time_delay_dev_control * (time_delay_base + interp_tdelay_amp * time_range);
            this_filter_delay_t[j] = delay_t;
            let delay_frames = (delay_t * frames_per_sec + 0.5).floor() as i64;
            this_filter_delay_t_in_frames[j] = delay_frames;

            let (amp, freq) = filter_ring.bin(filter_now_index, delay_frames, j);
            channel_filter[2 * j] = amp;
            channel_filter[2 * j + 1] = freq;
            channel_filter_delayed_inputs[2 * j] = amp;
            channel_filter_delayed_inputs[2 * j + 1] = freq;

            if frame_count == 0 {
                previous_channel_filter[2 * j] = channel_flat[2 * j];
                previous_channel_filter[2 * j + 1] = channel_flat[2 * j + 1];
            }

            channel_out_amp_sum += channel_filter[2 * j];

            let shifted_t = t - params.delay_time_scaler.at(t, dur) * delay_t;

            let decay_time_dev_control = params.decay_time_dev_control.at(shifted_t, dur);
            let base_decay = params.base_decay_time_secs.at(shifted_t, dur);
            let peak_decay = params.peak_decay_time_secs.at(shifted_t, dur);
            let feedback_decay_range = peak_decay - base_decay;
            // Reuses `interp_tdelay_amp`, not a `FdecayTimeWarp` lookup -
            // see this module's doc comment on the dead `-~` flag.
            let decay_t =
                decay_time_dev_control * (base_decay + interp_tdelay_amp * feedback_decay_range);
            this_filter_decay_t_in_frames[j] = (decay_t * frames_per_sec + 0.5).floor() as i64;

            let this_interp_filter_amp = filter_lookup(&f_amp, j, 1.0, fundamental, fs, fm);
            let source_floor_db = params.source_floor_db.at(shifted_t, dur);
            let sourceamp = db_to_amp.convert(source_floor_db);
            let filtamp = 1.0 - sourceamp;
            channel_filter[2 * j] *= filtamp * this_interp_filter_amp + sourceamp;

            let this_interp_filter_freq = filter_lookup(&f_freq, j, 1.0, fundamental, fs, fm);
            let freq_dev_control = params.freq_dev_control.at(shifted_t, dur);
            let fdev_minus = 1.0 - freq_dev_control;
            let fdev_base = params.freq_dev_base.at(shifted_t, dur);
            let fdev_peak = params.freq_dev_peak.at(shifted_t, dur);
            let fdm_diff = fdev_peak - fdev_base;
            let fshift_base = params.freq_shift_dev_base.at(shifted_t, dur);
            let fshift_peak = params.freq_shift_dev_peak.at(shifted_t, dur);
            let fs_diff = fshift_peak - fshift_base;

            match freq_dev_mode {
                FreqDevMode::File => {
                    let freq_dev_mode_val = params.freq_dev_mode.at(shifted_t, dur);
                    let fully_deviated_freq = semitones_to_mult
                        .convert(fdev_base + freq_dev_mode_val * fdm_diff)
                        * (channel_filter[2 * j + 1] + (fshift_base + freq_dev_mode_val * fs_diff));
                    channel_filter[2 * j + 1] += this_interp_filter_freq
                        * freq_dev_control
                        * (fully_deviated_freq - channel_filter[2 * j + 1]);
                }
                FreqDevMode::Response => {
                    let amp_factor = fdev_minus
                        + freq_dev_control
                            * semitones_to_mult
                                .convert(fdev_base + this_interp_filter_freq * fdm_diff);
                    channel_filter[2 * j + 1] = amp_factor
                        * (channel_filter[2 * j + 1]
                            + freq_dev_control * (fshift_base + this_interp_filter_freq * fs_diff));
                }
            }
        }

        // ---- FRAME NORMALIZATION ----
        let temp_channel_amp_sum: f32 = channel_filter.iter().step_by(2).sum();
        if temp_channel_amp_sum > 0.0 && frame_norm_amp_limit != 1.0 {
            let target = if params.normalize_to_filter {
                filter_channel_amp_sum
            } else {
                channel_out_amp_sum
            };
            let mut normalization_amp = target / temp_channel_amp_sum;
            if normalization_amp > frame_norm_amp_limit {
                normalization_amp = frame_norm_amp_limit;
            }
            for a in channel_filter.iter_mut().step_by(2) {
                *a *= normalization_amp;
            }
        }

        // ---- SMOOTH ----
        smoother.smooth(
            &mut channel_filter,
            attackc,
            minus_attackc,
            releasec,
            minus_releasec,
        );

        // ---- SECOND PER-BIN PASS: shift/transpose/gain ----
        for j in 0..=n2 {
            let shifted_t = t - params.delay_time_scaler.at(t, dur) * this_filter_delay_t[j];
            let harmadd = params.filter_freq_shift.at(shifted_t, dur);
            let pm = semitones_to_mult.convert(params.filter_pitch_transpose.at(shifted_t, dur));
            let temp = pm * (channel_filter[2 * j + 1] + harmadd);
            if temp <= 0.0 || temp >= nyquist {
                channel_filter[2 * j] = 0.0;
            } else {
                channel_filter[2 * j + 1] = temp;
            }
            let gain = db_to_amp.convert(params.filter_gain_db.at(shifted_t, dur));
            channel_filter[2 * j] *= gain;
        }

        // ---- DECAY/FEEDBACK ACCUMULATOR ----
        for j in 0..=n2 {
            if this_filter_decay_t_in_frames[j] <= 0 || this_filter_delay_t_in_frames[j] <= 0 {
                channel_filter_delayed_inputs[2 * j] = 0.0;
            } else {
                let ratio = this_filter_decay_t_in_frames[j] as f32
                    / this_filter_delay_t_in_frames[j] as f32;
                channel_filter_delayed_inputs[2 * j] *= db_to_amp.convert(-60.0 / ratio);
            }
        }
        let decayed_frame = Frame::from_pva_floats(&channel_filter_delayed_inputs);
        let buffer_decayed = decayed_input_synth.unconvert_only(&decayed_frame);
        let filter_channel_delay_now = filter_ring.frame(filter_now_index, 0).to_vec();
        let now_frame = Frame::from_pva_floats(&filter_channel_delay_now);
        let mut buffer_sum = delay_now_synth.unconvert_only(&now_frame);
        for (s, d) in buffer_sum.iter_mut().zip(&buffer_decayed) {
            *s += d;
        }
        let refreshed = delay_feedback_phase.convert(&buffer_sum);
        filter_ring.frames[filter_now_index] = refreshed.to_pva_floats();

        // ---- SOURCE ----
        let source_frame = if source_enabled {
            let source_delay_secs = params.source_delay_secs.at(t, dur);
            let delay_frames = (source_delay_secs * frames_per_sec + 0.5) as i64;
            let mut flat = source_ring.frame(source_now_index, delay_frames).to_vec();

            let shifted_t = t - source_delay_secs;
            let source_gain = db_to_amp.convert(params.source_gain_db.at(shifted_t, dur));
            let source_pm =
                semitones_to_mult.convert(params.source_pitch_transpose.at(shifted_t, dur));
            let source_fshift = params.source_freq_shift.at(shifted_t, dur);
            for j in 0..=n2 {
                flat[2 * j] *= source_gain;
                flat[2 * j + 1] = flat[2 * j + 1] * source_pm + source_fshift;
                if flat[2 * j + 1] < 0.0 {
                    flat[2 * j + 1] = 0.0;
                    flat[2 * j] = 0.0;
                }
            }
            Some(Frame::from_pva_floats(&flat))
        } else {
            None
        };

        let filter_frame = Frame::from_pva_floats(&channel_filter);
        let threshfac = db_to_amp.convert(params.threshold_db);
        let synt = if let Some(ref sf) = source_frame {
            getthresh(&channel_frame.bins[..n2], threshfac)
                .max(getthresh(&filter_frame.bins[..n2], threshfac))
                .max(getthresh(&sf.bins[..n2], threshfac))
        } else {
            getthresh(&filter_frame.bins[..n2], threshfac)
        };

        if obank {
            let filter_out = osc_filter.synthesize(&filter_frame, synt);
            let combined = if let Some(ref sf) = source_frame {
                let mut combined = filter_out;
                let source_out = osc_source.synthesize(sf, synt);
                for (c, s) in combined.iter_mut().zip(&source_out) {
                    *c += s;
                }
                combined
            } else {
                filter_out
            };
            on += i_factor as i64;
            if on + nw as i64 - i_factor as i64 >= 0 {
                output.extend(combined.iter().map(|&s| s * OSCILBANKGAIN));
                samps_written += i_factor;
            }
        } else {
            let mut combined = synth.unconvert_only(&filter_frame);
            if let Some(ref sf) = source_frame {
                let source_buf = source_synth.unconvert_only(sf);
                for (c, s) in combined.iter_mut().zip(&source_buf) {
                    *c += s;
                }
            }
            let hop_out = synth.finish(&mut combined);
            if !hop_out.is_empty() {
                output.extend(hop_out);
                samps_written += i_factor;
            }
        }

        frame_count += 1;
        if eof_after_this_hop {
            break;
        }
    }

    if obank {
        output.extend(vec![0.0f32; i_factor]);
    } else {
        output.extend(synth.flush());
    }

    output
}

fn interleave(amps: &[f32]) -> Vec<f32> {
    let mut out = vec![0.0f32; amps.len() * 2];
    for (j, &a) in amps.iter().enumerate() {
        out[2 * j] = a;
    }
    out
}

fn deinterleave_amps(interleaved: &[f32], amps: &mut [f32]) {
    for (j, a) in amps.iter_mut().enumerate() {
        *a = interleaved[2 * j];
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params() -> FiltdeviatorParams {
        FiltdeviatorParams {
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            source_gain_db: ControlFn::Const(0.0),
            source_freq_shift: ControlFn::Const(0.0),
            source_delay_secs: ControlFn::Const(0.0),
            source_pitch_transpose: ControlFn::Const(0.0),
            filter_gain_db: ControlFn::Const(0.0),
            filter_pitch_transpose: ControlFn::Const(0.0),
            filter_freq_shift: ControlFn::Const(0.0),
            response_pitch_transpose: ControlFn::Const(0.0),
            response_freq_shift: ControlFn::Const(0.0),
            source_floor_db: ControlFn::Const(-96.0),
            attack_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
            amp_warpshape: ControlFn::Const(0.0),
            freq_warpshape: ControlFn::Const(0.0),
            time_delay_warpshape: ControlFn::Const(0.0),
            band_reject: false,
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            time_delay_base: ControlFn::Const(0.0),
            time_delay_peak: ControlFn::Const(0.0),
            time_delay_dev_control: ControlFn::Const(1.0),
            delay_time_scaler: ControlFn::Const(0.0),
            peak_decay_time_secs: ControlFn::Const(0.0),
            base_decay_time_secs: ControlFn::Const(0.0),
            decay_time_dev_control: ControlFn::Const(1.0),
            freq_dev_base: ControlFn::Const(0.0),
            freq_dev_peak: ControlFn::Const(0.0),
            freq_shift_dev_base: ControlFn::Const(0.0),
            freq_shift_dev_peak: ControlFn::Const(0.0),
            freq_dev_control: ControlFn::Const(1.0),
            freq_dev_mode: ControlFn::Const(0.0),
            frame_norm_limit_db: ControlFn::Const(0.0),
            normalize_to_filter: false,
            threshold_db: -96.0,
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let fft = 1024;
        let response = vec![1.0f32; fft / 2 + 1];
        let params = default_params();
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(&input, &response, fft, 44100, 0.25, &params);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn flat_response_passes_sine_input_through_at_bounded_amplitude() {
        let fft = 1024;
        let response = vec![1.0f32; fft / 2 + 1];
        let params = default_params();
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let dur = input.len() as f32 / sample_rate as f32;
        let output = process_channel(&input, &response, fft, sample_rate, dur, &params);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.05, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn nonzero_time_delay_and_decay_run_without_panicking_and_stay_bounded() {
        let fft = 1024;
        let response = vec![1.0f32; fft / 2 + 1];
        let mut params = default_params();
        params.time_delay_base = ControlFn::Const(0.05);
        params.time_delay_peak = ControlFn::Const(0.05);
        params.base_decay_time_secs = ControlFn::Const(0.1);
        params.peak_decay_time_secs = ControlFn::Const(0.1);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let dur = input.len() as f32 / sample_rate as f32;
        let output = process_channel(&input, &response, fft, sample_rate, dur, &params);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak.is_finite());
        assert!(
            peak < 10.0,
            "peak {peak} unexpectedly large - possible feedback blowup"
        );
    }

    #[test]
    fn oscbank_path_selected_when_pitch_shifted() {
        let fft = 1024;
        let response = vec![1.0f32; fft / 2 + 1];
        let mut params = default_params();
        params.filter_pitch_transpose = ControlFn::Const(7.0);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let dur = input.len() as f32 / sample_rate as f32;
        let output = process_channel(&input, &response, fft, sample_rate, dur, &params);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.01, "peak {peak} too quiet");
        assert!(peak < 4.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn file_mode_freq_deviation_runs_without_panicking() {
        let fft = 1024;
        let response = vec![1.0f32; fft / 2 + 1];
        let mut params = default_params();
        params.freq_dev_mode = ControlFn::Table(vec![0.0, 1.0, 0.5]);
        params.freq_dev_peak = ControlFn::Const(2.0);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let dur = input.len() as f32 / sample_rate as f32;
        let output = process_channel(&input, &response, fft, sample_rate, dur, &params);
        assert!(!output.is_empty());
        assert!(output.iter().all(|s| s.is_finite()));
    }

    #[test]
    #[should_panic(expected = "illegal frequency deviation mode")]
    fn random_freq_dev_mode_is_rejected() {
        FreqDevMode::resolve(&ControlFn::Const(1.0));
    }
}
