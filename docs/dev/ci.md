# GitHub Actions CI (Task 0.7)

`.github/workflows/ci.yml` runs on push to `main`, on every pull request, and
on manual dispatch. Two jobs for now, matching what actually exists in the
repo (Rust and the golden-file harness aren't built yet):

- **`legacy-build`**: installs `build-essential cmake pkg-config
  libsndfile1-dev` directly on the runner (no Docker) and does exactly what a
  contributor would do locally — configure, build, install, then asserts all
  53 binaries got installed. Fastest signal that a change broke the C build.
- **`docker`**: `docker/setup-qemu-action` + `docker/setup-buildx-action`,
  then builds the `runtime` target for `linux/amd64,linux/arm64` and the
  `dev` target for `linux/amd64`, both with `push: false` (buildx layer
  caching via `type=gha` so repeat runs are fast). No registry credentials
  exist yet, so there's nothing to push to — Task 4.4 adds the GHCR push
  once that's wanted.

Planned additions, not yet applicable:
- **`rust`** (Phase 2+, once `rust/` exists): `cargo fmt --check`, `cargo
  clippy --all-targets -- -D warnings`, `cargo test --workspace`.
- **`golden`** (Phase 1, once `tests/golden/` exists): regenerate fixtures,
  run the C-oracle comparison.

## A real portability finding from testing this locally (2026-09-06)

Before trusting `runs-on: ubuntu-latest` I built the CMake project locally
against a few different Ubuntu bases to check what `legacy-build` would
actually hit:

| Base | gcc | Result |
|---|---|---|
| `ubuntu:24.04` | 13.3.0 | 0 errors |
| `ubuntu:latest` (= 26.04 today) | 15.2.0 | **28 errors** |

The failures are all the same root cause: old K&R-style declarations like
`char *space();` — empty parens meaning "takes unspecified arguments," a
pre-ANSI C idiom — used at a call site that actually passes arguments
(`space(N2+1, sizeof(float))`). GCC has tolerated this for decades; GCC 15
(C23-by-default) does not, and reports it as a hard "too many arguments"
error rather than a warning. One instance of this specific case (`space()`,
11 call sites across 7 `pvc_lib` files, wired up to a real 2-argument
definition in `space.c` but never given a shared prototype) is fixed in this
same change — `pv.h` now declares it properly, matching how every other
`pvc_lib`-wide function is exposed.

The other ~20 errors on gcc 15 (`index()`, `crack()`, `rfun()`/`rfunc()` in
`cmusic_gen/lib/libran/*.c`, plus a self-inflicted conflict where
`pv.h` and `crack.c` each redeclare `index()`/`crack()` with mismatched
signatures) are **not** fixed here — that's a much larger sweep across the
codebase, closer in scope to the Task 0.5 K&R-prototype cleanup than to
"set up CI," and gcc 13 (what `ubuntu-latest` actually is today) doesn't hit
any of them.

**Why this doesn't block using `ubuntu-latest` today:** GitHub has
historically taken 12+ months after a new Ubuntu LTS ships before flipping
the `ubuntu-latest` runner label to it (22.04 → 24.04 took roughly that
long). Ubuntu 26.04 shipped ~April 2026; as of this writing `ubuntu-latest`
is confirmed still 24.04 (gcc 13, builds clean). This will eventually stop
being true. When `legacy-build` starts failing on `ubuntu-latest` with
"too many arguments to function" errors, this is why — grep the codebase for
`();` declarations (empty-parens K&R forward declarations) of functions that
are called with arguments, starting with `index(`, `crack(`, `rfun(`/`rfunc(`.
