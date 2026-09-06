//! `pvc freqresponse`: reads audio, analyzes all channels into one
//! combined response, writes a `.fr` file.

use anyhow::{Context, Result};
use pvc_core::tools::freqresponse::{process, FreqresponseParams};

use crate::cli::FreqresponseArgs;

pub fn run(args: &FreqresponseArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let params = FreqresponseParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        method: args.spectrum_type,
        weight_average: args.weight_average,
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        eq_normalize_bypass: args.no_normalize,
        normalize_to_peaks: args.formant_normalize,
        companding_index: args.formant_warp,
        low_freq_limit: args.freq_low,
        high_freq_limit: args.freq_high,
        minimum_formant_db: args.formant_floor,
        formant_selection_threshold: args.formant_threshold,
    };

    let frame = process(&audio.channels, audio.sample_rate, &params);

    pvc_io::write_fr(&args.output, &frame.to_pva_floats())
        .with_context(|| format!("writing {}", args.output.display()))?;
    Ok(())
}
