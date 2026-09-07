//! `pvc fn response filtresponsemaker|chordresponsemaker`: synthesizes a
//! `.fr` file from a breakpoint/partial data file.

use anyhow::{bail, Context, Result};
use pvc_core::tools::chordresponsemaker::{self, ChordTone};
use pvc_core::tools::filtresponsemaker::{self, Breakpoint};
use pvc_core::tools::groupdelaymaker::{self, GroupDelayTone};
use pvc_core::Frame;

use crate::cli::ResponseCommand;

/// Reads whitespace-separated floats from an ASCII data file - the
/// plain-numbers case of the legacy `cut_data_lines()` + `fscanf(data, "
/// %f ", ...)` pipeline every data-file-reading legacy tool uses.
/// `{...}` block comments and `!`/`m` solo/mute line filtering (real
/// features of that legacy pipeline) aren't supported - no shipped
/// fixture or documented workflow exercises them, and the new CLI's own
/// data files are plain numbers.
fn read_floats(path: &std::path::Path) -> Result<Vec<f32>> {
    let text =
        std::fs::read_to_string(path).with_context(|| format!("reading {}", path.display()))?;
    text.split_whitespace()
        .map(|tok| {
            tok.parse::<f32>()
                .with_context(|| format!("bad number {tok:?} in {}", path.display()))
        })
        .collect()
}

fn sample_rate_of(path: &std::path::Path) -> Result<u32> {
    Ok(pvc_io::read_audio(path)
        .with_context(|| format!("reading {}", path.display()))?
        .sample_rate)
}

pub fn run(cmd: &ResponseCommand) -> Result<()> {
    match cmd {
        ResponseCommand::Filtresponsemaker {
            fft,
            breakpoints,
            target_sound_file,
            mode,
            output,
        } => {
            let values = read_floats(breakpoints)?;
            if values.len() % 2 != 0 {
                bail!(
                    "{}: odd number of values ({}) - expected (pitch-or-hz, db) duples",
                    breakpoints.display(),
                    values.len()
                );
            }
            let points: Vec<Breakpoint> = values
                .chunks_exact(2)
                .map(|c| Breakpoint {
                    pitch_or_hz: c[0],
                    db: c[1],
                })
                .collect();
            if points.len() < 2 {
                bail!("{}: need at least 2 breakpoints", breakpoints.display());
            }
            let sample_rate = sample_rate_of(target_sound_file)?;
            let frame = filtresponsemaker::synthesize(&points, *fft, sample_rate, *mode);
            write_frame(output, &frame)
        }

        ResponseCommand::Chordresponsemaker {
            fft,
            sample_rate,
            partials,
            accumulation,
            band_window,
            mode,
            output,
        } => {
            let values = read_floats(partials)?;
            if values.len() % 6 != 0 {
                bail!(
                    "{}: {} values isn't a multiple of 6 - expected (pitch, num_partials, \
                     bandwidth, db, spacing, rolloff) sextuples",
                    partials.display(),
                    values.len()
                );
            }
            let tones: Vec<ChordTone> = values
                .chunks_exact(6)
                .map(|c| ChordTone {
                    pitch_or_hz: c[0],
                    num_partials: c[1] as i32,
                    bandwidth: c[2],
                    db: c[3],
                    partial_spacing: c[4],
                    db_rolloff_per_octave: c[5],
                })
                .collect();
            let frame = chordresponsemaker::synthesize(
                &tones,
                *fft,
                *sample_rate,
                *accumulation,
                *band_window,
                *mode,
            );
            write_frame(output, &frame)
        }

        ResponseCommand::Groupdelaymaker {
            analysis,
            partials,
            edge_db,
            default_db,
            default_delay,
            method,
            output,
        } => {
            let pva = pvc_io::read_pva(analysis)
                .or_else(|_| pvc_io::read_legacy_pva(analysis))
                .with_context(|| format!("reading {}", analysis.display()))?;

            let values = read_floats(partials)?;
            if values.len() % 7 != 0 {
                bail!(
                    "{}: {} values isn't a multiple of 7 - expected (pitch, num_partials, \
                     bandwidth, db, spacing, rolloff, delay_secs) septuples",
                    partials.display(),
                    values.len()
                );
            }
            let tones: Vec<GroupDelayTone> = values
                .chunks_exact(7)
                .map(|c| GroupDelayTone {
                    pitch_or_hz: c[0],
                    num_partials: c[1] as i32,
                    bandwidth: c[2],
                    db: c[3],
                    partial_spacing: c[4],
                    db_rolloff_total: c[5],
                    delay_secs: c[6],
                })
                .collect();
            let bins = groupdelaymaker::synthesize(
                &tones,
                pva.header.n as usize,
                pva.header.sample_rate,
                *edge_db,
                *default_db,
                *default_delay,
                *method,
            );
            write_frame(output, &Frame { bins })
        }
    }
}

fn write_frame(output: &std::path::Path, frame: &Frame) -> Result<()> {
    pvc_io::write_fr(output, &frame.to_pva_floats())
        .with_context(|| format!("writing {}", output.display()))?;
    Ok(())
}
