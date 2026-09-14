# Baseline build status (pre-modernization)

Recorded: 2026-09-02 Branch: `project-bananas-rewrite` Command: `docker build -t pvcplus:baseline .` Full log: [`baseline-build.log`](./baseline-build.log)

## Result: FAILS

The stock `docker build` **does not complete**. It aborts at Dockerfile step `RUN cd cmusic_gen && make` (Dockerfile:51) with:

```
install cspline gen1 gen2 gen3 gen4 gen5 gen6 cannon ../../bin
install: target '../../bin' is not a directory
make[1]: *** [Makefile:46: install] Error 1
make[1]: Leaving directory '/src/PVCplus/cmusic_gen/gen'
make: *** [Makefile:4: all] Error 2
ERROR: failed to build: process "/bin/sh -c cd cmusic_gen && make" did not
complete successfully: exit code: 2
```

### Root cause

`cmusic_gen/Makefile`'s `all` target runs `cd gen; make; make install`, and `cmusic_gen/gen/Makefile`'s `install` target does `install <progs> ../../bin`. At that point in the Dockerfile the `bin/` directory does not exist yet — it is only created later by `pvc_src/Makefile`'s `install` target (`RUN cd pvc_src && make install`, which runs *after* the failing step). `install` into a non-existent multi-file target directory errors out.

The `cmusic_gen` C code itself compiles cleanly (all `gen*`/`cspline`/`cannon` binaries build); only the `install` step fails.

## Verified build path (manual, for reference)

Building the three components in a throwaway `ubuntu:22.04` container with the same `sed` patches the Dockerfile applies, but **skipping `cmusic_gen`'s failing `install`**, succeeds:

- `pvc_lib` -> `libpvoc.a` builds (34 warnings)
- `pvc_src` -> all 45 tools build via `make all` (219 warnings)
- `cmusic_gen` gen programs compile; only `make install` fails on the missing `bin/` directory

Warning classes observed (to be cleaned in Task 0.5):
- `-Wunused-result` (unchecked `fread`/`fscanf`/`system` returns) — dominant
- `-Wpointer-compare`
- `-Wformat-zero-length`
- implicit declarations / K&R prototypes (`reshape.c`, `crack.c`)

## Implication for the plan

- Task 0.4 (CMake build) must create the install/bin directory before installing and fix the component build order — this removes the whole class of failure and the Dockerfile `sed` patching.
- The current image on Docker Hub / any prior successful build must have predated this breakage or been built by hand; the checked-in Dockerfile as of this commit does not build end-to-end.
