# Multi-stage, multi-arch Dockerfile (Task 0.6)

Replaces the old single-stage `ubuntu:22.04` Dockerfile that hand-patched the
original Makefiles with `sed` (see `docs/dev/baseline.md`) — that approach
predates the CMake build (Task 0.4) and had been left in place, unused by
anything, since `legacy/CMakeLists.txt` was added.

## Stages

- **`legacy-build`** (`debian:bookworm-slim`): installs the C toolchain,
  configures and builds `legacy/` with CMake, installs 54 binaries to
  `/opt/pvc-legacy`.
- **`rust-build`** (`rust:1-bookworm`): `cargo build --release --locked
  -p pvc-cli`, the actual Rust CLI build. Wired up 2026-09-10 (Phase 4.4
  prep) — before that it was a placeholder stage that copied and built
  nothing.
- **`dev`** (`rust:1-bookworm`, changed 2026-09-10 from extending
  `legacy-build`): installs the C toolchain (`build-essential`/`cmake`/
  `pkg-config`/`libsndfile1-dev`) plus `zsh`/`git`/`curl`, and copies the
  already-built legacy binaries in from `legacy-build`. Before this
  change the devcontainer had no Rust toolchain at all — a real,
  previously-undocumented gap, found while starting Phase 4.4.
  `.devcontainer/devcontainer.json` builds this stage (`"target": "dev"`).
  `setup-zsh.sh` (oh-my-zsh) is unaffected; it still runs via
  `postCreateCommand`.
- **`runtime`** (`debian:bookworm-slim`, default target): installs only the
  `libsndfile1` runtime lib (not the `-dev` headers), copies the built
  legacy binaries from `legacy-build` and the `pvc` binary from
  `rust-build`. `ENTRYPOINT ["pvc"]`, `CMD ["--help"]` — changed
  2026-09-10 from a plain `sh -c` banner listing legacy tools. Before this
  change `docker build .` produced an image with the legacy C tools only;
  the `COPY --from=rust-build .../pvc` line and the `ENTRYPOINT` were both
  commented out since Task 0.6, and nothing had gone back to finish them.
  This is a real, breaking change for anyone who ran a legacy tool
  directly (`docker run pvcplus plainpv ...`) — that invocation now needs
  `docker run pvcplus legacy plainpv ...`, or the `legacy-runtime` target/
  `:legacy` GHCR tag below.
- **`legacy-runtime`** (`debian:bookworm-slim`, added 2026-09-10): the
  old `runtime` stage's own behavior, unchanged — legacy tools only, no
  `pvc`, a plain `sh -c` banner as `CMD`. Kept as its own target/GHCR tag
  (`:legacy`) for anyone who still wants the pre-4.4 direct-by-name
  invocation style.

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

## Verification (2026-09-10, Phase 4.4 prep — rust-build wired up, runtime gets pvc)

Started as "fix the Dockerfile before doing Phase 4.4 (GHCR distribution)":
`pvc` was never actually in the `runtime` image, despite Phase 3/5 being
long done in the Rust workspace — the `rust-build` stage and the
`COPY --from=rust-build` line in `runtime` had both stayed commented out
since Task 0.6. Real docker (not a dry run) was available locally, so all
of the below is a genuine build+run, not a read-through.

- `docker build --target runtime -t pvcplus-test:runtime .`: succeeds.
  Rust workspace compiles in ~64s (native amd64, warm apt cache).
- `docker run --rm pvcplus-test:runtime` (no args): prints `pvc --help`.
- `docker run --rm -v <input>:/audio/input -v <output>:/audio/output
  pvcplus-test:runtime stretch --factor 2.0 /audio/input/in.wav
  /audio/output/out.wav`: produces a real, correct output file (confirmed
  ~2x duration) and prints the Phase 4.3 run summary line.
- `docker run --rm pvcplus-test:runtime legacy plainpv`: reaches the
  wrapped legacy binary correctly.
- `docker build --target dev -t pvcplus-test:dev .`: succeeds.
  `rustc --version`/`cargo --version`/`cmake --version` and
  `/opt/pvc-legacy/bin/plainpv` (via `PATH`) all work inside the
  container — the devcontainer's own missing-Rust-toolchain gap is closed.
- `docker build --target legacy-runtime -t pvcplus-test:legacy .`:
  succeeds; `docker run --rm pvcplus-test:legacy` reproduces the old
  `runtime` stage's own banner output exactly, and `docker run --rm
  pvcplus-test:legacy plainpv` reaches the legacy binary directly by
  name, matching the pre-4.4 invocation style for anyone who still wants
  it.

Not verified this pass (left to CI, which already builds `runtime`/`dev`
for `linux/amd64,linux/arm64`/`linux/amd64` respectively via QEMU): actual
multi-arch build time. The Rust compile step is new inside `docker buildx`
now, and an emulated arm64 Rust build is expected to take meaningfully
longer than the ~64s native amd64 number above — worth watching CI
duration after this lands, though GitHub Actions' own default 360-minute
job timeout gives plenty of headroom either way.
