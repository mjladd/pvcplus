//! Ports `ratechanger.c`: a varispeed resampler that reads raw audio
//! directly (no `.pva` analysis file, unlike almost every other tool in
//! this project) and resynthesizes it at a possibly time-varying playback
//! rate (and/or semitone shift), using one of three per-sample synthesis
//! methods - windowed-sinc summation (`sumSincFunctions`, high quality),
//! sample-and-hold/decimation (`holdSampleOrDecimate`, an aliasing demo
//! mode), or linear interpolation (`linearlyInterpolate`, ditto).
//!
//! **Real, verified heap-buffer-overflow in the C**, not a mere reading
//! exercise: `sumSincFunctions`'s table-lookup path indexes
//! `halfSincLookupTable[i1]`/`[i2]` where `i1`/`i2` derive from
//! `xScaledForSinc * PIinterpolationPoints`, `xScaledForSinc` itself
//! derived from `x = |dataTpt*isr - n|` ranging up to `isr *
//! sincTruncationXinSeconds` *plus* the explicit `± 0.5`-sample rounding
//! pad `xBegin`/`xEnd` each add - while the table itself (`main()`,
//! `halfSincFunctionLength = 10 + floor(isr * sincTruncationXinSeconds) *
//! PIinterpolationPoints`) only pads by a flat `10` *index* units (0.1
//! samples' worth at the default `PIinterpolationPoints = 100`), nowhere
//! near enough to cover a full extra sample's worth of index range (100
//! units at that same default). Confirmed empirically, not just by
//! arithmetic: an ASan build of `ratechanger.c` (via this project's own
//! pinned Docker image, `cmake -DCMAKE_C_FLAGS="-fsanitize=address -g
//! -O0"`) crashes with a `heap-buffer-overflow` at `ratechanger.c:1515`
//! on the *very first output sample* of a completely ordinary run - the
//! tool's own ordinary defaults (`-X 0` sinc synthesis, `-t 1` table
//! lookup, no other flags) against any input file. [`sum_sinc_functions`]
//! clamps `i1`/`i2` to the table's own bounds rather than reproducing
//! this - the C's own behavior past its buffer is undefined (whatever
//! heap-adjacent bytes a given allocator happens to place there), not a
//! deterministic value there's anything meaningful to match.
//!
//! `ratechanger.c` has its own time-management machinery
//! (`setUpTimeManagementValues`/`advanceToNextDataTimePoint`/
//! `makeBoundaries`), genuinely distinct from `pvc_core::timenav`'s
//! `findFilterTimeAndConstrainByWindow`-based navigator used by
//! `twarp`/`convolver` (confirmed by reading both files in full - neither
//! function name appears in the other's source, and `ratechanger.c` has
//! no wrap/fold/clip/onset-release machinery at all, just a hard stop
//! when the time position exits its window or the output duration is
//! reached). [`TimeState`] is a fresh, self-contained port of that
//! machinery, not a reuse of `timenav`.
//!
//! **Multi-layer machinery preserved in the C but never actually
//! multi-layer**: `ratechanger.c` carries a general N-layer
//! rise/hold/fall crossfade system (`layerMode`/`layerDataTpt`/
//! `layerRiseTimeDur`/...), but `main()` always hardcodes
//! `numberOfLayers = 1` with `layerRiseTimeDur[0] = layerFallTimeDur[0] =
//! 0` and `layerHoldTimeDur[0] = outputDuration` - so the single layer is
//! always in `HOLD` mode with `rampValue == 1.0` for the tool's entire
//! run, and `layerDataTpt[0]` is fed the exact same per-frame `increment`
//! as the shared `dataTpt` (both initialized identically in
//! `setUpTimeManagementValues`/the pre-loop origin fetch), so the two are
//! always numerically identical. No CLI flag exposes more than one layer.
//! This port drops the dead layer loop entirely and resamples directly
//! from the shared time position - not a behavioral simplification, since
//! the general machinery could never produce anything else.
//!
//! **Dead `crack()` flags** (accepted by the parser, no `case` in the
//! `switch`, confirmed by cross-referencing both lists): `c`, `f`, `F`,
//! `g`, `i`, `Q`, `w`, `T`, `Y`. Not exposed as `pvc ratechanger` flags.
//!
//! **A real, narrow precision quirk, reproduced faithfully**:
//! `holdSampleOrDecimate`'s own `dataTpt` parameter is declared `float`,
//! while its two siblings `sumSincFunctions`/`linearlyInterpolate` both
//! declare theirs `double` - so synthesis mode 1 (sample-and-hold/
//! decimate) receives its time position narrowed to `f32` before use,
//! sacrificing precision the other two modes keep. Modeled here by
//! [`hold_sample_or_decimate`] taking `f32` where its siblings take
//! `f64`, with the narrowing applied at the call site in
//! [`synthesize_channel`], not inside a shared helper.
//!
//! **Window-boundary defaulting only ever touches a table's first
//! breakpoint**: `main()` defaults `-b`/`-e` once, before any per-frame
//! evaluation, by mutating `analysisDatawinlow.A[0]`/
//! `analysisDatawinhi.A[0]` in place (`< 0` and `<= 0` respectively) -
//! the *same* array element `fval()` returns for a constant function, but
//! only the *first* breakpoint of a real multi-point table. Modeled here
//! as [`RatechangerParams::resolve_window_defaults`], applied once by the
//! caller before any [`TimeState`] is constructed - matching the C's
//! "mutate once, read many times" behavior exactly, including the
//! obscure table-truncation edge case.
//!
//! **Carried-over antialiasing scaler**: `sincSumXscaler` (the sinc
//! window's own auto-widening factor, recomputed every
//! `advanceToNextDataTimePoint` call from the *local* rate of time
//! change) is a `main()`-scoped local, not reset between the optional
//! duration-search phase and per-channel synthesis, nor between channels.
//! Each channel's very first resampled sample uses whatever value the
//! *previous* channel (or the duration search, or the compile-time
//! default `1.0`) last left it at, one call stale relative to that
//! channel's own actual rate of change at time zero. [`synthesize_channel`]
//! and [`resolve_output_duration`] both take and return this value
//! explicitly so callers can thread it through in the same order.
//!
//! Not reproduced: `windowTpt`/`oldWindowTpt` (write-only bookkeeping,
//! confirmed unread anywhere), `realDataTpt` (incremented in parallel
//! with `dataTpt` but never read back), and `messageSent`/
//! `releaseTriggered` (`static` locals in `advanceToNextDataTimePoint`,
//! also write-only) - the same "dead C state" pattern already documented
//! in `pvc_core::timenav`'s own doc comment. `PI`/`TWOPI` are taken here
//! as `std::f32::consts::PI`/`2.0 * PI` rather than the C's own
//! `4.*atan(1.)`/`8.*(float)atan(1.)` derivation - both evaluate to the
//! same nearest `f32` in practice, and the values only ever feed a
//! window-function argument to `sin`/`cos`, well within this project's
//! existing golden-test tolerances.

