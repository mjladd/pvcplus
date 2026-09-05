//! `pvc info <path>`: print information about an audio file or a `.pva`
//! analysis file (the plan's replacement for `readheader`/`sndfile-info`;
//! full parity with `readheader`'s output lands in Task 3.3 alongside
//! `pvc analyze` - this is the Task 2.7 skeleton's basic version).

use std::path::Path;

use anyhow::{Context, Result};
use serde::Serialize;

#[derive(Serialize)]
struct AudioInfo {
    kind: &'static str,
    path: String,
    sample_rate: u32,
    channels: usize,
    frames: usize,
    duration_secs: f64,
}

#[derive(Serialize)]
struct PvaInfo {
    kind: &'static str,
    path: String,
    format: &'static str,
    fft_size: u32,
    hop_size: u32,
    sample_rate: u32,
    channels: u32,
    window_type: u32,
    frames_per_channel: usize,
    duration_secs: f64,
}

pub fn run(path: &Path, json: bool) -> Result<()> {
    let is_pva = path
        .extension()
        .is_some_and(|ext| ext.eq_ignore_ascii_case("pva"));

    if is_pva {
        print_pva_info(path, json)
    } else {
        print_audio_info(path, json)
    }
}

fn print_audio_info(path: &Path, json: bool) -> Result<()> {
    let buf = pvc_io::read_audio(path)
        .with_context(|| format!("reading audio file {}", path.display()))?;
    let frames = buf.num_frames();
    let info = AudioInfo {
        kind: "audio",
        path: path.display().to_string(),
        sample_rate: buf.sample_rate,
        channels: buf.num_channels(),
        frames,
        duration_secs: frames as f64 / buf.sample_rate as f64,
    };

    if json {
        println!("{}", serde_json::to_string_pretty(&info)?);
    } else {
        println!("{}", info.path);
        println!("  sample rate: {} Hz", info.sample_rate);
        println!("  channels:    {}", info.channels);
        println!("  frames:      {}", info.frames);
        println!("  duration:    {:.3} s", info.duration_secs);
    }
    Ok(())
}

fn print_pva_info(path: &Path, json: bool) -> Result<()> {
    // Try the new versioned format first; fall back to the legacy
    // pvanalysis.c layout, which has no magic to distinguish it up front.
    let (header, frames_per_channel, format) = match pvc_io::read_pva(path) {
        Ok(data) => {
            let frames = data.channels.first().map_or(0, |c| c.len());
            (data.header, frames, "PVA1 (new)")
        }
        Err(_) => {
            let data = pvc_io::read_legacy_pva(path)
                .with_context(|| format!("reading .pva file {}", path.display()))?;
            let frames = data.channels.first().map_or(0, |c| c.len());
            (data.header, frames, "legacy pvanalysis.c")
        }
    };

    let duration_secs = (frames_per_channel * header.d as usize) as f64 / header.sample_rate as f64;
    let info = PvaInfo {
        kind: "pva",
        path: path.display().to_string(),
        format,
        fft_size: header.n,
        hop_size: header.d,
        sample_rate: header.sample_rate,
        channels: header.channels,
        window_type: header.window_type,
        frames_per_channel,
        duration_secs,
    };

    if json {
        println!("{}", serde_json::to_string_pretty(&info)?);
    } else {
        println!("{} ({})", info.path, info.format);
        println!("  fft size:      {}", info.fft_size);
        println!("  hop size:      {}", info.hop_size);
        println!("  sample rate:   {} Hz", info.sample_rate);
        println!("  channels:      {}", info.channels);
        println!("  window type:   {}", info.window_type);
        println!("  frames:        {}", info.frames_per_channel);
        println!("  duration:      {:.3} s", info.duration_secs);
    }
    Ok(())
}
