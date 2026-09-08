//! Golden harness integration test for `pvc filtdeviator`: builds a
//! response file with `pvc response chordresponsemaker` (matching each
//! case's own `legacy_steps`), runs `pvc filtdeviator`, and compares
//! against the recorded C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/filtdeviator/` to already be
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

fn run_case(case: &str, filtdeviator_args: &[&str]) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/filtdeviator").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_filtdeviator::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_filtdeviator::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let table = root.join("tests/golden/fixtures/chord_table.txt");
    let response_path = std::env::temp_dir().join(format!("pvc_golden_filtdeviator_{case}.fr"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_filtdeviator_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("fn")
        .arg("response")
        .arg("chordresponsemaker")
        .args(["--fft", "1024"])
        .args(["--sample-rate", "44100"])
        .arg("--partials")
        .arg(&table)
        .arg(&response_path)
        .status()
        .expect("failed to run pvc fn response chordresponsemaker");
    assert!(
        status.success(),
        "`pvc fn response chordresponsemaker` failed for case {case}"
    );

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("filtdeviator")
        .arg("--response")
        .arg(&response_path)
        .args(filtdeviator_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc filtdeviator");
    assert!(
        status.success(),
        "`pvc filtdeviator {filtdeviator_args:?}` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("filtdeviator/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for filtdeviator/{case}:\n{}{}",
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
            "0.08",
            "--decay-time-base",
            "0.02",
            "--decay-time-peak",
            "0.05",
            "--freq-dev-base",
            "-1",
            "--freq-dev-peak",
            "1",
        ],
    );
}
