//! Golden harness integration test for `pvc irconvolver` (Phase 5): builds
//! a `.ir` file with `pvc impulseresponse`, runs `pvc irconvolver` with
//! flags equivalent to each `tests/golden/cases/irconvolver/*.toml`
//! case's `legacy_steps`, and compares against the recorded C-oracle
//! output via `compare.py`.
//!
//! Requires `tests/golden/expected/irconvolver/` to already be populated
//! via `bash tests/golden/run_legacy.sh` (not git-tracked) and a
//! `python3` on `PATH`. Each case skips itself with a clear message
//! rather than failing confusingly when either prerequisite is missing.

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

/// Runs `pvc impulseresponse` on `impulse_fixture` to build a `.ir` file,
/// then `pvc irconvolver <irconvolver_args> --ir <ir> <input> <out_path>`,
/// then `compare.py irconvolver/<case> <out_path>`.
fn run_case(case: &str, irconvolver_args: &[&str], impulse_fixture: &str, input_fixture: &str) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/irconvolver").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_irconvolver::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_irconvolver::{case}: python3 not found on PATH");
        return;
    }

    let impulse_input = root.join("tests/golden/fixtures").join(impulse_fixture);
    let input = root.join("tests/golden/fixtures").join(input_fixture);
    let ir_path = std::env::temp_dir().join(format!("pvc_golden_irconvolver_{case}.ir"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_irconvolver_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("impulseresponse")
        .args(["--begin", "0", "--end", "0.05"])
        .arg(&impulse_input)
        .arg(&ir_path)
        .status()
        .expect("failed to run pvc impulseresponse");
    assert!(
        status.success(),
        "`pvc impulseresponse` failed for case {case}"
    );

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("irconvolver")
        .arg("--ir")
        .arg(&ir_path)
        .args(irconvolver_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc irconvolver");
    assert!(
        status.success(),
        "`pvc irconvolver {irconvolver_args:?}` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("irconvolver/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for irconvolver/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn basic_convolution() {
    run_case(
        "basic_convolution",
        &[],
        "sine440_2s_44k.wav",
        "sweep_3s_44k.wav",
    );
}
