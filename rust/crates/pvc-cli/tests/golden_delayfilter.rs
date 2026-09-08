//! Golden harness integration test for `pvc delayfilter`: builds a source
//! analysis file with `pvc analyze`, a group-delay response file with
//! `pvc fn response groupdelaymaker` (both matching the legacy toolchain
//! calls the `basic_delay` case's own `legacy_steps` used to record the
//! oracle), runs `pvc delayfilter`, and compares against the recorded
//! C-oracle output via `compare.py`.
//!
//! Requires `tests/golden/expected/delayfilter/` to already be populated
//! via `bash tests/golden/run_legacy.sh` (not git-tracked) and a
//! `python3` on `PATH`. Skips itself with a clear message rather than
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

#[test]
fn basic_delay() {
    let root = repo_root();
    let expected_dir = root
        .join("tests/golden/expected/delayfilter")
        .join("basic_delay");
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_delayfilter::basic_delay: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_delayfilter::basic_delay: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let partials = root.join("tests/golden/fixtures/groupdelay_table.txt");
    let analysis_path = std::env::temp_dir().join("pvc_golden_delayfilter_basic_delay.pva");
    let delay_path = std::env::temp_dir().join("pvc_golden_delayfilter_basic_delay.fr");
    let out_path = std::env::temp_dir().join("pvc_golden_delayfilter_basic_delay.wav");

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("analyze")
        .args(["--fft", "1024"])
        .arg(&input)
        .arg(&analysis_path)
        .status()
        .expect("failed to run pvc analyze");
    assert!(status.success(), "`pvc analyze` failed");

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("fn")
        .arg("response")
        .arg("groupdelaymaker")
        .arg("--analysis")
        .arg(&analysis_path)
        .arg("--partials")
        .arg(&partials)
        .args(["--edge-db", "-6"])
        .args(["--default-db", "-96"])
        .args(["--default-delay", "0"])
        .args(["--method", "average"])
        .arg(&delay_path)
        .status()
        .expect("failed to run pvc fn response groupdelaymaker");
    assert!(status.success(), "`pvc fn response groupdelaymaker` failed");

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("delayfilter")
        .arg("--delay-filter")
        .arg(&delay_path)
        .arg("--delay-window")
        .arg("1")
        .arg("--duration")
        .arg("2.0")
        .arg(&analysis_path)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc delayfilter");
    assert!(status.success(), "`pvc delayfilter` failed");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg("delayfilter/basic_delay")
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for delayfilter/basic_delay:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}
