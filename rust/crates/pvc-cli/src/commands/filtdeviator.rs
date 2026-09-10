//! `pvc filtdeviator`: reads audio and a `.fr` response file, runs the
//! selected channel(s) through
//! `pvc_core::tools::filtdeviator::process_channel`, writes the result.

use anyhow::{Context, Result};
use pvc_core::tools::filtdeviator::{process_channel, FiltdeviatorParams};

use crate::cli::FiltdeviatorArgs;

pub fn run(args: &FiltdeviatorArgs, json: bool, quiet: bool) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;
    let (response_amps, analysis_n) = pvc_io::read_fr_amplitudes(&args.response)
        .with_context(|| format!("reading {}", args.response.display()))?;

    let sample_rate = audio.sample_rate;
    let idur = audio
        .channels
        .first()
        .map(|c| c.len() as f32 / sample_rate as f32)
        .unwrap_or(0.0);
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

    let params = FiltdeviatorParams {
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.time_factor,
        source_gain_db: args.source_gain.clone(),
        source_freq_shift: args.source_freq_shift.clone(),
        source_delay_secs: args.source_delay.clone(),
        source_pitch_transpose: args.source_pitch.clone(),
        filter_gain_db: args.filter_gain.clone(),
        filter_pitch_transpose: args.filter_pitch.clone(),
        filter_freq_shift: args.filter_freq_shift.clone(),
        response_pitch_transpose: args.response_pitch.clone(),
        response_freq_shift: args.response_freq_shift.clone(),
        source_floor_db: args.source_floor.clone(),
        attack_secs: args.attack.clone(),
        release_secs: args.release.clone(),
        amp_warpshape: args.amp_warp.clone(),
        freq_warpshape: args.freq_warp.clone(),
        time_delay_warpshape: args.delay_warp.clone(),
        band_reject: args.band_reject,
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        time_delay_base: args.time_delay_base.clone(),
        time_delay_peak: args.time_delay_peak.clone(),
        time_delay_dev_control: args.time_delay_control.clone(),
        delay_time_scaler: args.delay_time_scaler.clone(),
        peak_decay_time_secs: args.decay_time_peak.clone(),
        base_decay_time_secs: args.decay_time_base.clone(),
        decay_time_dev_control: args.decay_time_control.clone(),
        freq_dev_base: args.freq_dev_base.clone(),
        freq_dev_peak: args.freq_dev_peak.clone(),
        freq_shift_dev_base: args.freq_shift_dev_base.clone(),
        freq_shift_dev_peak: args.freq_shift_dev_peak.clone(),
        freq_dev_control: args.freq_dev_control.clone(),
        freq_dev_mode: args.freq_dev_mode.clone(),
        frame_norm_limit_db: args.normalization_limit.clone(),
        normalize_to_filter: args.normalize_to_response,
        threshold_db: args.threshold,
    };

    // `-C`: unlike `pvc delayfilter`'s own same-lettered flag, this
    // tool's channel selector really does use the number given (see
    // `pvc-core::tools::filtdeviator`'s doc comment) - `0` processes
    // every channel independently, `1..` picks exactly one, 1-based.
    let selected_channels: Vec<&Vec<f32>> = if args.channel == 0 {
        audio.channels.iter().collect()
    } else {
        let index = args.channel - 1;
        anyhow::ensure!(
            index < audio.channels.len(),
            "channel {} out of range (input has {} channel(s))",
            args.channel,
            audio.channels.len()
        );
        vec![&audio.channels[index]]
    };

    let mut out_channels = Vec::with_capacity(selected_channels.len());
    for channel in selected_channels {
        let end = end_sample.min(channel.len());
        let trimmed: &[f32] = if begin_sample < end {
            &channel[begin_sample..end]
        } else {
            &[]
        };
        let dur = (trimmed.len() as f32 / sample_rate as f32) * params.time_factor;
        out_channels.push(process_channel(
            trimmed,
            &response_amps,
            analysis_n,
            sample_rate,
            dur,
            &params,
        ));
    }

    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // Whole-file peak rescale to the input's own peak - the same
    // `rescalev`-defaults-to-1 convention already established for
    // `pv`/`twarp`/`filter` (see `commands::filter::run`'s doc comment).
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
        sample_rate,
        channels: out_channels,
    };
    pvc_io::write_wav(&args.output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", args.output.display()))?;
    crate::summary::RunSummary::from_buffer(&args.output, &out_buffer).print(json, quiet);
    Ok(())
}
