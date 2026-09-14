# NOTICE

This project (`pvcplus`, formerly `docker-pvcplus`) packages, modernizes, and extends a phase-vocoder signal-processing toolkit.

## Modernization work

The build system, Docker packaging, CMake build, Rust reimplementation (`pvc-core`, `pvc-io`, `pvc-cli`), test harness, and documentation authored for this repository are licensed under the MIT License. See `LICENSE`.

Copyright (c) 2026 Michael Ladd.

## Original C toolkit (`legacy/`)

The original phase-vocoder routines under `legacy/` (the `pvc_src`, `pvc_lib`, and related directories) are the **PVC / PVCplus** toolkit written by **Paul Koonce** (koonce@music.princeton.edu), obtained from:

  https://sourceforge.net/projects/pvcplus/

Per the project's own documentation, PVC follows in the lineage of phase-vocoder work by **Eric Lyon** (out of which PVC was built) and **Chris Penrose**, whose DSP research springs from the coding and tutorial work of **F. R. Moore** and **Mark Dolson**.

The `legacy/cmusic_gen/` directory contains CARL `cmusic` `gen` function generators from the CARL software distribution (University of California, San Diego, Computer Audio Research Laboratory).

The original PVCplus release did not ship with an explicit software license. The MIT license in `LICENSE` applies to the *new* modernization work in this repository, not to the original C sources under `legacy/`, whose authors retain their respective rights. If you intend to redistribute the legacy C sources or binaries built from them, confirm the original terms with the original authors. This NOTICE will be updated if those terms are clarified.
