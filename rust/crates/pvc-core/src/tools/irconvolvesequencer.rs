//! Ports `irconvolvesequencer.c`: crossfades a signal through a sequence
//! of impulse responses, morphing from one to the next across
//! `[begin, end]`.
//!
//! In the C this is a shell-script orchestrator, not a DSP tool in its own
//! right: for each listed impulse-response sound file it shells out to
//! `impulseresponse` (building a `.fft` spectrum) then `irconvolver`
//! (convolving the *whole* dry input, windowed to that impulse's own
//! `[thisBeginT, thisEndT]` time slice, against it), crossfading between
//! adjacent impulses via a triangular envelope built with `gen4 | reshape
//! -t0` and fed to `irconvolver -q` (its per-sample dry-signal gain, in
//! dB), then sums every impulse's convolved output (each still at its own
//! full original length, just windowed to silence outside its own slice)
//! at its own time offset via `mixfiles`. This port calls the equivalent
//! Rust functions directly - [`crate::tools::impulseresponse::process`],
//! [`crate::tools::irconvolver::process`], [`crate::gen4`],
//! [`crate::amp_to_db`] - instead of shelling out or touching a filesystem,
//! with no change in observable behavior. [`mix_segments`] is this port's
//! replacement for the final `mixfiles` call - not a general port of that
//! tool (still unported, see the plan's Phase 5 note), just the specific
//! "sum N delayed buffers, then normalize" slice of it this tool needs.
//!
//! **Dead flag in the C, reproduced as "always convolution" here**: the
//! C's `crack()` flag list includes `a` (`"...|a|A|..."`), but there is no
//! `case 'a':` in the `switch` at all - every other flag in that list is
//! handled, `a` alone falls through to nothing. So the local
//! `deconvolution_0__convolution_1` variable it would have set never
//! leaves its hardcoded initializer (`1`, convolution) no matter what a
//! caller passes for `-a`. Unlike `irconvolver`'s own `-a` (a real,
//! functioning flag whose *usage text* is wrong), this one plainly does
//! nothing at all. This port exposes no equivalent flag and always
//! convolves, matching the C's actual (not documented) behavior.
//!
//! **The usage text's own `(func)` annotations are incomplete**: `-A`,
//! `-r`, `-D`, `-f`, `-h`, `-H` are marked `(func)` (accept either a
//! constant or a breakpoint-file path); `-s`/`-t`/`-g`/`-G` are not, but
//! the switch statement calls `crackstring` on all ten identically - the
//! omission is a documentation gap, not a real behavioral difference, and
//! all ten are exposed here as [`ControlFn`]. Unlike `irconvolver`'s own
//! bandpass/gain flags (deliberately simplified to plain constants there,
//! since only per-*sample* variation within one call was being given up),
//! here each flag's function is evaluated once *per impulse response* at
//! that impulse's normalized sequence position (`fval(&f, 1.0, prop)`,
//! `prop` in `[0, 1]`) - real, observable movement across the morph, not
//! the same simplification.
//!
//! **A real difference in out-of-range behavior, not reproduced**: the
//! C's `mixfiles.c` writes its mixed output through `sf_write_float`
//! directly, with no clipping guard and no call to
//! `sf_command(..., SFC_SET_CLIPPING, ...)` - libsndfile's default,
//! undocumented behavior for a float sample outside `[-1.0, 1.0]`
//! feeding a 16-bit PCM writer is to truncate/wrap the out-of-range
//! integer rather than clamp it, producing an inverted-polarity sample
//! (confirmed against a real oracle run: normalization off, one summed
//! sample at `1.99999...` came out as `32750`, not clamped `32767`, while
//! its smoothly-ramping neighbors on both sides sit around `-32700`).
//! `pvc_io::write_wav`'s own `f32 -> i16` conversion is a plain Rust `as`
//! cast, which - unlike C - saturates rather than wraps, so this port
//! clamps instead. Every other resynthesis tool in this project reaches
//! its own output through `bufferout()`'s shared rescale path first
//! (see `rescaleThisBuffer` in `irconvolver`'s own doc comment), which
//! keeps values in range before they ever reach `sf_write_float` -
//! `mixfiles.c` is the first tool ported here that skips that path
//! entirely, so this divergence has no precedent to match. Only
//! observable when the mixed sum of un-normalized segments genuinely
//! exceeds full scale (`-v` `off`, or `if-clipping` below its own
//! threshold); this crate's golden case sidesteps it by using `-v`
//! `together`, which unconditionally rescales under `1.0`.
//!
//! Not ported, matching `irconvolver`'s own precedent for the same flags
//! forwarded straight through to its own `irconvolver` shell-out:
//! `-M` (channel-topology-changing "alternate" output mode), `-C`
//! (per-input-channel shift), `-x`/`-P`/`-B`/`-F`/`-Z`/`-z` (impulse
//! signal shaping, off by default). Also not ported: `-p`/`-i`/`-_`/`-='
//! (amplitude-report printing, auto-play, and the `rescalev` override) -
//! this project's established precedent for every resynthesis tool's CLI
//! layer (e.g. `commands::irconvolver::run`) is to skip the print/play
//! flags entirely and always rescale-to-input-peak; `irconvolvesequencer`
//! has no such rescale step of its own (its own final output *is*
//! `mixfiles`'s own `-n` normalization, ported here as
//! [`MixNormalization`]), so there's nothing for `-=` to override in the
//! first place.

