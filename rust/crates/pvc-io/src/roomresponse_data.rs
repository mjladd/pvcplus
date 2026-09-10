//! Room-response scaler-table file I/O: the six small per-wall/per-
//! reflection-order data files `roomresponsemaker.c` reads via its own
//! `get*` functions (channel assignments, dB gainscale levels, and dB
//! presence levels - one reader each for walls and for reflection orders).
//! See `pvc_core::tools::roomresponsemaker`'s own module doc comment
//! (Phase 10) for the loop-assign/cast/validation logic this module's
//! output feeds - this module only does the actual disk read.
//!
//! All six data files share one on-disk shape: `{...}`-bracketed comments,
//! then either "solo" (`!`-marked) or "mute" (`m`-marked) 2-field records,
//! followed by a plain whitespace-separated list of numbers, exactly like
//! the real C's own `fscanf(fp, " %f ", &value)` read loop. This module
//! calls `pvc_core::tools::roomresponsemaker::cut_data_lines` directly for
//! the comment-stripping step, since that function is already ported and
//! unit-tested there, rather than re-deriving the same logic here.

use std::fs;
use std::path::Path;

use thiserror::Error;

#[derive(Debug, Error)]
pub enum RoomResponseDataFileError {
    #[error(transparent)]
    Io(#[from] std::io::Error),
    #[error("{0}: {1}")]
    CommentStrip(String, String),
    #[error("{0}: could not parse {1:?} as a number")]
    Parse(String, String),
    #[error("{0}: file has no numeric values")]
    Empty(String),
}

/// Reads one of `roomresponsemaker.c`'s six channel-assignment/gainscale/
/// presence-level data files: strips `{...}` comments and solo/mute-marked
/// records via `cut_data_lines` (every one of the six real C readers
/// passes `field_count = 2` to it - confirmed by reading each call site),
/// then parses every whitespace-separated token in what's left as an
/// `f32`. Returns the raw values the file holds, before any of
/// `pvc_core::tools::roomresponsemaker`'s own loop-assign/cast/validation
/// logic runs on them.
///
/// Returns `Empty` for a file with no numeric values at all - the C's own
/// loop-assign arithmetic (`i % k`) is undefined behaviour for `k == 0`,
/// so every one of this module's callers needs a real error here instead,
/// matching `pvc_core::tools::roomresponsemaker::loop_assign_to_count`'s
/// own documented choice not to reproduce that undefined behaviour.
pub fn read_room_response_data_file(path: &Path) -> Result<Vec<f32>, RoomResponseDataFileError> {
    let display = path.display().to_string();
    let text = fs::read_to_string(path)?;
    let cut = pvc_core::tools::roomresponsemaker::cut_data_lines(&text, 2)
        .map_err(|e| RoomResponseDataFileError::CommentStrip(display.clone(), e))?;
    let values: Vec<f32> = cut
        .split_whitespace()
        .map(|tok| {
            tok.parse()
                .map_err(|_| RoomResponseDataFileError::Parse(display.clone(), tok.to_string()))
        })
        .collect::<Result<_, _>>()?;
    if values.is_empty() {
        return Err(RoomResponseDataFileError::Empty(display));
    }
    Ok(values)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tempfile(name: &str, contents: &str) -> std::path::PathBuf {
        let dir = std::env::temp_dir().join(format!(
            "pvc-io-roomresponse-data-test-{}-{}",
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
    fn reads_plain_whitespace_separated_values() {
        let path = tempfile("levels.txt", "1 2 3\n4 5\n");
        let values = read_room_response_data_file(&path).unwrap();
        assert_eq!(values, vec![1.0, 2.0, 3.0, 4.0, 5.0]);
    }

    #[test]
    fn strips_brace_comments() {
        let path = tempfile("levels.txt", "1 { this is a comment } 2 3");
        let values = read_room_response_data_file(&path).unwrap();
        assert_eq!(values, vec![1.0, 2.0, 3.0]);
    }

    #[test]
    fn negative_and_fractional_values_parse() {
        let path = tempfile("levels.txt", "-6.5 -3.25 0");
        let values = read_room_response_data_file(&path).unwrap();
        assert_eq!(values, vec![-6.5, -3.25, 0.0]);
    }

    #[test]
    fn empty_file_is_rejected() {
        let path = tempfile("empty.txt", "");
        let err = read_room_response_data_file(&path).unwrap_err();
        assert!(matches!(err, RoomResponseDataFileError::Empty(_)));
    }

    #[test]
    fn missing_file_is_an_io_error() {
        let path = std::env::temp_dir().join("pvc-io-roomresponse-data-test-does-not-exist.txt");
        let err = read_room_response_data_file(&path).unwrap_err();
        assert!(matches!(err, RoomResponseDataFileError::Io(_)));
    }

    #[test]
    fn mismatched_brace_is_a_comment_strip_error() {
        let path = tempfile("bad.txt", "1 { 2");
        let err = read_room_response_data_file(&path).unwrap_err();
        assert!(matches!(err, RoomResponseDataFileError::CommentStrip(_, _)));
    }
}
