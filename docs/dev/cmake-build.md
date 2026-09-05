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

## Task 0.5 warning cleanup status (2026-09-05)

Started at 2335 warnings (`-Wall`, gcc 12 / debian:bookworm-slim). Down to
**594**, in six commits, each verified with a full container rebuild (0
errors throughout) plus targeted smoke tests:

1. `fix(legacy): guard fclose(ifd/ofd) against NULL` — the crash above, across
   30 tools.
2. `fix(legacy): correct real bugs surfaced by -Wall` — a heap-buffer overflow
   in `ratechanger.c` (`-Warray-bounds`), 5 dropped `realloc()` return values
   in `roomresponsemaker.c` (`-Wunused-result`), an uninitialized default
   frequency bound in 6 tools (`-Wuninitialized`, verified live: `centroid`
   now reports `HIGH FREQUENCY BOUNDARY: 22050.000` instead of garbage), and a
   dropped-guard logic bug in `pitchtracker`'s median calculation.
3. `style(legacy): silence implicit-int, pointer-compare, stringop-truncation,
   comment, and misleading-indentation warnings` — all misleading-indentation
   cases here were confirmed false positives (independent statements sharing
   indentation, not actual guard-scope bugs).
4. `fix(legacy): bound sprintf calls into fixed-size buffers` — every
   `-Wformat-overflow` site converted to `snprintf`; also fixed a worse bug in
   `fixTildeInFilename.c` where `strncat` wrote directly into the
   `getenv("HOME")` pointer (corrupting the environment, not just a buffer
   overflow risk).
5. `refactor(legacy): remove dead declarations flagged by -Wunused-variable` —
   ~1695 variables across 93 files, removed with a purpose-built tool (see
   commit message for the approach and safety checks: skips any declarator
   whose initializer could have a side effect). 2 such cases remain, still
   flagged by the warning (`chordmapperplus.c`, `tvfilter.c`).

**Remaining 594, not yet addressed:**

| Category | Count | Notes |
|---|---|---|
| `-Wunused-but-set-variable` | 217 | Declaration is genuinely dead, but safely removing it means tracing every assignment site for a side-effecting RHS first — a slower, more manual pass than category 5 above. |
| `-Wmaybe-uninitialized` | 143 | Not yet triaged. Given a real bug was hiding in the 6-instance `-Wuninitialized` group, this larger group deserves the same care, not a blanket suppression. |
| `-Wformat-overflow=` | 109 | ~106 of these are in `roomresponsemaker.c` / `roomresponsesequencer.c` / `irconvolvesequencer.c` (the same self-appending `sprintf(cmd, "%s...", cmd, ...)` idiom as the `-Wrestrict` row below). |
| `-Wrestrict` | 101 | All in the same 3 long-tail files. The idiom is technically UB but appears safe in practice (source read always precedes the write position for this specific left-to-right `%s`-prepend pattern). Fixing for real means ~100 call sites rewritten to use a temp buffer or `sprintf(dst + strlen(dst), ...)`; these files are already flagged in the plan as "don't port unless someone asks" (Phase 5), so this is low priority unless one of them gets ported. |
| `-Wformat-truncation=` | 13 | A side effect of item 4 above: converting `sprintf`→`snprintf` trades a real overflow risk for a benign "could truncate if the source is already at its own max length" warning. Not chased further since it's inherent to the codebase's fixed-size string buffers. |
| `-Wstringop-overflow=` | 6 | Not yet triaged; at least one (`prb()` in `roomresponsemaker.c`) looked like a real over-read, worth a look before assuming it's long-tail-only. |
| `-Warray-parameter=` | 3 | `roomresponsemaker.c` — a forward declaration and definition disagree on array-parameter bounds (`float[4]` vs `float[]`). Harmless at the ABI level (arrays decay to pointers) but worth a one-line fix if that file is ever touched again. |

Next step, if resumed: `-Wmaybe-uninitialized` is the highest-value remaining
category (real-bug risk, same class as the `nyquist` fix above), followed by
`-Wunused-but-set-variable`. The `-Wrestrict`/`-Wformat-overflow` cluster in
the 3 long-tail files is the least urgent given Phase 5's own prioritization.
