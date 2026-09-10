//! `pvc convolver`: reads audio (Sound A) and a filter analysis file
//! (Sound B), runs each channel through `pvc_core::tools::
//! convolver::process_channel`, writes the result.
//!
//! Unlike `pvc pv`/`pvc filter` (whose `-b`/`-e` are `dur`-only
//! bookkeeping), `--begin`/`--end` here really do trim the input before
//! analysis - see `pvc-core::tools::convolver`'s doc comment.

use anyhow::{Context, Result};
use pvc_core::tools::convolver::{process_channel, ConvolverParams};

use crate::cli::ConvolverArgs;

/// Mirrors `commands::tvfilter::run`'s own `resolve_analysis_channel`,
/// adjusted for `convolver.c`'s own fallback target: an out-of-range or
/// auto-with-too-few-channels `-K` resets to filter channel `0` here
/// too (the real tool's own `achannelout = 1` one-based fallback), so
/// the two functions end up identical - kept separate since they port
/// different tools' own validation code, confirmed independently rather
/// than assumed to match.
fn resolve_analysis_channel(
    analysis_channel_arg: usize,
    audio_channel_index: usize,
    analysis_chan_count: usize,
    audio_chan_count: usize,
) -> usize {
    if analysis_channel_arg == 0 {
        if analysis_chan_count < audio_chan_count {
            0
        } else {
            audio_channel_index
        }
    } else {
        let requested = analysis_channel_arg - 1;
        if requested >= analysis_chan_count {
            0
        } else {
            requested
        }
    }
}

pub fn run(args: &ConvolverArgs, json: bool, quiet: bool) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let filter_data = pvc_io::read_pva(&args.filter_response)
        .or_else(|_| pvc_io::read_legacy_pva(&args.filter_response))
        .with_context(|| format!("reading {}", args.filter_response.display()))?;

    let analysis_chan_count = filter_data.channels.len();
    if analysis_chan_count == 0 {
        anyhow::bail!(
            "{}: filter response file has no channels",
            args.filter_response.display()
        );
    }

    // The *true* on-disk interleaved frame stream (frame 0 channel 0,
    // frame 0 channel 1, ..., frame 1 channel 0, ...) - `pvc_core::
    // tools::convolver::process_channel` needs this raw layout, not a
    // per-channel frame list, to reproduce a real bug in `convolver.c`'s
    // own filter-frame fetch (see that module's doc comment).
    let frames_per_channel = filter_data.channels[0].len();
    let mut filter_raw_stream = Vec::with_capacity(
        frames_per_channel * analysis_chan_count * (filter_data.header.n as usize + 2),
    );
    for f in 0..frames_per_channel {
        for ch in &filter_data.channels {
            filter_raw_stream.extend_from_slice(&ch[f]);
        }
    }

    if filter_data.header.sample_rate != audio.sample_rate {
        anyhow::bail!(
            "sample rates do not match: input is {}, filter response is {}",
            audio.sample_rate,
            filter_data.header.sample_rate
        );
    }

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

    let params = ConvolverParams {
        window_size: args.window_size,
        window: args.window,
        time_factor: args.time_factor,
        pitch_transpose: args.pitch.clone(),
        freq_shift: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        filter_time_origin: args.filter_time_origin.clone(),
        filter_rate: args.filter_rate.clone(),
        filter_window_low: args.filter_window_low.clone(),
        filter_window_high: args.filter_window_high.clone(),
        loop_mode: args.loop_mode,
        onset_release: args.onset_release,
        autostop: args.autostop,
        sound_a_db: args.sound_a_gain.clone(),
        sound_b_db: args.sound_b_gain.clone(),
        convolve_db: args.convolve_gain.clone(),
        pan: args.pan.clone(),
        panwarp_a: args.panwarp_a,
        panwarp_b: args.panwarp_b,
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        frame_norm_limit_db: args.frame_norm_limit.clone(),
        normalize_to_filter: args.normalize_to_filter,
        threshold_db: args.threshold,
    };

    let audio_chan_count = audio.channels.len();
    let mut out_channels = Vec::with_capacity(audio_chan_count);
    for (i, channel) in audio.channels.iter().enumerate() {
        let ainchan = resolve_analysis_channel(
            args.analysis_channel,
            i,
            analysis_chan_count,
            audio_chan_count,
        );
        let end = end_sample.min(channel.len());
        let trimmed: &[f32] = if begin_sample < end {
            &channel[begin_sample..end]
        } else {
            &[]
        };
        let dur = (trimmed.len() as f32 / sample_rate as f32) * params.time_factor;
        out_channels.push(process_channel(
            trimmed,
            &filter_raw_stream,
            filter_data.header.n as usize,
            filter_data.header.d,
            analysis_chan_count,
            ainchan,
            sample_rate,
            &params,
            dur,
        ));
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
