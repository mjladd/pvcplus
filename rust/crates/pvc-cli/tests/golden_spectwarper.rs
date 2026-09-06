//! Golden harness integration test for `pvc spectwarp` (Task 3.7): runs
//! `pvc spectwarp` with flags equivalent to each
//! `tests/golden/cases/spectwarper/*.toml` case's `legacy_steps`, then
//! compares against the recorded C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/spectwarper/` to already be populated
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

fn run_case(case: &str, spectwarp_args: &[&str], input_fixture: &str) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/spectwarper").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_spectwarper::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_spectwarper::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures").join(input_fixture);
    let out_path = std::env::temp_dir().join(format!("pvc_golden_spectwarper_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("spectwarp")
        .args(spectwarp_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc spectwarp");
    assert!(
        status.success(),
        "`pvc spectwarp {spectwarp_args:?}` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("spectwarper/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for spectwarper/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn basic_compand() {
    run_case(
        "basic_compand",
        &["--compress-threshold", "-40", "--compress-amount", "20"],
        "sine440_2s_44k.wav",
    );
}

#[test]
fn warp_curve() {
    run_case("warp_curve", &["--warp-curve", "2"], "sine440_2s_44k.wav");
}
