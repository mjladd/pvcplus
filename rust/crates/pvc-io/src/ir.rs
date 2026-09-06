//! `.ir` impulse-response file I/O: the format `impulseresponse.c` writes,
//! consumed by (not yet ported) `irconvolver`/`irconvolvesequencer` for
//! FFT-based convolution. Unlike `.pva`, this legacy layout is already
//! simple and unambiguous - a fixed four-`i32` header, no per-tool
//! seek-and-overwrite tricks - so there's no separate "new" format to
//! design; `pvc`'s own writer just produces the same thing, little-endian
//! (matching this crate's `.fr` convention, `response.rs`).
//!
//! Layout: `channels`, `fft_size`, `impulse_len`, `sample_rate` (four
//! little-endian `i32`s, in that order - the C's own `numOutChannels`,
//! `N`, `Lh`, `R`), followed by `channels` blocks of `fft_size`
//! little-endian `f32`s each: that channel's rfft-format spectrum (real/
//! imaginary interleaved per `rfft()`'s own packed layout, zero-padded to
//! `fft_size`), already peak-normalized.

use std::fs;
use std::io::Write;
use std::path::Path;

use thiserror::Error;

const HEADER_BYTES: usize = 16;

#[derive(Debug, Error)]
pub enum IrError {
    #[error(transparent)]
    Io(#[from] std::io::Error),
    #[error("{0}: file too short for an impulse-response header ({1} bytes, need at least {HEADER_BYTES})")]
    TooShort(String, usize),
    #[error("{0}: channel data ({1} bytes) isn't exactly channels*fft_size*4 ({2} bytes)")]
    SizeMismatch(String, usize, usize),
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct IrHeader {
    pub channels: u32,
    pub fft_size: u32,
    pub impulse_len: u32,
    pub sample_rate: u32,
}

/// `channels[ch]` is that channel's `header.fft_size` rfft-format floats.
#[derive(Debug, Clone, PartialEq)]
pub struct IrData {
    pub header: IrHeader,
    pub channels: Vec<Vec<f32>>,
}

pub fn read_ir(path: &Path) -> Result<IrData, IrError> {
    let display = path.display().to_string();
    let bytes = fs::read(path)?;
    if bytes.len() < HEADER_BYTES {
        return Err(IrError::TooShort(display, bytes.len()));
    }

    let read_u32 =
        |o: usize| u32::from_le_bytes([bytes[o], bytes[o + 1], bytes[o + 2], bytes[o + 3]]);
    let header = IrHeader {
        channels: read_u32(0),
        fft_size: read_u32(4),
        impulse_len: read_u32(8),
        sample_rate: read_u32(12),
    };

    let data = &bytes[HEADER_BYTES..];
    let per_channel_bytes = header.fft_size as usize * 4;
    let expected = header.channels as usize * per_channel_bytes;
    if data.len() != expected {
        return Err(IrError::SizeMismatch(display, data.len(), expected));
    }

    let channels = data
        .chunks_exact(per_channel_bytes)
        .map(|block| {
            block
                .chunks_exact(4)
                .map(|c| f32::from_le_bytes([c[0], c[1], c[2], c[3]]))
                .collect()
        })
        .collect();

    Ok(IrData { header, channels })
}

pub fn write_ir(path: &Path, data: &IrData) -> Result<(), IrError> {
    let mut f = fs::File::create(path)?;
    f.write_all(&data.header.channels.to_le_bytes())?;
    f.write_all(&data.header.fft_size.to_le_bytes())?;
    f.write_all(&data.header.impulse_len.to_le_bytes())?;
    f.write_all(&data.header.sample_rate.to_le_bytes())?;
    for channel in &data.channels {
        for &v in channel {
            f.write_all(&v.to_le_bytes())?;
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tempfile(name: &str) -> std::path::PathBuf {
        let dir = std::env::temp_dir().join(format!(
            "pvc-io-ir-test-{}-{}",
            std::process::id(),
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));
        std::fs::create_dir_all(&dir).unwrap();
        dir.join(name)
    }

    #[test]
    fn round_trip() {
        let data = IrData {
            header: IrHeader {
                channels: 2,
                fft_size: 8,
                impulse_len: 5,
                sample_rate: 44100,
            },
            channels: vec![
                (0..8).map(|i| i as f32 * 0.5).collect(),
                (0..8).map(|i| -(i as f32)).collect(),
            ],
        };
        let path = tempfile("test.ir");
        write_ir(&path, &data).unwrap();
        let read_back = read_ir(&path).unwrap();
        assert_eq!(read_back, data);
    }

    #[test]
    fn rejects_wrong_size() {
        let path = tempfile("bad.ir");
        // channels=1, fft_size=4, impulse_len/sample_rate irrelevant, then
        // only 8 bytes of channel data instead of the required 16.
        let mut bytes = Vec::new();
        bytes.extend_from_slice(&1u32.to_le_bytes());
        bytes.extend_from_slice(&4u32.to_le_bytes());
        bytes.extend_from_slice(&0u32.to_le_bytes());
        bytes.extend_from_slice(&44100u32.to_le_bytes());
        bytes.extend_from_slice(&[0u8; 8]);
        std::fs::write(&path, bytes).unwrap();
        let err = read_ir(&path).unwrap_err();
        assert!(matches!(err, IrError::SizeMismatch(_, 8, 16)));
    }

    #[test]
    fn rejects_too_short() {
        let path = tempfile("tiny.ir");
        std::fs::write(&path, vec![0u8; 4]).unwrap();
        let err = read_ir(&path).unwrap_err();
        assert!(matches!(err, IrError::TooShort(_, 4)));
    }
}
