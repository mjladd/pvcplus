//! Golden harness integration test for `pvc inharmonator`: runs the CLI
//! directly against a fixture and a partials data table (no upstream
//! analysis step - `inharmonator` reads raw audio) and compares against
//! the recorded C-oracle output via `compare.py`.
//!
//! `three_partials`/`delay_and_decay`/`source_excluded` all pass
//! `--partial-number-scale 1 --partial-db-scale 1 --partial-delay-scale 1`
//! explicitly, matching the oracle recording's own `-z1 -y1 -o1` -
//! `usage()`'s documented defaults for these three data-modifier scalers,
//! since the real C leaves them uninitialized unless passed (see
//! `pvc-core::tools::inharmonator`'s doc comment).
//!
//! Requires `tests/golden/expected/inharmonator/` to already be
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

fn run_case(name: &str, extra_args: &[&str]) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/inharmonator").join(name);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_inharmonator::{name}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_inharmonator::{name}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_faded_44k.wav");
    let partials = root.join("tests/golden/fixtures/inharmonator_partials.txt");
    let out_path = std::env::temp_dir().join(format!("pvc_golden_inharmonator_{name}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("inharmonator")
        .arg("--partials")
        .arg(&partials)
        .args(["--fundamental", "220"])
        .args(["--partial-number-scale", "1"])
        .args(["--partial-db-scale", "1"])
        .args(["--partial-delay-scale", "1"])
        .args(extra_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc inharmonator");
    assert!(status.success(), "`pvc inharmonator` failed");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("inharmonator/{name}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for inharmonator/{name}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn three_partials() {
    run_case("three_partials", &[]);
}

#[test]
fn delay_and_decay() {
    run_case(
        "delay_and_decay",
        &["--non-target-delay", "0.03", "--non-target-decay", "0.08"],
    );
}

#[test]
fn source_excluded() {
    run_case("source_excluded", &["--source-gain", "-120"]);
}
