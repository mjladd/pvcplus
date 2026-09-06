//! `pvc pitchtrack`: reads audio, runs it through
//! `pvc_core::tools::pitchtracker::process`, writes the resulting scalar
//! time-series as ASCII text or a headerless raw f32 stream.

use anyhow::{Context, Result};
use pvc_core::tools::pitchtracker::{process, PitchtrackerParams};

use crate::cli::PitchtrackArgs;
use crate::commands::envelope::write_series;

pub fn run(args: &PitchtrackArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let params = PitchtrackerParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        band_low: args.band_low.clone(),
        band_high: args.band_high.clone(),
        method: args.method,
        window_min_secs: args.window_min,
        window_max_secs: args.window_max,
        detect_threshold_db: args.detect_threshold,
        mode_filter_window_secs: args.mode_filter_window,
        oversample_factor: args.oversample,
        smooth_response_secs: args.smooth_response,
        channel_method: args.channel_method,
        compress_threshold_db: args.compress_threshold.clone(),
        compress_amount_db: args.compress_amount.clone(),
        gate_threshold_db: args.gate_threshold.clone(),
        warp: args.warp.clone(),
        attack: args.attack.clone(),
        release: args.release.clone(),
        output_rate: args.output_rate,
        output_format: args.output_format,
        reference_pitch: args.reference.clone(),
    };

    let dur = audio.channels[0].len() as f32 / audio.sample_rate as f32;
    let values = process(&audio.channels, audio.sample_rate, &params, dur);
    write_series(&args.output, &values, args.output_type)
}
