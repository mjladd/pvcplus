# Audio/preset QA harness

`harness/run.py` runs every covered `pvc` tool's own preset against every
audio file in a directory you choose. Use it to sanity-check the whole
toolkit against real material, or as a quick tour of most of the command
surface at once.

This is a development script. It does not ship in the `pvc` binary or in
any release artifact.

## Usage

```bash
make build
python3 harness/run.py --audio-dir /path/to/your/audio
```

Results land under `harness-output/<tool>/<audio-file-stem>.<ext>` by
default. Useful flags:

- `--output-dir <dir>`: write results somewhere else.
- `--tool <name>` (repeatable): only run specific tools.
- `--report-json <path>`: write the full result set as JSON.
- `--pvc <path>`: use a specific `pvc` binary instead of
  `rust/target/release/pvc`.
- `--fail-fast`: stop at the first failure, instead of running every
  combination.

The script prints one `OK`/`FAIL` line per (tool, audio file) pair, then a
final tally. It continues past failures by default, since this is an
exploration tool, not a hard gate.

## Presets

`harness/presets/<tool>.toml` holds one real `pvc run`-compatible preset
per covered tool. These are separate from the curated five under
`examples/presets/` (the ones `pvc preset list` and `make demo` use).
Growing that shipped, user-facing list to match every tool here is a
real behavior change to a documented feature. This harness keeps its
own, larger set instead.

## Coverage

These 18 tools need nothing but an audio file in and out. Presets exist
for all of them today:

- `analyze`, `centroid`, `denoise`, `envelope`, `flux`
- `freqresponse`, `impulseresponse`, `peakformant`, `pitch`, `pitchtrack`
- `pv`, `ratechanger`, `ring`, `specflattracker`, `spectralextractor`
- `spectrummapper`, `spectwarp`, `stretch`

Roughly 16 more tools need a companion file built first. Each one needs
a `.pva`, `.fr`, or `.ir` file, or a hand-authored data table. These are
not covered yet. `docs/dev/port-status.md` tracks this work the same
way it tracks the rest of the project.

Two tools are excluded on purpose, not just left for later:

- `convert-units` takes no audio file at all. It converts numbers
  between units.
- `pvc fn`'s own subcommands (`gen1` through `gen6`, `plot`, and the
  `response` family) generate or plot control-function data tables.
  None of them transform an arbitrary audio file, so none fit this
  harness's model.
