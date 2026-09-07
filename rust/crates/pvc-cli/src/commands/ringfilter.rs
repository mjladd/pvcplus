//! `pvc ringfilter`: reads audio and a `.fr` response file, runs each
//! channel through `pvc_core::tools::ringfilter::process_channel`, writes
//! the result. `-b`/`-e` are applied here, matching `commands::ring::run`.

use anyhow::{Context, Result};
use pvc_core::tools::ringfilter::{process_channel, RingfilterParams};

use crate::cli::RingfilterArgs;

pub fn run(args: &RingfilterArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;
    let r = audio.sample_rate as f32;

    let (filter_response, response_n) = pvc_io::read_fr_amplitudes(&args.filter_response)
        .with_context(|| format!("reading {}", args.filter_response.display()))?;
    if response_n != args.fft {
        anyhow::bail!(
            "{}: filter response file size ({} FFT) does not match --fft ({}) - \
             set --fft to match the file it was built with",
            args.filter_response.display(),
            response_n,
            args.fft
        );
    }

    let begin = args.begin.max(0.0);

    let params = RingfilterParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.time_factor,
        oscbank_threshold_db: args.oscbank_threshold,
        master_gain_db: args.master_gain.clone(),
        source_gain_db: args.source_gain.clone(),
        source_freq_shift_hz: args.source_freq_shift.clone(),
        source_pitch_semitones: args.source_pitch.clone(),
        feedback_gain_db: args.feedback_gain.clone(),
        feedback_freq_shift_hz: args.feedback_freq_shift.clone(),
        feedback_pitch_semitones: args.feedback_pitch.clone(),
        feedback_decay_secs: args.feedback_decay.clone(),
        feedback_threshold_db: args.feedback_threshold.clone(),
        feedback_threshold_pass_above: args.feedback_threshold_mode,
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
        filter_response,
        filter_warpshape: args.filter_warpshape.clone(),
        filter_transpose_semitones: args.filter_transpose.clone(),
        filter_freq_shift_hz: args.filter_shift.clone(),
        filter_source_db: args.filter_source_gain.clone(),
        filter_decay_secs: args.filter_decay.clone(),
        filter_postfilter: args.filter_placement,
        pitchflag: args.filter_pitch_mode,
    };

    // Matches `tools::ringfilter::process_channel`'s own correction of an
    // out-of-range time factor, so `dur` (computed here, at the CLI
    // layer) agrees with what the core function actually uses internally.
    let time_factor = if args.time_factor <= 0.0 {
        1.0
    } else {
        args.time_factor
    };

    let mut out_channels = Vec::with_capacity(audio.channels.len());
    for channel in &audio.channels {
        let end = if args.end <= 0.0 {
            channel.len() as f32 / r
        } else {
            args.end
        };
        let begin_sample = (begin * r) as usize;
        let end_sample = ((end * r) as usize).clamp(begin_sample, channel.len());
        let slice = &channel[begin_sample..end_sample];
        let dur = (slice.len() as f32 / r) * time_factor;
        out_channels.push(process_channel(slice, audio.sample_rate, &params, dur));
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
        .with_context(|| format!("writing {}", args.output.display()))
}
