# pvcplus

A phase-vocoder toolkit for transforming and analyzing sound: time
stretch, pitch transposition, spectral filtering, additive resynthesis,
resonance, feature extraction, and more, from the command line.

`pvc` is a modern Rust rewrite of the original **PVC / PVCplus**
toolkit by Paul Koonce, built on Eric Lyon's and Chris Penrose's
phase-vocoder work. The original C tools still ship alongside it. See
[NOTICE.md](NOTICE.md) for full attribution and
[docs/migration.md](docs/migration.md) for how each one maps to a new
`pvc` command.

## Quickstart

```bash
cd rust
cargo build --release -p pvc-cli
./target/release/pvc stretch --factor 2.0 input.wav output.wav
```

Full setup and a verified walkthrough live in
[docs/getting-started.md](docs/getting-started.md). Run `make demo`
for a ready-to-hear tour of five transforms, described in
[examples/README.md](examples/README.md).

## Documentation

- [Getting started](docs/getting-started.md): build from source, a
  verified quickstart, and where to go next.
- [Concepts](docs/concepts.md): the phase-vocoder ideas most `pvc`
  commands share.
- [Migration](docs/migration.md): every legacy tool name mapped to its
  `pvc` equivalent.
- [Presets](docs/presets.md): `pvc run` and TOML presets, the
  replacement for the old `S.*` shell scripts.
- `docs/tools/<name>.md`: one page per `pvc` subcommand, with its own
  flags and an example. Start from `pvc --help` for the full list.
- `docs/dev/`: internal notes for contributors (build, CI, port
  status, verification findings).

## Project layout

- `rust/`: the `pvc` CLI (`pvc-cli`), its DSP core (`pvc-core`), and
  its file I/O (`pvc-io`).
- `legacy/`: the original C toolkit, still buildable and still
  reachable through `pvc legacy <name> <flags>`.
- `tests/golden/`: a golden-file harness that checks each ported tool
  against the original C tool's own output.

## License

New work in this repository (the Rust CLI, build system, test
harness, and documentation) is MIT licensed. See
[LICENSE](LICENSE) and [NOTICE.md](NOTICE.md) for the original C
toolkit's own attribution.
