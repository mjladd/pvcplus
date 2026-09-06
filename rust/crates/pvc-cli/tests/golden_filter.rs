//! Golden harness integration test for `pvc filter` (Task 3.8): builds a
//! `.fr` response with the matching `pvc` response-producing subcommand,
//! runs `pvc filter` on it, and compares against the recorded C-oracle
//! output via `compare.py` - reusing the existing `tests/golden/cases/
//! filter/*.toml` cases (already exercising all three response-file
//! producers: `chordresponsemaker`, `filtresponsemaker`, `freqresponse`).
//!
//! Requires `tests/golden/expected/filter/` to already be populated via
//! `bash tests/golden/run_legacy.sh` (not git-tracked) and a `python3` on
//! `PATH`.

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

fn run_case(case: &str, build_response: impl FnOnce(&Path, &Path), filter_args: &[&str]) {
    let root = repo_root();
    let expected_dir = root.join("tests/golden/expected/filter").join(case);
    if !expected_dir.is_dir() {
        eprintln!(
            "SKIP golden_filter::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_dir.display()
        );
        return;
    }
    if !python3_available() {
        eprintln!("SKIP golden_filter::{case}: python3 not found on PATH");
        return;
    }

    let input = root.join("tests/golden/fixtures/sine440_2s_44k.wav");
    let response_path = std::env::temp_dir().join(format!("pvc_golden_filter_{case}.fr"));
    let out_path = std::env::temp_dir().join(format!("pvc_golden_filter_{case}.wav"));

    build_response(&input, &response_path);

    let mut args = vec!["filter", "--response"];
    let response_str = response_path.to_str().unwrap();
    args.push(response_str);
    args.extend_from_slice(filter_args);
    let input_str = input.to_str().unwrap();
    let out_str = out_path.to_str().unwrap();
    args.push(input_str);
    args.push(out_str);
    run_pvc(&args);

    let compare = Command::new("python3")
        .arg(root.join("tests/golden/compare.py"))
        .arg(format!("filter/{case}"))
        .arg(&out_path)
        .output()
        .expect("failed to run compare.py");
    assert!(
        compare.status.success(),
        "compare.py failed for filter/{case}:\n{}{}",
        String::from_utf8_lossy(&compare.stdout),
        String::from_utf8_lossy(&compare.stderr)
    );
}

#[test]
fn filtresponsemaker_driven() {
    let root = repo_root();
    let breakpoints = root.join("tests/golden/fixtures/filter_breakpoints.txt");
    run_case(
        "filtresponsemaker_driven",
        |input, response_path| {
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
        },
        &[],
    );
}

#[test]
fn chordresponsemaker_driven() {
    let root = repo_root();
    let partials = root.join("tests/golden/fixtures/chord_table.txt");
    run_case(
        "chordresponsemaker_driven",
        |_input, response_path| {
            run_pvc(&[
                "fn",
                "response",
                "chordresponsemaker",
                "--fft",
                "1024",
                "--sample-rate",
                "44100",
                "--partials",
                partials.to_str().unwrap(),
                response_path.to_str().unwrap(),
            ]);
        },
        &[],
    );
}

#[test]
fn freqresponse_driven() {
    run_case(
        "freqresponse_driven",
        |input, response_path| {
            run_pvc(&[
                "freqresponse",
                "--spectrum-type",
                "average",
                input.to_str().unwrap(),
                response_path.to_str().unwrap(),
            ]);
        },
        &["--band-reject"],
    );
}
