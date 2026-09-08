//! `pvc inharmonator`: reads audio and a partials data table, runs the
//! selected channel(s) through `pvc_core::tools::inharmonator::process_channel`,
//! writes the result.

use anyhow::{Context, Result};
use pvc_core::tools::inharmonator::{
    process_channel, resolve_partials, DataModifiers, InharmonatorParams, Partial,
};

use crate::cli::InharmonatorArgs;

/// The partials table is 5 whitespace-separated floats per row - the
/// same plain-parser simplification `commands::harmonize::read_table`
/// already established for this project's other legacy data tables (see
/// `pvc_core::tools::harmonizer`'s own doc comment on why the C's
/// `cut_data_lines()` comment/solo/mute machinery isn't reproduced).
fn read_partials(path: &std::path::Path) -> Result<Vec<Partial>> {
    let text =
        std::fs::read_to_string(path).with_context(|| format!("reading {}", path.display()))?;
    let values: Vec<f32> = text
        .split_whitespace()
        .map(|s| {
            s.parse::<f32>()
                .with_context(|| format!("parsing {s:?} as a number"))
        })
        .collect::<Result<_>>()?;
    anyhow::ensure!(
        values.len().is_multiple_of(5),
        "{}: expected a multiple of 5 values (partial number, shift, decibels, delay, decay), got {}",
        path.display(),
        values.len()
    );
    Ok(values
        .chunks_exact(5)
        .map(|c| Partial {
            number: c[0],
            shift_data: c[1],
            decibels: c[2],
            time_delay: c[3],
            feedback_decay_time: c[4],
        })
        .collect())
}

pub fn run(args: &InharmonatorArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;
    let raw_partials = read_partials(&args.partials)?;
    anyhow::ensure!(!raw_partials.is_empty(), "partials table is empty");

    let modifiers = DataModifiers {
        number_scaler: args.partial_number_scale,
        number_shifter: args.partial_number_shift,
        decibel_scaler: args.partial_db_scale,
        delay_scaler: args.partial_delay_scale,
        delay_shifter: args.partial_delay_shift,
        decay_scaler: args.partial_decay_scale,
        decay_shifter: args.partial_decay_shift,
    };
    let partials = resolve_partials(&raw_partials, &modifiers);

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

    let params = InharmonatorParams {
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.time_factor,
        method: args.method,
        partial_band_window: args.partial_window,
        partial_bandwidth: args.partial_bandwidth.clone(),
        master_gain_db: args.master_gain.clone(),
        target_harmadd: args.target_freq_shift.clone(),
        target_ptrans: args.target_pitch.clone(),
        target_db: args.target_gain.clone(),
        target_amp_interp: args.target_amp_interp.clone(),
        target_freq_interp: args.target_freq_interp.clone(),
        target_time_interp: args.target_time_interp.clone(),
        fundamental_freq_or_oppc: args.fundamental.clone(),
        non_target_harmadd: args.non_target_freq_shift.clone(),
        non_target_ptrans: args.non_target_pitch.clone(),
        non_target_db: args.non_target_gain.clone(),
        non_target_delay_t: args.non_target_delay.clone(),
        non_target_decay_t: args.non_target_decay.clone(),
        inharm_attack: args.attack.clone(),
        inharm_release: args.release.clone(),
        warpshape: args.warpshape.clone(),
        source_harmadd: args.source_freq_shift.clone(),
        source_ptrans: args.source_pitch.clone(),
        source_db: args.source_gain.clone(),
        source_release: args.source_release.clone(),
        source_attack: args.source_attack.clone(),
        source_delay_t: args.source_delay.clone(),
        threshold_db: args.threshold,
    };

    let num_channels = audio.channels.len();
    let selected_channels: Vec<&Vec<f32>> = if args.channel == 0 {
        audio.channels.iter().collect()
    } else {
        let index = args.channel - 1;
        anyhow::ensure!(
            index < num_channels,
            "--channel {} out of range (input has {num_channels} channel(s))",
            args.channel
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
            &partials,
            args.fft,
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
    // `pv`/`twarp`/`filter`/`filtdeviator` (see `commands::filter::run`'s
    // doc comment) - `inharmonator.c` itself also declares `rescalev`
    // via its own `-=` flag, confirmed by reading its `crack()` switch.
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
        .with_context(|| format!("writing {}", args.output.display()))
}
