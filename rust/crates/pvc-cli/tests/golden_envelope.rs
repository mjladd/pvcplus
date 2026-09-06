//! Golden harness integration test for `pvc envelope` (Task 3.10): runs
//! the default invocation against `tests/golden/cases/envelope/
//! basic_ascii.toml`, then compares against the recorded C-oracle output
//! via `compare.py`.

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
fn basic_ascii() {
    let root = repo_root();
    let case = "basic_ascii";
    let expected_dir = root.join("tests/golden/expected/envelope").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_envelope::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_envelope::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let out_path = std::env::temp_dir().join(format!("pvc_golden_envelope_{case}.txt"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("envelope")
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc envelope");
    assert!(status.success(), "`pvc envelope` failed for case {case}");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("envelope/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for envelope/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
