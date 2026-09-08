//! Golden harness integration test for `pvc tvfiltdeviator`: builds a
//! time-varying filter response file with `pvc analyze` (of the sweep
//! fixture, matching each case's own `legacy_steps`), runs `pvc
//! tvfiltdeviator`, and compares against the recorded C-oracle output
//! via `compare.py`.
//!
//! Requires `tests/golden/expected/tvfiltdeviator/` to already be
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

fn run_case(case: &str, tvfiltdeviator_args: &[&str]) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/tvfiltdeviator").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_tvfiltdeviator::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_tvfiltdeviator::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let sweep = root.join("tests/golden/fixtures/sweep_3s_44k.wav");
    let analysis_path = std::env::temp_dir().join(format!("pvc_golden_tvfiltdeviator_{case}.pva"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_tvfiltdeviator_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("analyze")
        .args(["--fft", "1024"])
        .arg(&sweep)
        .arg(&analysis_path)
        .status()
        .expect("failed to run pvc analyze");
    assert!(status.success(), "`pvc analyze` failed for case {case}");

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("tvfiltdeviator")
        .arg("--fft")
        .arg("1024")
        .arg("--filter-response")
        .arg(&analysis_path)
        .args(tvfiltdeviator_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc tvfiltdeviator");
    assert!(
        status.success(),
        "`pvc tvfiltdeviator {tvfiltdeviator_args:?}` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("tvfiltdeviator/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for tvfiltdeviator/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn baseline() {
    run_case("baseline", &[]);
}

#[test]
fn time_delay_and_deviation() {
    run_case(
        "time_delay_and_deviation",
        &[
            "--time-delay-base",
            "0.02",
            "--time-delay-peak",
            "0.06",
            "--freq-dev-control",
            "1",
            "--freq-dev-base",
            "-2",
            "--freq-dev-peak",
            "2",
        ],
    );
}
