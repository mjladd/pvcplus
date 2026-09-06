//! Golden harness integration test for `pvc denoise` (Task 3.6): runs
//! `pvc denoise` with flags equivalent to each
//! `tests/golden/cases/noisefilter/*.toml` case's `legacy_steps`, then
//! compares against the recorded C-oracle output using
//! `tests/golden/compare.py`.
//!
//! Requires `tests/golden/expected/noisefilter/` to already be populated
//! via `bash tests/golden/run_legacy.sh` and a `python3` on `PATH`. Each
//! case skips itself with a clear message rather than failing confusingly
//! when either prerequisite is missing.

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

/// Runs `pvc denoise <denoise_args> <input> <out_path>`, then
/// `compare.py noisefilter/<case> <out_path>`.
fn run_case(case: &str, denoise_args: &[&str], input_fixture: &str) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/noisefilter").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_noisefilter::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_noisefilter::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures").join(input_fixture);
    let out_path = std::env::temp_dir().join(format!("pvc_golden_noisefilter_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("denoise")
        .args(denoise_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc denoise");
    assert!(
        status.success(),
        "`pvc denoise {denoise_args:?}` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("noisefilter/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for noisefilter/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn basic_denoise() {
    run_case(
        "basic_denoise",
        &[
            "--fft",
            "1024",
            "--noise-begin",
            "0",
            "--noise-end",
            "0.5",
            "--noise-method",
            "peak",
        ],
        "noise_then_tone_44k.wav",
    );
}

#[test]
fn gain_adjust() {
    run_case(
        "gain_adjust",
        &[
            "--fft",
            "1024",
            "--noise-begin",
            "0",
            "--noise-end",
            "0.5",
            "--noise-method",
            "peak",
            "--noise-threshold-gain",
            "12",
        ],
        "noise_then_tone_44k.wav",
    );
}
