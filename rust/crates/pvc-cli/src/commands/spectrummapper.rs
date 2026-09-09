//! `pvc spectrummapper`: reads raw audio, runs the selected channel(s)
//! through `pvc_core::tools::spectrummapper::analyze_channel`, writes
//! the resulting formant tracks. Writes no audio at all - see
//! `pvc-core::tools::spectrummapper`'s own doc comment.

use std::fs::File;
use std::io::{BufWriter, Write};

use anyhow::{Context, Result};
use pvc_core::tools::spectrummapper::{analyze_channel, SpectrumMapperParams, TrackPoint};

use crate::cli::SpectrummapperArgs;

/// `-S`'s own binary layout: for each track, one little-endian `f32`
/// length prefix followed by that many 6-`f32` records (time, cf, amp,
/// db, bw, q) - matches `binaryOutputArray`'s own column order exactly.
fn write_segments_file(path: &std::path::Path, tracks: &[Vec<TrackPoint>]) -> Result<()> {
    let file = File::create(path).with_context(|| format!("creating {}", path.display()))?;
    let mut w = BufWriter::new(file);
    for track in tracks {
        w.write_all(&(track.len() as f32).to_le_bytes())?;
        for p in track {
            for v in [p.time, p.cf, p.amp, p.db, p.bw, p.q] {
                w.write_all(&v.to_le_bytes())?;
            }
        }
    }
    w.flush()?;
    Ok(())
}

fn write_ascii_segments_file(path: &std::path::Path, tracks: &[Vec<TrackPoint>]) -> Result<()> {
    let file = File::create(path).with_context(|| format!("creating {}", path.display()))?;
    let mut w = BufWriter::new(file);
    for track in tracks {
        for p in track {
            writeln!(w, "{} {} {} {} {} {}", p.time, p.cf, p.amp, p.db, p.bw, p.q)?;
        }
        writeln!(w)?;
    }
    w.flush()?;
    Ok(())
}

fn write_scatter_file(path: &std::path::Path, tracks: &[Vec<TrackPoint>]) -> Result<()> {
    // The real scatter file is written from the *raw per-frame* formant
    // points, before segmentation - approximated here from the
    // constructed tracks' own points, which for `--onset-release-mode
    // none` are exactly the raw points in time order.
    let file = File::create(path).with_context(|| format!("creating {}", path.display()))?;
    let mut w = BufWriter::new(file);
    for track in tracks {
        for p in track {
            writeln!(w, "{} {}\n", p.time, p.cf)?;
        }
    }
    w.flush()?;
    Ok(())
}

pub fn run(args: &SpectrummapperArgs) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let sample_rate = audio.sample_rate;
    let idur = audio
        .channels
        .first()
        .map(|c| c.len() as f32 / sample_rate as f32)
        .unwrap_or(0.0);
    let endt = if args.end <= 0.0 {
        idur
    } else {
        args.end.min(idur)
    };
    let begint = args.begin.max(0.0);
    anyhow::ensure!(
        endt > begint,
        "end time ({endt}) must be after begin time ({begint})"
    );
    let begin_sample = (begint * sample_rate as f32) as usize;
    let end_sample = (endt * sample_rate as f32) as usize;
    let dur = endt - begint;

    // `-j`'s own C initializer sets `highFreqLimit.A[0] = nyquist`
    // directly (not a runtime "0 means nyquist" fallback check) - the
    // CLI default (`0`, a placeholder that can't know the real sample
    // rate at parse time) is resolved to the same effective value here,
    // once, matching `tools::ratechanger`'s own established convention
    // for this kind of dynamic default.
    let nyquist = sample_rate as f32 / 2.0;
    let high_freq_limit = match &args.high_freq_limit {
        pvc_core::ControlFn::Const(v) if *v <= 0.0 => pvc_core::ControlFn::Const(nyquist),
        other => other.clone(),
    };

    let params = SpectrumMapperParams {
        window: args.window,
        window_size: args.window_size,
        frames_per_sec: args.frames_per_sec,
        shelf_low_db: args.shelf_low_gain,
        shelf_high_db: args.shelf_high_gain,
        shelf_low_freq: args.shelf_low_freq,
        shelf_high_freq: args.shelf_high_freq,
        eq_bypass: args.eq_bypass,
        low_freq_limit: args.low_freq_limit.clone(),
        high_freq_limit,
        minimum_formant_db: args.minimum_formant_db,
        formant_selection_threshold: args.formant_selection_threshold,
        minimum_decibels: args.minimum_decibels,
        maximum_decibels: args.maximum_decibels,
        minimum_segment_length: args.minimum_segment_length,
        maximum_segment_length: args.maximum_segment_length,
        minimum_segment_duration: args.minimum_segment_duration,
        maximum_segment_duration: args.maximum_segment_duration,
        max_frequency_change_per_ms: args.max_frequency_change_per_ms,
        max_decibel_rise_per_ms: args.max_decibel_rise_per_ms,
        max_decibel_fall_per_ms: args.max_decibel_fall_per_ms,
        linkage_time: args.linkage_time,
        maximum_frequency_linkage: args.maximum_frequency_linkage,
        onset_release_mode: args.onset_release_mode,
        onset_duration: args.onset_duration,
        release_duration: args.release_duration,
        time_shift: args.time_shift,
    };

    let num_channels = audio.channels.len();
    let selected_channels: Vec<&Vec<f32>> = if args.channel == 0 {
        audio.channels.iter().collect()
    } else {
        let index = args.channel - 1;
        anyhow::ensure!(
            index < num_channels,
            "--channel {} out of range (input has {num_channels} channel(s))",
            args.channel
        );
        vec![&audio.channels[index]]
    };

    let mut all_tracks: Vec<Vec<TrackPoint>> = Vec::new();
    for channel in selected_channels {
        let end = end_sample.min(channel.len());
        let trimmed: &[f32] = if begin_sample < end {
            &channel[begin_sample..end]
        } else {
            &[]
        };
        let tracks = analyze_channel(trimmed, sample_rate, dur, args.fft, &params);
        all_tracks.extend(tracks);
    }

    write_segments_file(&args.segments_file, &all_tracks)?;
    if let Some(path) = &args.ascii_segments_file {
        write_ascii_segments_file(path, &all_tracks)?;
    }
    if let Some(path) = &args.scatter_file {
        write_scatter_file(path, &all_tracks)?;
    }

    Ok(())
}
