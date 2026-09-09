//! Binary "formants file" I/O for `formantsmapper`/`spectrummapper`'s
//! `-E`/`-g` (source/target formant list) flags. Unlike every other
//! binary format this crate reads, this one is **not written by any C
//! tool in this codebase** - it's produced externally by a SuperCollider
//! script (`legacy/pvc_src/FixedFormantAnalysis.template`), confirmed by
//! grepping every `legacy/pvc_src/*.c` for a matching `fwrite` and
//! finding none.
//!
//! Layout (all little-endian, matching every other binary format this
//! project reads - x86 native order): `i32 num_formants`, `i32 n2`
//! (the analysis FFT's own `N/2`, checked by every reader against its
//! own `-N` at load time), then `num_formants` records of `f32
//! center_freq, f32 amp, f32 bw, f32 q, i32 index, i32
//! low_stop_band_index, i32 high_stop_band_index` (28 bytes each).

use std::fs;
use std::path::Path;

use thiserror::Error;

#[derive(Debug, Error)]
pub enum FormantsError {
    #[error(transparent)]
    Io(#[from] std::io::Error),
    #[error("{0}: {1} bytes on disk, expected exactly {2} for {3} formants")]
    SizeMismatch(String, usize, usize, usize),
}

/// One record as read directly off disk, before any of a caller's own
/// filtering/extension logic.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct FormantRecord {
    pub center_freq: f32,
    pub amp: f32,
    pub bw: f32,
    pub q: f32,
    pub index: i32,
    pub low_stop_band_index: i32,
    pub high_stop_band_index: i32,
}

/// Reads a formants file. Returns `(records, n2)` - callers must check
/// `n2` against their own analysis FFT size themselves (matching every
/// legacy tool's own `if (n2 != N2) exit(EXIT_FAILURE)` check).
pub fn read_formants(path: &Path) -> Result<(Vec<FormantRecord>, usize), FormantsError> {
    let bytes = fs::read(path)?;
    if bytes.len() < 8 {
        return Err(FormantsError::SizeMismatch(
            path.display().to_string(),
            bytes.len(),
            8,
            0,
        ));
    }
    let num_formants = i32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]]) as usize;
    let n2 = i32::from_le_bytes([bytes[4], bytes[5], bytes[6], bytes[7]]) as usize;

    let expected = 8 + num_formants * 28;
    if bytes.len() != expected {
        return Err(FormantsError::SizeMismatch(
            path.display().to_string(),
            bytes.len(),
            expected,
            num_formants,
        ));
    }

    let mut records = Vec::with_capacity(num_formants);
    for chunk in bytes[8..].chunks_exact(28) {
        let f32_at =
            |o: usize| f32::from_le_bytes([chunk[o], chunk[o + 1], chunk[o + 2], chunk[o + 3]]);
        let i32_at =
            |o: usize| i32::from_le_bytes([chunk[o], chunk[o + 1], chunk[o + 2], chunk[o + 3]]);
        records.push(FormantRecord {
            center_freq: f32_at(0),
            amp: f32_at(4),
            bw: f32_at(8),
            q: f32_at(12),
            index: i32_at(16),
            low_stop_band_index: i32_at(20),
            high_stop_band_index: i32_at(24),
        });
    }
    Ok((records, n2))
}
