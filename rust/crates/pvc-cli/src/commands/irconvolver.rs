//! `pvc irconvolver`: convolves (or deconvolves) each input channel
//! against a spectrum loaded from a `.ir` file. Pure I/O glue - all the
//! DSP lives in `pvc_core::tools::irconvolver`.

use anyhow::{bail, Context, Result};
use pvc_core::tools::irconvolver::{process, ImpulseSpectra, IrconvolverParams};

use crate::cli::IrconvolverArgs;

pub fn run(args: &IrconvolverArgs, json: bool, quiet: bool) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let ir_data =
        pvc_io::read_ir(&args.ir).with_context(|| format!("reading {}", args.ir.display()))?;

    if ir_data.header.sample_rate != audio.sample_rate {
        bail!(
            "impulse response sample rate ({}) does not match input sample rate ({})",
            ir_data.header.sample_rate,
            audio.sample_rate
        );
    }
    if args.impulse_channel > ir_data.header.channels as usize {
        bail!(
            "impulse channel {} does not exist ({} available)",
            args.impulse_channel,
            ir_data.header.channels
        );
    }

    let ir = ImpulseSpectra {
        fft_size: ir_data.header.fft_size as usize,
        impulse_len: ir_data.header.impulse_len as usize,
        channels: ir_data.channels,
    };

    let params = IrconvolverParams {
        begin_secs: args.begin,
        end_secs: args.end,
        add_ring_time: args.ring_tail,
        impulse_channel: args.impulse_channel,
        mode: args.mode,
        ir_low_freq: args.ir_low_freq,
        ir_high_freq: args.ir_high_freq,
        ir_low_rolloff_db_per_octave: args.ir_low_rolloff,
        ir_high_rolloff_db_per_octave: args.ir_high_rolloff,
        source_low_freq: args.source_low_freq,
        source_high_freq: args.source_high_freq,
        source_low_rolloff_db_per_octave: args.source_low_rolloff,
        source_high_rolloff_db_per_octave: args.source_high_rolloff,
        source_gain_db: args.source_gain.clone(),
        input_gain_db: args.input_gain.clone(),
        output_gain_db: args.output_gain.clone(),
    };

    let begin = params.begin_secs.max(0.0);
    let end = if params.end_secs <= 0.0 {
        audio.channels[0].len() as f32 / audio.sample_rate as f32
    } else {
        params.end_secs
    };
    let dur = end - begin;

    let mut out_channels = process(&audio.channels, audio.sample_rate, &ir, &params, dur);

    // `rescalev`'s shared default (`legacy/pvc_lib/fileio.c`'s
    // `rescaleThisBuffer`, applied by every tool's `bufferout()`): rescale
    // the whole output so its peak amplitude matches the input's - see
    // `commands::pv::run`'s doc comment for how this was found. Skipped,
    // like the C, if either peak is exactly zero.
    //
    // Approximation, not a bug: the real `rescaleThisBuffer` computes this
    // ratio *once*, from a snapshot of `ipeakamp`/`peakamp` taken as of
    // the very first output `bufferout()` block - not the true peak over
    // the whole file, which this port uses instead (simpler, and matches
    // every other tool's existing rescale port in this project). The two
    // agree whenever a signal's peak amplitude doesn't change much over
    // time; confirmed to diverge by a fraction of a percent for a
    // frequency sweep convolved against a resonant impulse response
    // (peak keeps changing well past the first block) - within this
    // tool's own golden case's tolerance, not chased further.
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
