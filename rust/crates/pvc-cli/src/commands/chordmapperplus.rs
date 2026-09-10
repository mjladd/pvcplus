//! `pvc chordmapperplus`: reads a `.pva` analysis file and a tone data
//! file, runs each output channel through
//! `pvc_core::tools::chordmapperplus::process_channel`, writes the
//! resynthesized audio.

use std::path::Path;

use anyhow::{Context, Result};
use pvc_core::tools::chordmapperplus::{
    compute_channel_average, compute_static_freq_response, detect_vibrato_periods,
    parse_tone_data_file, process_channel, setup_bands, setup_noise_bands, ChordmapperplusParams,
    StaticFreqResponse, ToneParams,
};
use pvc_core::ControlFn;

use crate::cli::ChordmapperplusArgs;

/// A tone-data-file field naming a breakpoint-table file (rather than a
/// bare constant) is resolved relative to the current directory, same
/// as every other `@path`-style control-function reference in this CLI.
fn resolve_table(token: &str) -> Result<Vec<f32>, String> {
    pvc_io::read_control_file(Path::new(token))
        .map(|data| data.values)
        .map_err(|e| format!("reading control file {token:?}: {e}"))
}

pub fn run(args: &ChordmapperplusArgs, json: bool, quiet: bool) -> Result<()> {
    let analysis = pvc_io::read_pva(&args.analysis)
        .or_else(|_| pvc_io::read_legacy_pva(&args.analysis))
        .with_context(|| format!("reading {}", args.analysis.display()))?;

    let tones_text = std::fs::read_to_string(&args.tones)
        .with_context(|| format!("reading {}", args.tones.display()))?;
    let tones: Vec<ToneParams> = parse_tone_data_file(&tones_text, resolve_table)
        .map_err(|e| anyhow::anyhow!("{}: {e}", args.tones.display()))?;
    anyhow::ensure!(
        !tones.is_empty(),
        "{}: no tones found",
        args.tones.display()
    );

    let n = analysis.header.n as usize;
    let r = analysis.header.sample_rate;
    let d = analysis.header.d;
    let nyquist = r as f32 / 2.0;
    let fundamental = r as f32 / n as f32;

    let band_setup = setup_bands(&tones, nyquist, fundamental, n)
        .map_err(|e| anyhow::anyhow!("setting up tone partials: {e}"))?;

    // `-R1`: natural-vibrato-period detection, run once (not per output
    // channel - the real C's own detected window is shared across every
    // channel's own loop, matching `main()`'s own single pre-loop
    // detection pass).
    let natural_vibrato = if args.natural_vibrato {
        let original_path = args
            .original_audio
            .as_ref()
            .ok_or_else(|| anyhow::anyhow!("--natural-vibrato requires --original-audio"))?;
        let reference_raw = args
            .vibrato_reference
            .ok_or_else(|| anyhow::anyhow!("--natural-vibrato requires --vibrato-reference"))?;
        let reference_hz = if (0.0..=12.0).contains(&reference_raw) {
            pvc_core::response::oppc_to_hz(reference_raw)
        } else {
            reference_raw
        };
        let original = pvc_io::read_audio(original_path)
            .with_context(|| format!("reading {}", original_path.display()))?;
        let pitch_params = pvc_core::tools::pitchtracker::PitchtrackerParams {
            fft_size: n,
            window_size: 2 * n,
            window: pvc_core::Window::Hamming,
            frames_per_sec: r as f32 / d as f32,
            band_low: ControlFn::Const((reference_hz * 5.0 / 6.0).max(0.0)),
            band_high: ControlFn::Const(reference_hz * 6.0 / 5.0),
            method: pvc_core::tools::pitchtracker::DetectMethod::OptimalComb,
            window_min_secs: 0.05,
            window_max_secs: 0.2,
            detect_threshold_db: -80.0,
            mode_filter_window_secs: 0.0,
            oversample_factor: 0.0,
            smooth_response_secs: 0.0,
            channel_method: pvc_core::tools::envelope::ChannelMethod::Average,
            compress_threshold_db: ControlFn::Const(0.0),
            compress_amount_db: ControlFn::Const(0.0),
            gate_threshold_db: ControlFn::Const(-96.0),
            warp: ControlFn::Const(0.0),
            attack: ControlFn::Const(0.0),
            release: ControlFn::Const(0.0),
            output_rate: r as f32 / d as f32,
            output_format: pvc_core::tools::pitchtracker::OutputFormat::Freq,
            reference_pitch: ControlFn::Const(reference_hz),
        };
        let dur = original
            .channels
            .first()
            .map(|c| c.len() as f32 / original.sample_rate as f32)
            .unwrap_or(0.0);
        let pitch_track = pvc_core::tools::pitchtracker::process(
            &original.channels,
            original.sample_rate,
            &pitch_params,
            dur,
        );
        let detection = detect_vibrato_periods(
            &pitch_track,
            pitch_params.output_rate,
            reference_hz,
            args.vibrato_deviation,
        )
        .ok_or_else(|| {
            anyhow::anyhow!(
                "--natural-vibrato: no usable vibrato periods found in {}",
                original_path.display()
            )
        })?;
        Some(detection)
    } else {
        None
    };

    let params = ChordmapperplusParams {
        window_size: args.window_size,
        frames_per_sec: args.frames_per_sec,
        duration: args.duration,
        time_origin: args.time_origin.clone(),
        rate: args.rate.clone(),
        window_low: args.window_low.clone(),
        window_high: args.window_high.clone(),
        loop_smooth: args.loop_smooth.clone(),
        autostop: args.window_mode,
        loop_mode: args.loop_mode,
        master_gain_db: args.master_gain.clone(),
        tones_master_gain_db: args.tones_gain.clone(),
        tones_master_freq_shift_hz: args.tones_freq_shift.clone(),
        tones_macro_pitch_semitones: args.tones_pitch.clone(),
        threshold_db: args.threshold,
        onset_release_mode: args.onset_release,
        loop_normalization: args.loop_normalize,
        rate_correlated_tone_control_db: args.rate_correlated_tone.clone(),
        rate_correlated_noise_control_db: args.rate_correlated_noise.clone(),
        rate_correlated_force_suppression: args.rate_correlated_force_suppression,
        noise_band_decibel_limit_db: args.noise_limit.clone(),
        noise_band_decibel_limit_rolloff_db: args.noise_limit_rolloff.clone(),
        pitch_change_expansion_db: args.pitch_change_expansion.clone(),
        frequency_change_suppression_threshold: args.frequency_change_threshold.clone(),
        frequency_change_suppression_threshold_increase_response_secs: args
            .frequency_change_response
            .clone(),
        natural_vibrato,
        vibrato_period_durations_mechanical_to_natural: args.vibrato_period_mechanical.clone(),
    };

    // `ochan`/`channelflag` in the C: every tone's own
    // `tone_channel_output_number` (`0` means "every channel") decides
    // how many output channels are actually needed, unless `--channel`
    // pins one specific channel.
    let max_channel_output_number = tones
        .iter()
        .map(|t| t.tone_channel_output_number as usize)
        .max()
        .unwrap_or(0);
    let analysis_chan = analysis.channels.len();
    let (begin_channel, output_channel_count) = if args.channel == 0 {
        (0usize, max_channel_output_number.max(analysis_chan).max(1))
    } else {
        (args.channel - 1, max_channel_output_number.max(1))
    };

    let mut out_channels = Vec::with_capacity(output_channel_count);
    for out_idx in begin_channel..(begin_channel + output_channel_count) {
        let ainchan = if args.channel == 0 {
            out_idx % analysis_chan.max(1)
        } else {
            args.channel - 1
        };
        let ainchan = ainchan.min(analysis_chan.saturating_sub(1));
        let channel_analysis = &analysis.channels[ainchan];

        let (channel_average, channel_average_peak_amp) =
            compute_channel_average(channel_analysis, r as f32 / d as f32, n + 2);
        let noise_setup = setup_noise_bands(
            &tones,
            &band_setup,
            &channel_average,
            channel_average_peak_amp,
            n,
            n,
        );
        let static_freq: StaticFreqResponse = compute_static_freq_response(
            channel_analysis,
            r as f32 / d as f32,
            n + 2,
            fundamental,
            &band_setup,
        );

        let noise_bands: Option<&_> = if noise_setup.bank_tone.is_empty() {
            None
        } else {
            Some(&noise_setup)
        };

        let out = process_channel(
            channel_analysis,
            n,
            d,
            r,
            &tones,
            &band_setup,
            &static_freq,
            noise_bands,
            out_idx - begin_channel,
            &params,
        );
        out_channels.push(out);
    }

    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // Same `ipeakamp`/analysis-header-peak output rescale
    // `tools::twarp`'s own CLI wiring already established for this
    // `-f<analysis.pva>` input style (`main()` here also does
    // `ipeakamp[k] = normamp[k]` at startup) - see that command's own
    // doc comment for why this can look like a near-silence bug on some
    // inputs; it's what the real C actually does by default.
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
        sample_rate: r,
        channels: out_channels,
    };
    pvc_io::write_wav(&args.output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", args.output.display()))?;
    crate::summary::RunSummary::from_buffer(&args.output, &out_buffer).print(json, quiet);

    Ok(())
}
