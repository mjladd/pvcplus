# Getting started

## Build from source

`pvc` is a Rust workspace. This is the working way to get a `pvc`
binary today.

```bash
git clone https://github.com/mjladd/docker-pvcplus.git
cd docker-pvcplus/rust
cargo build --release -p pvc-cli
./target/release/pvc --help
```

Put `./target/release/pvc` on your `PATH`, or run it by its full path.

## Docker

The project's `Dockerfile` builds the legacy C toolkit into a runtime
image today. It does not yet include the `pvc` binary itself. Until
that lands, `docker build .` gives you a container with the original C
tools only. Each one is reachable by its own name inside the
container, for example `plainpv` or `noisefilter`. The devcontainer
(`--target dev`) gives you the same legacy tools plus a shell, but no
Rust toolchain yet either.

If you need `pvc` inside a container today, build it from source
inside your own image or devcontainer, on top of a Rust base image.

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
