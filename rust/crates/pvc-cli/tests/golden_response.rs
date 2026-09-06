//! Golden harness integration test for `pvc fn response filtresponsemaker/
//! chordresponsemaker` (Task 3.5): runs each subcommand with flags
//! equivalent to its golden case's `legacy_steps`, then compares against
//! the recorded C-oracle `.fr` output using `compare.py`'s byte-exact
//! comparator - meaningful here (unlike `.pva`) because the `.fr` format
//! is just raw floats with no header at all, so `pvc`'s own writer
//! produces exactly the same bytes as the C for the same input.
//!
//! Requires `tests/golden/expected/{filtresponsemaker,chordresponsemaker}/`
//! to already be populated via `bash tests/golden/run_legacy.sh` (not
//! git-tracked) and a `python3` on `PATH`.

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

fn run_case(tool: &str, case: &str, pvc_args: &[&str]) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected").join(tool).join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_response::{tool}/{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_response::{tool}/{case}: python3 not found on PATH");
        return;
    }

    let out_path = std::env::temp_dir().join(format!("pvc_golden_response_{tool}_{case}.fr"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("fn")
        .arg("response")
        .args(pvc_args)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc");
    assert!(
        status.success(),
        "`pvc fn response {pvc_args:?}` failed for {tool}/{case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("{tool}/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for {tool}/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn filtresponsemaker_frequency_gradient() {
    let root = repo_root();
    let breakpoints = root.join("tests/golden/fixtures/filter_breakpoints.txt");
    let target = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    run_case(
        "filtresponsemaker",
        "frequency_gradient",
        &[
            "filtresponsemaker",
            "--fft",
            "1024",
            "--breakpoints",
            breakpoints.to_str().unwrap(),
            "--target-sound-file",
            target.to_str().unwrap(),
        ],
    );
}

#[test]
fn chordresponsemaker_harmonic_tone() {
    let root = repo_root();
    let partials = root.join("tests/golden/fixtures/chord_table.txt");
    run_case(
        "chordresponsemaker",
        "harmonic_tone",
        &[
            "chordresponsemaker",
            "--fft",
            "1024",
            "--sample-rate",
            "44100",
            "--partials",
            partials.to_str().unwrap(),
        ],
    );
}
