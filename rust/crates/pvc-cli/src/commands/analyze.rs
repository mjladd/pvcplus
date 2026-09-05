//! `pvc analyze`: reads audio, runs each channel through
//! `pvc_core::tools::analyze::process_channel`, writes a new-format
//! `.pva` file via `pvc_io::write_pva`.

use anyhow::{Context, Result};
use pvc_core::tools::analyze::{process_channel, AnalyzeParams};
use pvc_core::Window;

use crate::cli::AnalyzeArgs;

pub fn run(args: &AnalyzeArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let params = AnalyzeParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        gain_db: args.gain,
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        warpshape: args.warp,
    };

    let mut channels: Vec<Vec<Vec<f32>>> = Vec::with_capacity(audio.channels.len());
    for channel in &audio.channels {
        let frames = process_channel(channel, audio.sample_rate, &params);
        channels.push(frames.iter().map(|f| f.to_pva_floats()).collect());
    }

    // Same rationale as `pvc pv`'s per-channel length trim: each channel
    // is an independent loop and can come out one hop different in
    // length; the new `.pva` format has no per-channel frame count, so
    // channels must be uniform.
    let min_len = channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut channels {
        c.truncate(min_len);
    }

    let d = (audio.sample_rate as f32 / params.frames_per_sec) as u32;
    let data = pvc_io::PvaData {
        header: pvc_io::PvaHeader {
            n: params.fft_size as u32,
            d,
            sample_rate: audio.sample_rate,
            channels: channels.len() as u32,
            window_type: window_type_code(params.window),
        },
        channels,
    };

    pvc_io::write_pva(&args.output, &data)
        .with_context(|| format!("writing {}", args.output.display()))?;
    Ok(())
}

/// Maps a [`Window`] back to `makewindows.c`'s `window_type` integer code
/// (0=Hamming, 1=Rectangular, 2=Blackman, 3=Bartlett, 4..=12=Kaiser(alpha),
/// 13=BlackmanHarris, 14=Nuttall, 15=BlackmanNuttall, 16=FlatTop) for the
/// `.pva` header field - a pure label recording what shape was used, not
/// read back by `pvc-core` itself.
fn window_type_code(window: Window) -> u32 {
    match window {
        Window::Hamming => 0,
        Window::Rectangular => 1,
        Window::Blackman => 2,
        Window::Bartlett => 3,
        Window::Kaiser(alpha) => alpha.round().clamp(4.0, 12.0) as u32,
        Window::BlackmanHarris => 13,
        Window::Nuttall => 14,
        Window::BlackmanNuttall => 15,
        Window::FlatTop => 16,
    }
}
