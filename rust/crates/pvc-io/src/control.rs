//! Control-function file I/O: the "func-able parameter" files legacy tools
//! read via `crackstring`/`fval` (a plain number OR `@path` to a table of
//! values, time-normalized to the output duration by pvc-core's `ControlFn`
//! in Task 2.6 - this module only reads the raw values off disk).
//!
//! New/explicit rule (plan §2.1): `.txt` extension means ASCII (one value
//! per line, whitespace also accepted as a separator), anything else means
//! native/LE f32 binary - no sniffing needed when the caller names the file
//! properly. For files where that's not obvious (extensionless, or an
//! extension pvc doesn't recognize), falls back to content-sniffing the
//! legacy way (crackstring.c: is the file's content printable ASCII or
//! not?) and prints a warning to stderr, since silent format-guessing is
//! exactly what the new CLI's docs promise not to require.

use std::fs;
use std::path::Path;

use thiserror::Error;

#[derive(Debug, Error)]
pub enum ControlFileError {
    #[error(transparent)]
    Io(#[from] std::io::Error),
    #[error("{0}: could not parse ASCII value {1:?} on line {2}")]
    AsciiParse(String, String, usize),
    #[error("{0}: binary file size {1} is not a multiple of 4 bytes")]
    BadBinaryLength(String, usize),
    #[error("{0}: empty control file")]
    Empty(String),
}

/// How the file's format was determined - useful for tests/diagnostics;
/// `Sniffed` is also what triggers the stderr warning.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ControlFileFormat {
    Ascii,
    Binary,
}

#[derive(Debug)]
pub struct ControlFileData {
    pub values: Vec<f32>,
    pub format: ControlFileFormat,
}

/// Reads a control-function file: `.txt` => ASCII, otherwise binary f32,
/// falling back to content-sniffing (with a stderr warning) if the
/// extension doesn't disambiguate.
pub fn read_control_file(path: &Path) -> Result<ControlFileData, ControlFileError> {
    let display = path.display().to_string();
    let bytes = fs::read(path)?;
    if bytes.is_empty() {
        return Err(ControlFileError::Empty(display));
    }

    let is_txt = path
        .extension()
        .and_then(|e| e.to_str())
        .is_some_and(|e| e.eq_ignore_ascii_case("txt"));

    if is_txt {
        return Ok(ControlFileData {
            values: parse_ascii(&bytes, &display)?,
            format: ControlFileFormat::Ascii,
        });
    }

    if looks_like_ascii(&bytes) {
        eprintln!(
            "warning: {display} has no .txt extension but looks like ASCII text; reading it as \
             an ASCII control file. Rename it to .txt to make this explicit."
        );
        return Ok(ControlFileData {
            values: parse_ascii(&bytes, &display)?,
            format: ControlFileFormat::Ascii,
        });
    }

    Ok(ControlFileData {
        values: parse_binary(&bytes, &display)?,
        format: ControlFileFormat::Binary,
    })
}

/// Mirrors `crackstring.c`'s heuristic: sniff up to the first 1000 bytes: if
/// they're all printable ASCII/whitespace, it's a text file.
fn looks_like_ascii(bytes: &[u8]) -> bool {
    let sample = &bytes[..bytes.len().min(1000)];
    !sample.is_empty()
        && sample
            .iter()
            .all(|&b| b.is_ascii_graphic() || b.is_ascii_whitespace())
}

fn parse_ascii(bytes: &[u8], display: &str) -> Result<Vec<f32>, ControlFileError> {
    let text = String::from_utf8_lossy(bytes);
    let mut values = Vec::new();
    for (line_no, line) in text.lines().enumerate() {
        for tok in line.split_whitespace() {
            let v: f32 = tok.parse().map_err(|_| {
                ControlFileError::AsciiParse(display.to_string(), tok.to_string(), line_no + 1)
            })?;
            values.push(v);
        }
    }
    if values.is_empty() {
        return Err(ControlFileError::Empty(display.to_string()));
    }
    Ok(values)
}

fn parse_binary(bytes: &[u8], display: &str) -> Result<Vec<f32>, ControlFileError> {
    if !bytes.len().is_multiple_of(4) {
        return Err(ControlFileError::BadBinaryLength(
            display.to_string(),
            bytes.len(),
        ));
    }
    Ok(bytes
        .chunks_exact(4)
        .map(|c| f32::from_le_bytes([c[0], c[1], c[2], c[3]]))
        .collect())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tempfile(name: &str, contents: &[u8]) -> std::path::PathBuf {
        let dir = std::env::temp_dir().join(format!(
            "pvc-io-control-test-{}-{}",
            std::process::id(),
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));
        std::fs::create_dir_all(&dir).unwrap();
        let path = dir.join(name);
        std::fs::write(&path, contents).unwrap();
        path
    }

    #[test]
    fn reads_ascii_txt_one_value_per_line() {
        let path = tempfile("ramp.txt", b"0\n3\n6\n9\n12\n");
        let data = read_control_file(&path).unwrap();
        assert_eq!(data.format, ControlFileFormat::Ascii);
        assert_eq!(data.values, vec![0.0, 3.0, 6.0, 9.0, 12.0]);
    }

    #[test]
    fn reads_ascii_with_whitespace_separated_values() {
        let path = tempfile("ramp.txt", b"0 3 6\n9   12");
        let data = read_control_file(&path).unwrap();
        assert_eq!(data.values, vec![0.0, 3.0, 6.0, 9.0, 12.0]);
    }

    #[test]
    fn reads_binary_f32_le() {
        let mut bytes = Vec::new();
        for v in [1.0f32, -2.5, 3.25] {
            bytes.extend_from_slice(&v.to_le_bytes());
        }
        let path = tempfile("curve.bin", &bytes);
        let data = read_control_file(&path).unwrap();
        assert_eq!(data.format, ControlFileFormat::Binary);
        assert_eq!(data.values, vec![1.0, -2.5, 3.25]);
    }

    #[test]
    fn sniffs_ascii_without_txt_extension() {
        // no .txt extension, but content is plainly ASCII text
        let path = tempfile("ramp.curve", b"0\n3\n6\n");
        let data = read_control_file(&path).unwrap();
        assert_eq!(data.format, ControlFileFormat::Ascii);
        assert_eq!(data.values, vec![0.0, 3.0, 6.0]);
    }

    #[test]
    fn bad_binary_length_is_rejected() {
        let path = tempfile("bad.bin", &[0u8, 1, 2]); // 3 bytes, not a multiple of 4
        let err = read_control_file(&path).unwrap_err();
        assert!(matches!(err, ControlFileError::BadBinaryLength(_, 3)));
    }

    #[test]
    fn empty_file_is_rejected() {
        let path = tempfile("empty.txt", b"");
        let err = read_control_file(&path).unwrap_err();
        assert!(matches!(err, ControlFileError::Empty(_)));
    }
}
