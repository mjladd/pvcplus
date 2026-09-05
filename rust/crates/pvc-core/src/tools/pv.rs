//! The core `plainpv` phase-vocoder pipeline
//! (`legacy/pvc_src/plainpv.c`), minus its debug/display-only features:
//! the `-S` terminal bin-data display and the `-c`/`-d` ASCII graph-file
//! writer are both pure reporting (confirmed by reading them - neither
//! feeds back into the audio signal path), so neither is ported here.
//! `-C` (single-channel resynthesis) and the `-n`/`-u`/`-U` display-only
//! frame/bin windowing flags are likewise out of scope. `-b`/`-e`
//! (begin/end time) only affect `dur`, the duration control functions
//! are normalized against - not actual input trimming - so callers pick
//! `dur` directly instead of replicating that indirection.
//!
//! One structural finding worth restating here (see the pvoc.rs
//! `OscBank`/`getthresh` commit): `plainpv`'s resynthesis-method selector
//! (`phaseLockFlag == 1 || ptrans.n != 1 || ...`) is unconditionally
//! true, since `phaseLockFlag` is hardcoded to `1` and never set from
//! any flag. So the real tool always resynthesizes via [`OscBank`],
//! never [`crate::Synthesizer`]'s overlap-add path. Preserved exactly
//! here for golden-harness parity, not "fixed."

use crate::eq::{eq2, ShelfEq};
use crate::pvoc::{getthresh, phaselock, Analyzer, Frame, OscBank};
use crate::smooth::{smooth_setup, Smoother};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` (`#define OSCILBANKGAIN
/// 1.7782794`, i.e. `10^(5/20)`): a fixed +5dB makeup gain `bufferout()`
/// applies to every sample on the oscillator-bank resynthesis path only,
/// found while chasing an output-length discrepancy against the real
/// tool - see `process_channel`'s use of it.
const OSCILBANKGAIN: f32 = 1.7782794;

/// Ports `plainpv`'s `-T` flag (`filttype`): bandpass keeps bins inside
/// `[lowfreq, hifreq]`; band-reject keeps bins outside it.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FilterType {
    Bandpass,
    BandReject,
}

/// All of `plainpv`'s per-frame-varying parameters as [`ControlFn`]s
/// (each independently resolved via `.at(t, dur)` every frame, matching
/// `fval()`), plus its fixed structural parameters. Field names follow
/// the plan's proposed long-option names (`docs/dev/parameter-inventory.md`
/// §12/§14) rather than the C's single-letter flags.
#[derive(Debug, Clone)]
pub struct PvParams {
    pub fft_size: usize,
    /// `0` means "auto": `2 * fft_size`, or larger still if needed to fit
    /// the resynthesis hop (see `process_channel`). The C's own hardcoded
    /// default is a literal `2048`, *not* `2 * fft_size` (confirmed
    /// against the real tool: `--fft 2048` still reports window size
    /// 2048, not 4096) - that auto-scaling rule only actually triggers
    /// in the C via an explicit `-M0` or negative override. Callers
    /// wanting the C's real default behavior should pass `2048`
    /// directly, not `0` - `pvc-cli` does this.
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,
    pub pitch_transpose_semitones: ControlFn,
    pub freq_shift_hz: ControlFn,
    pub gain_db: ControlFn,
    pub attack_secs: ControlFn,
    pub release_secs: ControlFn,
    pub warpshape: ControlFn,
    pub shelf_low_db: ControlFn,
    pub shelf_high_db: ControlFn,
    pub shelf_low_freq: ControlFn,
    pub shelf_high_freq: ControlFn,
    pub threshold_db: f32,
    pub filter_type: FilterType,
    pub filter_lowfreq: f32,
    pub filter_hifreq: f32,
}

