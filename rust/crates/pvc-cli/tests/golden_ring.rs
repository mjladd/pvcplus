//! Golden harness integration test for `pvc ring`: runs `pvc ring` with
//! flags equivalent to `tests/golden/cases/ring/basic_reverb.toml`'s
//! `legacy_steps`, then compares against the recorded C-oracle output via
//! `compare.py`.
//!
//! Requires `tests/golden/expected/ring/` to already be populated via
//! `bash tests/golden/run_legacy.sh` (not git-tracked) and a `python3` on
//! `PATH`.

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
fn basic_reverb() {
    let root = repo_root();
    let case = "basic_reverb";
    let expected_dir = root.join("tests/golden/expected/ring").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_ring::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_ring::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let out_path = std::env::temp_dir().join(format!("pvc_golden_ring_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("ring")
        .args([
            "--input-eq-low-gain",
            "0",
            "--feedback-decay",
            "0.3",
            "--feedback-gain",
            "-6",
            "--feedback-threshold",
            "-40",
        ])
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc ring");
    assert!(status.success(), "`pvc ring` failed for case {case}");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("ring/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for ring/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