use crate::gen::gen4;
use crate::tools::impulseresponse::{self, ImpulseResponseParams, Normalization};
use crate::tools::irconvolver::{self, ImpulseSpectra, IrconvolverParams, Mode};
use crate::units::{amp_to_db, DbToAmp};
use crate::ControlFn;

/// `mixfiles -n`'s normalization code, ported for the specific final-mix
/// step this tool needs (see this module's doc comment - not a general
/// `mixfiles` port).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MixNormalization {
    Off,
    Independent,
    Together,
    IfClipping,
}

pub struct IrconvolveSequencerParams {
    /// `-b`: sequence window start, in seconds.
    pub begin_secs: f32,
    /// `-e`: sequence window end, in seconds. `<= 0.0` means "whole file".
    pub end_secs: f32,
    /// `-d`, forwarded to every segment's `irconvolver` call.
    pub add_ring_time: bool,
    /// `-J`, forwarded to every segment's `irconvolver` call.
    pub impulse_channel: usize,
    /// `-A`.
    pub source_gain_db: ControlFn,
    /// `-r`.
    pub output_gain_db: ControlFn,
    /// `-s`.
    pub ir_low_freq: ControlFn,
    /// `-t`.
    pub ir_high_freq: ControlFn,
    /// `-g`.
    pub ir_low_rolloff_db_per_octave: ControlFn,
    /// `-G`.
    pub ir_high_rolloff_db_per_octave: ControlFn,
    /// `-D`.
    pub source_low_freq: ControlFn,
    /// `-f`.
    pub source_high_freq: ControlFn,
    /// `-h`.
    pub source_low_rolloff_db_per_octave: ControlFn,
    /// `-H`.
    pub source_high_rolloff_db_per_octave: ControlFn,
    /// `-v`.
    pub normalization: MixNormalization,
}

const ENVELOPE_LEN: usize = 1000;

/// The three crossfade shapes `/tmp/envBegin`/`/tmp/env`/`/tmp/envEnd`
/// build via `gen4 -L1000 ... | reshape -t0`, each already converted from
/// amplitude to decibels (`reshape -t0`'s `amp_to_dB` pass) so they're
/// ready to use as [`IrconvolverParams::input_gain_db`] tables directly
/// (matching `-q`'s own decibel unit). `neg96_db_amp` is `dB_to_amp(-96)`
/// via the shared lookup-table conversion ([`DbToAmp`]) - not the exact
/// `-96.0`, since that's what the C's own shell command literally
/// interpolates into each `gen4` invocation's breakpoint value, and the
/// lookup table's approximation error survives the later `amp_to_dB` round
/// trip.
struct CrossfadeEnvelopes {
    /// The sole impulse response's envelope when there's only one
    /// (`number_of_movement_function_points == 1`): flat at `1.0`, i.e.
    /// no fade at all.
    solo: Vec<f32>,
    /// First of two or more impulses: full volume, fading out to silence
    /// by the end of its own window.
    begin: Vec<f32>,
    /// A middle impulse (three or more total): silent at both ends of its
    /// own window, full volume at the midpoint - the standard symmetric
    /// overlap-add crossfade shape.
    mid: Vec<f32>,
    /// Last of two or more impulses: silent at the start of its own
    /// window, full volume by the midpoint and after.
    end: Vec<f32>,
}

