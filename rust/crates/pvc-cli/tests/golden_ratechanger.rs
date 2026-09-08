//! Golden harness integration test for `pvc ratechanger`: runs the CLI
//! directly against a fixture (no upstream analysis step - `ratechanger`
//! reads raw audio) and compares against the recorded C-oracle output via
//! `compare.py`.
//!
//! `sinc_no_table`/`hold_decimate_rate_change`/`linear_interp_slow_rate`
//! each fix `--duration` explicitly (no duration-search flag) so both
//! sides skip the iterative output-duration convergence loop - see
//! `pvc-core::tools::ratechanger::resolve_output_duration`'s own doc
//! comment and its dedicated Rust unit test for that path instead. None
//! of these cases use the tool's own default (table-lookup) sinc path:
//! see `pvc-core::tools::ratechanger`'s doc comment on the real, verified
//! heap-buffer-overflow that path has in the C oracle itself.
//!
//! Requires `tests/golden/expected/ratechanger/` to already be populated
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

fn run_case(name: &str, extra_args: &[&str]) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/ratechanger").join(name);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_ratechanger::{name}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_ratechanger::{name}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let out_path = std::env::temp_dir().join(format!("pvc_golden_ratechanger_{name}.wav"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("ratechanger")
        .args(extra_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc ratechanger");
    assert!(status.success(), "`pvc ratechanger` failed");

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("ratechanger/{name}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for ratechanger/{name}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn sinc_no_table() {
    run_case(
        "sinc_no_table",
        &[
            "--synthesis-mode",
            "sinc",
            "--table-lookup",
            "off",
            "--duration",
            "1.0",
        ],
    );
}

#[test]
fn hold_decimate_rate_change() {
    run_case(
        "hold_decimate_rate_change",
        &[
            "--synthesis-mode",
            "hold",
            "--rate-in",
            "1.5",
            "--duration",
            "1.0",
        ],
    );
}

#[test]
fn linear_interp_slow_rate() {
    run_case(
        "linear_interp_slow_rate",
        &[
            "--synthesis-mode",
            "linear",
            "--rate-in",
            "0.75",
            "--duration",
            "1.0",
        ],
    );
}
