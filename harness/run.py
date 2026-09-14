#!/usr/bin/env python3
"""QA/demo harness: runs every covered pvc tool's own preset against every
audio file in a directory. See harness/README.md for scope and usage.
"""

import argparse
import json
import shutil
import subprocess
import sys
import time
import tomllib
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
PRESETS_DIR = Path(__file__).resolve().parent / "presets"
AUDIO_EXTS = {".wav", ".aiff", ".aif", ".flac"}

# Tools whose final result file is not the preset's own `output` field.
# Maps tool name -> (--set key, file extension).
SPECIAL_OUTPUT = {
    "spectrummapper": ("segments_file", "segments"),
}

# Tools needing one or more prerequisite pvc runs, against the same input
# audio file, before they can run for real. Each entry is a list of
# (prereq tool name, --set key on the target preset that receives the
# prerequisite's own output path).
#
# `twarp` is a special case: its own usage is `<ANALYSIS> <OUTPUT>`, with
# no separate audio <INPUT> positional at all, so its prereq's own output
# fills the target preset's `input` field directly (see run_one).
PREREQS = {
    "twarp": [("analyze", "input")],
    "tvfilter": [("analyze", "filter_response")],
    "ringtvfilter": [("analyze", "filter_response")],
    "tvfiltdeviator": [("analyze", "filter_response")],
    "compand": [("freqresponse", "peaks")],
    "filtdeviator": [("freqresponse", "response")],
    "filter": [("freqresponse", "response")],
    "ringfilter": [("freqresponse", "filter_response")],
    "convolver": [("analyze", "filter_response")],
    "irconvolver": [("impulseresponse", "ir")],
}

# Tools whose result file is playable audio, worth a pvc info sanity line.
AUDIO_OUTPUT_TOOLS = {
    "denoise", "pitch", "pv", "ratechanger", "ring", "spectralextractor",
    "spectwarp", "stretch",
    "twarp", "tvfilter", "ringtvfilter", "tvfiltdeviator", "compand",
    "filtdeviator", "filter", "ringfilter", "convolver", "irconvolver",
}


def find_presets(only=None):
    presets = sorted(PRESETS_DIR.glob("*.toml"))
    if only:
        presets = [p for p in presets if p.stem in only]
    return presets


def find_audio_files(audio_dir):
    return sorted(
        p for p in Path(audio_dir).iterdir()
        if p.is_file() and p.suffix.lower() in AUDIO_EXTS
    )


def output_spec(tool, preset_path):
    if tool in SPECIAL_OUTPUT:
        return SPECIAL_OUTPUT[tool]
    with open(preset_path, "rb") as f:
        data = tomllib.load(f)
    ext = Path(data.get("output", "out.wav")).suffix or ".wav"
    return ("output", ext.lstrip("."))


def run_pvc_preset(pvc_bin, preset_path, sets):
    args = [str(pvc_bin), "run", str(preset_path), "--quiet"]
    for key, value in sets:
        args += ["--set", f"{key}={value}"]
    start = time.time()
    proc = subprocess.run(args, capture_output=True, text=True)
    return proc, time.time() - start


def pvc_info_line(pvc_bin, output_path):
    proc = subprocess.run(
        [str(pvc_bin), "info", str(output_path)],
        capture_output=True, text=True,
    )
    if proc.returncode != 0:
        return None
    lines = proc.stdout.strip().splitlines()
    stats = [line.strip() for line in lines[1:]]
    return ", ".join(stats) if stats else None