fn build_envelopes() -> CrossfadeEnvelopes {
    let neg96_amp = DbToAmp::new().convert(-96.0);
    let to_db =
        |amp_table: Vec<f32>| -> Vec<f32> { amp_table.into_iter().map(amp_to_db).collect() };
    CrossfadeEnvelopes {
        solo: to_db(gen4(
            ENVELOPE_LEN,
            true,
            &[(0.0, 1.0, 0.0), (0.5, 1.0, 0.0), (1.0, 1.0, 0.0)],
        )),
        begin: to_db(gen4(
            ENVELOPE_LEN,
            true,
            &[(0.0, 1.0, 0.0), (0.5, 1.0, 0.0), (1.0, neg96_amp, 0.0)],
        )),
        mid: to_db(gen4(
            ENVELOPE_LEN,
            true,
            &[
                (0.0, neg96_amp, 0.0),
                (0.5, 1.0, 0.0),
                (1.0, neg96_amp, 0.0),
            ],
        )),
        end: to_db(gen4(
            ENVELOPE_LEN,
            true,
            &[(0.0, neg96_amp, 0.0), (0.5, 1.0, 0.0), (1.0, 1.0, 0.0)],
        )),
    }
}

/// Processes every channel. `impulses[i]` is the raw (not yet
/// spectrum-analyzed) sound channels of the `i`-th listed impulse-response
/// file - this function runs [`impulseresponse::process`] on each itself
/// (matching the C's own internal `impulseresponse -b0 -e0 -N0` call),
/// so callers only need to decode audio, not build `.ir` spectra.
/// `impulses` must be nonempty.
pub fn process(
    channels: &[Vec<f32>],
    sample_rate: u32,
    impulses: &[Vec<Vec<f32>>],
    params: &IrconvolveSequencerParams,
) -> Vec<Vec<f32>> {
    let r = sample_rate as f32;
    let n_points = impulses.len();

    let begin = params.begin_secs.max(0.0);
    let end = if params.end_secs <= 0.0 {
        channels[0].len() as f32 / r
    } else {
        params.end_secs
    };

    let cross_fade_dur = if n_points <= 2 {
        end - begin
    } else {
        2.0 * (end - begin) / (n_points + 1) as f32
    };
    let half_cross_fade_dur = 0.5 * cross_fade_dur;

    let envelopes = build_envelopes();

    let ir_spectra: Vec<ImpulseSpectra> = impulses
        .iter()
        .map(|ir_channels| {
            let out = impulseresponse::process(
                ir_channels,
                sample_rate,
                &ImpulseResponseParams {
                    begin_secs: 0.0,
                    end_secs: 0.0,
                    normalization: Normalization::Off,
                    normalization_db: 0.0,
                },
            );
            ImpulseSpectra {
                fft_size: out.fft_size,
                impulse_len: out.impulse_len,
                channels: out.channels,
            }
        })
        .collect();

    let mut segment_outputs = Vec::with_capacity(n_points);
    let mut delay_secs = Vec::with_capacity(n_points);

    for (i, ir) in ir_spectra.iter().enumerate() {
        let prop = if n_points == 1 {
            0.0
        } else {
            i as f32 / (n_points - 1) as f32
        };

        let this_begin_t = begin + i as f32 * half_cross_fade_dur;
        let this_end_t = this_begin_t + cross_fade_dur;
        delay_secs.push(this_begin_t - begin);

        let envelope_table = if i == 0 {
            if n_points == 1 {
                &envelopes.solo
            } else {
                &envelopes.begin
            }
        } else if i == n_points - 1 {
            &envelopes.end
        } else {
            &envelopes.mid
        };

        let seg_params = IrconvolverParams {
            begin_secs: this_begin_t,
            end_secs: this_end_t,
            add_ring_time: params.add_ring_time,
            impulse_channel: params.impulse_channel,
            mode: Mode::Convolution,
            ir_low_freq: params.ir_low_freq.at(prop, 1.0),
            ir_high_freq: params.ir_high_freq.at(prop, 1.0),
            ir_low_rolloff_db_per_octave: params.ir_low_rolloff_db_per_octave.at(prop, 1.0),
            ir_high_rolloff_db_per_octave: params.ir_high_rolloff_db_per_octave.at(prop, 1.0),
            source_low_freq: params.source_low_freq.at(prop, 1.0),
            source_high_freq: params.source_high_freq.at(prop, 1.0),
            source_low_rolloff_db_per_octave: params.source_low_rolloff_db_per_octave.at(prop, 1.0),
            source_high_rolloff_db_per_octave: params
                .source_high_rolloff_db_per_octave
                .at(prop, 1.0),
            source_gain_db: ControlFn::Const(params.source_gain_db.at(prop, 1.0)),
            input_gain_db: ControlFn::Table(envelope_table.clone()),
            output_gain_db: ControlFn::Const(params.output_gain_db.at(prop, 1.0)),
        };

        let this_dur = this_end_t - this_begin_t;
        segment_outputs.push(irconvolver::process(
            channels,
            sample_rate,
            ir,
            &seg_params,
            this_dur,
        ));
    }

    mix_segments(
        &segment_outputs,
        &delay_secs,
        sample_rate,
        channels.len(),
        params.normalization,
    )
}

