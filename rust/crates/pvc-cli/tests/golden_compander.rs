//! Golden harness integration test for `pvc compand` (Task 3.7): builds a
//! peaks/reference file with `pvc freqresponse`, runs `pvc compand` on
//! it, and compares against the recorded C-oracle output via
//! `compare.py` - reusing the existing `tests/golden/cases/compander/
//! freqresponse_driven.toml` case.
//!
//! Requires `tests/golden/expected/compander/` to already be populated
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

fn run_pvc(args: &[&str]) {
    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .args(args)
        .status()
        .unwrap_or_else(|e| panic!("failed to run pvc {args:?}: {e}"));
    assert!(status.success(), "`pvc {args:?}` failed");
}

#[test]
fn freqresponse_driven() {
    let root = repo_root();
    let case = "freqresponse_driven";
    let expected_dir = root.join("tests/golden/expected/compander").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_compander::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_compander::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let peaks_path = std::env::temp_dir().join(format!("pvc_golden_compander_{case}.fr"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_compander_{case}.wav"));

    // `freqresponse -c1 {input} response.fr` - `-c1` is freqresponse's
    // own peak/average method selector, not a channel flag.
    run_pvc(&[
        "freqresponse",
        "--spectrum-type",
        "peak",
        input.to_str().unwrap(),
        peaks_path.to_str().unwrap(),
    ]);

    // `compander -Fresponse.fr -o-30 -O15 {input} {output}`.
    run_pvc(&[
        "compand",
        "--peaks",
        peaks_path.to_str().unwrap(),
        "--compress-threshold",
        "-30",
        "--compress-amount",
        "15",
        input.to_str().unwrap(),
        out_path.to_str().unwrap(),
    ]);

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("compander/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for compander/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
