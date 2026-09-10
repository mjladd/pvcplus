//! The "what did this run produce" summary every audio-writing command
//! prints after a successful write. Replaces the legacy tools' own
//! verbose banner-style reporting with one line (or one JSON object) of
//! the numbers the plan's own Phase 4.3 item asks for: peak level,
//! clipping, and duration - the same three the old README's own "OUTPUT
//! STATISTICS" section (see `docs/concepts.md`) described, just no
//! longer buried in per-frame banner spam.
//!
//! `clipped_samples` counts samples whose absolute value exceeds `1.0`
//! in the already-resynthesized `f32` buffer, before any integer
//! quantization `write_wav`/`write_flac` does - the same convention the
//! legacy tools used ("output values exceeding the normalized peak
//! amplitude of 1. are clipped"), and format-agnostic, since it doesn't
//! depend on which `SampleFormat` the caller happens to write.

use std::path::Path;

use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct RunSummary {
    pub output: String,
    pub sample_rate: u32,
    pub channels: usize,
    pub duration_secs: f32,
    /// `None` for total silence (`peak == 0.0`, where `20 * log10` is
    /// undefined) - serializes as JSON `null`, prints as "-inf dB".
    pub peak_db: Option<f32>,
    pub clipped_samples: usize,
}

impl RunSummary {
    pub fn from_buffer(output: &Path, buffer: &pvc_io::AudioBuffer) -> Self {
        let duration_secs = buffer.num_frames() as f32 / buffer.sample_rate as f32;
        let mut peak = 0.0f32;
        let mut clipped = 0usize;
        for channel in &buffer.channels {
            for &s in channel {
                let a = s.abs();
                if a > peak {
                    peak = a;
                }
                if a > 1.0 {
                    clipped += 1;
                }
            }
        }
        let peak_db = (peak > 0.0).then(|| 20.0 * peak.log10());
        RunSummary {
            output: output.display().to_string(),
            sample_rate: buffer.sample_rate,
            channels: buffer.channels.len(),
            duration_secs,
            peak_db,
            clipped_samples: clipped,
        }
    }

    /// Prints this summary, unless `quiet`. `--quiet` wins over `--json`,
    /// matching `commands::run`'s own established precedent, where
    /// `--quiet` already suppresses its printed preset regardless of
    /// `--json`.
    pub fn print(&self, json: bool, quiet: bool) {
        if quiet {
            return;
        }
        if json {
            if let Ok(text) = serde_json::to_string_pretty(self) {
                println!("{text}");
            }
            return;
        }
        let peak = match self.peak_db {
            Some(db) => format!("{db:.2} dB"),
            None => "-inf dB (silence)".to_string(),
        };
        let clip = if self.clipped_samples > 0 {
            format!(", {} clipped sample(s)", self.clipped_samples)
        } else {
            String::new()
        };
        println!(
            "{}: {:.3}s, {} channel(s) @ {} Hz, peak {peak}{clip}",
            self.output, self.duration_secs, self.channels, self.sample_rate,
        );
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pvc_io::AudioBuffer;
    use std::path::PathBuf;

    #[test]
    fn from_buffer_computes_peak_db_and_duration() {
        let buffer = AudioBuffer {
            sample_rate: 1000,
            channels: vec![vec![0.5, -1.0, 0.25]],
        };
        let s = RunSummary::from_buffer(&PathBuf::from("out.wav"), &buffer);
        assert_eq!(s.duration_secs, 0.003);
        assert_eq!(s.channels, 1);
        assert!((s.peak_db.unwrap() - 0.0).abs() < 1e-4); // peak 1.0 == 0 dB
        assert_eq!(s.clipped_samples, 0);
    }

    #[test]
    fn from_buffer_counts_clipped_samples() {
        let buffer = AudioBuffer {
            sample_rate: 1000,
            channels: vec![vec![1.5, -2.0, 0.1], vec![0.9, 1.01, -1.0]],
        };
        let s = RunSummary::from_buffer(&PathBuf::from("out.wav"), &buffer);
        // 1.5, -2.0, 1.01 exceed 1.0 in absolute value; -1.0 does not.
        assert_eq!(s.clipped_samples, 3);
    }

    #[test]
    fn from_buffer_silence_has_no_peak_db() {
        let buffer = AudioBuffer {
            sample_rate: 44100,
            channels: vec![vec![0.0, 0.0, 0.0]],
        };
        let s = RunSummary::from_buffer(&PathBuf::from("out.wav"), &buffer);
        assert_eq!(s.peak_db, None);
        assert_eq!(s.clipped_samples, 0);
    }
}
