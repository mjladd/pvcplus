//! Golden harness integration test for `pvc analyze` (Task 3.3): runs
//! `pvc analyze` with flags equivalent to each `tests/golden/cases/
//! pvanalysis/*.toml` case's `legacy_steps`, then compares the resulting
//! frames directly against the recorded C-oracle `.pva` output.
//!
//! Unlike `golden_plainpv.rs`, this doesn't go through
//! `tests/golden/compare.py`'s generic `kind = "exact"` byte comparison:
//! `pvc analyze` only ever writes the new `PVA1` format (see
//! `pvc_io::pva`'s module doc comment), which never byte-matches the
//! legacy file layout the C oracle wrote - a deliberate, documented
//! design decision (plan §8.1.3), not a bug to work around. So this test
//! reads both files through `pvc_io::pva` (`read_legacy_pva`/`read_pva`)
//! and compares the decoded header fields and frame data instead - what
//! each case's `.toml` `notes` field already says the comparison method
//! would be.
//!
//! Requires `tests/golden/expected/pvanalysis/` to already be populated
//! via `bash tests/golden/run_legacy.sh` (not git-tracked). Each case
//! skips itself with a clear message rather than failing confusingly
//! when that prerequisite is missing.

use std::path::{Path, PathBuf};
use std::process::Command;

fn repo_root() -> PathBuf {
    // This crate lives at <repo>/rust/crates/pvc-cli.
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../../..")
        .canonicalize()
        .expect("repo root should exist")
}

/// Runs `pvc analyze <pvc_args> <input> <out_path>`, then compares the
/// resulting frames against `tests/golden/expected/pvanalysis/<case>/
/// output.pva` (a real legacy-format file). Skips (prints and returns) if
/// the recorded expected output isn't available.
fn run_case(case: &str, pvc_args: &[&str], input_fixture: &str) {
    let root = repo_root();
    let expected_path = root
        .join("tests/golden/expected/pvanalysis")
        .join(case)
        .join("output.pva");
    if !expected_path.is_file() {
        eprintln!(
            "SKIP golden_pvanalysis::{case}: {} not found - run `bash tests/golden/run_legacy.sh` first",
            expected_path.display()
        );
        return;
    }

    let input = root.join("tests/golden/fixtures").join(input_fixture);
    let out_path = std::env::temp_dir().join(format!("pvc_golden_pvanalysis_{case}.pva"));

    let status = Command::new(env!("CARGO_BIN_EXE_pvc"))
        .arg("analyze")
        .args(pvc_args)
        .arg(&input)
        .arg(&out_path)
        .status()
        .expect("failed to run pvc");
    assert!(
        status.success(),
        "`pvc analyze {pvc_args:?}` failed for case {case}"
    );

    let expected =
        pvc_io::read_legacy_pva(&expected_path).expect("failed to read expected legacy .pva");
    let candidate = pvc_io::read_pva(&out_path).expect("failed to read candidate .pva");

    assert_eq!(expected.header.n, candidate.header.n, "case {case}: N");
    assert_eq!(expected.header.d, candidate.header.d, "case {case}: D");
    assert_eq!(
        expected.header.sample_rate, candidate.header.sample_rate,
        "case {case}: sample_rate"
    );
    assert_eq!(
        expected.header.channels, candidate.header.channels,
        "case {case}: channels"
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
            "case {case}: channel {ch} frame count"
        );
        let mut max_mag_err = 0.0f32;
        let mut max_freq_err = 0.0f32;
        for (frame_idx, (exp_frame, cand_frame)) in exp_ch.iter().zip(cand_ch).enumerate() {
            assert_eq!(
                exp_frame.len(),
                cand_frame.len(),
                "case {case}: channel {ch} frame {frame_idx} length"
            );
            // Frequency error is only meaningful for bins with real
            // energy: `convert()`'s frequency estimate for a near-silent
            // bin comes from `atan2` of near-zero real/imaginary parts,
            // which is hugely sensitive to floating-point rounding noise
            // - the C and a bit-faithful Rust port can legitimately
            // disagree by hundreds of Hz on a bin whose magnitude is
            // ~1e-10 (verified against the real oracle: one such outlier
            // in this exact case, at a bin with expected magnitude
            // 2.2e-10, is what first exposed this and isn't a structural
            // bug). Magnitude error has no such floor - it's checked on
            // every bin.
            for bin_pair in exp_frame.chunks_exact(2).zip(cand_frame.chunks_exact(2)) {
                let (exp_pair, cand_pair) = bin_pair;
                let (e_mag, e_freq) = (exp_pair[0], exp_pair[1]);
                let (c_mag, c_freq) = (cand_pair[0], cand_pair[1]);
                max_mag_err = max_mag_err.max((e_mag - c_mag).abs());
                if e_mag > 1e-4 {
                    max_freq_err = max_freq_err.max((e_freq - c_freq).abs());
                }
            }
        }
        // fold/rfft/convert/eq/spectmagwarp were each individually
        // verified bit-exact-ish against the C oracle (see
        // docs/dev/rust-verification.md) - a tight tolerance here is
        // checking for a structural mismatch, not chasing floating-point
        // summation-order noise.
        assert!(
            max_mag_err < 1e-3,
            "case {case}: channel {ch} max magnitude error {max_mag_err} too large"
        );
        assert!(
            max_freq_err < 1e-1,
            "case {case}: channel {ch} max frequency error {max_freq_err} Hz too large"
        );
    }
}

#[test]
fn basic_analysis() {
    run_case("basic_analysis", &["--fft", "1024"], "sine440_2s_44k.wav");
}

#[test]
fn stereo_analysis() {
    run_case(
        "stereo_analysis",
        &["--fft", "1024"],
        "stereo_two_tones_44k.wav",
    );
}
