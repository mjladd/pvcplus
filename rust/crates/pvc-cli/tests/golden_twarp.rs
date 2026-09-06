//! Golden harness integration test for `pvc twarp` (Task 3.4): builds an
//! analysis file with `pvc analyze` (well, with the real `pvanalysis`
//! oracle, since these cases' `legacy_steps` do that themselves and the
//! recorded expected output already reflects it), runs `pvc twarp` with
//! flags equivalent to each `tests/golden/cases/twarp/*.toml` case, and
//! compares against the recorded C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/twarp/` to already be populated via
//! `bash tests/golden/run_legacy.sh` (not git-tracked) and a `python3` on
//! `PATH`. Each case skips itself with a clear message rather than
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

/// Runs `pvc analyze` on `input_fixture` to build an analysis file (using
/// the same `-N1024` FFT size every case's `legacy_steps` uses), then
/// `pvc twarp <twarp_args> <analysis> <out_path>`, then `compare.py
/// twarp/<case> <out_path>`.
fn run_case(case: &str, twarp_args: &[&str], input_fixture: &str) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/twarp").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_twarp::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_twarp::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures").join(input_fixture);
    let analysis_path = std::env::temp_dir().join(format!("pvc_golden_twarp_{case}.pva"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_twarp_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("analyze")
        .args(["--fft", "1024"])
        .arg(&input)
        .arg(&analysis_path)
        .status()
        .expect("failed to run pvc analyze");
    assert!(status.success(), "`pvc analyze` failed for case {case}");

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("twarp")
        .args(twarp_args)
        .arg(&analysis_path)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc twarp");
    assert!(
        status.success(),
        "`pvc twarp {twarp_args:?}` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("twarp/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for twarp/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn basic_chain() {
    run_case("basic_chain", &["--duration", "2.0"], "sine440_2s_44k.wav");
}

#[test]
fn rate_multiplier() {
    run_case(
        "rate_multiplier",
        &["--duration", "1.0", "--rate", "2.0"],
        "sine440_2s_44k.wav",
    );
}

#[test]
fn pitch_shift_oscbank() {
    run_case(
        "pitch_shift_oscbank",
        &["--duration", "2.0", "--pitch", "7"],
        "sine440_2s_44k.wav",
    );
}
