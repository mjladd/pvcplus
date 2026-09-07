//! Ports `ring.c`: "phase vocoder reverberator/resonator" - a feedback
//! delay network built out of phase-vocoder frames rather than time-domain
//! samples. Each analysis frame's spectrum is copied into a "feedback"
//! path that gets input-EQ'd, gated by a per-bin attack/release envelope
//! follower, mixed with a decayed copy of *last* frame's own feedback
//! spectrum (`feedlevel * next_buffer` - the actual recirculating delay
//! line), EQ'd again "in the loop" (shaping future iterations, not this
//! one - see below), then resynthesized via the oscillator bank alongside
//! an independently pitch/frequency-shiftable copy of the plain source
//! spectrum. `ring.c` always resynthesizes via the oscillator bank -
//! confirmed by reading the whole file: the block that would compute
//! `obank` from whether any pitch/frequency-shift/dither flag is active
//! is entirely commented out, replaced by a bare `P = 1.; obank = 1;` -
//! so the overlap-add branch (`leanunconvert` + `rfft` INVERSE +
//! `overlapadd`) is dead code, not ported here.
//!
//! **Real, confirmed dead flags** (accepted by the C's `crack()` flag
//! list with no `case` in the `switch` beneath it, so they silently do
//! nothing): `-R`, `-a`, `-B`, `-h`. None are documented in `usage()`
//! either - purely leftover accepted characters, not exposed here.
//!
//! **Real, confirmed default-value bug, reproduced faithfully - and
//! present independently in all three of `ring`'s shelf-EQ stages**: each
//! stage's low-shelf gain and frequency defaults are swapped in the C's
//! own initializers - e.g. `INPUT_dBlow.A[0] = 200.` (a comment right
//! above it says "INPUT EQ LOW SHELF: DB") and `INPUT_freqlow.A[0] = 0.`
//! (commented "INPUT EQ LOW SHELF: FREQ"), contradicting `usage()`'s own
//! documented defaults (`-O` "\[0.\]", `-d` "\[200.\]"). The *identical*
//! copy-paste mistake recurs verbatim for the in-loop feedback EQ
//! (`FEEDBACK_dBlow.A[0] = 200.`/`FEEDBACK_freqlow.A[0] = 0.`, vs. `-X`
//! "\[0.\]"/`-U` "\[200.\]") and the output EQ (`OUTPUT_dBlow.A[0] =
//! 200.`/`OUTPUT_freqlow.A[0] = 0.`, vs. `-k` "\[0.\]"/`-s` "\[200.\]") -
//! three separately-declared but structurally identical bugs, not one
//! bug that happens to be read three times. Since `eq()`'s "flat gain"
//! fast path only triggers when `dBhi == dBlow` exactly, each of these
//! default combinations (`dBlow=200, dBhi=0`) takes the *shelf* branch on
//! every frame with no flags passed at all - confirmed empirically, not
//! just by reading: the output EQ alone measured an 8x (~+18dB) too-loud
//! oscillator-bank peak against a real oracle run before `-k0` (or the
//! equivalent `RingParams::output_eq_low_db = 0.0`) was found to explain
//! it. [`RingParams::input_eq_low_db`]/[`RingParams::loop_eq_low_db`]/
//! [`RingParams::output_eq_low_db`] (and their paired `*_low_freq`
//! fields) default to `200.0`/`0.0` at the CLI layer to match - not
//! `0.0`/`200.0`, which is what a reading of `usage()` alone would
//! suggest.
//!
//! **Not reproduced** (a narrow, hard-to-trigger cross-frame state
//! artifact, simplified rather than chased): the C's `prebalancesum`
//! (used by the feedback-loop EQ's decaying-signal balance limiter) is
//! declared once in `main()`, outside even the channel loop, and is only
//! *written* inside `if (FEEDBACK_dBhitemp != 0.) && balanceflag`, but
//! *read* under the broader `FEEDBACK_dBhitemp != 0. || FEEDBACK_dBlowtemp
//! != 0.` - so a frame with a nonzero low-shelf decay but a zero
//! high-shelf one reads whatever `prebalancesum` was left over from a
//! previous frame (or channel) rather than a fresh sum of its own bins.
//! This port recomputes `prebalancesum` fresh every frame (`0.0` when the
//! narrower gate is false) instead of replicating that stale-read
//! artifact, which only differs from the C in that specific, rare flag
//! combination (`-X` nonzero with `-Q` left at its `0.0` default).
//!
//! **Not reproduced** (bit-exact glibc `random()` is a disproportionate
//! side quest for a family of flags off by default): `-j`/`-K` (random
//! per-bin frequency deviation) and `-x`/`-q` (random per-bin amplitude
//! deviation), both smoothed through a one-pole lowpass. Unlike `gen6`
//! (`pvc fn gen6`, which already replicates glibc's simpler `rand()`),
//! these call `randf()` -> glibc's `random()`, a much more involved
//! nonlinear additive-feedback generator; deferred rather than ported,
//! matching this project's "port what's exercised/on by default, document
//! the rest" precedent.
//!
//! Not ported, matching this project's established precedent for the same
//! flags across every other multi-channel tool: `-C` (process one
//! channel, optionally shifted - always processes every channel present).
//! Also not ported: `-w`/`-i`/`-_`/`-=` (amplitude-report printing,
//! auto-play, and the peak-rescale-level override) - every resynthesis
//! tool's CLI layer in this project skips the print/play flags entirely
//! and always rescales the whole output to match the input's peak
//! amplitude, ignoring `-=`'s own code (see `commands::pv::run`'s doc
//! comment on `rescaleThisBuffer`).
//!
//! **Worth noting, not a bug**: the "REVERB (feedback) EQ" flags
//! (`-T`/`-X`/`-Q`/`-U`/`-m`, `usage()`'s own `y(n-1)` heading) shape only
//! the *delayed* tail fed into *future* frames (`next_buffer`, built from
//! the post-EQ/-balance `feedback_channel` after the phase-advance step) -
//! the *current* frame's audible resynthesis input (`feedback_buffer`,
//! obank-converted a few lines later) is read from *before* that EQ/
//! balance step ever runs. Confirmed by reading the exact read/write
//! order in `ring.c`, not assumed from the flag's own "in-loop" framing.

