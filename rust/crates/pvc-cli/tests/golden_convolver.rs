//! Golden harness integration test for `pvc convolver`: builds a `.pva`
//! analysis of the sweep fixture with `pvc analyze` (matching
//! `tests/golden/cases/convolver/basic_convolution.toml`'s own
//! `legacy_steps`, which use the real C `pvanalysis` - see
//! `golden_tvfilter.rs`'s own precedent for using the Rust-ported
//! analyzer here instead of a legacy binary that isn't available outside
//! the Docker oracle environment), then runs `pvc convolver` and
//! compares against the recorded C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/convolver/` to already be populated
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
fn basic_convolution() {
    let root = repo_root();
    let case = "basic_convolution";
    let expected_dir = root.join("tests/golden/expected/convolver").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_convolver::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_convolver::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let sweep = root.join("tests/golden/fixtures/sweep_3s_44k.wav");
    let analysis_path = std::env::temp_dir().join(format!("pvc_golden_convolver_{case}.pva"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_convolver_{case}.wav"));

    run_pvc(&[
        "analyze",
        "--fft",
        "1024",
        sweep.to_str().unwrap(),
        analysis_path.to_str().unwrap(),
    ]);

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("convolver")
        .arg("--filter-response")
        .arg(&analysis_path)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc convolver");
    assert!(status.success(), "`pvc convolver` failed for case {case}");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("convolver/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for convolver/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
