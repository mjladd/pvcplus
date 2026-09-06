//! `pvc impulseresponse`: analyzes each input channel into a `.ir` file
//! (`pvc_io::ir`). Pure I/O glue - all the DSP lives in `pvc_core::tools::
//! impulseresponse` (`pvc-core` has no I/O dependency of its own).

use anyhow::{Context, Result};
use pvc_core::tools::impulseresponse::{process, ImpulseResponseParams};
use pvc_io::{IrData, IrHeader};

use crate::cli::ImpulseresponseArgs;

pub fn run(args: &ImpulseresponseArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let params = ImpulseResponseParams {
        begin_secs: args.begin,
        end_secs: args.end,
        normalization: args.normalization,
        normalization_db: args.normalization_db,
    };

    let out = process(&audio.channels, audio.sample_rate, &params);

    let data = IrData {
        header: IrHeader {
            channels: out.channels.len() as u32,
            fft_size: out.fft_size as u32,
            impulse_len: out.impulse_len as u32,
            sample_rate: out.sample_rate,
        },
        channels: out.channels,
    };

    pvc_io::write_ir(&args.output, &data)
        .with_context(|| format!("writing {}", args.output.display()))
}