use crate::eq::eq;
use crate::fft::rfft;
use crate::pvoc::{fold, getthresh, OscBank, PhaseTracker};
use crate::smooth::smooth_setup;
use crate::units::{db_to_amp_exact, DbToAmp, SemitonesToMult};
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant. `ring` always resynthesizes via the
/// oscillator bank, so this always applies.
const OSCILBANKGAIN: f32 = 1.7782794;

pub struct RingParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,
    /// `-t`: oscillator-bank resynthesis threshold, in dB (a plain
    /// constant in the C, not a `(func)`).
    pub oscbank_threshold_db: f32,
    /// `-A`.
    pub master_gain_db: ControlFn,
    /// `-S`.
    pub source_gain_db: ControlFn,
    /// `-f`.
    pub source_freq_shift_hz: ControlFn,
    /// `-p`.
    pub source_pitch_semitones: ControlFn,
    /// `-F`.
    pub feedback_gain_db: ControlFn,
    /// `-H`.
    pub feedback_freq_shift_hz: ControlFn,
    /// `-P`.
    pub feedback_pitch_semitones: ControlFn,
    /// `-Z`: the feedback delay line's decay time, in seconds (resolved
    /// each frame via `smooth_setup` into a per-hop multiplier - `0.0`
    /// disables the loop entirely).
    pub feedback_decay_secs: ControlFn,
    /// `-z`: the envelope follower's gate threshold, in dB.
    pub feedback_threshold_db: ControlFn,
    /// `-V`: `true` (the default, matching `[1]`) passes/holds bins
    /// *above* threshold and releases those below it; `false` inverts
    /// that.
    pub feedback_threshold_pass_above: bool,
    /// `-l`.
    pub envelope_attack_secs: ControlFn,
    /// `-L`.
    pub envelope_release_secs: ControlFn,
    /// `-O`. Defaults to `200.0`, not `0.0` - see this module's doc
    /// comment on the real swapped-default bug this reproduces.
    pub input_eq_low_db: ControlFn,
    /// `-Y`.
    pub input_eq_high_db: ControlFn,
    /// `-d`. Defaults to `0.0`, not `200.0` - see this module's doc
    /// comment.
    pub input_eq_low_freq: ControlFn,
    /// `-n`.
    pub input_eq_high_freq: ControlFn,
    /// `-T`: the in-loop feedback EQ's own decay time, in seconds - the
    /// loop EQ's dB values are scaled by `(dB * hop_secs) / this` before
    /// being applied, so a shorter decay time makes the same dB setting
    /// bite harder per hop.
    pub loop_eq_decay_secs: ControlFn,
    /// `-E`: `0` disables the decaying-signal balance limiter entirely
    /// (matching the C's own `balancelimitdB <= 0.` gate); a plain
    /// constant, not a `(func)`.
    pub loop_balance_limit_db: f32,
    /// `-X`. Defaults to `200.0`, not `0.0` at the CLI layer - see this
    /// module's doc comment on the real swapped-default bug this
    /// reproduces (independently present here too, not just on `-O`).
    pub loop_eq_low_db: ControlFn,
    /// `-Q`.
    pub loop_eq_high_db: ControlFn,
    /// `-U`. Defaults to `0.0`, not `200.0` at the CLI layer - see this
    /// module's doc comment.
    pub loop_eq_low_freq: ControlFn,
    /// `-m`.
    pub loop_eq_high_freq: ControlFn,
    /// `-k`. Defaults to `200.0`, not `0.0` at the CLI layer - see this
    /// module's doc comment on the real swapped-default bug this
    /// reproduces (independently present here too, not just on `-O`).
    pub output_eq_low_db: ControlFn,
    /// `-c`.
    pub output_eq_high_db: ControlFn,
    /// `-s`. Defaults to `0.0`, not `200.0` at the CLI layer - see this
    /// module's doc comment.
    pub output_eq_low_freq: ControlFn,
    /// `-G`.
    pub output_eq_high_freq: ControlFn,
}

