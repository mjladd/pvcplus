# Multi-stage, multi-arch Dockerfile (Task 0.6)

Replaces the old single-stage `ubuntu:22.04` Dockerfile that hand-patched the
original Makefiles with `sed` (see `docs/dev/baseline.md`) — that approach
predates the CMake build (Task 0.4) and had been left in place, unused by
anything, since `legacy/CMakeLists.txt` was added.

## Stages

- **`legacy-build`** (`debian:bookworm-slim`): installs the C toolchain,
  configures and builds `legacy/` with CMake, installs 53 binaries to
  `/opt/pvc-legacy`.
- **`dev`** (extends `legacy-build`): adds `zsh`/`git`/`curl` for the
  devcontainer. `.devcontainer/devcontainer.json` now builds this stage
  (`"target": "dev"`) instead of duplicating the build in a separate
  Dockerfile, and no longer force-pins `--platform=linux/arm64` — the
  platform now inherits the host's, so it no longer breaks under emulation
  on x86 machines. `setup-zsh.sh` (oh-my-zsh) is unaffected; it still runs
  via `postCreateCommand`.
- **`rust-build`** (`rust:1-bookworm`): placeholder stage, not wired to
  anything yet — will build the Rust CLI starting in Phase 2.
- **`runtime`** (`debian:bookworm-slim`, default target): installs only the
  `libsndfile1` runtime lib (not the `-dev` headers), copies the built
  binaries from `legacy-build`, no compiler/toolchain/zsh/git. This is what
  `docker build .` (no `--target`) produces.

## Verification (2026-09-06)

- `docker build -t pvcplus .` (default = `runtime`): succeeds, 0 errors.
- `docker build --target dev -t pvcplus:dev .`: succeeds; `zsh`, `git`,
  `curl` and the built tools are all on `PATH`.
- `docker buildx build --platform linux/amd64,linux/arm64 --target runtime .`:
  succeeds via QEMU emulation (`docker run --privileged --rm tonistiigi/binfmt
  --install all` to register the emulators on a host with no prior multi-arch
  setup). Ran an arm64 binary under emulation (`plainpv`) to confirm it's not
  just compiling but actually executing correctly.
- End-to-end functional check: generated a test WAV with `sox` in a
  throwaway container, ran it through `pvcplus` `runtime`'s `plainpv
  -N1024 -I2`, got a correct, complete output file — the image isn't just
  structurally right, it does the tool's actual job.

### Image size: 132 MB, not <120 MB

The plan's target was `<120 MB` for the runtime image. Measured: **132 MB**
(`docker images`). This isn't reachable with `debian:bookworm-slim` as the
base — that image alone, with *nothing* added, is already **116 MB**
(`docker pull debian:bookworm-slim && docker images`). The remaining ~16 MB
is `libsndfile1` and its transitive codec dependencies (FLAC/Vorbis/Opus,
~3.5 MB) plus the 53 built binaries (~9.5 MB); there's no meaningful fat to
trim from either — `--no-install-recommends` is already in use, and
`/usr/share/doc`+`/usr/share/man` together are under 2 MB.

Getting under 120 MB would require a smaller base image (e.g. Alpine/musl),
which is a bigger, separate decision: this codebase uses several GNU/BSD
libc extensions (`getlogin()`, `index()`, `random()`, GNU-style `crack()`
argument parsing) that would need re-verifying against musl before trusting
it, and CI (Task 0.7) doesn't exist yet to catch a regression there. Given
the plan's Task 0.5 work only just finished making the legacy tools
*correct* on glibc, swapping the C library out from under them isn't a
"cheap" follow-on to bundle into this task. Documenting the gap here rather
than either silently missing the target or chasing a base-image migration
that wasn't asked for.