/// Processes one channel of audio start to finish, matching `plainpv`'s
/// per-channel frame loop exactly (fold/rfft/convert via [`Analyzer`],
/// `phaselock`, envelope `smooth`ing, frequency-shift/pitch-transpose
/// with brickwall filtering, gain, `spectmagwarp`, shelf `eq2`, then
/// [`OscBank`] resynthesis). `dur` is the control-function normalization
/// duration in seconds (legacy `dur = (endt - begint) * I / D`) - pass
/// the natural output duration (`input.len() as f32 * time_factor /
/// sample_rate as f32`) unless replicating `-b`/`-e` trimming.
pub fn process_channel(input: &[f32], sample_rate: u32, params: &PvParams, dur: f32) -> Vec<f32> {
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
    let ir = i_factor as f32 / r;
    // Exact `pow`, not the `DbToAmp` lookup table - matches
    // `threshfac = pow((double)10.0, (double)(threshfacdB/20.));`.
    let threshfac = 10.0f64.powf(params.threshold_db as f64 / 20.0) as f32;

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);
    // `P` is always 1.0 in the real tool (see this module's doc comment)
    // - pitch transposition happens by directly scaling bin frequencies
    // below, not through OscBank's own pitch parameter. `n2`, not `n` -
    // see OscBank::new's doc comment for why that distinction matters.
    let mut osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut smoother = Smoother::new(n + 2);

    let db_to_amp = DbToAmp::new();
    let semitones_to_mult = SemitonesToMult::new();

    // Replicates `shiftin`'s own end-of-input bookkeeping exactly (see
    // that function's doc comment in pvoc.rs's module docs and
    // `docs/dev/rust-verification.md`) rather than a rough "enough hops"
    // estimate: `valid` starts at `nw` and stays there as long as real
    // input remains; once a hop can't fully fill with `d` real samples,
    // it drops by `d` more each following hop until `<= 0`, at which
    // point that hop is the last one processed - matching this exactly
    // is what makes the output length match the real tool's.
    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;

    // Replicates `shiftout`'s write gate for the oscillator-bank path
    // (`shiftout(output, Nw, I, on + Nw - I, 0)`), which suppresses the
    // first several hops of output entirely while the window is still
    // filling - not a cosmetic startup transient, an actual "not enough
    // real data yet" gate matching `Synthesizer::overlap_add`'s own
    // documented latency (see pvoc.rs).
    let mut on: i64 = (-(nw as i64) * i_factor as i64) / d as i64;

    let mut output = Vec::new();
    // `timenow(dur)`: `t = samps / R`, where the global `samps` counts
    // samples *actually written so far* - incremented inside
    // `bufferout()`, which only ever runs once the write gate above has
    // passed. `t` (and therefore every `fval()`-driven control value
    // this frame) reflects output written through the *previous* frame,
    // not a plain per-hop `t += IR`: during the suppressed startup hops,
    // `t` stays at exactly `0.0`, not advancing at all. Confirmed to
    // matter, not just a cosmetic startup delay: a control function that
    // ramps over the file's duration (e.g. `-P@ramp.txt`, semitones 0..12
    // linearly) came out audibly mistimed against real `plainpv` output
    // until this replaced a naive running `t`.
    let mut samps_written: usize = 0;

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
        let mut frame = frame;
        phaselock(&mut frame);

        let mut flat = frame.to_pva_floats();
        let mut channel_freqdev = vec![0.0f32; n + 2];
        for k in (1..n).step_by(2) {
            channel_freqdev[k] = 1.0;
        }

        let t = samps_written as f32 / r;
        let harmadd = params.freq_shift_hz.at(t, dur);
        let gain = db_to_amp.convert(params.gain_db.at(t, dur));
        let pm = semitones_to_mult.convert(params.pitch_transpose_semitones.at(t, dur));

        let (attackc, minusattackc) = smooth_setup(params.attack_secs.at(t, dur), ir);
        let (releasec, minusreleasec) = smooth_setup(params.release_secs.at(t, dur), ir);
        let warpshape = params.warpshape.at(t, dur);
        let shelf = ShelfEq {
            d_blow: params.shelf_low_db.at(t, dur),
            d_bhi: params.shelf_high_db.at(t, dur),
            freqlow: params.shelf_low_freq.at(t, dur),
            freqhi: params.shelf_high_freq.at(t, dur),
        };

        smoother.smooth(&mut flat, attackc, minusattackc, releasec, minusreleasec);

        // Frequency shift / pitch transpose / brickwall filter. Loop
        // bound matches the C's `for(i=1; i<N; i+=2)` exactly: bins
        // 0..n2, NOT including the Nyquist bin (bins[n2]) - the C's loop
        // stops one short of it, so Nyquist's frequency/amplitude passes
        // through this stage completely untouched (still reachable by
        // `spectmagwarp`/`eq2` below, which do cover the full array).
        for j in 0..n2 {
            let i = 1 + 2 * j;
            channel_freqdev[i] *= pm;
            channel_freqdev[i - 1] += harmadd;

            let temp = pm * (harmadd + flat[i]);
            let binfreq = j as f32 * fundamental;
            let in_band = binfreq >= params.filter_lowfreq && binfreq <= params.filter_hifreq;
            let keep = temp > 0.0
                && temp < nyquist
                && match params.filter_type {
                    FilterType::Bandpass => in_band,
                    FilterType::BandReject => !in_band,
                };
            if keep {
                flat[i] = temp;
            } else {
                flat[i - 1] = 0.0;
            }
        }

        // Gain - same `i < N` bound, so Nyquist's amplitude is untouched
        // here too, matching the C.
        for k in (1..n).step_by(2) {
            flat[k - 1] *= gain;
        }

        spectmagwarp(&mut flat, warpshape, false);
        eq2(
            &mut flat,
            &shelf,
            fundamental,
            &channel_freqdev,
            false,
            &db_to_amp,
        );

        let frame = Frame::from_pva_floats(&flat);
        // `getthresh(channel, N, threshfac)` in the C - passing the FFT
        // size, not N+2 - excludes the Nyquist bin too, not just bin 0
        // (see getthresh's own doc comment).
        let synt = getthresh(&frame.bins[..n2], threshfac);
        let hop_out = osc.synthesize(&frame, synt);

        on += i_factor as i64;
        if on + nw as i64 - i_factor as i64 >= 0 {
            // `bufferout()`'s `in[numsamps] = outbuff[outbuffpt] * gain`
            // with `gain = OSCILBANKGAIN` whenever `oscilbankon` - a
            // fixed +5dB makeup gain applied to *every* oscillator-bank
            // sample on its way to the output file (never applied to the
            // overlap-add path). Easy to miss since it lives in a
            // buffering/file-I/O function, not the DSP code, but it's a
            // real, audible amplitude difference this port would
            // otherwise silently omit.
            output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
            samps_written += i_factor;
        }

        if eof_after_this_hop {
            break;
        }
    }

    // `shiftout(output, Nw, I, 1, 1)` - the unconditional final flush
    // called once after the frame loop ends, regardless of the write
    // gate above. It still transfers one more `I`-sized chunk from the
    // (already fully shifted-and-zeroed, for the oscillator-bank path -
    // `noscbank` only ever writes indices `0..I`, so nothing survives
    // repeated shift+zero-pad) output ring, i.e. `I` samples of silence.
    // Confirmed empirically against a direct `shiftin`/`shiftout` C
    // harness before trusting it: omitting this trailing chunk undercuts
    // the real tool's output length by exactly one hop.
    output.extend(vec![0.0f32; i_factor]);

    output
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params(fft_size: usize) -> PvParams {
        PvParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            pitch_transpose_semitones: ControlFn::Const(0.0),
            freq_shift_hz: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            attack_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
            warpshape: ControlFn::Const(0.0),
            shelf_low_db: ControlFn::Const(0.0),
            shelf_high_db: ControlFn::Const(0.0),
            shelf_low_freq: ControlFn::Const(200.0),
            shelf_high_freq: ControlFn::Const(2000.0),
            threshold_db: -96.0,
            filter_type: FilterType::Bandpass,
            filter_lowfreq: 0.0,
            filter_hifreq: 22_050.0,
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
    fn sine_input_produces_bounded_nonzero_output() {
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
        assert!(
            peak > 0.1,
            "peak {peak} too quiet - resynthesis produced almost nothing"
        );
        assert!(
            peak < 2.0,
            "peak {peak} unexpectedly large - possible runaway gain"
        );
    }
}