use crate::control::ControlFn;
use crate::units::{DbToAmp, SemitonesToMult};

const PI: f32 = std::f32::consts::PI;
const TWOPI: f32 = 2.0 * PI;

/// `synthesisMode`: which per-sample resampling method to use.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SynthesisMode {
    /// Windowed-sinc summation (`-X 0`, the default, highest quality).
    Sinc,
    /// Sample-and-hold/decimation (`-X 1`) - an aliasing demo mode per
    /// the tool's own `usage()` text.
    HoldDecimate,
    /// Linear interpolation (`-X 2`) - ditto.
    LinearInterp,
}

/// `normalizeFlag`: post-synthesis, pre-interleave amplitude scaling,
/// applied by the caller (this crate has no file-level orchestration) -
/// see `main()`'s own `normalizeFlag == 1..4` branches in the interleave
/// loop. `None` is `normalizeFlag == 0` (no scaling at all).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NormalizeMode {
    /// `0`: no scaling.
    None,
    /// `1` (the default): each channel independently rescaled so its own
    /// peak matches the *input* channel's own peak (not clamped to
    /// full scale - can still clip or stay quiet).
    Input,
    /// `2`: each channel independently rescaled to just under full
    /// scale (`32767/32768`).
    Independent,
    /// `3`: every channel scaled by the same factor, based on the single
    /// largest peak across all channels - preserves relative levels.
    Together,
    /// `4`: like `Together`, but only if the largest peak would
    /// otherwise exceed full scale.
    IfClipping,
}

