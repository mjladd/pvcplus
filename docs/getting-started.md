# Getting started

## Build from source

`pvc` is a Rust workspace. This is the working way to get a `pvc`
binary today.

```bash
git clone https://github.com/mjladd/pvcplus.git
cd pvcplus/rust
cargo build --release -p pvc-cli
./target/release/pvc --help
```

Put `./target/release/pvc` on your `PATH`, or run it by its full path.

## Docker

```bash
docker build -t pvcplus .
mkdir -p ~/pvctest/input ~/pvctest/output
docker run --rm -v ~/pvctest/input:/audio/input -v ~/pvctest/output:/audio/output \
  pvcplus stretch --factor 2.0 /audio/input/input.wav /audio/output/stretched.wav
```

The image's entrypoint is `pvc` itself, so `docker run --rm pvcplus`
with no arguments prints `pvc --help`. Legacy tools are still there,
reached through `docker run --rm pvcplus legacy <tool> <flags>`
instead of by their own name directly. See [Migration](migration.md).
No image is published yet, so `docker build .` is the only way to get
one today.

The devcontainer (`--target dev`) has both the Rust and C toolchains,
plus a shell, for working on `pvc` itself.

## Verify your build

Generate a short test tone, then run a couple of `pvc` commands
against it.

```bash
mkdir -p ~/pvctest
sox -n -r 44100 -c 1 ~/pvctest/input.wav synth 2 sine 440
```

No `sox`? Any short mono WAV file works, including one you already
have, or one from a few lines of Python using the standard `wave`
module.

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

Play both output files. The stretched one keeps the same pitch, over
roughly twice the time. The pitched one sounds higher, around 659 Hz
for a 440 Hz input, at close to the original duration.

## Where to go next

- [Tutorial](tutorial.md) walks through a full session: chaining
  tools, presets, `--set` overrides, `--json` scripting, and migrating
  an old `S.*` script.
- [Concepts](concepts.md) explains the phase-vocoder ideas most
  commands share (FFT size, window type, warp index, and more).
- If you are coming from the original C toolkit, read
  [Migration](migration.md). It maps every legacy tool name to its
  `pvc` equivalent.
- [Presets](presets.md) covers `pvc run` and TOML presets, the
  replacement for the old `S.*` shell scripts. Run `make demo` for a
  ready-to-hear walkthrough of five of them.
- `docs/tools/<name>.md` documents one `pvc` subcommand's own flags in
  detail. Start from `pvc --help` to see the full command list.
- Every legacy tool, ported or not, still runs through `pvc legacy
  <name> <flags>` (its own original flags, unchanged).
- `pvc completions <shell>` prints a shell completion script (bash,
  zsh, fish, elvish, or powershell). Man pages for every subcommand
  live under `man/` in this repository.
