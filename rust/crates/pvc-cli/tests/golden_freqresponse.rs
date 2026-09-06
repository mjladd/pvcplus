//! Golden harness integration test for `pvc freqresponse` (Task 3.5):
//! runs `pvc freqresponse` with flags equivalent to the golden case's
//! `legacy_steps`, then compares the raw `.fr` floats directly against
//! the recorded C-oracle output - not `compare.py`'s generic comparator
//! (see the case `.toml`'s notes for why: some deep-noise-floor bins are
//! expected to diverge in floating-point accumulation, the same
//! phenomenon already documented for `pvanalysis`'s near-silent-bin
//! frequencies).
//!
//! Requires `tests/golden/expected/freqresponse/` to already be
//! populated via `bash tests/golden/run_legacy.sh` (not git-tracked).

use std::path::{Path, PathBuf};
use std::process::Command;

fn repo_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../../..")
        .canonicalize()
        .expect("repo root should exist")
}

fn read_fr(path: &Path) -> Vec<f32> {
    let bytes = std::fs::read(path).unwrap_or_else(|e| panic!("reading {}: {e}", path.display()));
    bytes
        .chunks_exact(4)
        .map(|c| f32::from_le_bytes([c[0], c[1], c[2], c[3]]))
        .collect()
}

#[test]
fn average_spectrum() {
    let root = repo_root();
    let expected_path = root.join("tests/golden/expected/freqresponse/average_spectrum/output.fr");
    if !expected_path.is_file() {
        eprintln!(
            "SKIP golden_freqresponse::average_spectrum: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_path.display()
        );
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let out_path = std::env::temp_dir().join("pvc_golden_freqresponse_average_spectrum.fr");

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("freqresponse")
        .args(["--spectrum-type", "average"])
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc freqresponse");
    assert!(status.success(), "pvc freqresponse failed");

    let expected = read_fr(&expected_path);
    let candidate = read_fr(&out_path);
    assert_eq!(expected.len(), candidate.len());

    let mut max_amp_err = 0.0f32;
    let mut max_freq_err = 0.0f32;
    for pair in expected.chunks_exact(2).zip(candidate.chunks_exact(2)) {
        let (e, c) = pair;
        max_amp_err = max_amp_err.max((e[0] - c[0]).abs());
        // Frequency error is only meaningful for bins with real energy -
        // same rationale as golden_pvanalysis.rs.
        if e[0] > 1e-3 {
            max_freq_err = max_freq_err.max((e[1] - c[1]).abs());
        }
    }

    let expected_peak = expected.iter().step_by(2).copied().fold(0.0f32, f32::max);
    let candidate_peak = candidate.iter().step_by(2).copied().fold(0.0f32, f32::max);
    assert!(
        (expected_peak - candidate_peak).abs() < 1e-5,
        "peak amplitude mismatch: expected {expected_peak}, got {candidate_peak}"
    );
    assert!(
        max_amp_err < 0.01,
        "max amplitude error {max_amp_err} too large"
    );
    assert!(
        max_freq_err < 1.0,
        "max frequency error {max_freq_err} Hz too large"
    );
}
