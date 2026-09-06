//! Ports `spectwarper.c`: a per-bin dynamics processor, distinct from
//! `compander.c` (ported separately as `pvc compand`) in that it
//! compands each bin against *its own frame's live spectral peak*
//! (a self-referential envelope-follower) rather than a separately
//! loaded, static peaks file - there is no `-F`/response-file flag here
//! at all (`-F` appears in the C's `crack()` accept string but has no
//! `case` in its switch - dead, like `-h`, not ported). The peak
//! reference is either one global peak over the whole companding band
//! (`-S`/`compress_window <= 0`, the default) or a per-bin local-window
//! peak (`-S > 0`) - both re-derived fresh every frame from the current
//! input, then attack/release-smoothed via
//! [`crate::smooth::smooth_one_value`] (a different, single-scalar
//! primitive from [`crate::smooth::Smoother`]/`smooth_setup`'s
//! whole-array use elsewhere in this crate).
//!
//! **Real bug, reproduced faithfully, not fixed:** the per-bin local
//! peak window's upper bound clamp (`spectwarper.c`, both the `<=8`-
//! octave and `>8`-Hz branches) reads `if (hib < N) hib = N - 1;` -
//! backwards from what a bounds clamp should be (`if (hib > N) ...`).
//! Since `hib` is almost always much smaller than `N` for any
//! realistic per-bin window, this condition is true on nearly every
//! call, so the "local" peak search actually spans `[lowb, N-1)` -
//! nearly the *entire* spectrum - for nearly every bin, defeating the
//! sliding-window design `-S > 0` is meant to provide. Confirmed by
//! reading the whole function twice (identical bug, both branches);
//! reproduced here exactly rather than "fixed" to a real upper clamp,
//! since golden-harness parity requires matching the real tool's actual
//! (buggy) behavior.
//!
//! Compression/expansion math is entirely in the dB domain
//! (`normampdB` vs. `compthresh_db`/`expandthresh_db`, both compared and
//! shaped via [`crate::warp::curve`] before a single `dB_to_amp` at the
//! end) - unlike `compander.c`, which precomputes amplitude-domain
//! thresholds up front. `compthreshamp`/`compamp`/`expandamp`-style
//! precomputed amplitude values exist in the C but are dead (confirmed
//! by reading - the live comparisons and gradient math all use the raw
//! dB `ControlFn` values directly), so this port skips them entirely.
//!
//! `getthresh` is called as `getthresh(channel, N, threshfac)` here -
//! `N`, not `N+2` like `compander.c`'s call - so `bins[..n2]` (excluding
//! Nyquist) is the right slice, matching `plainpv`/`twarp`'s convention.
//! Confirmed independently from `spectwarper.c`'s own source rather than
//! assumed from `compander.c`'s sibling call one file over, which passes
//! `N+2` instead - the two tools disagree on this despite otherwise
//! near-identical structure, underscoring why this project re-derives
//! the bound at every call site rather than pattern-matching a sibling.
//!
//! Not ported (dead in the C, confirmed by reading): `previous_channel`
//! (written on frame 0, never read).

use crate::eq::{eq2, ShelfEq};
use crate::pvoc::{getthresh, Analyzer, Frame, OscBank, Synthesizer};
use crate::smooth::{smooth_one_value, smooth_setup};
use crate::units::{DbToAmp, SemitonesToMult};
use crate::warp::curve;
use crate::window::{make_windows, Window};
use crate::ControlFn;

/// `legacy/pvc_lib/fileio.c`'s `OSCILBANKGAIN` - see `tools::pv`'s doc
/// comment on the same constant.
const OSCILBANKGAIN: f32 = 1.7782794;

