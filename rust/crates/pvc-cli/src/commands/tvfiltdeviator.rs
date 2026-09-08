//! `pvc tvfiltdeviator`: reads audio and a time-varying filter response
//! (a `.pva` file, either format), runs each channel through
//! `pvc_core::tools::tvfiltdeviator::process_channel`, writes the result.

use anyhow::{Context, Result};
use pvc_core::tools::tvfiltdeviator::{process_channel, TvfiltdeviatorParams};

use crate::cli::TvfiltdeviatorArgs;

/// `ainchan = (achannelout == 0) ? channow : (achannelout - 1)`, plus the
/// C's own fallback-with-warning validation - see `commands::tvfilter::
/// run`'s own identical helper (not reused directly: a small enough
/// function that duplicating it beats a cross-command dependency for two
/// otherwise-independent CLI modules).
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

pub fn run(args: &TvfiltdeviatorArgs) -> Result<()> {
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
    let analysis_n = filter_data.header.n as usize;

    // The true on-disk interleaved frame stream (frame-major, channel-
    // minor) - needed for the buggy main filter fetch, see
    // `pvc-core::tools::tvfiltdeviator`'s doc comment.
    let frames_per_channel = filter_data.channels[0].len();
    let mut filter_raw_stream =
        Vec::with_capacity(frames_per_channel * analysis_chan_count * (analysis_n + 2));
    for f in 0..frames_per_channel {
        for ch in &filter_data.channels {
            filter_raw_stream.extend_from_slice(&ch[f]);
        }
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

    let params = TvfiltdeviatorParams {
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.time_factor,
        time_origin: args.time_origin.clone(),
        rate: args.rate.clone(),
        window_low: args.window_low.clone(),
        window_high: args.window_high.clone(),
        loop_mode: args.loop_mode,
        onset_release: args.onset_release,
        autostop: args.autostop,
        loop_normalization: args.loop_normalization,
        peak_loop_smooth_time: args.loop_smooth.clone(),
        pitch_transpose: args.pitch.clone(),
        freq_shift: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        pitchflag: args.filter_pitch_mode,
        invert_mode: args.invert_mode,
        filter_transpose: args.filter_transpose.clone(),
        filter_shift: args.filter_shift.clone(),
        filter_release_secs: args.filter_release.clone(),
        filter_attack_secs: args.filter_attack.clone(),
        filter_source_db: args.filter_source_gain.clone(),
        filter_warpshape: args.filter_warpshape.clone(),
        filter_smoothing_bw: args.filter_smoothing.clone(),
        comp_threshold_db: args.comp_threshold.clone(),
        comp_db: args.comp_db.clone(),
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        attack_secs: args.attack.clone(),
        release_secs: args.release.clone(),
        time_delay_base: args.time_delay_base.clone(),
        time_delay_peak: args.time_delay_peak.clone(),
        time_delay_dev_control: args.time_delay_control.clone(),
        time_delay_warpshape: args.time_delay_warp.clone(),
        freq_dev_base: args.freq_dev_base.clone(),
        freq_dev_peak: args.freq_dev_peak.clone(),
        freq_shift_dev_base: args.freq_shift_dev_base.clone(),
        freq_shift_dev_peak: args.freq_shift_dev_peak.clone(),
        freq_dev_control: args.freq_dev_control.clone(),
        freq_dev_mode: args.freq_dev_mode.clone(),
        freq_dev_response_secs: args.freq_dev_response.clone(),
        freq_warpshape: args.freq_dev_warp.clone(),
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
        out_channels.push(process_channel(
            trimmed,
            &filter_raw_stream,
            &filter_data.channels[ainchan],
            analysis_n,
            filter_data.header.d,
            filter_data.header.sample_rate,
            analysis_chan_count,
            ainchan,
            filter_data.peak_amps[ainchan],
            args.fft,
            sample_rate,
            &params,
        ));
    }

    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // Whole-file peak rescale to the input's own peak - the same
    // `rescalev`-defaults-to-1 convention already established for
    // `pv`/`twarp`/`filter`/`tvfilter`.
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
