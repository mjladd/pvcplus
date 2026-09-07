//! `pvc ringtvfilter`: reads audio and a time-varying filter response (a
//! `.pva` file, either format - see `commands::tvfilter::run`'s own
//! precedent), runs each channel through `pvc_core::tools::
//! ringtvfilter::process_channel`, writes the result. `-b`/`-e` are
//! applied here (slicing each channel to `[begin, end)` before handing it
//! to the core function), matching `commands::ring::run`'s own precedent.

use anyhow::{Context, Result};
use pvc_core::tools::ringtvfilter::{process_channel, RingtvfilterParams};

use crate::cli::RingtvfilterArgs;

/// Same fallback-with-warning validation as `commands::tvfilter::run`'s
/// own `resolve_analysis_channel` - see that function's doc comment.
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

pub fn run(args: &RingtvfilterArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;
    let r = audio.sample_rate as f32;

    // Accepts either PVA format, matching `commands::tvfilter::run`'s own
    // precedent.
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

    let begin = args.begin.max(0.0);

    let params = RingtvfilterParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.time_factor,
        oscbank_threshold_db: args.oscbank_threshold,
        source_gain_db: args.source_gain.clone(),
        source_freq_shift_hz: args.source_freq_shift.clone(),
        source_pitch_semitones: args.source_pitch.clone(),
        feedback_gain_db: args.feedback_gain.clone(),
        feedback_freq_shift_hz: args.feedback_freq_shift.clone(),
        feedback_pitch_semitones: args.feedback_pitch.clone(),
        feedback_decay_secs: args.feedback_decay.clone(),
        feedback_threshold_db: args.feedback_threshold.clone(),
        envelope_attack_secs: args.attack.clone(),
        envelope_release_secs: args.release.clone(),
        input_eq_low_db: args.input_eq_low_gain.clone(),
        input_eq_high_db: args.input_eq_high_gain.clone(),
        input_eq_low_freq: args.input_eq_low_freq.clone(),
        input_eq_high_freq: args.input_eq_high_freq.clone(),
        loop_eq_decay_secs: args.loop_eq_decay.clone(),
        loop_balance_limit_db: args.loop_balance_limit,
        loop_eq_low_db: args.loop_eq_low_gain.clone(),
        loop_eq_high_db: args.loop_eq_high_gain.clone(),
        loop_eq_low_freq: args.loop_eq_low_freq.clone(),
        loop_eq_high_freq: args.loop_eq_high_freq.clone(),
        output_eq_low_db: args.output_eq_low_gain.clone(),
        output_eq_high_db: args.output_eq_high_gain.clone(),
        output_eq_low_freq: args.output_eq_low_freq.clone(),
        output_eq_high_freq: args.output_eq_high_freq.clone(),
        time_origin: args.time_origin.clone(),
        rate: args.rate.clone(),
        window_low: args.window_low.clone(),
        window_high: args.window_high.clone(),
        loop_mode: args.loop_mode,
        onset_release: args.onset_release,
        autostop: args.autostop,
        loop_normalization: args.loop_normalization,
        peak_loop_smooth_time: args.loop_smooth.clone(),
        filter_warpshape: args.filter_warpshape.clone(),
        comp_threshold_db: args.comp_threshold,
        comp_db: args.comp_db,
        filter_transpose_semitones: args.filter_transpose.clone(),
        filter_freq_shift_hz: args.filter_shift.clone(),
        filter_source_db: args.filter_source_gain.clone(),
        filter_decay_secs: args.filter_decay.clone(),
        filter_postfilter: args.filter_placement,
        pitchflag: args.filter_pitch_mode,
    };

    // Matches `tools::ringtvfilter::process_channel`'s own correction of
    // an out-of-range time factor, so `dur` (computed here, at the CLI
    // layer) agrees with what the core function actually uses internally.
    let time_factor = if args.time_factor <= 0.0 {
        1.0
    } else {
        args.time_factor
    };

    let audio_chan_count = audio.channels.len();
    let mut out_channels = Vec::with_capacity(audio_chan_count);
    for (i, channel) in audio.channels.iter().enumerate() {
        let end = if args.end <= 0.0 {
            channel.len() as f32 / r
        } else {
            args.end
        };
        let begin_sample = (begin * r) as usize;
        let end_sample = ((end * r) as usize).clamp(begin_sample, channel.len());
        let slice = &channel[begin_sample..end_sample];
        let dur = (slice.len() as f32 / r) * time_factor;

        let ainchan = resolve_analysis_channel(
            args.analysis_channel,
            i,
            analysis_chan_count,
            audio_chan_count,
        );
        out_channels.push(process_channel(
            slice,
            &filter_data.channels[ainchan],
            filter_data.header.n as usize,
            filter_data.header.d,
            filter_data.header.sample_rate,
            filter_data.peak_amps[ainchan],
            audio.sample_rate,
            &params,
            dur,
        ));
    }

    // Independent per-channel loop (like `tools::harmonizer`/`tools::pv`)
    // can produce slightly different lengths per channel - trim to the
    // shortest, matching that same established precedent.
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
        .with_context(|| format!("writing {}", args.output.display()))
}
