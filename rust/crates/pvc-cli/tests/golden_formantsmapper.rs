//! Golden harness integration test for `pvc formantsmapper`: runs the
//! CLI directly against a fixture and the two synthetic binary
//! formant-list fixtures `gen.sh` produces, and compares against the
//! recorded C-oracle output via `compare.py`.
//!
//! Uses `sine440_2s_faded_44k.wav`, not the plain `sine440_2s_44k.wav` -
//! see `tests/golden/cases/formantsmapper/basic_mapping.toml`'s own
//! notes on why (the same abrupt-truncation resynthesis sensitivity
//! already documented for `pvc inharmonator`).
//!
//! Requires `tests/golden/expected/formantsmapper/` to already be
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
    let expected_dir = root.join("tests/golden/expected/formantsmapper").join(name);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_formantsmapper::{name}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_formantsmapper::{name}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_faded_44k.wav");
    let source_formants = root.join("tests/golden/fixtures/formantsmapper_source.formants");
    let target_formants = root.join("tests/golden/fixtures/formantsmapper_target.formants");
    if !source_formants.is_file() || !target_formants.is_file() {
        eprintln!(
            "SKIP golden_formantsmapper::{name}: formant fixtures not found - run `bash tests/golden/run_legacy.sh` first"
        );
        return;
    }
    let out_path = std::env::temp_dir().join(format!("pvc_golden_formantsmapper_{name}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("formantsmapper")
        .arg("--source-formants")
        .arg(&source_formants)
        .arg("--target-formants")
        .arg(&target_formants)
        .args(extra_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc formantsmapper");
    assert!(status.success(), "`pvc formantsmapper` failed");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("formantsmapper/{name}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for formantsmapper/{name}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn basic_mapping() {
    run_case("basic_mapping", &[]);
}

#[test]
fn residue_bins() {
    run_case("residue_bins", &["--residue-bins"]);
}

#[test]
fn dual_bank_with_residue() {
    run_case(
        "dual_bank_with_residue",
        &["--dual-bank", "--bank-b-gain", "-6", "--residue-bins"],
    );
}
