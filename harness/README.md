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

These 10 more need one companion file first. The file is a `.pva` or a
`.fr` file, from `pvc analyze` or `pvc freqresponse`. Two tools need a
`.ir` file instead, from `pvc impulseresponse`. `run.py`'s own
`PREREQS` mapping builds that companion file from the same audio file.
It then feeds the companion file's path into the target preset:

- `twarp`, `tvfilter`, `ringtvfilter`, `tvfiltdeviator` (`.pva`)
- `compand`, `filtdeviator`, `filter`, `ringfilter` (`.fr`)
- `convolver` (`.pva`), `irconvolver` (`.ir`)

These four more need a multi-step chain, or a fixed, hand-authored
data table instead of a same-file companion. `run.py`'s own
`SPECIAL_PREREQS` and `STATIC_FIXTURES` mappings cover them:

- `delayfilter` chains two prerequisites: `pvc analyze` builds a
  `.pva`, then `pvc fn response groupdelaymaker` builds a `.fr` from
  that `.pva` plus a fixed partials table under `harness/fixtures/`.
  Like `twarp`, this tool has no audio `<INPUT>` positional at all.
- `irconvolvesequencer` needs a directory holding an
  `impulseFileNames` list file. `run.py` writes a single-impulse list
  naming the same audio file under test.
- `harmonize` and `inharmonator` each read a fixed data table under
  `harness/fixtures/`. Neither table depends on the audio file under
  test.

`chordmapperplus` needs a `.pva` (via `pvc analyze`, like the tools
above) and its own 23-field tone data table. That table's format was
only partly documented on its own doc page, six of the fields, not
all 23. This preset's fixture comes from the Rust source instead:
`rust/crates/pvc-core/src/tools/chordmapperplus.rs`'s own `ToneParams`
struct, and its `parse_tone_data_file_reads_all_23_fields` test.

Like `twarp` and `delayfilter`,
this tool has no audio `<INPUT>` positional at all, and unlike either
of them, it has no other positional either. Only `--analysis` and
`--tones` name its two input files.

Two tools are excluded on purpose, not just left for later:

- `convert-units` takes no audio file at all. It converts numbers
  between units.
- `pvc fn`'s own subcommands (`gen1` through `gen6`, `plot`, and the
  `response` family) generate or plot control-function data tables.
  None of them transform an arbitrary audio file, so none fit this
  harness's model.
