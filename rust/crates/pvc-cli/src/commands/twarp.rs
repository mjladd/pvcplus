//! `pvc twarp`: reads a `.pva` analysis file, runs each channel through
//! `pvc_core::tools::twarp::process_channel`, writes the resynthesized
//! audio.

use anyhow::{Context, Result};
use pvc_core::tools::twarp::{process_channel, TwarpParams};

use crate::cli::TwarpArgs;

pub fn run(args: &TwarpArgs, json: bool, quiet: bool) -> Result<()> {
    let analysis = pvc_io::read_pva(&args.analysis)
        .or_else(|_| pvc_io::read_legacy_pva(&args.analysis))
        .with_context(|| format!("reading {}", args.analysis.display()))?;

    let params = TwarpParams {
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        duration: args.duration,
        time_origin: args.time_origin.clone(),
        rate: args.rate.clone(),
        window_low: args.window_low.clone(),
        window_high: args.window_high.clone(),
        time_response: args.time_response.clone(),
        loop_smooth: args.loop_smooth.clone(),
        autostop: args.window_mode,
        loop_mode: args.loop_mode,
        onset_release: args.onset_release,
        pitch_transpose_semitones: args.pitch.clone(),
        freq_shift_hz: args.freq_shift.clone(),
        gain_db: args.gain.clone(),
        attack_secs: args.attack.clone(),
        release_secs: args.release.clone(),
        freq_response_secs: args.freq_response_time.clone(),
        warpshape: args.warp.clone(),
        shelf_low_db: args.shelf_low_gain.clone(),
        shelf_high_db: args.shelf_high_gain.clone(),
        shelf_low_freq: args.shelf_low_freq.clone(),
        shelf_high_freq: args.shelf_high_freq.clone(),
        threshold_db: args.threshold,
    };

    let mut out_channels = Vec::with_capacity(analysis.channels.len());
    for channel in &analysis.channels {
        out_channels.push(process_channel(
            channel,
            analysis.header.n as usize,
            analysis.header.d,
            analysis.header.sample_rate,
            &params,
        ));
    }

    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // `twarp.c` copies the analysis file's own stored peak amplitudes
    // into `ipeakamp[]` (`for (k...) ipeakamp[k] = normamp[k];`) and,
    // with `rescalev` defaulting to `1` (same shared global `pvc pv`'s
    // rescale already found - see `commands::pv::run_pv`), rescales the
    // *entire resynthesized output* so its peak matches that stored
    // value. That value is an FFT-*magnitude*-domain peak from
    // `pvanalysis.c`'s own analysis pass, not a waveform-amplitude peak
    // - wildly mismatched in scale from the actual resynthesized audio
    // (confirmed empirically: a 0.5-amplitude 440Hz sine's analysis peak
    // came out around 0.00046, and real `twarp`'s default output for
    // that file was rescaled down to a peak 16-bit sample value of just
    // 15 - audible as near-total silence). This looks like a genuine bug
    // in the real tool (reusing a generic "rescale to the input's peak"
    // mechanism with a value that was never meant to represent a
    // waveform peak), but it's what `twarp -F<analysis>.pva` actually
    // does by default, so it's reproduced here for golden-harness parity
    // rather than silently "fixed" - skipped, like the C, if either peak
    // is exactly zero (including `pvc analyze`-written files, which
    // don't populate `peak_amps` yet).
    let target_peak = analysis.peak_amps.iter().copied().fold(0.0f32, f32::max);
    let output_peak = out_channels
        .iter()
        .flat_map(|c| c.iter())
        .copied()
        .fold(0.0f32, |a, b| a.max(b.abs()));
    if target_peak > 0.0 && output_peak > 0.0 {
        let ampval = target_peak / output_peak;
        for c in &mut out_channels {
            for s in c.iter_mut() {
                *s *= ampval;
            }
        }
    }

    let out_buffer = pvc_io::AudioBuffer {
        sample_rate: analysis.header.sample_rate,
        channels: out_channels,
    };
    pvc_io::write_wav(&args.output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", args.output.display()))?;
    crate::summary::RunSummary::from_buffer(&args.output, &out_buffer).print(json, quiet);
    Ok(())
}