#[derive(Debug, Clone)]
pub struct SpectwarpParams {
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
    /// `-W`: shape exponent for the compress/expand gradient curve
    /// (`curve(0, 1, gradient, warp_curve_index)`), *not* an
    /// expand/compress direction selector - that's controlled entirely
    /// by `-o`/`-O`/`-q`/`-Q`.
    pub warp_curve_index: ControlFn,
    /// `-r`: smoothing time (seconds) for the per-bin amplitude-change
    /// multiplier - a plain lerp against the previous frame's value, not
    /// attack/release-conditional (distinct from `attack_secs`/
    /// `release_secs`, which smooth the *peak reference* itself).
    pub compander_response_secs: ControlFn,
    /// `-g`: proportion (`0..1`) of the unmodified source blended back
    /// into the companded result.
    pub complement: ControlFn,
    /// `-S`: sliding compression window size - `<= 0` (the default)
    /// means one global peak for the whole band; `<= 8` is octaves,
    /// `> 8` is Hz (see this module's doc comment on the real bound bug
    /// this triggers for local windows).
    pub compress_window: ControlFn,
    pub attack_secs: ControlFn,
    pub release_secs: ControlFn,
    /// `-n`: per-frame amplitude normalization limit in dB (`0` - the
    /// default - disables normalization entirely, since `dB_to_amp(0.0)
    /// == 1.0` and the C's own gate is `frameNormalizationAmpLimit !=
    /// 1.0`).
    pub normalize_limit_db: ControlFn,
    pub shelf_low_db: ControlFn,
    pub shelf_high_db: ControlFn,
    pub shelf_low_freq: ControlFn,
    pub shelf_high_freq: ControlFn,
    pub threshold_db: f32,
}

fn wants_oscbank(cf: &ControlFn) -> bool {
    !matches!(cf, ControlFn::Const(v) if *v == 0.0)
}