/// Ports `leanconvert()`/`leanconvert2()`/`leanconvert3()`: a stateless
/// rectangular-to-polar conversion (magnitude, *raw* phase in radians -
/// no phase unwrapping, no frequency estimate, no memory across calls)
/// unlike [`PhaseTracker::convert`]'s phase-vocoder frequency tracking.
fn lean_convert(s: &[f32], n2: usize) -> Vec<(f32, f32)> {
    (0..=n2)
        .map(|i| {
            let real = i << 1;
            let imag = real + 1;
            let a = if i == n2 { s[1] } else { s[real] };
            let b = if i == 0 || i == n2 { 0.0 } else { s[imag] };
            (a.hypot(b), -b.atan2(a))
        })
        .collect()
}

/// Ports `leanunconvert()`/`leanunconvert2()`/`leanunconvert3()`: the
/// stateless inverse of [`lean_convert`] - (magnitude, raw phase) pairs
/// back to an rfft-format spectrum of length `n`.
fn lean_unconvert(bins: &[(f32, f32)], n: usize) -> Vec<f32> {
    let n2 = bins.len() - 1;
    let mut s = vec![0.0f32; n];
    for (i, &(amp, phase)) in bins.iter().enumerate() {
        let mut real = i << 1;
        let imag = real + 1;
        if i == n2 {
            real = 1;
        }
        s[real] = amp * phase.cos();
        if i != n2 {
            s[imag] = -amp * phase.sin();
        }
    }
    s
}

