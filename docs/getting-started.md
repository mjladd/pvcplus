# Getting started

## Prerequisites

Pick one of the three paths below. Prebuilt binaries need nothing but `curl` or a browser. Building from source needs a Rust toolchain, version 1.87 or newer, from [rustup.rs](https://rustup.rs). Run `cargo --version` first to check whether you already have one. The Docker path needs only Docker itself. The image builds `pvc` for you.

## Prebuilt binaries

Linux (x86_64, aarch64) and macOS (Intel, Apple Silicon) binaries are attached to every [release](https://github.com/mjladd/pvcplus/releases). Run this command to install the latest one:

```bash
curl --proto '=https' --tlsv1.2 -LsSf https://github.com/mjladd/pvcplus/releases/latest/download/pvc-cli-installer.sh | sh
```

The installer downloads the right binary for your machine. It makes sure that the binary matches its checksum, and puts `pvc` on your `PATH`.

Restart your shell afterward. You can also run the `source` command the installer prints instead of restarting.

To install a specific version, replace `latest/download` with `download/vX.Y.Z`. For example, use `download/v0.1.1` for version 0.1.1.

If you do not want to run a script from the internet, download the tarball instead. Get it from the [releases page](https://github.com/mjladd/pvcplus/releases). Make sure that it matches the checksum file next to it. Then extract it, and put the `pvc` binary from inside on your `PATH` yourself.

## Build from source

```bash
git clone https://github.com/mjladd/pvcplus.git
cd pvcplus/rust
cargo build --release -p pvc-cli
./target/release/pvc --help
```

Put `./target/release/pvc` on your `PATH`, or run it by its full path.

## Docker

```bash
docker pull ghcr.io/mjladd/pvcplus:latest
mkdir -p ~/pvctest/input ~/pvctest/output
docker run --rm -v ~/pvctest/input:/audio/input -v ~/pvctest/output:/audio/output \
  ghcr.io/mjladd/pvcplus:latest stretch --factor 2.0 /audio/input/input.wav /audio/output/stretched.wav
```

The image's entrypoint is `pvc` itself, so `docker run --rm ghcr.io/mjladd/pvcplus:latest` with no arguments prints `pvc --help`. Legacy tools are still there, reached through `docker run --rm ghcr.io/mjladd/pvcplus:latest legacy <tool> <flags>` instead of by their own name directly. See [Migration](migration.md). A `:legacy` tag has the original C tools only, with no `pvc` binary, for the old direct-by-name invocation style. Building your own image locally still works too: `docker build -t pvcplus .`, then use `pvcplus` in place of `ghcr.io/mjladd/pvcplus:latest` above.

The devcontainer (`--target dev`) has both the Rust and C toolchains, plus a shell, for working on `pvc` itself.

## Verify your build

Generate a short test tone, then run a couple of `pvc` commands against it.

```bash
mkdir -p ~/pvctest
sox -n -r 44100 -c 1 ~/pvctest/input.wav synth 2 sine 440
```

No `sox`? Any short mono WAV file works, including one you already have, or one from a few lines of Python using the standard `wave` module.

Check that `pvc` reads it:

```bash
pvc info ~/pvctest/input.wav
```

Time-stretch it to twice its length:

```bash
pvc stretch --factor 2.0 ~/pvctest/input.wav ~/pvctest/stretched.wav
pvc info ~/pvctest/stretched.wav
```

The stretched file's duration comes out to about twice the original's.

Transpose it up a perfect fifth (7 semitones):

```bash
pvc pitch --semitones 7 ~/pvctest/input.wav ~/pvctest/pitched.wav
```

Play both output files. The stretched one keeps the same pitch, over roughly twice the time. The pitched one sounds higher, around 659 Hz for a 440 Hz input, at close to the original duration.

## Uninstall

Steps depend on how you installed `pvc`.

### Prebuilt binary

The shell installer creates no separate uninstaller. Remove the files it placed instead:

```bash
rm -f ~/.cargo/bin/pvc ~/.cargo/bin/gen-man
rm -f ~/.config/pvc-cli/pvc-cli-receipt.json
```

The installer also adds a line to your shell profile. When `~/.cargo/bin` is already on your `PATH`, it skips that step. A Rust toolchain from rustup.rs is one common reason to already have it there. If you have no other reason to keep `~/.cargo/bin` on your `PATH`, remove that line from your profile too.

### Docker

Remove the image instead of a file:

```bash
docker rmi ghcr.io/mjladd/pvcplus:latest
```

Repeat for any other tag you pulled, such as `:legacy` or a version tag.

### Build from source

Delete the cloned repository. If you copied `rust/target/release/pvc` somewhere else first, delete that copy too.

## Where to go next

- [Tutorial](tutorial.md) walks through a full session: chaining tools, presets, `--set` overrides, `--json` scripting, and migrating an old `S.*` script.
- [Concepts](concepts.md) explains the phase-vocoder ideas most commands share (FFT size, window type, warp index, and more).
- If you are coming from the original C toolkit, read [Migration](migration.md). It maps every legacy tool name to its `pvc` equivalent.
- [Presets](presets.md) covers `pvc run` and TOML presets, the replacement for the old `S.*` shell scripts. Run `make demo` for a ready-to-hear walkthrough of five of them.
- `docs/tools/<name>.md` documents one `pvc` subcommand's own flags in detail. Start from `pvc --help` to see the full command list.
- Every legacy tool, ported or not, still runs through `pvc legacy <name> <flags>` (its own original flags, unchanged).
- `pvc completions <shell>` prints a shell completion script (bash, zsh, fish, elvish, or powershell). Man pages for every subcommand live under `man/` in this repository.