#[derive(Debug, Clone)]
pub struct RatechangerParams {
    pub window_low: ControlFn,
    pub window_high: ControlFn,
    pub time_origin: ControlFn,
    pub rate_for_input: ControlFn,
    pub rate_for_output: ControlFn,
    pub semitones_for_input: ControlFn,
    pub semitones_for_output: ControlFn,
    pub amp_env_for_input: ControlFn,
    pub amp_env_for_output: ControlFn,
    pub synthesis_mode: SynthesisMode,
    pub use_table_lookup: bool,
    pub pi_interpolation_points: f32,
    pub sinc_truncation_db: f32,
}

impl RatechangerParams {
    /// `main()`'s upfront `-b`/`-e` boundary defaulting - see this
    /// module's own doc comment. Must be called exactly once, before
    /// constructing any [`TimeState`], with the result reused for every
    /// subsequent evaluation (matching the C's own "mutate the stored
    /// function once" behavior).
    pub fn resolve_window_defaults(mut self, input_duration: f32) -> Self {
        self.window_low = apply_low_boundary_default(&self.window_low);
        self.window_high = apply_high_boundary_default(&self.window_high, input_duration);
        self
    }
}

fn apply_low_boundary_default(f: &ControlFn) -> ControlFn {
    match f {
        ControlFn::Const(v) if *v < 0.0 => ControlFn::Const(0.0),
        ControlFn::Table(t) if !t.is_empty() && t[0] < 0.0 => {
            let mut t = t.clone();
            t[0] = 0.0;
            ControlFn::Table(t)
        }
        other => other.clone(),
    }
}

fn apply_high_boundary_default(f: &ControlFn, input_duration: f32) -> ControlFn {
    match f {
        ControlFn::Const(v) if *v <= 0.0 => ControlFn::Const(input_duration),
        ControlFn::Table(t) if !t.is_empty() && t[0] <= 0.0 => {
            let mut t = t.clone();
            t[0] = input_duration;
            ControlFn::Table(t)
        }
        other => other.clone(),
    }
}

/// Ports the shared time-position state driven by
/// `setUpTimeManagementValues`/`advanceToNextDataTimePoint`/
/// `makeBoundaries`. `dataTpt` is `f64` throughout (the C's own `double`),
/// matching [`crate::control::ControlFn::at`]'s `f32` time argument only
/// where the C itself narrows for a `fval()` call.
struct TimeState {
    data_tpt: f64,
    old_data_tpt_origin: f32,
}

impl TimeState {
    fn new(params: &RatechangerParams, output_duration: f32, input_duration: f32) -> Self {
        let origin0 = params.time_origin.at(0.0, output_duration);
        let _ = input_duration; // boundaries are recomputed fresh on the first `advance` call.
        TimeState {
            data_tpt: origin0 as f64,
            old_data_tpt_origin: origin0,
        }
    }

    /// One call to `advanceToNextDataTimePoint`. Returns the freshly
    /// recomputed `sincSumXscaler` and whether synthesis should stop.
    #[allow(clippy::too_many_arguments)]
    fn advance(
        &mut self,
        params: &RatechangerParams,
        isr: f64,
        t: f32,
        input_duration: f32,
        output_duration: f32,
        semitones_to_mult: &SemitonesToMult,
        not_searching_for_duration: bool,
    ) -> (f64, bool) {
        let rate_input = params
            .rate_for_input
            .at(self.data_tpt as f32, input_duration);
        let rate_output = params.rate_for_output.at(t, output_duration);
        let semi_input = params
            .semitones_for_input
            .at(self.data_tpt as f32, input_duration);
        let semi_output = params.semitones_for_output.at(t, output_duration);

        let numerator: f32 =
            rate_input * rate_output * semitones_to_mult.convert(semi_input + semi_output);
        let rate_change_increment: f32 = (numerator as f64 / isr) as f32;

        let origin_val = params.time_origin.at(t, output_duration);
        let origin_change_difference = origin_val - self.old_data_tpt_origin;
        self.old_data_tpt_origin = origin_val;

        let increment: f64 = rate_change_increment as f64 + origin_change_difference as f64;
        self.data_tpt += increment;

        let sinc_sum_xscaler = (isr * increment.abs()).max(1.0);

        let win_low_raw = params.window_low.at(t, output_duration);
        let win_hi_raw = params.window_high.at(t, output_duration);
        let (mut low, mut high) = if win_hi_raw >= win_low_raw {
            (win_low_raw, win_hi_raw)
        } else {
            (win_hi_raw, win_low_raw)
        };
        if low < 0.0 {
            low = 0.0;
        }
        if high > input_duration {
            high = input_duration;
        }

        let stop = self.data_tpt > high as f64
            || self.data_tpt < low as f64
            || (t >= output_duration && not_searching_for_duration);

        (sinc_sum_xscaler, stop)
    }
}

