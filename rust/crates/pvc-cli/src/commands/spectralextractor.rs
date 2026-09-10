//! `pvc spectralextractor`: reads audio, runs each channel through
//! `pvc_core::tools::spectralextractor::process_channel`, writes the
//! result.
//!
//! Unlike `pvc pv`/`pvc filter` (whose `-b`/`-e` are `dur`-only
//! bookkeeping - see `pvc-core::tools::pv`'s doc comment), `--begin`/
//! `--end` here really do trim the input before analysis, matching the
//! real tool's `setupfiles()`/`getInputFileDataToSetOutputChannels()`
//! call chain - see `pvc-core::tools::spectralextractor`'s doc comment.

use anyhow::{Context, Result};
use pvc_core::tools::spectralextractor::{process_channel, SpectralExtractorParams};

use crate::cli::SpectralExtractorArgs;

pub fn run(args: &SpectralExtractorArgs, json: bool, quiet: bool) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let sample_rate = audio.sample_rate;
    let idur = audio
        .channels
        .first()
        .map(|c| c.len() as f32 / sample_rate as f32)
        .unwrap_or(0.0);

    // Matches `getInputFileDataToSetOutputChannels`: `endt <= 0` means
    // "end of file"; an `end` beyond the file's own duration is clamped
    // to it.
    let endt = if args.end <= 0.0 {
        idur
    } else {
        args.end.min(idur)
    };
    let begint = args.begin.max(0.0);
    anyhow::ensure!(
        endt > begint,
        "end time ({endt}) must be after begin time ({begint})"
    );

    let begin_sample = (begint * sample_rate as f32) as usize;
    let end_sample = (endt * sample_rate as f32) as usize;

    let params = SpectralExtractorParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.time_factor,
        pitch_transpose: args.pitch.clone(),
        freq_shift: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        spectral_type: args.spectral_type,
        freq_change_threshold_hz: args.freq_change_threshold.clone(),
        freq_change_response_secs: args.freq_change_response.clone(),
        release_secs: args.release.clone(),
        complement_prop: args.complement.clone(),
        frame_norm_limit_db: args.frame_norm_limit.clone(),
        warpshape: args.warp.clone(),
        shelf_low_db: args.shelf_low_gain.clone(),
        shelf_high_db: args.shelf_high_gain.clone(),
        shelf_low_freq: args.shelf_low_freq.clone(),
        shelf_high_freq: args.shelf_high_freq.clone(),
        threshold_db: args.threshold,
    };

    let mut out_channels = Vec::with_capacity(audio.channels.len());
    for channel in &audio.channels {
        let end = end_sample.min(channel.len());
        let trimmed: &[f32] = if begin_sample < end {
            &channel[begin_sample..end]
        } else {
            &[]
        };
        let dur = (trimmed.len() as f32 / sample_rate as f32) * params.time_factor;
        out_channels.push(process_channel(trimmed, sample_rate, &params, dur));
    }

    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // `rescalev`'s shared default: rescale the whole output to match the
    // input's peak amplitude - see `commands::pv::run`'s doc comment.
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
    crate::summary::RunSummary::from_buffer(&args.output, &out_buffer).print(json, quiet);
    Ok(())
}