/// Applies [`eq`] to the amplitude half of a (magnitude, phase-or-freq)
/// pair slice, round-tripping through a flat buffer since [`eq`] only
/// ever touches even-indexed (amplitude) slots regardless of what the
/// odd slots mean.
#[allow(clippy::too_many_arguments)]
fn apply_shelf_eq(
    bins: &mut [(f32, f32)],
    d_blow: f32,
    d_bhi: f32,
    freqlow: f32,
    freqhi: f32,
    fundamental: f32,
    db_to_amp: &DbToAmp,
) {
    let mut flat: Vec<f32> = Vec::with_capacity(bins.len() * 2);
    for &(a, p) in bins.iter() {
        flat.push(a);
        flat.push(p);
    }
    eq(
        &mut flat,
        d_blow,
        d_bhi,
        freqlow,
        freqhi,
        fundamental,
        1.0,
        0.0,
        false,
        db_to_amp,
    );
    for (i, bin) in bins.iter_mut().enumerate() {
        bin.0 = flat[2 * i];
    }
}

/// A minimal shiftin+fold+rfft front end - the raw-spectrum half of what
/// [`crate::pvoc::Analyzer::push`] does, without its bundled `convert()`
/// call, since `ring.c` needs the plain post-FFT buffer itself (to seed
/// the feedback path) as well as its phase-vocoder conversion (for the
/// oscillator-bank-ready source spectrum) - two independent uses of the
/// same buffer that `Analyzer`'s all-in-one `push` can't serve.
struct RawAnalyzer {
    d: usize,
    analysis_window: Vec<f32>,
    input_ring: Vec<f32>,
    fft_buf: Vec<f32>,
    fold_pos: i64,
    n: usize,
}

impl RawAnalyzer {
    fn new(n: usize, analysis_window: Vec<f32>, d: usize) -> Self {
        let nw = analysis_window.len();
        RawAnalyzer {
            d,
            analysis_window,
            input_ring: vec![0.0; nw],
            fft_buf: vec![0.0; n],
            fold_pos: -(nw as i64),
            n,
        }
    }

    fn push(&mut self, hop: &[f32]) -> Vec<f32> {
        let nw = self.input_ring.len();
        self.input_ring.copy_within(self.d..nw, 0);
        self.input_ring[nw - self.d..].copy_from_slice(hop);
        self.fold_pos += self.d as i64;
        fold(
            &self.input_ring,
            &self.analysis_window,
            &mut self.fft_buf,
            self.fold_pos,
        );
        rfft(&mut self.fft_buf, self.n / 2, true);
        self.fft_buf.clone()
    }
}

fn control_fn_max(cf: &ControlFn) -> f32 {
    match cf {
        ControlFn::Const(v) => *v,
        ControlFn::Table(vals) => vals.iter().copied().fold(f32::MIN, f32::max),
    }
}

