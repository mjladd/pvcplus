//! Golden harness integration test for `pvc specflattracker`: runs the
//! default invocation against `tests/golden/cases/specflattracker/
//! basic_ascii.toml`, then compares against the recorded C-oracle output
//! via `compare.py`. Mirrors `golden_centroid.rs`'s/`golden_peakformant.
//! rs`'s own shape.

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
    let expected_dir = root
        .join("tests/golden/expected/specflattracker")
        .join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_specflattracker::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_specflattracker::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/noise_then_tone_44k.wav");
    let out_path = std::env::temp_dir().join(format!("pvc_golden_specflattracker_{case}.txt"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("specflattracker")
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc specflattracker");
    assert!(
        status.success(),
        "`pvc specflattracker` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("specflattracker/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for specflattracker/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
