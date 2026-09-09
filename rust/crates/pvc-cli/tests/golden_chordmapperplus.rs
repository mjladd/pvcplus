//! Golden harness integration test for `pvc chordmapperplus`: builds an
//! analysis file with `pvc analyze` (matching each case's own
//! `legacy_steps`, which do the same with the real `pvanalysis` oracle),
//! runs `pvc chordmapperplus` with the tone data file and flags
//! equivalent to that case, and compares against the recorded C-oracle
//! output via `compare.py`.
//!
//! Requires `tests/golden/expected/chordmapperplus/` to already be
//! populated via `bash tests/golden/run_legacy.sh` (not git-tracked) and
//! a `python3` on `PATH`. Skips itself with a clear message rather than
//! failing confusingly when either prerequisite is missing.

use std::path::{Path, PathBuf};
use std::process::Command;

fn repo_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../../..")
        .canonicalize()
        .expect("repo root should exist")
}

fn python3_available() -> bool {
    Command::new("python3")
        .arg("--version")
        .output()
        .is_ok_and(|o| o.status.success())
}

#[test]
fn basic_two_tone_chord() {
    let root = repo_root();
    let case = "basic_two_tone_chord";
    let expected_dir = root
        .join("tests/golden/expected/chordmapperplus")
        .join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_chordmapperplus::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_chordmapperplus::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let tones = root.join("tests/golden/fixtures/chordmapperplus_tones.txt");
    let analysis_path = std::env::temp_dir().join(format!("pvc_golden_chordmapperplus_{case}.pva"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_chordmapperplus_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("analyze")
        .args(["--fft", "1024"])
        .arg(&input)
        .arg(&analysis_path)
        .status()
        .expect("failed to run pvc analyze");
    assert!(status.success(), "`pvc analyze` failed for case {case}");

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("chordmapperplus")
        .arg("--analysis")
        .arg(&analysis_path)
        .arg("--tones")
        .arg(&tones)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc chordmapperplus");
    assert!(
        status.success(),
        "`pvc chordmapperplus` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("chordmapperplus/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for chordmapperplus/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
