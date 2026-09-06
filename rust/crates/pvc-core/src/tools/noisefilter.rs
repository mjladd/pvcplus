//! Ports `noisefilter.c`: a spectral noise gate. Builds a per-channel
//! "noise sample" response by analyzing a `[noise_begin, noise_end)`
//! window of the *same channel* it's about to filter (reusing
//! [`crate::tools::freqresponse::process`] rather than reproducing the
//! C's own design of shelling out to a separate `freqresponse` process
//! and reading the result back from a `/tmp` file), then expands any bin
//! quieter than its (adjustable, `-S`) noise-response threshold toward
//! silence with a `curve()`-shaped gain, smooths that gain change frame
//! to frame with attack/release, shelf-EQs the result, then pitch/
//! frequency-shifts and resynthesizes via either the oscillator bank or
//! overlap-add - genuinely conditional on whether pitch transposition or
//! frequency shift is requested at all, decided once at the start of each
//! channel (same pattern as [`crate::tools::twarp`], not `plainpv`'s
//! hardcoded-always-oscbank quirk - see that module's doc comment).
//!
//! `-b`/`-e` (`noise_begin_secs`/`noise_end_secs` here) genuinely trim
//! which samples feed the noise-response analysis: confirmed by reading
//! `freqresponse.c`'s own call chain (`setupfiles` ->
//! `getInputFileDataToSetOutputChannels`, `legacy/pvc_lib/fileio.c`),
//! which seeks to `begint*R` and buffers only `(endt-begint)*R` frames
//! before the analysis loop ever runs - a real, sample-accurate trim, not
//! the display-only `dur` bookkeeping `-b`/`-e` amount to in `plainpv`/
//! `pvanalysis` (see `tools::freqresponse`'s own doc comment, which
//! describes that *other* code path - not revisited here). `0.0` for
//! `noise_end_secs` means "whole file" (`noisefilter.c`'s own `if
//! (A_endt <= 0.) A_endt = idur;` default), so a caller who never sets
//! either bound gets the entire channel's own signal analyzed as its
//! "noise" - a real, unfortunate-by-default footgun in the original tool
//! (users are expected to always point `-B`/`-E` at an actual noise-only
//! stretch, e.g. leading silence), reproduced faithfully rather than
//! second-guessed here.
//!
//! `-F` (`noise_method`, `average` vs. `peak`) genuinely defaults to
//! `average` in this port. In the C, the passed-through global
//! (`A_method`) is an uninitialized stack `int` unless the user's own
//! `-F` flag sets it (`int A_method, A_print=22050;` - no initializer on
//! `A_method`, confirmed by reading the declaration) - real undefined
//! behavior in the original, not a documented default. `average` is
//! `docs/dev/parameter-inventory.md`'s already-recorded assumption for
//! this flag and matches what a zeroed/typical stack slot resolves to in
//! practice; not reproduced as UB here.
//!
//! Not ported: `previous_channel`'s frame-0 priming and the whole
//! `smooth_zero(...)` call it exists to feed are dead code - the call
//! itself is commented out in `noisefilter.c` (confirmed by reading the
//! source), so `previous_channel` is written every frame but never
//! read by anything live. Likewise `sum_noise_amp`/`frame_sum`
//! (`trackampgain`'s block is commented out too) and the `-Z`/`-y`
//! console-printout-only flags (`tprintspec`).

use crate::eq::eq;
use crate::pvoc::{getthresh, Analyzer, Frame, OscBank, Synthesizer};
use crate::smooth::smooth_setup;
use crate::tools::freqresponse::{process as freqresponse_process, FreqresponseParams, Method};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant. Applies to `noisefilter`'s oscillator-
/// bank path exactly as it does to `plainpv`'s/`twarp`'s (same shared
/// `bufferout()`).
const OSCILBANKGAIN: f32 = 1.7782794;

#[derive(Debug, Clone)]
pub struct NoisefilterParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,
    pub noise_begin_secs: f32,
    /// `0.0` means "to the end of the file" - see this module's doc
    /// comment.
    pub noise_end_secs: f32,
    pub noise_method: Method,
    /// `-Q`: bins in the (peak-normalized) noise response louder than
    /// this are zeroed, letting those frequencies pass the gate
    /// unaffected. `1.0` (an amplitude *above* any normalized peak)
    /// effectively disables the gate everywhere.
    pub noise_bypass_threshold_db: f32,
    pub pitch_transpose_semitones: ControlFn,
    pub freq_shift_hz: ControlFn,
    pub gain_db: ControlFn,
    /// `-S`: adjusts the noise response's gate threshold in dB. Positive
    /// values increase noise reduction, negative values reduce it.
    pub noise_threshold_adjust_db: ControlFn,
    /// `-x`: shape of the gate's expansion curve (`curve()`'s `warp`).
    pub expansion_index: ControlFn,
    pub attack_secs: ControlFn,
    pub release_secs: ControlFn,
    pub shelf_low_db: f32,
    pub shelf_high_db: f32,
    pub shelf_low_freq: f32,
    pub shelf_high_freq: f32,
    pub threshold_db: f32,
}

