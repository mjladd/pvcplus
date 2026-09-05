#!/usr/bin/env python3
"""Runs every case in tests/golden/cases/ against the legacy C tools and
records the result under tests/golden/expected/<tool>/<case>/ - the output
file itself, a hash, and lightweight stats (peak/RMS/duration for audio,
line/value count for text/float files). This is the C oracle: Phase 3 ports
get checked against what's recorded here, via compare.py.

Usage: run_legacy.py [--bin-dir DIR] [--case tool/name] [--keep-going]

Requires the legacy tools already built and installed (e.g.
/opt/pvc-legacy/bin from the Dockerfile's legacy-build stage, or a local
`cmake --install` prefix) - pass that directory as --bin-dir, or it's
picked up from $PVC_LEGACY_BIN, or defaults to /opt/pvc-legacy/bin.
"""
import argparse
import hashlib
import json
import math
import os
import shlex
import shutil
import struct
import subprocess
import sys
import tomllib
import wave

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GOLDEN = os.path.join(REPO, "tests", "golden")
CASES_DIR = os.path.join(GOLDEN, "cases")
FIXTURES_DIR = os.path.join(GOLDEN, "fixtures")
EXPECTED_DIR = os.path.join(GOLDEN, "expected")


def find_cases(only=None):
    cases = []
    for tool in sorted(os.listdir(CASES_DIR)):
        tool_dir = os.path.join(CASES_DIR, tool)
        if not os.path.isdir(tool_dir):
            continue
        for fname in sorted(os.listdir(tool_dir)):
            if not fname.endswith(".toml"):
                continue
            name = fname[:-len(".toml")]
            if only and f"{tool}/{name}" != only:
                continue
            with open(os.path.join(tool_dir, fname), "rb") as f:
                case = tomllib.load(f)
            case["_tool"] = tool
            case["_name"] = name
            cases.append(case)
    return cases


def wav_stats(path):
    try:
        with wave.open(path, "rb") as w:
            n = w.getnframes()
            sw = w.getsampwidth()
            ch = w.getnchannels()
            rate = w.getframerate()
            raw = w.readframes(n)
    except wave.Error:
        return None
    if sw == 2:
        fmt = f"<{len(raw)//2}h"
        samples = struct.unpack(fmt, raw)
        maxval = 32768.0
    elif sw == 1:
        samples = [b - 128 for b in raw]
        maxval = 128.0
    else:
        return {"channels": ch, "sample_rate": rate, "frames": n, "sample_width": sw,
                "note": "stats skipped for this sample width"}
    if samples:
        peak = max(abs(s) for s in samples) / maxval
        rms = math.sqrt(sum((s / maxval) ** 2 for s in samples) / len(samples))
    else:
        peak = rms = 0.0
    return {
        "channels": ch, "sample_rate": rate, "frames": n,
        "duration_s": n / rate if rate else 0,
        "peak": peak, "rms": rms,
    }


def text_stats(path):
    with open(path, "rb") as f:
        data = f.read()
    lines = data.count(b"\n")
    return {"bytes": len(data), "lines": lines}


def sha256_of(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def run_case(case, bin_dir, keep_going):
    tool, name = case["_tool"], case["_name"]
    label = f"{tool}/{name}"
    workdir = os.path.join("/tmp", "golden_run", tool, name)
    shutil.rmtree(workdir, ignore_errors=True)
    os.makedirs(workdir, exist_ok=True)

    output_ext = case["output_ext"]
    output_path = os.path.join(workdir, f"output.{output_ext}")
    subs = {
        "fixtures": FIXTURES_DIR,
        "output": output_path,
    }
    if case.get("input"):
        subs["input"] = os.path.join(FIXTURES_DIR, case["input"])

    env = dict(os.environ)
    env["PATH"] = bin_dir + os.pathsep + env.get("PATH", "")

    for step in case["legacy_steps"]:
        cmd = step.format(**subs)
        proc = subprocess.run(cmd, shell=True, cwd=workdir, env=env,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if proc.returncode != 0:
            print(f"FAIL  {label}: `{cmd}` exited {proc.returncode}")
            print("      " + proc.stdout.decode(errors="replace").replace("\n", "\n      ")[-2000:])
            if not keep_going:
                sys.exit(1)
            return False

    if not os.path.exists(output_path) or os.path.getsize(output_path) == 0:
        print(f"FAIL  {label}: no output produced at {output_path}")
        if not keep_going:
            sys.exit(1)
        return False

    out_dir = os.path.join(EXPECTED_DIR, tool, name)
    os.makedirs(out_dir, exist_ok=True)
    dest = os.path.join(out_dir, f"output.{output_ext}")
    shutil.copyfile(output_path, dest)

    stats = {"sha256": sha256_of(dest), "size_bytes": os.path.getsize(dest)}
    if output_ext == "wav":
        s = wav_stats(dest)
        if s:
            stats.update(s)
    elif output_ext in ("txt", "pva", "f32"):
        stats.update(text_stats(dest))

    with open(os.path.join(out_dir, "stats.json"), "w") as f:
        json.dump(stats, f, indent=2)

    print(f"OK    {label}")
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin-dir", default=os.environ.get("PVC_LEGACY_BIN", "/opt/pvc-legacy/bin"))
    ap.add_argument("--case", help="run only tool/case-name")
    ap.add_argument("--keep-going", action="store_true", help="don't stop at the first failing case")
    args = ap.parse_args()

    if not os.path.isdir(args.bin_dir):
        print(f"error: --bin-dir {args.bin_dir} does not exist "
              f"(build+install the legacy tools first, or pass --bin-dir)", file=sys.stderr)
        sys.exit(2)

    cases = find_cases(args.case)
    if not cases:
        print("error: no matching cases found", file=sys.stderr)
        sys.exit(2)

    passed = 0
    for case in cases:
        if run_case(case, args.bin_dir, args.keep_going):
            passed += 1

    print(f"\n{passed}/{len(cases)} cases recorded")
    if passed != len(cases):
        sys.exit(1)


if __name__ == "__main__":
    main()
