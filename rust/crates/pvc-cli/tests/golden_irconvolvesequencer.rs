//! Golden harness integration test for `pvc irconvolvesequencer` (Phase
//! 5): writes an `impulseFileNames`-style list file, runs `pvc
//! irconvolvesequencer` with flags equivalent to each `tests/golden/
//! cases/irconvolvesequencer/*.toml` case's `legacy_steps`, and compares
//! against the recorded C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/irconvolvesequencer/` to already be
//! populated via `bash tests/golden/run_legacy.sh` (not git-tracked) and a
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

/// Writes `<dir>/impulseFileNames` (count, then one path per line) then
/// runs `pvc irconvolvesequencer --impulse-list-dir <dir> <irconvolvesequencer_args>
/// <input> <out_path>`, then `compare.py irconvolvesequencer/<case> <out_path>`.
fn run_case(case: &str, extra_args: &[&str], impulse_fixtures: &[&str], input_fixture: &str) {
    let root = repo_root();
    let expected_dir = root
        .join("tests/golden/expected/irconvolvesequencer")
        .join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_irconvolvesequencer::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_irconvolvesequencer::{case}: python3 not found on PATH");
        return;
    }

    let fixtures_dir = root.join("tests/golden/fixtures");
    let input = fixtures_dir.join(input_fixture);

    let workdir =
        std::env::temp_dir().join(format!("pvc_golden_irconvolvesequencer_{case}_workdir"));
    let _ = std::fs::remove_dir_all(&workdir);
    std::fs::create_dir_all(&workdir).expect("failed to create workdir");

    let mut list_contents = format!("{}\n", impulse_fixtures.len());
    for name in impulse_fixtures {
        list_contents.push_str(&fixtures_dir.join(name).display().to_string());
        list_contents.push('\n');
    }
    std::fs::write(workdir.join("impulseFileNames"), list_contents)
        .expect("failed to write impulseFileNames");

    let out_path = std::env::temp_dir().join(format!("pvc_golden_irconvolvesequencer_{case}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("irconvolvesequencer")
        .arg("--impulse-list-dir")
        .arg(&workdir)
        .args(extra_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc irconvolvesequencer");
    assert!(
        status.success(),
        "`pvc irconvolvesequencer {extra_args:?}` failed for case {case}"
    );

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("irconvolvesequencer/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for irconvolvesequencer/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn basic_sequence() {
    run_case(
        "basic_sequence",
        &["--normalization", "together"],
        &["sine440_2s_44k.wav", "noise_then_tone_44k.wav"],
        "sweep_3s_44k.wav",
    );
}