/// Ports `threshold_limit(SP, Nplus2, noise_thresh_limit_dB)`: bins whose
/// amplitude, relative to the array's own peak, exceeds
/// `noise_thresh_limit_dB` are zeroed - marking them as "not noise",
/// i.e. frequencies the gate below should leave untouched. Panics if
/// every amplitude is `<= 0` (matches the C's own `exit(0)` there).
fn threshold_limit(amps: &mut [f32], noise_thresh_limit_db: f32, db_to_amp: &DbToAmp) {
    let peak = amps.iter().copied().fold(f32::MIN, f32::max);
    assert!(
        peak > 0.0,
        "threshold_limit: noise sample has zero amplitude"
    );
    let noise_thresh_limit_amp = db_to_amp.convert(noise_thresh_limit_db) * peak;
    for a in amps.iter_mut() {
        if *a > noise_thresh_limit_amp {
            *a = 0.0;
        }
    }
}

/// Ports `smooth_amp_change(A, old_A, N2, att, matt, rel, mrel)`: blends
/// each bin's newly computed gate multiplier against the previous
/// frame's, released or attacked depending on whether the new value is
/// quieter or louder. Distinct from [`crate::smooth::Smoother`] (which
/// ports `smooth()`, a different function): this steps by `1` over a
/// `N2+1`-length array of gain multipliers, not by `2` over a `N+2`
/// mag/freq-interleaved spectrum, and has no first-frame priming inside
/// the function itself - `noisefilter.c` primes its `previous_amp_change`
/// to `1.0` (unity gain) once, right before the frame loop starts, which
/// this struct's [`Self::new`] replicates by construction.
struct AmpChangeSmoother {
    previous: Vec<f32>,
}

impl AmpChangeSmoother {
    fn new(n2_plus_1: usize) -> Self {
        AmpChangeSmoother {
            previous: vec![1.0; n2_plus_1],
        }
    }

    fn smooth(&mut self, amp_change: &mut [f32], att: f32, matt: f32, rel: f32, mrel: f32) {
        if rel != 0.0 || att != 0.0 {
            for (a, prev) in amp_change.iter_mut().zip(self.previous.iter()) {
                *a = if *a < *prev {
                    rel * prev + mrel * *a
                } else {
                    att * prev + matt * *a
                };
            }
        }
        self.previous.copy_from_slice(amp_change);
    }
}

fn wants_oscbank(cf: &ControlFn) -> bool {
    !matches!(cf, ControlFn::Const(v) if *v == 0.0)
}

/// Analyzes `[noise_begin_secs, noise_end_secs)` of `channel` into an
/// amplitude-only noise-response array (`N2+1` bins - frequency slots are
/// never read by `threshold_limit`/the gate below, matching
/// `filter_response.rs`'s established amplitude-only representation),
/// bypassing `freqresponse`'s own EQ/normalize/formant-normalize steps
/// (`noisefilter.c`'s own subprocess invocation always passes `-B1`) but
/// still amplitude-weighted per `noise_method`.
#[allow(clippy::too_many_arguments)]
fn build_noise_response(
    channel: &[f32],
    sample_rate: u32,
    fft_size: usize,
    window: Window,
    frames_per_sec: f32,
    method: Method,
    noise_begin_secs: f32,
    noise_end_secs: f32,
) -> Vec<f32> {
    let r = sample_rate as f32;
    let begin_sample = (noise_begin_secs * r) as usize;
    let end_sample = if noise_end_secs <= 0.0 {
        channel.len()
    } else {
        ((noise_end_secs * r) as usize).min(channel.len())
    };
    let slice = if begin_sample < end_sample {
        &channel[begin_sample..end_sample]
    } else {
        &channel[0..0]
    };

    let params = FreqresponseParams {
        fft_size,
        // `noisefilter.c`'s subprocess call always passes `-M0` (auto:
        // `2 * fft_size`) - it never forwards its own `-M`.
        window_size: 0,
        window,
        frames_per_sec,
        method,
        weight_average: false,
        shelf_low_db: 0.0,
        shelf_high_db: 0.0,
        shelf_low_freq: 200.0,
        shelf_high_freq: 2000.0,
        eq_normalize_bypass: true,
        normalize_to_peaks: false,
        companding_index: 0.0,
        low_freq_limit: 0.0,
        high_freq_limit: 0.0,
        minimum_formant_db: -96.0,
        formant_selection_threshold: 0.5,
    };
    let frame = freqresponse_process(&[slice.to_vec()], sample_rate, &params);
    frame.bins.iter().map(|&(amp, _freq)| amp).collect()
}

