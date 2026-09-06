//! Ports `compander.c`: a per-bin dynamics processor that compresses or
//! expands each bin's amplitude relative to a *separately supplied,
//! static* peaks/reference file (`-F`, required - the same raw `N+2`
//! binary-float layout as `filter.c`'s `.fr`, loaded once via
//! `fillfunc()` then smoothed via `smoothspec()` - callers should apply
//! `filter_response::smooth_response` to the loaded amplitude array
//! themselves before calling [`process_channel`], since `-S`/smoothing
//! is a load-time concern, not a per-frame one, and `smoothspec` never
//! touches frequency slots). Distinct from
//! `spectwarper.c` (a separate, self-referential per-frame dynamics
//! processor ported separately as `pvc spectwarp`), which compands
//! against each frame's *own* live spectrum instead of a static file.
//!
//! Compression/expansion only ever applies within `[lowbin, hibin)` - a
//! band computed each frame from `-c`/`-d`/`-f` (`band_low_hz`/
//! `band_high_hz`/`band_rolloff_octaves`), with a linear taper between
//! the rolloff-widened edges (`lowbin`/`hibin`) and the hard band edges
//! (`lowcutbin`/`hicutbin`) - bins outside `[lowbin, hibin)` are left
//! completely untouched, not just unmodified-this-frame: their own
//! `previous_change` entry also stays stale until the band widens back
//! over them (matches `compander.c:592-688`'s loop bounds exactly, not a
//! simplification).
//!
//! **Real bug, reproduced faithfully, not fixed:** the attack/decay
//! smoothing step (`compander.c:645-658`) has *identical* code in both
//! its `ATTACK` and `DECAY` branches - both use `envattack`/`minusattack`
//! unconditionally. `-L`/`release` is therefore a complete no-op in the
//! real tool (fully wired up through `smooth_setup` but its result,
//! `envrelease`/`minusrelease`, is never read anywhere) - confirmed by
//! reading the whole file. Not exposed as a `pvc compand` flag at all,
//! since exposing a provably-dead flag would be misleading.
//!
//! `-H`/`-X`/`-m`/`-R` (shelf EQ) are `crackstring`/func-able here -
//! unlike `noisefilter.c`'s equivalents, which are plain `crackfloat`
//! numbers. Confirmed per-tool from `compander.c`'s own `case` block
//! rather than assumed from a sibling's flag types.
//!
//! Not ported (dead in the C, confirmed by reading): `previous_channel`
//! (written on frame 0, never read - the `smooth()` call it would feed
//! is commented out), `compthreshampInverse`/`normal` (computed, never
//! read), the whole commented-out "FRAME NORMALIZATION" block. The
//! `-W`/`-s`/`-h` letters appear in `compander.c`'s `crack()` accept
//! string but have no `case` in its switch - dead flags in the original,
//! not ported.
//!
//! `getthresh` is called here as `getthresh(channel, N+2, threshfac)` -
//! note `N+2`, not `N` like `plainpv`/`twarp`'s calls - so the *full*
//! bins array (including the Nyquist bin) is passed here, not
//! `bins[..n2]`. Yet another instance of this project's recurring
//! "which N reaches the call site" gotcha (see `pvc-core::formant`/
//! `filter_response`'s doc comments for the others) - re-derived from
//! `compander.c`'s own source rather than assumed from a sibling.

use crate::eq::{eq2, ShelfEq};
use crate::pvoc::{getthresh, Analyzer, Frame, OscBank, Synthesizer};
use crate::smooth::smooth_setup;
use crate::units::{DbToAmp, SemitonesToMult};
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant.
const OSCILBANKGAIN: f32 = 1.7782794;

#[derive(Debug, Clone)]
pub struct CompanderParams {
    pub fft_size: usize,
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub time_factor: f32,
    pub pitch_transpose_semitones: ControlFn,
    pub freq_shift_hz: ControlFn,
    pub gain_db: ControlFn,
    pub comp_threshold_db: ControlFn,
    pub comp_amount_db: ControlFn,
    pub expand_threshold_db: ControlFn,
    pub expand_amount_db: ControlFn,
    pub band_low_hz: ControlFn,
    /// `< 0` means Nyquist (the C's own `hicut.A[0] = -1.` default).
    pub band_high_hz: ControlFn,
    pub band_rolloff_octaves: ControlFn,
    pub attack_secs: ControlFn,
    pub shelf_low_db: ControlFn,
    pub shelf_high_db: ControlFn,
    pub shelf_low_freq: ControlFn,
    pub shelf_high_freq: ControlFn,
    pub threshold_db: f32,
}