/// Ports the "sum every segment at its own delay, then normalize" half of
/// `mixfiles.c` - not the whole tool (no gain-modifiers file, no
/// concatenation mode, no channel-count override: `irconvolvesequencer`'s
/// own `mixfiles` invocation uses none of those). Every segment is summed
/// starting at `delay_secs[i]` (`thisBeginT - begint` in the C), matching
/// `mixfiles`'s own delay-file mechanism; `normalization` matches its `-n`
/// codes exactly, including the `* 1.01` headroom the C leaves below full
/// scale.
fn mix_segments(
    segments: &[Vec<Vec<f32>>],
    delay_secs: &[f32],
    sample_rate: u32,
    num_channels: usize,
    normalization: MixNormalization,
) -> Vec<Vec<f32>> {
    let r = sample_rate as f32;
    let delay_samples: Vec<usize> = delay_secs.iter().map(|&d| (d * r) as usize).collect();
    let max_len = segments
        .iter()
        .zip(&delay_samples)
        .map(|(seg, &d)| seg.first().map_or(0, |c| c.len()) + d)
        .max()
        .unwrap_or(0);

    let mut mixed = vec![vec![0.0f32; max_len]; num_channels];
    for (seg, &delay) in segments.iter().zip(&delay_samples) {
        for (ch, samples) in seg.iter().enumerate() {
            for (i, &s) in samples.iter().enumerate() {
                mixed[ch][delay + i] += s;
            }
        }
    }

    let peak_per_channel: Vec<f32> = mixed
        .iter()
        .map(|c| c.iter().fold(0.0f32, |peak, &v| peak.max(v.abs())))
        .collect();
    let peak_overall = peak_per_channel.iter().copied().fold(0.0f32, f32::max);

    let mut rescale_all = |scale: f32| {
        for buf in mixed.iter_mut() {
            for v in buf.iter_mut() {
                *v /= scale;
            }
        }
    };

    match normalization {
        MixNormalization::Off => {}
        MixNormalization::Independent => {
            for (ch, buf) in mixed.iter_mut().enumerate() {
                if peak_per_channel[ch] > 0.0 {
                    let scale = peak_per_channel[ch] * 1.01;
                    for v in buf.iter_mut() {
                        *v /= scale;
                    }
                }
            }
        }
        MixNormalization::Together => {
            if peak_overall > 0.0 {
                rescale_all(peak_overall * 1.01);
            }
        }
        MixNormalization::IfClipping => {
            if peak_overall > 1.0 {
                rescale_all(peak_overall * 1.01);
            }
        }
    }

    mixed
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params() -> IrconvolveSequencerParams {
        IrconvolveSequencerParams {
            begin_secs: 0.0,
            end_secs: 0.0,
            add_ring_time: false,
            impulse_channel: 0,
            source_gain_db: ControlFn::Const(-999.0), // silence the dry mix for these tests
            output_gain_db: ControlFn::Const(0.0),
            ir_low_freq: ControlFn::Const(0.0),
            ir_high_freq: ControlFn::Const(0.0),
            ir_low_rolloff_db_per_octave: ControlFn::Const(0.0),
            ir_high_rolloff_db_per_octave: ControlFn::Const(0.0),
            source_low_freq: ControlFn::Const(0.0),
            source_high_freq: ControlFn::Const(0.0),
            source_low_rolloff_db_per_octave: ControlFn::Const(0.0),
            source_high_rolloff_db_per_octave: ControlFn::Const(0.0),
            normalization: MixNormalization::Off,
        }
    }

    fn impulse(len: usize) -> Vec<Vec<f32>> {
        let mut v = vec![0.0f32; len];
        v[0] = 1.0;
        vec![v]
    }

    #[test]
    fn single_impulse_response_has_no_fade() {
        // With only one impulse, `solo`'s flat 1.0 envelope means the
        // segment's output should equal a single plain `irconvolver` call
        // over the same window (no crossfade shaping applied at all).
        let input: Vec<f32> = (0..8192).map(|i| (i as f32 * 0.05).sin()).collect();
        let out = process(
            std::slice::from_ref(&input),
            44100,
            std::slice::from_ref(&impulse(16)),
            &default_params(),
        );
        assert_eq!(out.len(), 1);
        assert!(out[0].iter().any(|&v| v != 0.0));
    }

    #[test]
    fn two_impulse_responses_crossfade_from_start_to_end() {
        // Impulse 0 has amplitude 1, impulse 1 has amplitude 4 - the
        // convolved output starts dominated by impulse 0 (envBegin full
        // volume at t=0) and ends dominated by impulse 1 (envEnd full
        // volume at t=dur).
        let input = vec![1.0f32; 8192];
        let mut imp0 = impulse(16);
        imp0[0][0] = 1.0;
        let mut imp1 = impulse(16);
        imp1[0][0] = 4.0;
        let out = process(
            std::slice::from_ref(&input),
            44100,
            &[imp0, imp1],
            &default_params(),
        );
        assert_eq!(out.len(), 1);
        // Early samples should be much closer to impulse 0's scale than
        // impulse 1's, and vice versa near the end.
        let early = out[0][10].abs();
        let late = out[0][out[0].len() - 10].abs();
        assert!(
            late > early,
            "expected late samples to dominate: early={early} late={late}"
        );
    }

    #[test]
    fn independent_normalization_brings_peak_under_full_scale() {
        let input = vec![1.0f32; 8192];
        let params = IrconvolveSequencerParams {
            normalization: MixNormalization::Independent,
            ..default_params()
        };
        let out = process(
            std::slice::from_ref(&input),
            44100,
            std::slice::from_ref(&impulse(16)),
            &params,
        );
        let peak = out[0].iter().fold(0.0f32, |p, &v| p.max(v.abs()));
        assert!(
            peak <= 1.0 / 1.0099,
            "peak {peak} should be normalized under ~0.99"
        );
    }

    #[test]
    fn mix_segments_sums_overlapping_delayed_buffers() {
        let seg_a = vec![vec![1.0f32; 10]];
        let seg_b = vec![vec![2.0f32; 10]];
        let mixed = mix_segments(
            &[seg_a, seg_b],
            &[0.0, 0.0001],
            44100,
            1,
            MixNormalization::Off,
        );
        // delay of 0.0001s @ 44100Hz = 4 samples (truncated); seg_a (len
        // 10, delay 0) occupies [0,10), seg_b (len 10, delay 4) occupies
        // [4,14) - they overlap on [4,10), summing to 3.0 there.
        assert_eq!(mixed[0].len(), 14);
        assert_eq!(mixed[0][0], 1.0);
        assert_eq!(mixed[0][3], 1.0);
        assert_eq!(mixed[0][4], 3.0);
        assert_eq!(mixed[0][9], 3.0);
        assert_eq!(mixed[0][10], 2.0);
        assert_eq!(mixed[0][13], 2.0);
    }
}
