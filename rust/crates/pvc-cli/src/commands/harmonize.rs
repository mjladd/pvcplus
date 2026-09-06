//! `pvc harmonize`: reads audio and an 8-column data table, runs each
//! channel through `pvc_core::tools::harmonizer::process_channel`,
//! writes the result.

use anyhow::{bail, Context, Result};
use pvc_core::tools::harmonizer::{process_channel, HarmonizerParams, TableRow};

use crate::cli::HarmonizeArgs;

fn read_table(path: &std::path::Path) -> Result<Vec<TableRow>> {
    let text =
        std::fs::read_to_string(path).with_context(|| format!("reading {}", path.display()))?;
    let values: Vec<f32> = text
        .split_whitespace()
        .map(|tok| {
            tok.parse::<f32>()
                .with_context(|| format!("bad number {tok:?} in {}", path.display()))
        })
        .collect::<Result<_>>()?;
    if !values.len().is_multiple_of(8) {
        bail!(
            "{}: expected a multiple of 8 values (one 8-column row per band), got {}",
            path.display(),
            values.len()
        );
    }
    Ok(values
        .chunks_exact(8)
        .map(|c| TableRow {
            shift_factor: c[0],
            low_freq: c[1],
            high_freq: c[2],
            center_freq: c[3],
            peak_db: c[4],
            stopband_db: c[5],
            q_index: c[6],
            delay_time: c[7],
        })
        .collect())
}

pub fn run(args: &HarmonizeArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;
    let table = read_table(&args.table)?;

    let params = HarmonizerParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.stretch,
        shift_format: args.shift_format,
        table,
        table_shift_scale: args.table_shift_scale,
        table_shift_shift: args.table_shift_shift,
        table_freq_scale: args.table_freq_scale,
        table_freq_shift: args.table_freq_shift,
        table_peak_scale: args.table_peak_scale,
        table_stopband_scale: args.table_stopband_scale,
        table_stopband_shift: args.table_stopband_shift,
        table_q_shift: args.table_q_shift,
        table_delay_scale: args.table_delay_scale,
        table_delay_shift: args.table_delay_shift,
        gain_db: args.gain.clone(),
        voice_freq_shift_hz: args.voice_freq_shift.clone(),
        voice_pitch_semitones: args.voice_pitch.clone(),
        voice_gain_db: args.voice_gain.clone(),
        voice_warpshape: args.voice_warp.clone(),
        freq_interp: args.freq_interp.clone(),
        time_interp: args.time_interp.clone(),
        source_freq_shift_hz: args.source_freq_shift.clone(),
        source_pitch_semitones: args.source_pitch.clone(),
        source_gain_db: args.source_gain.clone(),
        source_delay_secs: args.source_delay.clone(),
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

    // `harmonizer`'s default `rescalev == 1`: the same whole-file
    // peak-rescale post-pass already found for `pv`/`twarp`/`filter`/
    // `denoise`/`compand`/`spectwarp` (see `commands::pv::run_pv`'s doc
    // comment for the detail).
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
