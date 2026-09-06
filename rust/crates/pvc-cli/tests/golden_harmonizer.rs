//! Golden harness integration test for `pvc harmonize` (Task 3.9): runs
//! `pvc harmonize` with flags equivalent to
//! `tests/golden/cases/harmonizer/basic_harmony.toml`'s `legacy_steps`,
//! then compares against the recorded C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/harmonizer/` to already be populated
//! via `bash tests/golden/run_legacy.sh` (not git-tracked) and a
//! `python3` on `PATH`.

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
fn basic_harmony() {
    let root = repo_root();
    let case = "basic_harmony";
    let expected_dir = root.join("tests/golden/expected/harmonizer").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_harmonizer::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_harmonizer::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let table = root.join("tests/golden/fixtures/harmonizer_table.txt");
    let out_path = std::env::temp_dir().join(format!("pvc_golden_harmonizer_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("harmonize")
        .arg("--table")
        .arg(&table)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc harmonize");
    assert!(status.success(), "`pvc harmonize` failed for case {case}");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("harmonizer/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for harmonizer/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
