//! `pvc delayfilter`: reads a source `.pva` analysis file and a
//! `groupdelaymaker`-produced group-delay response file, runs each
//! channel through `pvc_core::tools::delayfilter::process_channel`,
//! writes the resynthesized audio.

use anyhow::{Context, Result};
use pvc_core::tools::delayfilter::{process_channel, DelayfilterParams};

use crate::cli::DelayfilterArgs;

pub fn run(args: &DelayfilterArgs, json: bool, quiet: bool) -> Result<()> {
    let analysis = pvc_io::read_pva(&args.analysis)
        .or_else(|_| pvc_io::read_legacy_pva(&args.analysis))
        .with_context(|| format!("reading {}", args.analysis.display()))?;

    let (delay_pairs, _n) = pvc_io::read_fr_pairs(&args.delay_filter)
        .with_context(|| format!("reading {}", args.delay_filter.display()))?;

    // The C's own `normamppk`: the maximum stored peak amplitude across
    // *every* analysis channel, computed once before the per-channel
    // loop - see `pvc-core::tools::delayfilter::process_channel`'s doc
    // comment.
    let master_peak_amp = analysis.peak_amps.iter().copied().fold(0.0f32, f32::max);

    let params = DelayfilterParams {
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        duration: args.duration,
        delay_time_scaler: args.delay_time_scaler.clone(),
        pitch_transpose: args.pitch.clone(),
        freq_shift: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        filter_time_origin: args.time_origin.clone(),
        filter_rate: args.rate.clone(),
        delay_transpose: args.delay_transpose.clone(),
        delay_shift: args.delay_shift.clone(),
        delay_warpshape: args.delay_warpshape.trunc(),
        zero_delay_db: args.zero_delay_gain.clone(),
        max_delay_db: args.max_delay_gain.clone(),
        delay_db_warp: args.delay_gain_curve.clone(),
        max_delay_time_mult: args.delay_window.clone(),
        comp_threshold_db: args.comp_threshold,
        comp_db: args.comp_db,
        release_secs: args.release.clone(),
        attack_secs: args.attack.clone(),
        freq_smooth_secs: args.freq_response_time.clone(),
        input_warpshape: args.warp.clone(),
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        threshold_db: args.threshold,
    };

    let mut out_channels = Vec::with_capacity(analysis.channels.len());
    for channel in &analysis.channels {
        out_channels.push(process_channel(
            channel,
            analysis.header.n as usize,
            analysis.header.d,
            analysis.header.sample_rate,
            master_peak_amp,
            &delay_pairs,
            &params,
        ));
    }

    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // Rescales the whole resynthesized output to match the source file's
    // own stored peak amplitude - the same `rescalev`-default convention
    // (and the same FFT-magnitude-vs-waveform-amplitude scale mismatch)
    // as `pvc twarp`'s own default; see `commands::twarp::run`'s doc
    // comment for why this is reproduced rather than "fixed".
    let target_peak = master_peak_amp;
    let output_peak = out_channels
        .iter()
        .flat_map(|c| c.iter())
        .copied()
        .fold(0.0f32, |a, b| a.max(b.abs()));
    if target_peak > 0.0 && output_peak > 0.0 {
        let ampval = target_peak / output_peak;
        for c in &mut out_channels {
            for s in c.iter_mut() {
                *s *= ampval;
            }
        }
    }

    let out_buffer = pvc_io::AudioBuffer {
        sample_rate: analysis.header.sample_rate,
        channels: out_channels,
    };
    pvc_io::write_wav(&args.output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", args.output.display()))?;
    crate::summary::RunSummary::from_buffer(&args.output, &out_buffer).print(json, quiet);
    Ok(())
}
