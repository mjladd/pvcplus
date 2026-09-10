//! `pvc formantsmapper`: reads audio plus source/target formant files,
//! builds the per-bin output mapping once, runs the selected channel(s)
//! through `pvc_core::tools::formantsmapper::process_channel`, writes
//! the result.

use anyhow::{Context, Result};
use pvc_core::tools::formantsmapper::{
    apply_bandwidth_extension_factor, build_output_bins, compute_residue_bins,
    extend_source_formants, extend_target_formants, filter_formants, process_channel, BankParams,
    Formant, FormantsMapperParams,
};

use crate::cli::FormantsmapperArgs;

fn to_formant(r: pvc_io::FormantRecord) -> Formant {
    Formant {
        center_freq: r.center_freq,
        amp: r.amp,
        bw: r.bw,
        q: r.q,
        index: r.index,
        low_stop_band_index: r.low_stop_band_index,
        high_stop_band_index: r.high_stop_band_index,
    }
}

pub fn run(args: &FormantsmapperArgs, json: bool, quiet: bool) -> Result<()> {
    let audio = pvc_io::read_audio(&args.input)
        .with_context(|| format!("reading {}", args.input.display()))?;

    let n2 = args.fft / 2;
    let fundamental = audio.sample_rate as f32 / args.fft as f32;
    let nyquist = audio.sample_rate as f32 / 2.0;

    let (source_records, source_n2) = pvc_io::read_formants(&args.source_formants)
        .with_context(|| format!("reading {}", args.source_formants.display()))?;
    anyhow::ensure!(
        source_n2 == n2,
        "{}: formant file's own analysis bin count ({source_n2}) does not match --fft {} (N2={n2})",
        args.source_formants.display(),
        args.fft
    );
    let (target_records, target_n2) = pvc_io::read_formants(&args.target_formants)
        .with_context(|| format!("reading {}", args.target_formants.display()))?;
    anyhow::ensure!(
        target_n2 == n2,
        "{}: formant file's own analysis bin count ({target_n2}) does not match --fft {} (N2={n2})",
        args.target_formants.display(),
        args.fft
    );

    let mut source_formants: Vec<Formant> = source_records.into_iter().map(to_formant).collect();
    apply_bandwidth_extension_factor(&mut source_formants, args.bandwidth_extension_factor, n2);
    let source_formants = filter_formants(
        &source_formants,
        args.source_db_threshold,
        args.source_low_freq,
        args.source_high_freq,
        nyquist,
    );
    anyhow::ensure!(
        !source_formants.is_empty(),
        "no source formants remain after filtering"
    );
    let source_ext = extend_source_formants(
        &source_formants,
        args.extend_source,
        args.source_extend_db_threshold,
        args.source_extend_peak_partial,
        args.source_extend_octaves,
        args.extend_partial_rolloff,
        fundamental,
        nyquist,
    );

    let target_formants: Vec<Formant> = target_records.into_iter().map(to_formant).collect();
    let transpose = pvc_core::units::SemitonesToMult::new().convert(args.target_transpose);
    let target_formants: Vec<Formant> = target_formants
        .into_iter()
        .map(|f| Formant {
            center_freq: f.center_freq * transpose,
            bw: f.bw * transpose,
            ..f
        })
        .collect();
    let target_formants = filter_formants(
        &target_formants,
        args.target_db_threshold,
        args.target_low_freq,
        args.target_high_freq,
        nyquist,
    );
    anyhow::ensure!(
        !target_formants.is_empty(),
        "no target formants remain after filtering"
    );
    let target_ext = extend_target_formants(
        &target_formants,
        args.extend_target,
        args.target_extend_db_threshold,
        args.target_extend_peak_partial,
        args.target_extend_octaves,
        args.extend_partial_rolloff,
        nyquist,
    );

    let output_bins = build_output_bins(
        &source_ext,
        &target_ext,
        fundamental,
        n2,
        args.amp_gain_limit,
    );
    let residue_bins = if args.residue_bins {
        compute_residue_bins(&output_bins, n2)
    } else {
        Vec::new()
    };

    let params = FormantsMapperParams {
        window_size: args.window_size,
        window: args.window,
        frames_per_sec: args.frames_per_sec,
        time_factor: args.time_factor,
        gain_db: args.gain.clone(),
        pitch_transpose: args.pitch.clone(),
        freq_shift: args.freq_shift.clone(),
        attack: args.attack.clone(),
        release: args.release.clone(),
        shelf_low_db: args.shelf_low_gain.clone(),
        shelf_high_db: args.shelf_high_gain.clone(),
        shelf_low_freq: args.shelf_low_freq.clone(),
        shelf_high_freq: args.shelf_high_freq.clone(),
        bank_a: BankParams {
            freq_interp: args.bank_a_freq_interp.clone(),
            amp_interp: args.bank_a_amp_interp.clone(),
            gain_db: args.bank_a_gain.clone(),
            rolloff_per_octave: args.bank_a_rolloff.clone(),
            pitch_transpose: args.bank_a_pitch.clone(),
        },
        bank_b: if args.dual_bank {
            Some(BankParams {
                freq_interp: args.bank_b_freq_interp.clone(),
                amp_interp: args.bank_b_amp_interp.clone(),
                gain_db: args.bank_b_gain.clone(),
                rolloff_per_octave: args.bank_b_rolloff.clone(),
                pitch_transpose: args.bank_b_pitch.clone(),
            })
        } else {
            None
        },
        residue_gain_db: args.residue_gain.clone(),
        threshold_db: args.threshold,
    };

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

    let mut out_channels = Vec::with_capacity(selected_channels.len());
    for channel in selected_channels {
        let end = end_sample.min(channel.len());
        let trimmed: &[f32] = if begin_sample < end {
            &channel[begin_sample..end]
        } else {
            &[]
        };
        let dur = (trimmed.len() as f32 / sample_rate as f32) * params.time_factor;
        out_channels.push(process_channel(
            trimmed,
            &output_bins,
            &residue_bins,
            args.fft,
            sample_rate,
            dur,
            &params,
        ));
    }

    let min_len = out_channels.iter().map(|c| c.len()).min().unwrap_or(0);
    for c in &mut out_channels {
        c.truncate(min_len);
    }

    // `-=`'s own documented default (`1 = Rescale to level of input
    // file`) - the same convention already established for
    // `pv`/`twarp`/`filter`/`filtdeviator`/`inharmonator`.
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
        sample_rate,
        channels: out_channels,
    };
    pvc_io::write_wav(&args.output, &out_buffer, pvc_io::SampleFormat::I16)
        .with_context(|| format!("writing {}", args.output.display()))?;
    crate::summary::RunSummary::from_buffer(&args.output, &out_buffer).print(json, quiet);
    Ok(())
}
