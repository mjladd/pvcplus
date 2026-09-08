//! `pvc specflattracker`: reads audio, runs it through
//! `pvc_core::tools::specflattracker::process`, writes the resulting
//! scalar time-series as ASCII text or a headerless raw f32 stream.
//! Mirrors `commands::centroid::run`'s own shape.

use anyhow::{Context, Result};
use pvc_core::tools::specflattracker::{process, SpecflattrackerParams};

use crate::cli::SpecflattrackerArgs;
use crate::commands::envelope::write_series;

pub fn run(args: &SpecflattrackerArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let params = SpecflattrackerParams {
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
        warp: args.warp.clone(),
        output_rate: args.output_rate,
        output_format: args.output_format,
        method: args.method,
        amplitude_threshold_db: args.amplitude_threshold,
    };

    let dur = audio.channels[0].len() as f32 / audio.sample_rate as f32;
    let values = process(&audio.channels, audio.sample_rate, &params, dur);
    write_series(&args.output, &values, args.output_type)
}
