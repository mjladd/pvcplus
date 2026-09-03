# CMake build (legacy toolkit)

The legacy C toolkit builds with CMake instead of the original hand-rolled
Makefiles. This removes the MacPorts `/opt/local` hard-coding, the macOS-only
`ranlib -s`, the wrong link order, the broken `cmusic_gen` install step, and all
the Dockerfile `sed` patching.

## Build

```bash
cmake -S legacy -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build --prefix /opt/pvc-legacy   # installs 53 binaries to <prefix>/bin
```

Requires: a C compiler, CMake >= 3.16, pkg-config, libsndfile dev headers
(`libsndfile1-dev` on Debian/Ubuntu).

## What it builds

- `libcarl` (static) — CARL cmusic support libs (libran/libdgl/libprocom/libfrm).
  `libran/gamma.c` and `libdgl/crack.OLD.c` are excluded, matching the upstream
  Makefiles.
- 8 gen tools: `cspline gen1 gen2 gen3 gen4 gen5 gen6 cannon` (`gen1x.c` excluded).
- `libpvoc` (static) — 87 objects, exactly the `BINARIES` list from
  `pvc_lib/Makefile` (dead `*.OLD.c` / `*.old.c` / `*.TEST.c` and the unlisted
  `readin.c`/`openfile.c` are excluded).
- 45 PVC tools (the `MOLES` list from `pvc_src/Makefile`).

Total installed: **53 executables**. Verified in a clean `debian:bookworm-slim`
container: configure + build + install succeed with 0 errors.

## Known issue surfaced by the build (tracked for Task 0.5)

Compiling with `-Wall` exposes ~2300 warnings (the original Makefiles compiled
nearly silently). More importantly, a functional smoke test revealed a
**pre-existing latent crash**: several tools call `fclose()` on a NULL `FILE*`
at the end of `main()` and segfault *after* writing correct output.

Confirmed for `plainpv` at `legacy/pvc_src/plainpv.c:746`:

```
Program received signal SIGSEGV
0  _IO_new_fclose (fp=0x0)
1  main () at legacy/pvc_src/plainpv.c:746   ->   fclose(ifd) ;
```

`ifd` (a legacy stdio input pointer) is never opened — I/O actually goes through
libsndfile's `infile`. `fclose(NULL)` crashes on glibc. Output audio is correct
and complete (2.0s -> 4.05s for `-I2`); only the process exit code is bad.

Because a non-zero exit would break the golden-file harness, this NULL-`fclose`
class is fixed first in Task 0.5. It is a **source** bug, not a build bug — the
CMake build is complete and correct.