/// Processes one channel start to finish. `dur` is the control-function
/// normalization duration in seconds, matching `tools::pv`.
pub fn process_channel(
    channel: &[f32],
    sample_rate: u32,
    params: &SpectwarpParams,
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

    // Frame-0-primed state (`spectwarper.c:522-534`).
    let mut previous_change = vec![1.0f32; n2 + 1];
    let mut old_peakamp = 0.0f32;
    let mut old_peakamps = vec![0.0f32; n + 2];

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
        let mut temp = flat.clone();
        let mut channel_freqdev = vec![0.0f32; n + 2];
        for k in (1..n).step_by(2) {
            channel_freqdev[k] = 1.0;
        }

        let t = samps_written as f32 / r;
        let harmadd = params.freq_shift_hz.at(t, dur);
        let gain = db_to_amp.convert(params.gain_db.at(t, dur));
        let pm = semitones_to_mult.convert(params.pitch_transpose_semitones.at(t, dur));

        let (envattack, minusattack) = smooth_setup(params.attack_secs.at(t, dur), ir);
        let (envrelease, minusrelease) = smooth_setup(params.release_secs.at(t, dur), ir);
        let compthresh_db = params.comp_threshold_db.at(t, dur);
        let compdb = params.comp_amount_db.at(t, dur);
        let mut expandthresh_db = params.expand_threshold_db.at(t, dur);
        if expandthresh_db < -95.0 {
            expandthresh_db = -95.0;
        }
        let expanddb = params.expand_amount_db.at(t, dur);
        let warpcurve_index = params.warp_curve_index.at(t, dur);
        let (c_response, minusc_response) =
            smooth_setup(params.compander_response_secs.at(t, dur), ir);
        let complementprop = params.complement.at(t, dur);
        let frame_norm_amp_limit = db_to_amp.convert(params.normalize_limit_db.at(t, dur));

        let octaves_rolloff = params.band_rolloff_octaves.at(t, dur);
        assert!(
            octaves_rolloff >= 0.0,
            "spectwarp: band rolloff must be >= 0, got {octaves_rolloff}"
        );
        let octrollmult = 2.0f32.powf(octaves_rolloff);
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
        let mut lowbin = (((temp1 * (1.0 / octrollmult)) / freqdiff) as i64) * 2 + 1;
        let mut hibin = (((temp2 * octrollmult) / freqdiff) as i64) * 2 + 1;
        if hibin > n as i64 {
            hibin = n as i64;
        }
        if lowbin < 0 {
            lowbin = 1;
        }

        let winsize = params.compress_window.at(t, dur);

        let global_peakamp = if winsize <= 0.0 {
            let mut peak = f32::MIN;
            let mut i = lowbin;
            while i < hibin {
                if flat[(i - 1) as usize] > peak {
                    peak = flat[(i - 1) as usize];
                }
                i += 2;
            }
            let smoothed = smooth_one_value(
                peak,
                old_peakamp,
                envattack,
                minusattack,
                envrelease,
                minusrelease,
            );
            old_peakamp = smoothed;
            smoothed
        } else {
            0.0
        };

        let mut i = lowbin;
        while i < hibin {
            let peakamp = if winsize > 0.0 {
                let bin_freq = ((i - 1) / 2) as f32 * fundamental;
                let (upper_freq, lower_freq) = if winsize <= 8.0 {
                    let mult = 2.0f32.powf(winsize * 0.5);
                    (bin_freq * mult, bin_freq / mult)
                } else {
                    let half = winsize * 0.5;
                    (bin_freq + half, bin_freq - half)
                };
                let mut hib = 2 * (((upper_freq / fundamental) + 0.5) as i64);
                let mut lowb = 2 * (((lower_freq / fundamental) + 0.5) as i64);
                if lowb < 1 {
                    lowb = 0;
                }
                // Real bug, reproduced faithfully - see this module's
                // doc comment.
                if hib < n as i64 {
                    hib = n as i64 - 1;
                }
                // Not reproduced: for bins near Nyquist with a wide
                // (especially octave-mode, `mult > 1`) window, the
                // *un*-clamped `hib` (the buggy clamp above only ever
                // lowers it, never raises it) can genuinely exceed the
                // array's real size (`N+2`) - a real out-of-bounds read
                // in the C (undefined behavior, silently returns
                // adjacent heap memory as "peak"), not just a
                // theoretical edge case (triggered by this module's own
                // `sliding_window_compression_runs_without_panicking`
                // test). Same class of bug as `filter_response::
                // smooth_response`'s documented OOB read - clamped to
                // the last valid index here instead of reproducing it.
                hib = hib.min(n as i64 + 2);

                let mut peak = f32::MIN;
                let mut ii = lowb;
                while ii < hib {
                    if flat[ii as usize] > peak {
                        peak = flat[ii as usize];
                    }
                    ii += 2;
                }
                let smoothed = smooth_one_value(
                    peak,
                    old_peakamps[i as usize],
                    envattack,
                    minusattack,
                    envrelease,
                    minusrelease,
                );
                old_peakamps[i as usize] = smoothed;
                smoothed
            } else {
                global_peakamp
            };

            if peakamp > 0.0 && flat[(i - 1) as usize] > 0.0 {
                let normamp = flat[(i - 1) as usize] / peakamp;
                let normampdb = (20.0 * (normamp as f64).log10()) as f32;
                if normampdb >= -96.0 {
                    let bin = ((i - 1) / 2) as usize;
                    let (mult, clamp_to_zero) = if normampdb > compthresh_db {
                        if compdb < 0.0 {
                            let gradient = (normampdb - compthresh_db) / compthresh_db.abs();
                            let gradient = curve(0.0, 1.0, gradient, warpcurve_index);
                            (db_to_amp.convert(compdb * gradient), false)
                        } else {
                            (1.0, false)
                        }
                    } else if normampdb < expandthresh_db {
                        if expanddb < 0.0 {
                            let gradient =
                                (normampdb - expandthresh_db) / (-96.0 - expandthresh_db);
                            let gradient = curve(0.0, 1.0, gradient, warpcurve_index);
                            (db_to_amp.convert(expanddb * gradient), true)
                        } else {
                            (1.0, false)
                        }
                    } else {
                        (1.0, false)
                    };

                    let smoothed_mult = minusc_response * mult + c_response * previous_change[bin];
                    previous_change[bin] = smoothed_mult;
                    let mut temp2_amp = flat[(i - 1) as usize] * smoothed_mult;
                    if clamp_to_zero && temp2_amp < 0.0 {
                        temp2_amp = 0.0;
                    }

                    if i < lowcutbin {
                        temp[(i - 1) as usize] = flat[(i - 1) as usize]
                            + (temp2_amp - flat[(i - 1) as usize])
                                * ((i - lowbin) as f32 / (lowcutbin - lowbin) as f32);
                    } else if i > hicutbin {
                        temp[(i - 1) as usize] = flat[(i - 1) as usize]
                            + (temp2_amp - flat[(i - 1) as usize])
                                * ((i - hibin) as f32 / (hicutbin - hibin) as f32);
                    } else {
                        temp[(i - 1) as usize] = temp2_amp;
                    }
                }
            }
            i += 2;
        }

        // SOURCE/COMPLEMENT MIX - amp slots only.
        for j in 0..=n2 {
            let idx = 2 * j;
            temp[idx] += complementprop * (flat[idx] - 2.0 * temp[idx]);
        }

        // FRAME NORMALIZATION.
        let mut channel_amp_sum = 0.0f32;
        let mut temp_amp_sum = 0.0f32;
        for j in 0..=n2 {
            channel_amp_sum += flat[2 * j];
            temp_amp_sum += temp[2 * j];
        }
        if temp_amp_sum > 0.0 && frame_norm_amp_limit != 1.0 {
            let mut normalization_amp = channel_amp_sum / temp_amp_sum;
            if normalization_amp > frame_norm_amp_limit {
                normalization_amp = frame_norm_amp_limit;
            }
            for j in 0..=n2 {
                flat[2 * j] = temp[2 * j] * normalization_amp;
            }
        } else {
            for j in 0..=n2 {
                flat[2 * j] = temp[2 * j];
            }
        }

        // PITCH/FREQ SHIFT + GAIN: full spectrum, every bin including
        // Nyquist.
        for j in 0..=n2 {
            let idx = 1 + 2 * j;
            channel_freqdev[idx] *= pm;
            channel_freqdev[idx - 1] += harmadd;

            let tmp = pm * (flat[idx] + harmadd);
            if tmp <= 0.0 || tmp >= nyquist {
                flat[idx - 1] = 0.0;
            } else {
                flat[idx] = tmp;
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

        let frame_out = Frame::from_pva_floats(&flat);
        // `getthresh(channel, N, threshfac)` in the C - `N`, not `N+2`
        // (see this module's doc comment).
        let synt = getthresh(&frame_out.bins[..n2], threshfac);

        if obank {
            let hop_out = osc.synthesize(&frame_out, synt);
            on += i_factor as i64;
            if on + nw as i64 - i_factor as i64 >= 0 {
                output.extend(hop_out.iter().map(|&s| s * OSCILBANKGAIN));
                samps_written += i_factor;
            }
        } else {
            let hop_out = synth.overlap_add(&frame_out);
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

    fn default_params(fft_size: usize) -> SpectwarpParams {
        SpectwarpParams {
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
            warp_curve_index: ControlFn::Const(0.0),
            compander_response_secs: ControlFn::Const(0.0),
            complement: ControlFn::Const(0.0),
            compress_window: ControlFn::Const(0.0),
            attack_secs: ControlFn::Const(0.0),
            release_secs: ControlFn::Const(0.0),
            normalize_limit_db: ControlFn::Const(0.0),
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

    #[test]
    fn sliding_window_compression_runs_without_panicking() {
        let mut params = default_params(1024);
        params.compress_window = ControlFn::Const(2.0);
        params.comp_threshold_db = ControlFn::Const(-20.0);
        params.comp_amount_db = ControlFn::Const(-10.0);
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
