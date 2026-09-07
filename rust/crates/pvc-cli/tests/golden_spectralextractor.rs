//! Golden harness integration test for `pvc spectralextractor`: runs
//! `pvc spectralextractor` with flags equivalent to each
//! `tests/golden/cases/spectralextractor/*.toml` case's `legacy_steps`,
//! then compares against the recorded C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/spectralextractor/` to already be
//! populated via `bash tests/golden/run_legacy.sh` (not git-tracked) and
//! a `python3` on `PATH`.

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

fn run_case(case: &str, extra_args: &[&str], input_fixture: &str) {
    let root = repo_root();
    let expected_dir = root
        .join("tests/golden/expected/spectralextractor")
        .join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_spectralextractor::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_spectralextractor::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures").join(input_fixture);
    let out_path = std::env::temp_dir().join(format!("pvc_golden_spectralextractor_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("spectralextractor")
        .args(extra_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc spectralextractor");
    assert!(
        status.success(),
        "`pvc spectralextractor {extra_args:?}` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("spectralextractor/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for spectralextractor/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn periodic_extraction() {
    run_case(
        "periodic_extraction",
        &[
            "--spectral-type",
            "periodic",
            "--freq-change-threshold",
            "30",
            "--freq-change-response",
            "0.05",
            "--release",
            "0.05",
        ],
        "noise_then_tone_44k.wav",
    );
}

#[test]
fn noise_mode() {
    run_case(
        "noise_mode",
        &[
            "--spectral-type",
            "noise",
            "--freq-change-threshold",
            "30",
            "--freq-change-response",
            "0.05",
            "--release",
            "0.05",
        ],
        "noise_then_tone_44k.wav",
    );
}