/// Ports the output-duration convergence search (the `-D`/auto-triggered
/// `synthesizeOutputDuration` block). Runs the time-management state
/// machine with no audio, adjusting `output_duration` up to 100 times
/// until the actual stop time lands within 1% of the tested duration (or
/// the attempt cap is hit, at which point the last tested duration is
/// accepted as-is). Returns the resolved duration and the
/// `sincSumXscaler` left over from the final search attempt - feed that
/// into the first call to [`synthesize_channel`] to match the C's shared,
/// carried-over local (see this module's own doc comment).
///
/// Takes no incoming `sincSumXscaler` seed: `advanceToNextDataTimePoint`
/// only ever *writes* that value, never reads it, so the search's very
/// first call already overwrites whatever a caller could have supplied
/// before it's ever used - confirmed dead in the C itself, not just this
/// port, by reading `advanceToNextDataTimePoint`'s own parameter list.
pub fn resolve_output_duration(
    params: &RatechangerParams,
    isr: u32,
    input_duration: f32,
    initial_output_duration: f32,
) -> (f32, f64) {
    let isr_f64 = isr as f64;
    let semitones_to_mult = SemitonesToMult::new();

    let mut output_duration = if initial_output_duration == 0.0 {
        input_duration
    } else {
        initial_output_duration
    };
    let mut previous_output_duration = output_duration;
    let mut old_compliance_ratio = 0.0f32;
    let mut convergence_prop = 1.0f32;
    // Always overwritten before being read (the inner loop below runs at
    // least once) - kept only as the binding the loop assigns into.
    #[allow(unused_assignments)]
    let mut xscaler = 1.0f64;
    let mut attempts = 0;

    loop {
        let mut state = TimeState::new(params, output_duration, input_duration);
        let mut frames: i64 = 0;
        let t = loop {
            frames += 1;
            let t = (frames as f64 / isr_f64) as f32;
            let (new_xscaler, stop) = state.advance(
                params,
                isr_f64,
                t,
                input_duration,
                output_duration,
                &semitones_to_mult,
                false,
            );
            xscaler = new_xscaler;
            if stop {
                break t;
            }
        };

        let compliance_ratio = t / output_duration;
        if compliance_ratio < 1.01 && compliance_ratio > (1.0 / 1.01) {
            break;
        }

        if (1.0 - compliance_ratio) < (1.0 - old_compliance_ratio) {
            previous_output_duration = output_duration;
            output_duration += convergence_prop * (t - output_duration);
        } else {
            convergence_prop *= 0.75;
            output_duration =
                previous_output_duration + convergence_prop * (t - previous_output_duration);
        }
        old_compliance_ratio = compliance_ratio;

        attempts += 1;
        if attempts > 100 {
            break;
        }
    }

    (output_duration, xscaler)
}

