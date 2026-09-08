//! `pvc ratechanger`: reads raw audio directly (no `.pva` analysis step),
//! resamples each selected channel through
//! `pvc_core::tools::ratechanger::synthesize_channel`, applies the
//! selected post-synthesis normalization, and writes the result.

use anyhow::{Context, Result};
use pvc_core::tools::ratechanger::{
    resolve_output_duration, synthesize_channel, NormalizeMode, RatechangerParams,
};

use crate::cli::RatechangerArgs;

/// `32767/32768` - the same just-under-full-scale ceiling `ratechanger.c`
/// itself uses for every normalization mode but `Input`.
const NEAR_FULL_SCALE: f32 = 32767.0 / 32768.0;

pub fn run(args: &RatechangerArgs) -> Result<()> {
    anyhow::ensure!(
        args.truncation_db < 0.0,
        "--truncation-db must be < 0 (got {})",
        args.truncation_db
    );

    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let num_channels = audio.channels.len();
    anyhow::ensure!(
        num_channels > 0,
        "{}: input has no channels",
        args.input.display()
    );
    anyhow::ensure!(
        args.channel <= num_channels,
        "--channel {} exceeds input channel count {num_channels}",
        args.channel
    );

    let isr = audio.sample_rate;
    let input_duration = audio
        .channels
        .first()
        .map(|c| c.len() as f32 / isr as f32)
        .unwrap_or(0.0);

    let params = RatechangerParams {
        window_low: args.window_low.clone(),
        window_high: args.window_high.clone(),
        time_origin: args.time_origin.clone(),
        rate_for_input: args.rate_in.clone(),
        rate_for_output: args.rate_out.clone(),
        semitones_for_input: args.semitones_in.clone(),
        semitones_for_output: args.semitones_out.clone(),
        amp_env_for_input: args.gain_in.clone(),
        amp_env_for_output: args.gain_out.clone(),
        synthesis_mode: args.synthesis_mode,
        use_table_lookup: args.table_lookup,
        pi_interpolation_points: args.pi_interpolation_points,
        sinc_truncation_db: args.truncation_db,
    }
    .resolve_window_defaults(input_duration);

    // Matches `main()`'s own `if (outputDuration <= 0.) synthesizeOutputDuration = true;`.
    let synthesize_duration = args.synthesize_duration || args.duration <= 0.0;
    let (output_duration, mut xscaler) = if synthesize_duration {
        resolve_output_duration(&params, isr, input_duration, args.duration)
    } else {
        (args.duration, 1.0)
    };

    let (begin_chan, end_chan) = if args.channel == 0 {
        (0, num_channels - 1)
    } else {
        (args.channel - 1, args.channel - 1)
    };

    let mut out_channels = Vec::new();
    let mut output_peaks = Vec::new();
    for chan in begin_chan..=end_chan {
        let (samples, peak, next_xscaler) = synthesize_channel(
            &audio.channels[chan],
            isr,
            input_duration,
            output_duration,
            &params,
            xscaler,
        );
        xscaler = next_xscaler;
        output_peaks.push(peak);
        out_channels.push(samples);
    }

    // `main()`'s own `inputChannelPeakAmps[chan] >= 1.0` clamp, applied
    // before it's read by `NormalizeMode::Input` below.
    let mut input_peaks: Vec<f32> = audio
        .channels
        .iter()
        .map(|c| c.iter().fold(0.0f32, |a, &s| a.max(s.abs())))
        .collect();
    for p in &mut input_peaks {
        if *p >= 1.0 {
            *p = NEAR_FULL_SCALE;
        }
    }

    let global_output_peak = output_peaks.iter().copied().fold(0.0f32, f32::max);

    match args.normalize {
        NormalizeMode::None => {}
        NormalizeMode::Input => {
            for (i, chan_idx) in (begin_chan..=end_chan).enumerate() {
                let out_peak = output_peaks[i];
                if out_peak > 0.0 {
                    let scale = input_peaks[chan_idx] / out_peak;
                    for s in out_channels[i].iter_mut() {
                        *s *= scale;
                    }
                }
            }
        }
        NormalizeMode::Independent => {
            for (samples, &out_peak) in out_channels.iter_mut().zip(output_peaks.iter()) {
                if out_peak > 0.0 {
                    let scale = NEAR_FULL_SCALE / out_peak;
                    for s in samples.iter_mut() {
                        *s *= scale;
                    }
                }
            }
        }
        NormalizeMode::Together => {
            if global_output_peak > 0.0 {
                let scale = NEAR_FULL_SCALE / global_output_peak;
                for samples in out_channels.iter_mut() {
                    for s in samples.iter_mut() {
                        *s *= scale;
                    }
                }
            }
        }
        NormalizeMode::IfClipping => {
            if global_output_peak > 1.0 {
                let scale = NEAR_FULL_SCALE / global_output_peak;
                for samples in out_channels.iter_mut() {
                    for s in samples.iter_mut() {
                        *s *= scale;
                    }
                }
            }
        }
    }

    let out_sample_rate = if args.new_sample_rate <= 0 {
        isr
    } else {
        args.new_sample_rate as u32
    };

    let out_buffer = pvc_io::AudioBuffer {
        sample_rate: out_sample_rate,
        channels: out_channels,
    };
    pvc_io::write_wav(&args.output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", args.output.display()))
}
