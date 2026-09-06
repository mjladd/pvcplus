//! Golden harness integration test for `pvc impulseresponse` (Phase 5):
//! runs `pvc impulseresponse` with flags equivalent to each `tests/golden/
//! cases/impulseresponse/*.toml` case's `legacy_steps`, then compares the
//! resulting `.ir` file directly against the recorded C-oracle output.
//!
//! Unlike `golden_plainpv.rs`, this doesn't go through `tests/golden/
//! compare.py`'s generic comparators - `.ir` is a binary format (a 4-int
//! header plus raw rfft-format spectra), not text `compare_numeric` can
//! parse. So this test reads both files through `pvc_io::ir::read_ir` and
//! compares the decoded header and spectrum values directly - what the
//! case's `.toml` `notes` field already says the comparison method would
//! be.
//!
//! Requires `tests/golden/expected/impulseresponse/` to already be
//! populated via `bash tests/golden/run_legacy.sh` (not git-tracked). Each
//! case skips itself with a clear message rather than failing confusingly
//! when that prerequisite is missing.

use std::path::{Path, PathBuf};
use std::process::Command;

fn repo_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../../..")
        .canonicalize()
        .expect("repo root should exist")
}

fn run_case(case: &str, pvc_args: &[&str], input_fixture: &str) {
    let root = repo_root();
    let expected_path = root
        .join("tests/golden/expected/impulseresponse")
        .join(case)
        .join("output.ir");
    if !expected_path.is_file() {
        eprintln!(
            "SKIP golden_impulseresponse::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_path.display()
        );
        return;
    }

    let input = root.join("tests/golden/fixtures").join(input_fixture);
    let out_path = std::env::temp_dir().join(format!("pvc_golden_impulseresponse_{case}.ir"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("impulseresponse")
        .args(pvc_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc");
    assert!(
        status.success(),
        "`pvc impulseresponse {pvc_args:?}` failed for case {case}"
    );

    let expected = pvc_io::read_ir(&expected_path).expect("failed to read expected .ir");
    let candidate = pvc_io::read_ir(&out_path).expect("failed to read candidate .ir");

    assert_eq!(
        expected.header, candidate.header,
        "case {case}: header mismatch"
    );
    assert_eq!(
        expected.channels.len(),
        candidate.channels.len(),
        "case {case}: channel count"
    );

    for (ch, (exp_ch, cand_ch)) in expected
        .channels
        .iter()
        .zip(&candidate.channels)
        .enumerate()
    {
        assert_eq!(
            exp_ch.len(),
            cand_ch.len(),
            "case {case}: channel {ch} spectrum length"
        );
        let max_err = exp_ch
            .iter()
            .zip(cand_ch)
            .fold(0.0f32, |acc, (&e, &c)| acc.max((e - c).abs()));
        // rfft/DbToAmp were each individually verified bit-exact-ish
        // against the C oracle (see docs/dev/rust-verification.md) - a
        // tight tolerance here is checking for a structural mismatch, not
        // chasing floating-point summation-order noise (observed max abs
        // error for this exact case: ~6e-8, one f32 ULP).
        assert!(
            max_err < 1e-4,
            "case {case}: channel {ch} max spectrum value error {max_err} too large"
        );
    }
}

#[test]
fn basic() {
    run_case(
        "basic",
        &["--begin", "0", "--end", "0.1"],
        "sine440_2s_44k.wav",
    );
}