def run_one(pvc_bin, tool, preset_path, audio_file, output_dir):
    key, ext = output_spec(tool, preset_path)
    stem = audio_file.stem
    result_path = output_dir / tool / f"{stem}.{ext}"
    result_path.parent.mkdir(parents=True, exist_ok=True)
    workdir = result_path.parent

    sets = []
    prereq_targets = set()

    for prereq_tool, target_key in PREREQS.get(tool, []):
        prereq_preset = PRESETS_DIR / f"{prereq_tool}.toml"
        prereq_key, prereq_ext = output_spec(prereq_tool, prereq_preset)
        prereq_out = workdir / f"{stem}.{prereq_tool}.{prereq_ext}"
        proc, _ = run_pvc_preset(
            pvc_bin, prereq_preset, [("input", audio_file), (prereq_key, prereq_out)]
        )
        if proc.returncode != 0 or not prereq_out.exists():
            return {
                "tool": tool, "audio_file": str(audio_file), "ok": False,
                "stage": f"prereq:{prereq_tool}",
                "error": (proc.stderr or proc.stdout).strip()[-500:],
            }
        sets.append((target_key, prereq_out))
        prereq_targets.add(target_key)

    # `twarp` has no <INPUT> positional at all (usage: <ANALYSIS> <OUTPUT>),
    # so its own prereq fills the preset's `input` field directly instead
    # of the raw audio file (see PREREQS and harness/README.md).
    if "input" not in prereq_targets:
        sets.append(("input", audio_file))
    sets.append((key, result_path))
    proc, elapsed = run_pvc_preset(pvc_bin, preset_path, sets)
    ok = proc.returncode == 0 and result_path.exists() and result_path.stat().st_size > 0

    record = {
        "tool": tool,
        "audio_file": str(audio_file),
        "output": str(result_path),
        "ok": ok,
        "elapsed_s": round(elapsed, 3),
    }
    if not ok:
        record["error"] = (proc.stderr or proc.stdout).strip()[-500:]
    elif tool in AUDIO_OUTPUT_TOOLS:
        record["info"] = pvc_info_line(pvc_bin, result_path)
    return record


def resolve_pvc_bin(explicit):
    if explicit:
        path = Path(explicit)
        if not path.is_file():
            print(f"error: --pvc {explicit!r} is not a file", file=sys.stderr)
            sys.exit(1)
        return path
    built = REPO_ROOT / "rust" / "target" / "release" / "pvc"
    if built.exists():
        return built
    on_path = shutil.which("pvc")
    if on_path:
        return Path(on_path)
    print(
        "error: no pvc binary found. Run `make build` first, "
        "or pass --pvc /path/to/pvc.",
        file=sys.stderr,
    )
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--audio-dir", required=True, type=Path)
    parser.add_argument("--output-dir", default=Path("harness-output"), type=Path)
    parser.add_argument("--tool", action="append", help="Only run this tool (repeatable).")
    parser.add_argument("--pvc", help="Path to the pvc binary.")
    parser.add_argument("--report-json", type=Path)
    parser.add_argument(
        "--fail-fast", action="store_true",
        help="Stop at the first failure instead of running every combination.",
    )
    args = parser.parse_args()

    pvc_bin = resolve_pvc_bin(args.pvc)
    presets = find_presets(only=set(args.tool) if args.tool else None)
    if not presets:
        print("error: no matching presets found under harness/presets/", file=sys.stderr)
        sys.exit(1)

    audio_files = find_audio_files(args.audio_dir)
    if not audio_files:
        print(f"error: no audio files found in {args.audio_dir}", file=sys.stderr)
        sys.exit(1)

    args.output_dir.mkdir(parents=True, exist_ok=True)

    results = []
    for preset_path in presets:
        tool = preset_path.stem
        for audio_file in audio_files:
            record = run_one(pvc_bin, tool, preset_path, audio_file, args.output_dir)
            results.append(record)
            if record["ok"]:
                extra = f" ({record['info']})" if record.get("info") else ""
                print(f"OK   {tool:<20} {audio_file.name}{extra}")
            else:
                print(f"FAIL {tool:<20} {audio_file.name}: {record.get('error', '')}")
                if args.fail_fast:
                    break
        else:
            continue
        break

    passed = sum(1 for r in results if r["ok"])
    print(f"\n{passed}/{len(results)} combinations succeeded")

    if args.report_json:
        args.report_json.write_text(json.dumps(results, indent=2, default=str))
        print(f"Report written to {args.report_json}")

    sys.exit(0 if passed == len(results) else 1)


if __name__ == "__main__":
    main()
