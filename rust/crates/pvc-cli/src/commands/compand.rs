//! `pvc compand`: reads audio and a peaks/reference file, runs each
//! channel through `pvc_core::tools::compander::process_channel`, writes
//! the result.

use anyhow::{bail, Context, Result};
use pvc_core::filter_response::smooth_response;
use pvc_core::tools::compander::{process_channel, CompanderParams};

use crate::cli::CompanderArgs;

pub fn run(args: &CompanderArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;
    let (mut peaks_amps, peaks_n) = pvc_io::read_fr_amplitudes(&args.peaks)
        .with_context(|| format!("reading {}", args.peaks.display()))?;
    if peaks_n != args.fft {
        bail!(
            "peaks file {} has FFT size {peaks_n}, but --fft is {} - the real tool requires these \
             to match exactly (it errors rather than adjusting either)",
            args.peaks.display(),
            args.fft
        );
    }
    smooth_response(&mut peaks_amps, args.peaks_smoothing, audio.sample_rate);

    let params = CompanderParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.stretch,
        pitch_transpose_semitones: args.pitch.clone(),
        freq_shift_hz: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        comp_threshold_db: args.compress_threshold.clone(),
        comp_amount_db: args.compress_amount.clone(),
        expand_threshold_db: args.expand_threshold.clone(),
        expand_amount_db: args.expand_amount.clone(),
        band_low_hz: args.band_low.clone(),
        band_high_hz: args.band_high.clone(),
        band_rolloff_octaves: args.band_rolloff.clone(),
        attack_secs: args.attack.clone(),
        shelf_low_db: args.shelf_low_gain.clone(),
        shelf_high_db: args.shelf_high_gain.clone(),
        shelf_low_freq: args.shelf_low_freq.clone(),
        shelf_high_freq: args.shelf_high_freq.clone(),
        threshold_db: args.threshold,
    };

    let mut out_channels = Vec::with_capacity(audio.channels.len());
    for channel in &audio.channels {
        let dur = (channel.len() as f32 / audio.sample_rate as f32) * params.time_factor;
        out_channels.push(process_channel(
            channel,
            &peaks_amps,
            audio.sample_rate,
            &params,
            dur,
        ));
    }

    // Channels can come out slightly different lengths (each is an
    // independent per-channel loop) - trim to the shortest, same as
    // `pvc pv`.
    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // `compander`'s default `rescalev == 1`: the same whole-file
    // peak-rescale post-pass already found for `pv`/`twarp`/`filter`/
    // `denoise` (see `commands::pv::run_pv`'s doc comment for the
    // detail).
    let peak = |chans: &[Vec<f32>]| -> f32 {
        chans
            .iter()
            .flat_map(|c| c.iter())
            .copied()
            .fold(0.0f32, |a, b| a.max(b.abs()))
    };
    let input_peak = peak(&audio.channels);
    let output_peak = peak(&out_channels);
    if input_peak > 0.0 && output_peak > 0.0 {
        let ampval = input_peak / output_peak;
        for c in &mut out_channels {
            for s in c.iter_mut() {
                *s *= ampval;
            }
        }
    }

    let out_buffer = pvc_io::AudioBuffer {
        sample_rate: audio.sample_rate,
        channels: out_channels,
    };
    pvc_io::write_wav(&args.output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", args.output.display()))?;
    Ok(())
}