/// Ports the Blackman-windowed sinc lookup table built once in `main()`
/// when `-t` (table lookup) is on. Only needed for [`SynthesisMode::Sinc`],
/// unlike the C, which builds it unconditionally whenever `-t` is on
/// regardless of `-X`, wasting the work for the other two modes; not
/// reproduced since it has no behavioral effect.
pub fn build_sinc_table(
    isr: u32,
    sinc_truncation_x_seconds: f64,
    pi_interpolation_points: f32,
) -> Vec<f32> {
    let isr_f64 = isr as f64;
    let half_len_f64 =
        10.0 + (isr_f64 * sinc_truncation_x_seconds).floor() * pi_interpolation_points as f64;
    let half_len = (half_len_f64 as i64).max(1) as usize;

    let mut table = vec![0.0f32; half_len];
    table[0] = 1.0;

    let denom = half_len_f64 - 1.0;
    let mut z = 0.5 * half_len_f64;
    for (i, slot) in table.iter_mut().enumerate().skip(1) {
        let x = (i as f32 as f64 / pi_interpolation_points as f64) as f32;
        let xpi = x * PI;
        let sinc = (xpi as f64).sin() as f32 / xpi;

        let arg1 = (TWOPI as f64 / denom) * z;
        let arg2 = (2.0 * TWOPI as f64 / denom) * z;
        let window = (0.42 - 0.5 * arg1.cos() + 0.08 * arg2.cos()) as f32;

        *slot = sinc * window;
        z += 0.5;
    }
    table
}

/// Ports `sumSincFunctions()`. `data_tpt`/`sinc_sum_xscaler` are `f64`
/// (the C's own `double` parameters); `sinc_table` is required (and used)
/// only when `use_table_lookup` is set.
#[allow(clippy::too_many_arguments)]
fn sum_sinc_functions(
    isr: f64,
    data_tpt: f64,
    sinc_sum_xscaler: f64,
    sinc_truncation_x_seconds: f64,
    audio: &[f32],
    sinc_table: Option<&[f32]>,
    pi_interpolation_points: f32,
    use_table_lookup: bool,
) -> f32 {
    let n_frames = audio.len() as i64;

    let mut x_begin =
        ((isr * (data_tpt - sinc_sum_xscaler * sinc_truncation_x_seconds)) - 0.5) as i64;
    if x_begin < 0 {
        x_begin = 0;
    }
    let mut x_end =
        ((isr * (data_tpt + sinc_sum_xscaler * sinc_truncation_x_seconds)) + 0.5) as i64;
    if x_end >= n_frames {
        x_end = n_frames - 1;
    }

    let mut sum = 0.0f32;
    let mut n = x_begin;
    while n <= x_end {
        let x = ((data_tpt * isr) - n as f64).abs() as f32;
        let x_scaled_for_sinc = (x as f64 / sinc_sum_xscaler) as f32;

        if use_table_lookup {
            let table = sinc_table.expect("sinc table required when use_table_lookup is set");
            let temp = (x_scaled_for_sinc as f64 * pi_interpolation_points as f64) as f32;
            let i1 = temp as i64;
            let i2 = i1 + 1;
            let frac = temp - i1 as f32;
            // Real, verified heap-buffer-overflow in the C at this exact
            // lookup (confirmed under ASan - see this module's own doc
            // comment): `i1`/`i2` can run past `halfSincLookupTable`'s
            // own end by close to a full sample's worth of index units.
            // Clamped here rather than reproduced, since the C's own
            // behavior past its buffer is undefined (whatever heap-
            // adjacent bytes happen to sit there), not a deterministic
            // value worth matching.
            let last = table.len() as i64 - 1;
            let (t1, t2) = (
                table[i1.clamp(0, last) as usize],
                table[i2.clamp(0, last) as usize],
            );
            sum += audio[n as usize] * (t1 + frac * (t2 - t1));
        } else {
            let v_base = if x_scaled_for_sinc > 0.0 {
                let xpi = x_scaled_for_sinc * PI;
                audio[n as usize] * (xpi as f64).sin() as f32 / xpi
            } else {
                0.0
            };
            let denom = isr * sinc_truncation_x_seconds * 2.0;
            let arg_base = x as f64 + isr * sinc_truncation_x_seconds;
            let arg1 = (TWOPI as f64 / denom) * arg_base;
            let arg2 = (2.0 * TWOPI as f64 / denom) * arg_base;
            let window = (0.42 - 0.5 * arg1.cos() + 0.08 * arg2.cos()) as f32;
            sum += v_base * window;
        }
        n += 1;
    }
    sum
}

