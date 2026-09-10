//! `pvc tvfilter`: reads audio and a time-varying filter response (a
//! `.pva` file, either format - see `commands::twarp::run`'s own
//! precedent), runs each channel through `pvc_core::tools::
//! tvfilter::process_channel`, writes the result.

use anyhow::{Context, Result};
use pvc_core::tools::tvfilter::{process_channel, TvfilterParams};

use crate::cli::TvfilterArgs;

/// `ainchan = (achannelout == 0) ? channow : (achannelout - 1)`, plus
/// `tvfilter.c`'s own fallback-with-warning validation: auto-pairing
/// (`-K 0`) falls back to filter channel `0` for every audio channel
/// when the filter file has fewer channels than the input sound file;
/// an explicit out-of-range `-K` falls back to filter channel `0` too.
/// Both fallbacks are silent here - this project's convention is to skip
/// replicating the C's own diagnostic banners, not the behavior itself.
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

pub fn run(args: &TvfilterArgs, json: bool, quiet: bool) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;
    // Accepts either PVA format, matching `commands::twarp::run`'s own
    // precedent - a real `-F` file from the legacy `pvanalysis` is
    // always the legacy layout, but `pvc analyze`'s own PVA1 output
    // works equally well as a time-varying filter source.
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

    let params = TvfilterParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.time_factor,
        pitch_transpose: args.pitch.clone(),
        freq_shift: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        pitchflag: args.filter_pitch_mode,
        invert_mode: args.invert_mode,
        time_origin: args.time_origin.clone(),
        rate: args.rate.clone(),
        window_low: args.window_low.clone(),
        window_high: args.window_high.clone(),
        loop_mode: args.loop_mode,
        onset_release: args.onset_release,
        autostop: args.autostop,
        loop_normalization: args.loop_normalization,
        peak_loop_smooth_time: args.loop_smooth.clone(),
        comp_threshold_db: args.comp_threshold.clone(),
        comp_db: args.comp_db.clone(),
        filter_transpose: args.filter_transpose.clone(),
        filter_shift: args.filter_shift.clone(),
        filter_release_secs: args.filter_release.clone(),
        filter_attack_secs: args.filter_attack.clone(),
        filter_source_db: args.filter_source_gain.clone(),
        filter_warpshape: args.filter_warpshape.clone(),
        filter_smoothing_bw: args.filter_smoothing.clone(),
        shelf_low_db: args.shelf_low_gain.clone(),
        shelf_high_db: args.shelf_high_gain.clone(),
        shelf_low_freq: args.shelf_low_freq.clone(),
        shelf_high_freq: args.shelf_high_freq.clone(),
        frame_norm_limit_db: args.frame_norm_limit.clone(),
        normalize_to_filter: args.normalize_to_filter,
        attack_secs: args.attack.clone(),
        release_secs: args.release.clone(),
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
        out_channels.push(process_channel(
            channel,
            &filter_data.channels[ainchan],
            filter_data.header.n as usize,
            filter_data.header.d,
            filter_data.header.sample_rate,
            filter_data.peak_amps[ainchan],
            audio.sample_rate,
            &params,
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