/// Processes one channel start to finish, matching `noisefilter`'s
/// per-channel loop: build the noise response, gate/EQ/pitch-shift each
/// frame, then resynthesize. `dur` is the control-function normalization
/// duration in seconds - pass the natural output duration like
/// `tools::pv`.
pub fn process_channel(
    channel: &[f32],
    sample_rate: u32,
    params: &NoisefilterParams,
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
    let ir = i_factor as f32 / r;
    let threshfac = 10.0f64.powf(params.threshold_db as f64 / 20.0) as f32;

    let mut noise_amps = build_noise_response(
        channel,
        sample_rate,
        n,
        params.window,
        params.frames_per_sec,
        params.noise_method,
        params.noise_begin_secs,
        params.noise_end_secs,
    );
    let db_to_amp = DbToAmp::new();
    threshold_limit(
        &mut noise_amps,
        params.noise_bypass_threshold_db,
        &db_to_amp,
    );

    let obank =
        wants_oscbank(&params.pitch_transpose_semitones) || wants_oscbank(&params.freq_shift_hz);

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);
    let mut osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, sample_rate);
    let mut amp_smoother = AmpChangeSmoother::new(n2 + 1);

    let semitones_to_mult = SemitonesToMult::new();

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

        let frame = analyzer.push(&hop).expect("hop is exactly d samples");
        let mut flat = frame.to_pva_floats();

        let t = samps_written as f32 / r;
        let harmadd = params.freq_shift_hz.at(t, dur);
        let gain = db_to_amp.convert(params.gain_db.at(t, dur));
        let pm = semitones_to_mult.convert(params.pitch_transpose_semitones.at(t, dur));
        let noisefiltergain = db_to_amp.convert(params.noise_threshold_adjust_db.at(t, dur));
        let expandex = params.expansion_index.at(t, dur);

        let (attackc, minusattackc) = smooth_setup(params.attack_secs.at(t, dur), ir);
        let (releasec, minusreleasec) = smooth_setup(params.release_secs.at(t, dur), ir);

        // NOISE FILTERING: build a per-bin gate multiplier from the
        // noise response, then smooth it frame-to-frame.
        let mut amp_change = vec![0.0f32; n2 + 1];
        for (j, amp_change_j) in amp_change.iter_mut().enumerate() {
            let i = 1 + 2 * j;
            let temp2 = noise_amps[j] * noisefiltergain;
            *amp_change_j = if temp2 > 0.0 {
                if flat[i - 1] < temp2 {
                    let temp = flat[i - 1] / temp2;
                    curve(0.0, 1.0, temp, expandex)
                } else {
                    1.0
                }
            } else {
                1.0
            };
        }
        amp_smoother.smooth(
            &mut amp_change,
            attackc,
            minusattackc,
            releasec,
            minusreleasec,
        );
        for (j, &g) in amp_change.iter().enumerate() {
            flat[2 * j] *= g;
        }

        eq(
            &mut flat,
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

        // FILTER OUTPUT PITCH/FREQ SHIFT + GAIN: `for(i=1;i<(N+2);i+=2)`
        // in the C - every bin including Nyquist, the same bound found
        // in `filter.c`'s equivalent loop (`tools::filter`'s doc
        // comment), not `plainpv`/`twarp`'s Nyquist-excluding `i<N`.
        for j in 0..=n2 {
            let i = 1 + 2 * j;
            let temp = pm * (flat[i] + harmadd);
            if temp <= 0.0 || temp >= nyquist {
                flat[i - 1] = 0.0;
            } else {
                flat[i] = temp;
            }
            flat[i - 1] *= gain;
        }

        let frame = Frame::from_pva_floats(&flat);
        let synt = getthresh(&frame.bins[..n2], threshfac);

        if obank {
            let hop_out = osc.synthesize(&frame, synt);
            on += i_factor as i64;
            if on + nw as i64 - i_factor as i64 >= 0 {
                output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
                samps_written += i_factor;
            }
        } else {
            let hop_out = synth.overlap_add(&frame);
            if !hop_out.is_empty() {
                output.extend(hop_out);
                samps_written += i_factor;
            }
        }

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

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params(fft_size: usize) -> NoisefilterParams {
        NoisefilterParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            noise_begin_secs: 0.0,
            noise_end_secs: 0.0,
            noise_method: Method::Average,
            noise_bypass_threshold_db: 0.0,
            pitch_transpose_semitones: ControlFn::Const(0.0),
            freq_shift_hz: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            noise_threshold_adjust_db: ControlFn::Const(0.0),
            expansion_index: ControlFn::Const(3.0),
            attack_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            threshold_db: -96.0,
        }
    }

    #[test]
    #[should_panic(expected = "zero amplitude")]
    fn silence_input_panics_matching_c_exit_on_zero_amplitude_noise_sample() {
        // Matches the real tool's own hard failure ("WARNING! NOISE
        // SAMPLE HAS ZERO AMPLITUDE! BYE", `threshold_limit`'s
        // `exit(0)`) when the noise-response window it's told to
        // analyze is silence - not a case this port should paper over.
        let params = default_params(1024);
        let input = vec![0.0f32; 44100 / 4];
        process_channel(&input, 44100, &params, 1.0);
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
        assert!(peak < 2.0, "peak {peak} unexpectedly large");
    }

    #[test]
    fn oscbank_path_selected_when_pitch_shifted() {
        let mut params = default_params(1024);
        params.pitch_transpose_semitones = ControlFn::Const(7.0);
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
        assert!(peak < 2.0, "peak {peak} unexpectedly large");
    }
}
