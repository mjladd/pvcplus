//! `pvc irconvolvesequencer`: crossfades a signal through a sequence of
//! impulse responses read from an `impulseFileNames`-style list file.
//! Pure I/O glue - all the DSP lives in `pvc_core::tools::
//! irconvolvesequencer`.

use anyhow::{bail, Context, Result};
use pvc_core::tools::irconvolvesequencer::{process, IrconvolveSequencerParams};

use crate::cli::IrconvolvesequencerArgs;

/// Reads `<dir>/impulseFileNames`: the first whitespace-separated token is
/// the impulse count, followed by that many impulse-response sound file
/// paths (`legacy/pvc_src/irconvolvesequencer.c`'s own `fscanf(fp, " %d
/// ", &n)` then `fscanf(fp, " %s ", fileName)` loop - whitespace-delimited
/// tokens, not one-per-line, though every real list file in practice is
/// one-per-line).
fn read_impulse_file_names(dir: &std::path::Path) -> Result<Vec<std::path::PathBuf>> {
    let list_path = dir.join("impulseFileNames");
    let contents = std::fs::read_to_string(&list_path)
        .with_context(|| format!("reading {}", list_path.display()))?;
    let mut tokens = contents.split_whitespace();
    let count: usize = tokens
        .next()
        .with_context(|| format!("{} is empty", list_path.display()))?
        .parse()
        .with_context(|| {
            format!(
                "{}: expected an impulse count on the first line",
                list_path.display()
            )
        })?;
    let names: Vec<std::path::PathBuf> = tokens.map(std::path::PathBuf::from).collect();
    if names.len() < count {
        bail!(
            "{}: declared {count} impulse response files but only {} listed",
            list_path.display(),
            names.len()
        );
    }
    Ok(names[..count].to_vec())
}

pub fn run(args: &IrconvolvesequencerArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let impulse_file_names = read_impulse_file_names(&args.impulse_list_dir)?;
    if impulse_file_names.is_empty() {
        bail!(
            "{}: no impulse response files listed",
            args.impulse_list_dir.display()
        );
    }

    let mut impulses = Vec::with_capacity(impulse_file_names.len());
    for path in &impulse_file_names {
        let ir_audio =
            pvc_io::read_audio(path).with_context(|| format!("reading {}", path.display()))?;
        if ir_audio.sample_rate != audio.sample_rate {
            bail!(
                "impulse response {} sample rate ({}) does not match input sample rate ({})",
                path.display(),
                ir_audio.sample_rate,
                audio.sample_rate
            );
        }
        if args.impulse_channel > ir_audio.channels.len() {
            bail!(
                "impulse channel {} does not exist in {} ({} available)",
                args.impulse_channel,
                path.display(),
                ir_audio.channels.len()
            );
        }
        impulses.push(ir_audio.channels);
    }

    let params = IrconvolveSequencerParams {
        begin_secs: args.begin,
        end_secs: args.end,
        add_ring_time: args.ring_tail,
        impulse_channel: args.impulse_channel,
        source_gain_db: args.source_gain.clone(),
        output_gain_db: args.output_gain.clone(),
        ir_low_freq: args.ir_low_freq.clone(),
        ir_high_freq: args.ir_high_freq.clone(),
        ir_low_rolloff_db_per_octave: args.ir_low_rolloff.clone(),
        ir_high_rolloff_db_per_octave: args.ir_high_rolloff.clone(),
        source_low_freq: args.source_low_freq.clone(),
        source_high_freq: args.source_high_freq.clone(),
        source_low_rolloff_db_per_octave: args.source_low_rolloff.clone(),
        source_high_rolloff_db_per_octave: args.source_high_rolloff.clone(),
        normalization: args.normalization,
    };

    let out_channels = process(&audio.channels, audio.sample_rate, &impulses, &params);

    let out_buffer = pvc_io::AudioBuffer {
        sample_rate: audio.sample_rate,
        channels: out_channels,
    };
    pvc_io::write_wav(&args.output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", args.output.display()))
}