/// Processes one channel start to finish. `dur` is the control-function
/// normalization duration in seconds, matching `tools::pv`/`tools::
/// harmonizer` (`(channel.len() / sample_rate) * time_factor`).
pub fn process_channel(
    channel: &[f32],
    sample_rate: u32,
    params: &RingParams,
    dur: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let n2 = n / 2;

    // `ring.c`'s own `ringTime` (`funcStats(&feedback_level, ...).hi`,
    // consumed by the shared `fileio.c`'s `bufferin()`): the input stream
    // is extended with this many extra seconds of silence *before* EOF is
    // reported, so the reverb tail can actually ring out past the real
    // input's end rather than being cut off at it. Keyed off `-Z`'s own
    // peak value regardless of where in time it occurs - not, say, its
    // value at the end of the file - matching `funcStats`' whole-domain
    // min/max, not a time-indexed lookup. `dur` (already computed by the
    // caller from the *original* span) is deliberately left unchanged -
    // this only affects how much silence [`RawAnalyzer`] gets fed, not
    // the control-function normalization window.
    let ring_time_secs = control_fn_max(&params.feedback_decay_secs).max(0.0);
    let ring_time_samples = (ring_time_secs * r) as usize;
    let mut extended_channel = channel.to_vec();
    extended_channel.extend(std::iter::repeat_n(0.0f32, ring_time_samples));
    let channel = extended_channel.as_slice();

    // "YOU MUST SPECIFY 32 OR MORE FRAMES PER SECOND" / "> 0" resets -
    // ring.c silently corrects an out-of-range -D/-I to its own default
    // rather than erroring.
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

    let nyquist = r / 2.0;
    let fundamental = r / n as f32;

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();
    let threshfac = db_to_amp.convert(params.oscbank_threshold_db);

    let balance_limit_amp = db_to_amp_exact(params.loop_balance_limit_db);
    let balance_flag = params.loop_balance_limit_db > 0.0;

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut raw_analyzer = RawAnalyzer::new(n, window_pair.analysis, d);

    let mut channel_phase = PhaseTracker::new_analysis(n2, d, sample_rate);
    let mut feedback_phase = PhaseTracker::new_analysis(n2, d, sample_rate);

    // One shared cosine table (`noscbank2()`'s single `static table`,
    // built once from its first array's N2/Nw and reused unmodified for
    // its second - see `OscBank::with_shared_table`'s doc comment).
    let mut source_osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut feedback_osc =
        OscBank::with_shared_table(n2, sample_rate, i_factor, 1.0, source_osc.table());

    // Per-bin state persisting across frames: `previous.0` is the
    // envelope-followed feedback magnitude, `previous.1` is the feedback
    // path's own raw phase from the previous frame (both zero-initialized,
    // matching the C's `fvec()`).
    let mut previous: Vec<(f32, f32)> = vec![(0.0, 0.0); n2 + 1];
    // The delayed feedback tail carried into next frame - the actual
    // recirculating delay line content.
    let mut next_buffer: Vec<f32> = vec![0.0; n];

    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;
    let mut on: i64 = (-(nw as i64) * i_factor as i64) / d as i64;
    let mut output = Vec::new();
    let mut samps_written: usize = 0;

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

        let t = samps_written as f32 / r;

        let master_gain = db_to_amp.convert(params.master_gain_db.at(t, dur));

        let source_gain = db_to_amp.convert(params.source_gain_db.at(t, dur));
        let source_harmadd = params.source_freq_shift_hz.at(t, dur);
        let source_pm = semitones_to_mult.convert(params.source_pitch_semitones.at(t, dur));

        let feedback_gain = db_to_amp.convert(params.feedback_gain_db.at(t, dur));
        let feedback_harmadd = params.feedback_freq_shift_hz.at(t, dur);
        let feedback_pm = semitones_to_mult.convert(params.feedback_pitch_semitones.at(t, dur));

        let (feedlevel, _) = smooth_setup(params.feedback_decay_secs.at(t, dur), ir);
        let feedback_thresh_amp = db_to_amp.convert(params.feedback_threshold_db.at(t, dur));

        let (envrelease, minusrelease) = smooth_setup(params.envelope_release_secs.at(t, dur), ir);
        let (envattack, minusattack) = smooth_setup(params.envelope_attack_secs.at(t, dur), ir);

        // Raw post-FFT spectrum, shared by both the source and feedback
        // paths.
        let buffer = raw_analyzer.push(&hop);

        let mut feedback_buffer = buffer.clone();
        let mut feedback_bins = lean_convert(&feedback_buffer, n2);

        // THRESHOLD ENVELOPE: gate x(n) (this frame's dry, pre-loop
        // spectrum) against last frame's own envelope-followed magnitude.
        let feedt = getthresh(&feedback_bins, feedback_thresh_amp);
        for (j, bin) in feedback_bins.iter_mut().enumerate() {
            let below_gate = if params.feedback_threshold_pass_above {
                bin.0 < feedt
            } else {
                bin.0 > feedt
            };
            bin.0 = if below_gate {
                envrelease * previous[j].0
            } else if bin.0 < previous[j].0 {
                envrelease * previous[j].0 + minusrelease * bin.0
            } else {
                envattack * previous[j].0 + minusattack * bin.0
            };
            previous[j].0 = bin.0;
        }

        // INPUT EQ (pre-transpose/shift, "x(n)").
        apply_shelf_eq(
            &mut feedback_bins,
            params.input_eq_low_db.at(t, dur),
            params.input_eq_high_db.at(t, dur),
            params.input_eq_low_freq.at(t, dur),
            params.input_eq_high_freq.at(t, dur),
            fundamental,
            &db_to_amp,
        );

        feedback_buffer = lean_unconvert(&feedback_bins, n);

        // ADD IN THE DELAY LINE: last frame's own (already EQ'd/balanced)
        // feedback tail, decayed by `feedlevel`.
        for i in 0..n {
            feedback_buffer[i] += feedlevel * next_buffer[i];
        }
        feedback_bins = lean_convert(&feedback_buffer, n2);

        // FEEDBACK (in-loop) EQ, "y(n-1)": decay-time-scaled, with an
        // optional balance limiter restoring the pre-EQ aggregate energy
        // (clamped to `balance_limit_amp`). Shapes only the delay line
        // fed into *future* frames - see this module's doc comment.
        let loop_decay_secs = params.loop_eq_decay_secs.at(t, dur);
        let dbhitemp = (params.loop_eq_high_db.at(t, dur) * ir) / loop_decay_secs;
        let dblowtemp = (params.loop_eq_low_db.at(t, dur) * ir) / loop_decay_secs;
        let loop_freqlow = params.loop_eq_low_freq.at(t, dur);
        let loop_freqhi = params.loop_eq_high_freq.at(t, dur);

        let prebalancesum = if dbhitemp != 0.0 && balance_flag {
            feedback_bins.iter().map(|b| b.0).sum::<f32>()
        } else {
            0.0
        };

        apply_shelf_eq(
            &mut feedback_bins,
            dblowtemp,
            dbhitemp,
            loop_freqlow,
            loop_freqhi,
            fundamental,
            &db_to_amp,
        );

        if balance_flag && (dbhitemp != 0.0 || dblowtemp != 0.0) {
            let postbalancesum = feedback_bins.iter().map(|b| b.0).sum::<f32>();
            if prebalancesum > 0.0 && postbalancesum > 0.0 {
                let mut temp = prebalancesum / postbalancesum;
                if temp > balance_limit_amp {
                    temp = balance_limit_amp;
                }
                for bin in feedback_bins.iter_mut() {
                    bin.0 *= temp;
                }
            }
        }

        // Advance the feedback path's own raw phase by one frame's worth
        // of its own last change, building the delayed tail for next
        // frame - and save this frame's (post-EQ/-balance amplitude,
        // pre-advance phase) state for the next iteration's threshold
        // envelope and phase-diff.
        let mut next_bins = vec![(0.0f32, 0.0f32); n2 + 1];
        for j in 0..=n2 {
            let phasediff = feedback_bins[j].1 - previous[j].1;
            next_bins[j] = (feedback_bins[j].0, feedback_bins[j].1 + phasediff);
            previous[j].1 = feedback_bins[j].1;
        }
        next_buffer = lean_unconvert(&next_bins, n);

        // Oscillator-bank-ready Hz conversions: the feedback path from
        // *before* this frame's in-loop EQ/balance (see this module's doc
        // comment), the source path from the plain, unmodified analysis
        // spectrum.
        let mut feedback_frame = feedback_phase.convert(&feedback_buffer);
        apply_shelf_eq(
            &mut feedback_frame.bins,
            params.output_eq_low_db.at(t, dur),
            params.output_eq_high_db.at(t, dur),
            params.output_eq_low_freq.at(t, dur),
            params.output_eq_high_freq.at(t, dur),
            fundamental,
            &db_to_amp,
        );
        let mut source_frame = channel_phase.convert(&buffer);

        for bin in feedback_frame.bins.iter_mut() {
            let (amp, freq) = *bin;
            let temp = feedback_pm * (feedback_harmadd + freq);
            *bin = if temp > 0.0 && temp < nyquist {
                (amp, temp)
            } else {
                (0.0, freq)
            };
            bin.0 *= feedback_gain * master_gain;
        }
        for bin in source_frame.bins.iter_mut() {
            let (amp, freq) = *bin;
            let temp = source_pm * (source_harmadd + freq);
            *bin = if temp > 0.0 && temp < nyquist {
                (amp, temp)
            } else {
                (0.0, freq)
            };
            bin.0 *= source_gain * master_gain;
        }

        let synt = getthresh(&source_frame.bins, threshfac)
            .max(getthresh(&feedback_frame.bins, threshfac));

        let mut hop_out = vec![0.0f32; i_factor];
        for (o, s) in hop_out
            .iter_mut()
            .zip(source_osc.synthesize(&source_frame, synt))
        {
            *o += s;
        }
        for (o, s) in hop_out
            .iter_mut()
            .zip(feedback_osc.synthesize(&feedback_frame, synt))
        {
            *o += s;
        }

        on += i_factor as i64;
        if on + nw as i64 - i_factor as i64 >= 0 {
            output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
            samps_written += i_factor;
        }

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

    fn default_params(fft_size: usize) -> RingParams {
        RingParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            oscbank_threshold_db: -96.0,
            master_gain_db: ControlFn::Const(0.0),
            source_gain_db: ControlFn::Const(0.0),
            source_freq_shift_hz: ControlFn::Const(0.0),
            source_pitch_semitones: ControlFn::Const(0.0),
            feedback_gain_db: ControlFn::Const(-999.0), // no feedback voice for basic tests
            feedback_freq_shift_hz: ControlFn::Const(0.0),
            feedback_pitch_semitones: ControlFn::Const(0.0),
            feedback_decay_secs: ControlFn::Const(0.0), // loop off
            feedback_threshold_db: ControlFn::Const(-96.0),
            feedback_threshold_pass_above: true,
            envelope_attack_secs: ControlFn::Const(0.0),
            envelope_release_secs: ControlFn::Const(0.0),
            input_eq_low_db: ControlFn::Const(0.0),
            input_eq_high_db: ControlFn::Const(0.0),
            input_eq_low_freq: ControlFn::Const(0.0),
            input_eq_high_freq: ControlFn::Const(2000.0),
            loop_eq_decay_secs: ControlFn::Const(1.0),
            loop_balance_limit_db: 0.0,
            loop_eq_low_db: ControlFn::Const(0.0),
            loop_eq_high_db: ControlFn::Const(0.0),
            loop_eq_low_freq: ControlFn::Const(0.0),
            loop_eq_high_freq: ControlFn::Const(2000.0),
            output_eq_low_db: ControlFn::Const(0.0),
            output_eq_high_db: ControlFn::Const(0.0),
            output_eq_low_freq: ControlFn::Const(0.0),
            output_eq_high_freq: ControlFn::Const(2000.0),
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let params = default_params(1024);
        let input = vec![0.0f32; 44100 / 4];
        let output = process_channel(&input, 44100, &params, 1.0);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn sine_input_produces_bounded_output() {
        let params = default_params(1024);
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
    fn feedback_loop_adds_energy_that_decays_over_time() {
        // A short burst followed by silence: with the feedback loop on
        // (a real decay time, feedback gain at unity, threshold low
        // enough to pass everything), the tail should keep producing
        // sound well after the dry burst ends - basic confirmation the
        // delay line is actually recirculating.
        let sample_rate = 44100u32;
        let mut params = default_params(1024);
        params.feedback_gain_db = ControlFn::Const(0.0);
        params.feedback_decay_secs = ControlFn::Const(0.5);
        params.feedback_threshold_db = ControlFn::Const(-200.0);
        params.source_gain_db = ControlFn::Const(-999.0); // isolate the reverb tail

        let burst_len = sample_rate as usize / 10;
        let mut input: Vec<f32> = (0..burst_len)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        input.extend(vec![0.0f32; sample_rate as usize]);

        let output = process_channel(&input, sample_rate, &params, 1.0);
        let tail_start = burst_len + sample_rate as usize / 4;
        let tail_energy: f32 = output[tail_start..].iter().map(|&s| s * s).sum();
        assert!(
            tail_energy > 0.0,
            "expected a decaying reverb tail past the dry burst"
        );
    }
}
