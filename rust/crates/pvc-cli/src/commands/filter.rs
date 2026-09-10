//! `pvc filter`: reads audio and a `.fr` response file, runs each
//! channel through `pvc_core::tools::filter::process_channel`, writes
//! the result.

use anyhow::{Context, Result};
use pvc_core::tools::filter::{process_channel, FilterParams};

use crate::cli::FilterArgs;

pub fn run(args: &FilterArgs, json: bool, quiet: bool) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;
    let (response_amps, analysis_n) = pvc_io::read_fr_amplitudes(&args.response)
        .with_context(|| format!("reading {}", args.response.display()))?;

    let params = FilterParams {
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.stretch,
        filter_pitch_transpose: args.filter_pitch.clone(),
        filter_freq_shift: args.filter_freq_shift.clone(),
        filter_gain_db: args.filter_gain.clone(),
        filter_time_delay: args.filter_time_delay.clone(),
        delay_time_scaler: args.delay_time_scaler.clone(),
        source_enabled: args.source_enabled,
        source_gain_db: args.source_gain.clone(),
        source_pitch_transpose: args.source_pitch.clone(),
        source_freq_shift: args.source_freq_shift.clone(),
        source_time_delay: args.source_time_delay.clone(),
        response_pitch_transpose: args.response_pitch.clone(),
        response_freq_shift: args.response_freq_shift.clone(),
        response_warpshape: args.response_warp.clone(),
        response_smoothing_bw: args.response_smoothing.clone(),
        source_floor_db: args.source_floor.clone(),
        pitch_shift_source_only: args.pitch_shift_source_only,
        band_reject: args.band_reject,
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        comp_threshold_db: args.comp_threshold,
        comp_db: args.comp_amount,
        exp_threshold_db: args.expand_threshold,
        exp_db: args.expand_amount,
        frame_normalization_limit_db: args.normalization_limit.clone(),
        normalize_to_filter: args.normalize_to_response,
        attack_secs: args.attack.clone(),
        release_secs: args.release.clone(),
    };

    let mut out_channels = Vec::with_capacity(audio.channels.len());
    for channel in &audio.channels {
        out_channels.push(process_channel(
            channel,
            &response_amps,
            analysis_n,
            audio.sample_rate,
            &params,
        ));
    }

    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // `filter.c` shares the same `rescalev`-defaults-to-1 whole-file
    // peak-rescale post-pass already found for `pv`/`twarp` (same
    // generic `bufferout`/`rescaleThisBuffer` machinery) - see
    // `commands::pv::run_pv`'s doc comment for the detail. Not yet
    // exposed as a CLI override (`-=`), matching every other tool ported
    // so far.
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
