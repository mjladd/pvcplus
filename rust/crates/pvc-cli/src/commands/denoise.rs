//! `pvc denoise`: reads audio, runs each channel through
//! `pvc_core::tools::noisefilter::process_channel`, writes the result.

use anyhow::{Context, Result};
use pvc_core::tools::noisefilter::{process_channel, NoisefilterParams};

use crate::cli::DenoiseArgs;

pub fn run(args: &DenoiseArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let params = NoisefilterParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.stretch,
        noise_begin_secs: args.noise_begin,
        noise_end_secs: args.noise_end,
        noise_method: args.noise_method,
        noise_bypass_threshold_db: args.noise_bypass_threshold,
        pitch_transpose_semitones: args.pitch.clone(),
        freq_shift_hz: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        noise_threshold_adjust_db: args.noise_threshold_gain.clone(),
        expansion_index: args.expansion_index.clone(),
        attack_secs: args.attack.clone(),
        release_secs: args.release.clone(),
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        threshold_db: args.threshold,
    };

    let mut out_channels = Vec::with_capacity(audio.channels.len());
    for channel in &audio.channels {
        let dur = (channel.len() as f32 / audio.sample_rate as f32) * params.time_factor;
        out_channels.push(process_channel(channel, audio.sample_rate, &params, dur));
    }

    // Channels can come out slightly different lengths (each is an
    // independent per-channel loop) - trim to the shortest, same as
    // `pvc pv`.
    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // `noisefilter`'s default `rescalev == 1`: the same whole-file
    // peak-rescale post-pass already found for `pv`/`twarp`/`filter` (see
    // `commands::pv::run_pv`'s doc comment for the detail).
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