/// Ports `holdSampleOrDecimate()`. `data_tpt` is `f32` - see this
/// module's doc comment on the real signature-mismatch precision quirk
/// this reproduces.
fn hold_sample_or_decimate(data_tpt: f32, audio: &[f32], isr: f64) -> f32 {
    let n = audio.len() as i64;
    let mut low_index = ((data_tpt as f64 * isr) + 0.5) as i64;
    if low_index < 0 {
        low_index = 0;
    }
    if low_index >= n {
        low_index = n - 1;
    }
    audio[low_index as usize]
}

/// Ports `linearlyInterpolate()`. Defensively clamps to `0` for a
/// 0-or-1-sample input rather than reproducing the C's out-of-bounds
/// `numberOfFrames - 2` read in that degenerate case - not a real usage
/// scenario (an audio file with fewer than 2 samples).
fn linearly_interpolate(data_tpt: f64, audio: &[f32], isr: f64) -> f32 {
    if audio.len() < 2 {
        return audio.first().copied().unwrap_or(0.0);
    }
    let real_index = data_tpt * isr;
    let mut low_index = real_index as i64;
    if low_index < 0 {
        low_index = 0;
    }
    let n = audio.len() as i64;
    if low_index >= n - 1 {
        low_index = n - 2;
    }
    let high_index = low_index + 1;
    let frac_index = (real_index - real_index.floor()) as f32;
    let lo = audio[low_index as usize];
    let hi = audio[high_index as usize];
    lo + frac_index * (hi - lo)
}