fn wants_oscbank(cf: &ControlFn) -> bool {
    !matches!(cf, ControlFn::Const(v) if *v == 0.0)
}

/// Processes one channel against the (already peaks-smoothed) reference
/// amplitude array `peaks_amps` (length `fft_size/2 + 1`, amplitude-only;
/// see this module's doc comment on why frequency slots are never
/// needed). `dur` is the control-function normalization duration in
/// seconds, matching `tools::pv`.
pub fn process_channel(
    channel: &[f32],
    peaks_amps: &[f32],
    sample_rate: u32,
    params: &CompanderParams,
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
    let freqdiff = fundamental;
    let ir = i_factor as f32 / r;
    let threshfac = 10.0f64.powf(params.threshold_db as f64 / 20.0) as f32;

    let obank =
        wants_oscbank(&params.pitch_transpose_semitones) || wants_oscbank(&params.freq_shift_hz);

    let window_pair = make_windows(params.window, nw, n, i_factor);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);
    let mut osc = OscBank::new(n2, nw, sample_rate, i_factor, 1.0);
    let mut synth = Synthesizer::new(n, window_pair.synthesis, i_factor, d, sample_rate);
    // Frame-0-primed to unity gain, matching `compander.c:503-504`'s
    // `previous_change[i]=1.` setup (see `tools::noisefilter`'s
    // `AmpChangeSmoother` for the same pattern).
    let mut previous_change = vec![1.0f32; n2 + 1];

    let db_to_amp = DbToAmp::new();
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
        let mut channel_freqdev = vec![0.0f32; n + 2];
        for k in (1..n).step_by(2) {
            channel_freqdev[k] = 1.0;
        }

        let t = samps_written as f32 / r;
        let harmadd = params.freq_shift_hz.at(t, dur);
        let gain = db_to_amp.convert(params.gain_db.at(t, dur));
        let pm = semitones_to_mult.convert(params.pitch_transpose_semitones.at(t, dur));

        // `-L`/release is dead in the C (see this module's doc comment) -
        // only the attack coefficients are ever used.
        let (envattack, minusattack) = smooth_setup(params.attack_secs.at(t, dur), ir);
        let compthreshamp = db_to_amp.convert(params.comp_threshold_db.at(t, dur));
        let compamp = db_to_amp.convert(params.comp_amount_db.at(t, dur));
        let expandthreshamp = db_to_amp.convert(params.expand_threshold_db.at(t, dur));
        let expandamp = 1.0 / db_to_amp.convert(params.expand_amount_db.at(t, dur));

        let octrollmult = 2.0f32.powf(params.band_rolloff_octaves.at(t, dur));
        let mut temp1 = params.band_low_hz.at(t, dur);
        if temp1 < 0.0 {
            temp1 = 0.0;
        }
        let mut temp2 = params.band_high_hz.at(t, dur);
        if temp2 < 0.0 || temp2 > nyquist {
            temp2 = nyquist;
        }

        let lowcutbin = ((temp1 / freqdiff) as i64) * 2 + 1;
        let hicutbin = ((temp2 / freqdiff) as i64) * 2 + 1;
        let lowbin = (((temp1 * (1.0 / octrollmult)) / freqdiff) as i64) * 2 + 1;
        let mut hibin = (((temp2 * octrollmult) / freqdiff) as i64) * 2 + 1;
        if hibin > n as i64 {
            hibin = n as i64;
        }

        // COMPANDING: `compander.c:592-688`'s `[lowbin, hibin)`-bounded
        // loop, indexed by the flat odd (frequency-slot) index `i` -
        // `previous_change`/`peaks_amps` are addressed by bin number
        // `(i-1)/2` directly, not by a separate per-band counter (the
        // C's own `j` loop variable is incremented but never actually
        // used inside the loop body).
        let mut i = lowbin;
        while i < hibin {
            let bin = ((i - 1) / 2) as usize;
            let amp = flat[(i - 1) as usize];
            let peak = peaks_amps[bin];
            if amp > 0.0 && peak > 0.0 {
                let normamp = amp / peak;
                // `normamp <= 0.` is unreachable given `amp>0.`/`peak>0.`
                // above (the C's own "ZERO AMP" branch is dead code for
                // the same reason) - omitted here.
                let ampchange = if normamp > compthreshamp {
                    if compamp < 1.0 {
                        (compthreshamp + (normamp - compthreshamp) * compamp) / normamp
                    } else {
                        1.0
                    }
                } else if normamp < expandthreshamp {
                    if expandamp > 1.0 {
                        let ac =
                            (expandthreshamp - (expandthreshamp - normamp) * expandamp) / normamp;
                        ac.max(0.0)
                    } else {
                        1.0
                    }
                } else {
                    1.0
                };

                // ATTACK/DECAY smoothing - both branches identical in the
                // C (see this module's doc comment): always the attack
                // coefficients.
                let smoothed = minusattack * ampchange + envattack * previous_change[bin];
                previous_change[bin] = smoothed;
                let newamp = amp * smoothed;

                if i < lowcutbin {
                    flat[(i - 1) as usize] +=
                        (newamp - amp) * ((i - lowbin) as f32 / (lowcutbin - lowbin) as f32);
                } else if i > hicutbin {
                    flat[(i - 1) as usize] +=
                        (newamp - amp) * ((i - hibin) as f32 / (hicutbin - hibin) as f32);
                } else {
                    flat[(i - 1) as usize] = newamp;
                }
            }
            i += 2;
        }

        // FILTER OUTPUT PITCH/FREQ SHIFT + GAIN: full spectrum,
        // `for(i=1;i<(N+2);i+=2)` - every bin including Nyquist.
        for j in 0..=n2 {
            let idx = 1 + 2 * j;
            channel_freqdev[idx] *= pm;
            channel_freqdev[idx - 1] += harmadd;

            let temp = pm * (flat[idx] + harmadd);
            if temp <= 0.0 || temp >= nyquist {
                flat[idx - 1] = 0.0;
            } else {
                flat[idx] = temp;
            }
            flat[idx - 1] *= gain;
        }

        let shelf = ShelfEq {
            d_blow: params.shelf_low_db.at(t, dur),
            d_bhi: params.shelf_high_db.at(t, dur),
            freqlow: params.shelf_low_freq.at(t, dur),
            freqhi: params.shelf_high_freq.at(t, dur),
        };
        eq2(
            &mut flat,
            &shelf,
            fundamental,
            &channel_freqdev,
            false,
            &db_to_amp,
        );

        let frame = Frame::from_pva_floats(&flat);
        // `getthresh(channel, N+2, threshfac)` in the C - the *full*
        // bins array, not `bins[..n2]` (see this module's doc comment).
        let synt = getthresh(&frame.bins, threshfac);

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

    fn default_params(fft_size: usize) -> CompanderParams {
        CompanderParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            time_factor: 1.0,
            pitch_transpose_semitones: ControlFn::Const(0.0),
            freq_shift_hz: ControlFn::Const(0.0),
            gain_db: ControlFn::Const(0.0),
            comp_threshold_db: ControlFn::Const(0.0),
            comp_amount_db: ControlFn::Const(0.0),
            expand_threshold_db: ControlFn::Const(-96.0),
            expand_amount_db: ControlFn::Const(0.0),
            band_low_hz: ControlFn::Const(0.0),
            band_high_hz: ControlFn::Const(-1.0),
            band_rolloff_octaves: ControlFn::Const(0.0),
            attack_secs: ControlFn::Const(0.0),
            shelf_low_db: ControlFn::Const(0.0),
            shelf_high_db: ControlFn::Const(0.0),
            shelf_low_freq: ControlFn::Const(200.0),
            shelf_high_freq: ControlFn::Const(2000.0),
            threshold_db: -96.0,
        }
    }

    #[test]
    fn silence_in_silence_out() {
        let params = default_params(1024);
        let input = vec![0.0f32; 44100 / 4];
        let peaks = vec![1.0f32; 513];
        let output = process_channel(&input, &peaks, 44100, &params, 1.0);
        assert!(!output.is_empty());
        assert!(output.iter().all(|&s| s.abs() < 1e-6));
    }

    #[test]
    fn sine_input_with_flat_peaks_produces_bounded_output() {
        let params = default_params(1024);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let peaks = vec![1.0f32; 513];
        let output = process_channel(&input, &peaks, sample_rate, &params, 1.0);
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
        let peaks = vec![1.0f32; 513];
        let output = process_channel(&input, &peaks, sample_rate, &params, 1.0);
        assert!(!output.is_empty());
        let peak = output.iter().copied().fold(0.0f32, |a, b| a.max(b.abs()));
        assert!(peak > 0.0, "peak {peak} too quiet");
        assert!(peak < 2.0, "peak {peak} unexpectedly large");
    }
}
