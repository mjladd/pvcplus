#!/usr/bin/env python3
"""Compares a candidate output (from a ported Rust tool, Phase 3+) against
the C oracle recorded by run_legacy.py under tests/golden/expected/, using
the tolerance declared in that case's tests/golden/cases/<tool>/<case>.toml.

Usage: compare.py <tool>/<case> <candidate-file>

Exit 0 = within tolerance, 1 = out of tolerance or a structural mismatch
(different length/channel count/etc.), 2 = usage/setup error.

No candidate exists yet (there is no Rust CLI as of Phase 1) - this is
written now, against real recorded expected/ output, so it's ready the
day Phase 3 needs it rather than being designed blind.
"""
import argparse
import math
import os
import struct
import sys
import tomllib
import wave

GOLDEN = os.path.dirname(os.path.abspath(__file__))
CASES_DIR = os.path.join(GOLDEN, "cases")
EXPECTED_DIR = os.path.join(GOLDEN, "expected")


def load_case(tool_slash_name):
    tool, name = tool_slash_name.split("/", 1)
    path = os.path.join(CASES_DIR, tool, f"{name}.toml")
    with open(path, "rb") as f:
        case = tomllib.load(f)
    case["_tool"] = tool
    case["_name"] = name
    return case


def read_wav_samples(path):
    with wave.open(path, "rb") as w:
        n, sw, ch, rate = w.getnframes(), w.getsampwidth(), w.getnchannels(), w.getframerate()
        raw = w.readframes(n)
    if sw != 2:
        raise ValueError(f"{path}: only 16-bit PCM WAV supported by this comparator (got {sw*8}-bit)")
    samples = struct.unpack(f"<{len(raw)//2}h", raw)
    return samples, ch, rate


def compare_sample(expected_path, candidate_path, max_abs_error):
    exp, ech, erate = read_wav_samples(expected_path)
    cand, cch, crate = read_wav_samples(candidate_path)
    if ech != cch:
        return False, f"channel count differs: expected {ech}, got {cch}"
    if erate != crate:
        return False, f"sample rate differs: expected {erate}, got {crate}"
    if len(exp) != len(cand):
        return False, f"length differs: expected {len(exp)} samples, got {len(cand)}"
    max_err = 0.0
    for a, b in zip(exp, cand):
        err = abs(a - b) / 32768.0
        if err > max_err:
            max_err = err
    if max_err > max_abs_error:
        return False, f"max abs sample error {max_err:.6g} > tolerance {max_abs_error:.6g}"
    return True, f"max abs sample error {max_err:.6g} (tolerance {max_abs_error:.6g})"


def compare_spectral(expected_path, candidate_path, max_db_error):
    # Coarse spectral-domain comparison for oscillator-bank tools, where
    # summation-order differences make sample-exact comparison meaningless:
    # per-block RMS-in-dB, compared block by block. A real spectrogram (STFT
    # magnitude) comparison is the better long-term version of this; this is
    # deliberately simple until Phase 3 shows what precision is actually
    # needed against a real ported tool.
    exp, ech, erate = read_wav_samples(expected_path)
    cand, cch, crate = read_wav_samples(candidate_path)
    if ech != cch:
        return False, f"channel count differs: expected {ech}, got {cch}"
    if erate != crate:
        return False, f"sample rate differs: expected {erate}, got {crate}"
    block = 1024
    n = min(len(exp), len(cand))
    if abs(len(exp) - len(cand)) > block:
        return False, f"length differs by more than one block: expected {len(exp)}, got {len(cand)}"
    max_db_diff = 0.0
    for start in range(0, n, block):
        e_block = exp[start:start + block]
        c_block = cand[start:start + block]
        e_rms = math.sqrt(sum((s / 32768.0) ** 2 for s in e_block) / len(e_block)) if e_block else 0.0
        c_rms = math.sqrt(sum((s / 32768.0) ** 2 for s in c_block) / len(c_block)) if c_block else 0.0
        e_db = 20 * math.log10(max(e_rms, 1e-9))
        c_db = 20 * math.log10(max(c_rms, 1e-9))
        max_db_diff = max(max_db_diff, abs(e_db - c_db))
    if max_db_diff > max_db_error:
        return False, f"max per-block RMS dB difference {max_db_diff:.3g} > tolerance {max_db_error:.3g}"
    return True, f"max per-block RMS dB difference {max_db_diff:.3g} (tolerance {max_db_error:.3g})"


def compare_numeric(expected_path, candidate_path, max_abs_error):
    with open(expected_path) as f:
        exp_vals = [float(x) for x in f.read().split()]
    with open(candidate_path) as f:
        cand_vals = [float(x) for x in f.read().split()]
    if len(exp_vals) != len(cand_vals):
        return False, f"value count differs: expected {len(exp_vals)}, got {len(cand_vals)}"
    max_err = max((abs(a - b) for a, b in zip(exp_vals, cand_vals)), default=0.0)
    if max_err > max_abs_error:
        return False, f"max abs value error {max_err:.6g} > tolerance {max_abs_error:.6g}"
    return True, f"max abs value error {max_err:.6g} (tolerance {max_abs_error:.6g})"


def compare_exact(expected_path, candidate_path):
    with open(expected_path, "rb") as f:
        exp = f.read()
    with open(candidate_path, "rb") as f:
        cand = f.read()
    if exp != cand:
        return False, f"byte-exact comparison failed ({len(exp)} vs {len(cand)} bytes)"
    return True, "byte-exact match"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("case", help="tool/case-name, e.g. plainpv/stretch_overlap_add")
    ap.add_argument("candidate", help="path to the candidate output file")
    args = ap.parse_args()

    case = load_case(args.case)
    expected_path = os.path.join(EXPECTED_DIR, case["_tool"], case["_name"], f"output.{case['output_ext']}")
    if not os.path.exists(expected_path):
        print(f"error: no recorded expected output at {expected_path} - run run_legacy.sh first", file=sys.stderr)
        sys.exit(2)
    if not os.path.exists(args.candidate):
        print(f"error: candidate file not found: {args.candidate}", file=sys.stderr)
        sys.exit(2)

    tol = case["tolerance"]
    kind = tol["kind"]
    if kind == "sample":
        ok, msg = compare_sample(expected_path, args.candidate, tol["max_abs_error"])
    elif kind == "spectral":
        ok, msg = compare_spectral(expected_path, args.candidate, tol["max_db_error"])
    elif kind == "numeric":
        ok, msg = compare_numeric(expected_path, args.candidate, tol["max_abs_error"])
    elif kind == "exact":
        ok, msg = compare_exact(expected_path, args.candidate)
    else:
        print(f"error: unknown tolerance kind {kind!r}", file=sys.stderr)
        sys.exit(2)

    status = "PASS" if ok else "FAIL"
    print(f"{status}  {args.case}: {msg}")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
