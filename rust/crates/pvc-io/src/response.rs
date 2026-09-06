//! `.fr` frequency-response file I/O: the format `freqresponse.c`,
//! `filtresponsemaker.c`, and `chordresponsemaker.c` all write and
//! `filter.c` reads. Unlike `.pva`, there's no legacy-vs-new distinction
//! to make here - the legacy format is already about as simple as a
//! format can be, so `pvc`'s own writer just produces the same thing.
//!
//! The file is exactly `n + 2` little-endian f32s - `n/2 + 1`
//! (magnitude, frequency-in-Hz) pairs, same interleaving as `.pva`
//! frames and `pvc_core::pvoc::Frame` - with **no header at all**, not
//! even `N`. A reader must already know the FFT size from context,
//! matching how every legacy tool that consumes one takes an explicit
//! `-N` alongside `-F<response file>`.

use std::fs;
use std::io::Write;
use std::path::Path;

use thiserror::Error;

#[derive(Debug, Error)]
pub enum ResponseError {
    #[error(transparent)]
    Io(#[from] std::io::Error),
    #[error("{0}: {1} bytes on disk, expected exactly {2} for N={3} ({2} = (N+2)*4)")]
    SizeMismatch(String, usize, usize, usize),
}

/// Reads a `.fr` file's `n + 2` floats. `n` must be supplied externally
/// - see this module's doc comment.
pub fn read_fr(path: &Path, n: usize) -> Result<Vec<f32>, ResponseError> {
    let bytes = fs::read(path)?;
    let expected = (n + 2) * 4;
    if bytes.len() != expected {
        return Err(ResponseError::SizeMismatch(
            path.display().to_string(),
            bytes.len(),
            expected,
            n,
        ));
    }
    Ok(bytes
        .chunks_exact(4)
        .map(|c| f32::from_le_bytes([c[0], c[1], c[2], c[3]]))
        .collect())
}

/// Reads a `.fr` file without knowing `n` ahead of time, inferring it
/// from the file's own byte size instead (`n = bytes/4 - 2`) - what
/// `filter.c` effectively does too (it takes an explicit `-N`, but the
/// file's size is all that's actually needed; see `pvc-core::
/// tools::filter`'s doc comment on why this port derives its FFT size
/// from the response file rather than requiring a separately-specified,
/// easy-to-mismatch one). Returns `(amplitudes_only, n)` - just the even
/// (magnitude) slots, since nothing in the `.fr`-consuming tools this
/// crate supports ever reads the odd (frequency) ones back.
pub fn read_fr_amplitudes(path: &Path) -> Result<(Vec<f32>, usize), ResponseError> {
    let bytes = fs::read(path)?;
    if !bytes.len().is_multiple_of(4) || bytes.len() < 8 {
        return Err(ResponseError::SizeMismatch(
            path.display().to_string(),
            bytes.len(),
            0,
            0,
        ));
    }
    let n = bytes.len() / 4 - 2;
    let amps = bytes
        .chunks_exact(8)
        .map(|c| f32::from_le_bytes([c[0], c[1], c[2], c[3]]))
        .collect();
    Ok((amps, n))
}

/// Writes a `.fr` file: `data` (expected to be `n + 2` floats, but
/// written as-is - the caller owns validating its length) as raw
/// little-endian f32s.
pub fn write_fr(path: &Path, data: &[f32]) -> Result<(), ResponseError> {
    let mut f = fs::File::create(path)?;
    for &v in data {
        f.write_all(&v.to_le_bytes())?;
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tempfile(name: &str) -> std::path::PathBuf {
        let dir = std::env::temp_dir().join(format!(
            "pvc-io-fr-test-{}-{}",
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
        let data: Vec<f32> = (0..10).map(|i| i as f32 * 0.5).collect();
        let path = tempfile("test.fr");
        write_fr(&path, &data).unwrap();
        let read_back = read_fr(&path, 8).unwrap();
        assert_eq!(read_back, data);
    }

    #[test]
    fn rejects_wrong_size() {
        let path = tempfile("bad.fr");
        std::fs::write(&path, vec![0u8; 20]).unwrap();
        let err = read_fr(&path, 8).unwrap_err();
        assert!(matches!(err, ResponseError::SizeMismatch(_, 20, 40, 8)));
    }
}
