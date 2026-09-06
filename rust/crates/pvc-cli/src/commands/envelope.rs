//! `pvc envelope`: reads audio, runs it through
//! `pvc_core::tools::envelope::process`, writes the resulting scalar
//! time-series as ASCII text or a headerless raw f32 stream - never
//! audio.

use anyhow::{Context, Result};
use pvc_core::tools::envelope::{process, EnvelopeParams};

use crate::cli::{EnvelopeArgs, OutputType};

pub fn write_series(path: &std::path::Path, values: &[f32], output_type: OutputType) -> Result<()> {
    match output_type {
        OutputType::Ascii => {
            let mut text = String::with_capacity(values.len() * 12);
            for v in values {
                text.push_str(&format!("{v:.6}\n"));
            }
            std::fs::write(path, text).with_context(|| format!("writing {}", path.display()))?;
        }
        OutputType::Float => {
            let mut bytes = Vec::with_capacity(values.len() * 4);
            for v in values {
                bytes.extend_from_slice(&v.to_le_bytes());
            }
            std::fs::write(path, bytes).with_context(|| format!("writing {}", path.display()))?;
        }
    }
    Ok(())
}

pub fn run(args: &EnvelopeArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let params = EnvelopeParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        band_octave_pitchclass: args.band_octave_pitchclass,
        band_low: args.band_low.clone(),
        band_high: args.band_high.clone(),
        channel_method: args.channel_method,
        attack: args.attack.clone(),
        release: args.release.clone(),
        filtered_attack: args.filtered_attack.clone(),
        filtered_release: args.filtered_release.clone(),
        filtered_cut: args.filtered_cut.clone(),
        compress_threshold_db: args.compress_threshold.clone(),
        compress_amount_db: args.compress_amount.clone(),
        gate_threshold_db: args.gate_threshold.clone(),
        warp: args.warp.clone(),
        output_rate: args.output_rate,
        output_scale: args.output_scale,
    };

    let dur = audio.channels[0].len() as f32 / audio.sample_rate as f32;
    let values = process(&audio.channels, audio.sample_rate, &params, dur);
    write_series(&args.output, &values, args.output_type)
}
