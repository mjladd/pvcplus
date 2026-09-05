//! `pvc pv`/`pvc stretch`/`pvc pitch`: reads audio, runs each channel
//! through `pvc_core::tools::pv::process_channel`, writes the result.

use std::path::Path;

use anyhow::{Context, Result};
use pvc_core::tools::pv::{process_channel, FilterType, PvParams};
use pvc_core::{ControlFn, Window};

use crate::cli::PvArgs;

pub fn run(args: &PvArgs) -> Result<()> {
    let params = PvParams {
        fft_size: args.fft,
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.stretch,
        pitch_transpose_semitones: args.pitch.clone(),
        freq_shift_hz: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        attack_secs: args.attack.clone(),
        release_secs: args.release.clone(),
        warpshape: args.warp.clone(),
        shelf_low_db: args.shelf_low_gain.clone(),
        shelf_high_db: args.shelf_high_gain.clone(),
        shelf_low_freq: args.shelf_low_freq.clone(),
        shelf_high_freq: args.shelf_high_freq.clone(),
        threshold_db: args.threshold,
        filter_type: args.filter_type,
        filter_lowfreq: args.filter_low,
        filter_hifreq: args.filter_high,
    };
    run_pv(&args.input, &args.output, params)
}

/// `pvc stretch --factor <f>`: `pv` with only the time-stretch factor set.
pub fn run_stretch(factor: f32, input: &Path, output: &Path) -> Result<()> {
    run_pv(input, output, default_params(factor, ControlFn::Const(0.0)))
}

/// `pvc pitch --semitones <s>`: `pv` with only the pitch transposition set.
pub fn run_pitch(semitones: f32, input: &Path, output: &Path) -> Result<()> {
    run_pv(
        input,
        output,
        default_params(1.0, ControlFn::Const(semitones)),
    )
}

fn default_params(time_factor: f32, pitch: ControlFn) -> PvParams {
    PvParams {
        fft_size: 1024,
        window_size: 2048,
        window: Window::Hamming,
        frames_per_sec: 200.0,
        time_factor,
        pitch_transpose_semitones: pitch,
        freq_shift_hz: ControlFn::Const(0.0),
        gain_db: ControlFn::Const(0.0),
        attack_secs: ControlFn::Const(0.0),
        release_secs: ControlFn::Const(0.0),
        warpshape: ControlFn::Const(0.0),
        shelf_low_db: ControlFn::Const(0.0),
        shelf_high_db: ControlFn::Const(0.0),
        shelf_low_freq: ControlFn::Const(200.0),
        shelf_high_freq: ControlFn::Const(2000.0),
        threshold_db: -96.0,
        filter_type: FilterType::Bandpass,
        filter_lowfreq: 0.0,
        // Resolved to the real Nyquist frequency once the input's sample
        // rate is known, in `run_pv` - matches the C's own `hifreq < 0 ->
        // nyquist` default.
        filter_hifreq: -1.0,
    }
}

fn run_pv(input: &Path, output: &Path, mut params: PvParams) -> Result<()> {
    let audio =
        pvc_io::read_audio(input).with_context(|| format!("reading {}", input.display()))?;

    if params.filter_hifreq < 0.0 {
        params.filter_hifreq = audio.sample_rate as f32 / 2.0;
    }

    let mut out_channels = Vec::with_capacity(audio.channels.len());
    for channel in &audio.channels {
        let dur = (channel.len() as f32 / audio.sample_rate as f32) * params.time_factor;
        out_channels.push(process_channel(channel, audio.sample_rate, &params, dur));
    }

    // Channels can come out slightly different lengths (each is an
    // independent per-channel loop) - trim to the shortest so the
    // written file has uniform channel lengths.
    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // `plainpv`'s default `rescalev == 1`: after all frames are written,
    // rescale the *entire* output file so its peak amplitude (across all
    // channels) matches the *input* file's peak amplitude - found while
    // chasing a whole-file amplitude mismatch against real `plainpv`
    // output (`legacy/pvc_lib/fileio.c`'s `rescaleThisBuffer`, called
    // from a post-pass over a temp file, easy to miss since it's file-
    // I/O code, not DSP). A real, on-by-default behavior, not a
    // debug/display feature, so this port needs it despite living in the
    // "reporting" half of the legacy tool. Skipped, like the C, if
    // either peak is exactly zero (silence).
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
    // 16-bit PCM, not the C's "copy the input file's format" - pvc-io's
    // reader normalizes everything to f32 and doesn't retain the
    // source's original bit depth, so exact format preservation isn't
    // available yet; 16-bit is the most common case and, unlike f32,
    // readable by both other tools' PCM-only paths and this repo's own
    // Python-based golden-harness comparator.
    pvc_io::write_wav(output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", output.display()))?;
    Ok(())
}
