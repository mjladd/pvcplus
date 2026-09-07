//! Golden harness integration test for `pvc ringfilter`: builds a `.fr`
//! response with `pvc fn response filtresponsemaker` (matching `tests/
//! golden/cases/ringfilter/basic_reverb.toml`'s own `legacy_steps`, which
//! use the real C `filtresponsemaker` to build the same response against
//! the same `filter_breakpoints.txt` fixture - see `golden_filter.rs`'s
//! own precedent for using the Rust-ported response producer here
//! instead of shelling out to a legacy binary that isn't available
//! outside the Docker oracle environment), then runs `pvc ringfilter` and
//! compares against the recorded C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/ringfilter/` to already be populated
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
fn basic_reverb() {
    let root = repo_root();
    let case = "basic_reverb";
    let expected_dir = root.join("tests/golden/expected/ringfilter").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_ringfilter::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_ringfilter::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sweep_3s_44k.wav");
    let breakpoints = root.join("tests/golden/fixtures/filter_breakpoints.txt");
    let response_path = std::env::temp_dir().join(format!("pvc_golden_ringfilter_{case}.fr"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_ringfilter_{case}.wav"));

    run_pvc(&[
        "fn",
        "response",
        "filtresponsemaker",
        "--fft",
        "1024",
        "--breakpoints",
        breakpoints.to_str().unwrap(),
        "--target-sound-file",
        input.to_str().unwrap(),
        response_path.to_str().unwrap(),
    ]);

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("ringfilter")
        .args([
            "--input-eq-low-gain",
            "0",
            "--filter-response",
            response_path.to_str().unwrap(),
            "--feedback-decay",
            "0.3",
            "--feedback-gain",
            "-6",
            "--feedback-threshold",
            "-40",
        ])
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc ringfilter");
    assert!(status.success(), "`pvc ringfilter` failed for case {case}");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("ringfilter/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for ringfilter/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