/// Resamples one channel of raw audio. `initial_sinc_sum_xscaler` should
/// be `1.0` for the very first channel processed when no duration search
/// ran, or the value returned by [`resolve_output_duration`] /
/// a prior call to this function otherwise (see this module's doc
/// comment on the carried-over scaler). Returns the resampled samples,
/// the channel's own peak amplitude, and the final `sincSumXscaler` to
/// feed into the next channel.
pub fn synthesize_channel(
    audio: &[f32],
    isr: u32,
    input_duration: f32,
    output_duration: f32,
    params: &RatechangerParams,
    initial_sinc_sum_xscaler: f64,
) -> (Vec<f32>, f32, f64) {
    let isr_f64 = isr as f64;
    let semitones_to_mult = SemitonesToMult::new();
    let db_to_amp = DbToAmp::new();

    let sinc_truncation_amp = db_to_amp.convert(params.sinc_truncation_db);
    let sinc_truncation_x_seconds = (1.0 / sinc_truncation_amp as f64) / isr_f64;

    let sinc_table = if params.synthesis_mode == SynthesisMode::Sinc && params.use_table_lookup {
        Some(build_sinc_table(
            isr,
            sinc_truncation_x_seconds,
            params.pi_interpolation_points,
        ))
    } else {
        None
    };

    let mut state = TimeState::new(params, output_duration, input_duration);
    let mut t = 0.0f32;
    let mut frames: i64 = 0;
    let mut xscaler = initial_sinc_sum_xscaler;
    let mut out = Vec::new();
    let mut peak = 0.0f32;

    loop {
        let mut sample = match params.synthesis_mode {
            SynthesisMode::Sinc => sum_sinc_functions(
                isr_f64,
                state.data_tpt,
                xscaler,
                sinc_truncation_x_seconds,
                audio,
                sinc_table.as_deref(),
                params.pi_interpolation_points,
                params.use_table_lookup,
            ),
            SynthesisMode::HoldDecimate => {
                hold_sample_or_decimate(state.data_tpt as f32, audio, isr_f64)
            }
            SynthesisMode::LinearInterp => linearly_interpolate(state.data_tpt, audio, isr_f64),
        };

        let amp_in = params
            .amp_env_for_input
            .at(state.data_tpt as f32, input_duration);
        let amp_out = params.amp_env_for_output.at(t, output_duration);
        sample *= db_to_amp.convert(amp_in + amp_out);

        peak = peak.max(sample.abs());
        out.push(sample);

        frames += 1;
        t = (frames as f64 / isr_f64) as f32;
        let (new_xscaler, stop) = state.advance(
            params,
            isr_f64,
            t,
            input_duration,
            output_duration,
            &semitones_to_mult,
            true,
        );
        xscaler = new_xscaler;
        if stop {
            break;
        }
    }

    (out, peak, xscaler)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn const_params() -> RatechangerParams {
        RatechangerParams {
            window_low: ControlFn::Const(0.0),
            window_high: ControlFn::Const(-1.0),
            time_origin: ControlFn::Const(0.0),
            rate_for_input: ControlFn::Const(1.0),
            rate_for_output: ControlFn::Const(1.0),
            semitones_for_input: ControlFn::Const(0.0),
            semitones_for_output: ControlFn::Const(0.0),
            amp_env_for_input: ControlFn::Const(0.0),
            amp_env_for_output: ControlFn::Const(0.0),
            synthesis_mode: SynthesisMode::Sinc,
            use_table_lookup: true,
            pi_interpolation_points: 100.0,
            sinc_truncation_db: -60.0,
        }
        .resolve_window_defaults(1.0)
    }

    #[test]
    fn silence_in_silence_out() {
        let params = const_params();
        let audio = vec![0.0f32; 4410];
        let (out, peak, _) = synthesize_channel(&audio, 44100, 0.1, 0.1, &params, 1.0);
        assert!(!out.is_empty());
        assert_eq!(peak, 0.0);
        assert!(out.iter().all(|&s| s == 0.0));
    }

    #[test]
    fn unity_rate_sine_stays_bounded() {
        let params = const_params();
        let isr = 44100u32;
        let n = 4410usize;
        let audio: Vec<f32> = (0..n)
            .map(|i| (2.0 * PI * 440.0 * i as f32 / isr as f32).sin() * 0.5)
            .collect();
        let dur = n as f32 / isr as f32;
        let (out, peak, _) = synthesize_channel(&audio, isr, dur, dur, &params, 1.0);
        assert!(!out.is_empty());
        assert!(peak <= 1.0, "peak {peak} should stay bounded");
        assert!(out.iter().all(|s| s.is_finite()));
    }

    #[test]
    fn double_rate_produces_roughly_half_the_output_frames() {
        let mut params = const_params();
        params.rate_for_input = ControlFn::Const(2.0);
        let isr = 44100u32;
        let n = 4410usize;
        let audio = vec![0.0f32; n];
        let dur = n as f32 / isr as f32;
        // Fixed output duration (no search) so frame count is directly comparable.
        // At 2x input-rate, the window (clamped to the input's own 0.1s
        // duration) is consumed in about half the output time, so the
        // window-exit stop condition should fire well before the output
        // duration itself does.
        let (out, _, _) = synthesize_channel(&audio, isr, dur, dur, &params, 1.0);
        assert!(
            out.len() < n && out.len() > n / 4,
            "expected roughly half of {n} frames, got {}",
            out.len()
        );
    }

    #[test]
    fn hold_decimate_and_linear_modes_stay_bounded() {
        let isr = 44100u32;
        let n = 4410usize;
        let audio: Vec<f32> = (0..n)
            .map(|i| (2.0 * PI * 220.0 * i as f32 / isr as f32).sin() * 0.4)
            .collect();
        let dur = n as f32 / isr as f32;
        for mode in [SynthesisMode::HoldDecimate, SynthesisMode::LinearInterp] {
            let mut params = const_params();
            params.synthesis_mode = mode;
            let (out, peak, _) = synthesize_channel(&audio, isr, dur, dur, &params, 1.0);
            assert!(!out.is_empty());
            assert!(
                peak <= 1.0,
                "mode {mode:?}: peak {peak} should stay bounded"
            );
        }
    }

    #[test]
    fn resolve_output_duration_converges_at_unity_rate() {
        let params = const_params();
        let isr = 44100u32;
        let input_duration = 0.5f32;
        let (resolved, _) = resolve_output_duration(&params, isr, input_duration, 0.0);
        assert!(
            (resolved - input_duration).abs() < input_duration * 0.02,
            "resolved {resolved} should be close to input duration {input_duration}"
        );
    }

    #[test]
    fn window_defaults_only_touch_first_breakpoint() {
        let f = ControlFn::Table(vec![-1.0, 2.0, 3.0]);
        let resolved = apply_low_boundary_default(&f);
        match resolved {
            ControlFn::Table(t) => assert_eq!(t, vec![0.0, 2.0, 3.0]),
            _ => panic!("expected table"),
        }
    }
}
