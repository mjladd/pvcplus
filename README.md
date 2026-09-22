# pvcplus

A phase-vocoder toolkit for transforming and analyzing sound: time stretch, pitch transposition, spectral filtering, additive resynthesis, resonance, feature extraction, and more, from the command line.

`pvc` is a modern Rust rewrite of the original **PVC / PVCplus** toolkit by Paul Koonce, built on Eric Lyon's and Chris Penrose's phase-vocoder work. The original C tools still ship alongside it. See [NOTICE.md](NOTICE.md) for full attribution and [docs/migration.md](docs/migration.md) for how each one maps to a new `pvc` command.

## Prerequisites

Building from source needs a Rust toolchain, version 1.87 or newer. Run `cargo --version` to check whether you already have one. If not, install one from [rustup.rs](https://rustup.rs). `pvc` uses pure-Rust file I/O, so no other system library is needed.

If you have no Rust toolchain on your machine, install a [prebuilt binary](docs/getting-started.md#prebuilt-binaries) instead, or use [Docker](docs/getting-started.md#docker). The image builds `pvc` for you.

## Quickstart

Install the latest release for Linux or macOS with one command:

```bash
curl --proto '=https' --tlsv1.2 -LsSf https://github.com/mjladd/pvcplus/releases/latest/download/pvc-cli-installer.sh | sh
pvc stretch --factor 2.0 input.wav output.wav
```

On macOS, a binary you download with a browser is quarantined, and it does not open. Remove the quarantine attribute before you run it:

```bash
xattr -d com.apple.quarantine /path/to/pvc
```

Building from source instead:

```bash
cd rust
cargo build --release -p pvc-cli
./target/release/pvc stretch --factor 2.0 input.wav output.wav
```

Full setup and a verified walkthrough live in [docs/getting-started.md](docs/getting-started.md). Run `make demo` for a ready-to-hear tour of five transforms, described in [examples/README.md](examples/README.md).

## Documentation

- [Getting started](docs/getting-started.md): build from source or Docker, a verified quickstart, and where to go next.
- [Tutorial](docs/tutorial.md): a full walkthrough, chaining tools, building presets, using `--set` overrides, scripting with `--json`, and migrating an old `S.*` script.
- [Concepts](docs/concepts.md): the phase-vocoder ideas most `pvc` commands share.
- [Migration](docs/migration.md): every legacy tool name mapped to its `pvc` equivalent.
- [Presets](docs/presets.md): `pvc run` and TOML presets, the replacement for the old `S.*` shell scripts.
- `docs/tools/<name>.md`: one page per `pvc` subcommand, with its own flags and an example. Start from `pvc --help` for the full list.
- `docs/dev/`: internal notes for contributors (build, CI, port status, verification findings).
- [harness/README.md](harness/README.md): a QA script that runs every covered `pvc` tool's own preset against a directory of your own audio files.

## Project layout

- `rust/`: the `pvc` CLI (`pvc-cli`), its DSP core (`pvc-core`), and its file I/O (`pvc-io`).
- `legacy/`: the original C toolkit, still buildable and still reachable through `pvc legacy <name> <flags>`.
- `tests/golden/`: a golden-file harness that checks each ported tool against the original C tool's own output.

## License

New work in this repository (the Rust CLI, build system, test harness, and documentation) is MIT licensed. See [LICENSE](LICENSE) and [NOTICE.md](NOTICE.md) for the original C toolkit's own attribution.
