# Migration: legacy tool name to new `pvc` command

This page maps every tool in `legacy/pvc_src/` to its `pvc` equivalent. Every legacy tool still works today through `pvc legacy <name> <flags>` (see [Getting started](getting-started.md)), whether or not it has a native `pvc` command yet. Use this page to find a starting point, then read the tool's own page under `tools/` for its exact new flags.

## Basic routines

| Legacy tool | New command | Notes |
|---|---|---|
| `plainpv` | `pvc pv` (also `pvc stretch`, `pvc pitch`) | Same core engine: pitch, frequency shift, time scale, shelf EQ. `stretch` and `pitch` are narrower, single-purpose aliases of `pv`. |
| `twarp` | `pvc twarp` | Reads a `.pva` file (see `pvc analyze`) and moves through it over time. |
| `noisefilter` | `pvc denoise` | Builds its noise profile from a window of the input itself. |

## Amplitude warping

| Legacy tool | New command | Notes |
|---|---|---|
| `compander` | `pvc compand` | Compands each bin against a static reference response (from `pvc freqresponse`, for example). |
| `spectwarper` | `pvc spectwarp` | Compands each bin against its own frame's live peak instead of a static reference. |

## Additive synthesis

| Legacy tool | New command | Notes |
|---|---|---|
| `harmonizer` | `pvc harmonize` | Builds one or more transposed/shifted copies of frequency bands from a data table. |
| `chordmapperplus` | `pvc chordmapperplus` | Data-file-driven multi-tone chord/harmony synthesizer. |
| `inharmonator` | `pvc inharmonator` | Remaps a fundamental's partials from a data table of target partials. |

## Subtractive synthesis

| Legacy tool | New command | Notes |
|---|---|---|
| `filter` | `pvc filter` | Filters by a static `.fr` response file. |
| `freqresponse` | `pvc freqresponse` | Analyzes a sound file into a `.fr` response. |
| `chordresponsemaker` | `pvc fn response chordresponsemaker` | Synthesizes a `.fr` response as a stack of harmonic tones. |
| `filtresponsemaker` | `pvc fn response filtresponsemaker` | Synthesizes a `.fr` response from breakpoints. |
| `groupdelaymaker` | `pvc fn response groupdelaymaker` | Synthesizes a per-bin delay-time response, for `pvc delayfilter`. |
| `pvanalysis` | `pvc analyze` | Writes a `.pva` analysis file. `pvc info` replaces `readheader` for reading one back. |
| `tvfilter` | `pvc tvfilter` | Time-varying form of `filter`, driven by a `.pva` file. |
| `convolver` | `pvc convolver` | Cartesian-domain spectral multiply, rather than `tvfilter`'s polar-domain one. |

## Resonance and reverb

| Legacy tool | New command | Notes |
|---|---|---|
| `ring` | `pvc ring` | Phase-vocoder feedback resonator. |
| `ringfilter` | `pvc ringfilter` | `ring` plus a static `.fr` filter on the feedback path. |
| `ringtvfilter` | `pvc ringtvfilter` | `ring` plus a time-varying `.pva` filter on the feedback path. |

## Nonlinear frequency deviation

| Legacy tool | New command | Notes |
|---|---|---|
| `filtdeviator` | `pvc filtdeviator` | `filter` plus response-shaped frequency deviation and delay. |
| `tvfiltdeviator` | `pvc tvfiltdeviator` | `tvfilter` plus the same response-shaped deviation and delay. |

## Feature extraction

| Legacy tool | New command | Notes |
|---|---|---|
| `envelope` | `pvc envelope` | Amplitude envelope over a frequency band. |
| `centroid` | `pvc centroid` | Amplitude-weighted mean frequency over a band. |
| `fluxoid` | `pvc flux` | Frame-to-frame frequency change over a band. |
| `pitchtracker` | `pvc pitchtrack` | Fundamental-frequency tracker. |

## Control function processing

| Legacy tool | New command | Notes |
|---|---|---|
| `gen1` through `gen6` | `pvc fn gen1` through `pvc fn gen6` | The CARL/cmusic GEN control-function family. |
| `reshape` | Not ported, beyond one use below | `reshape`'s roughly 40 transformation flags stay legacy-only (`pvc legacy reshape`). No script in this project used them. |
| `cspline` | Not ported | Use `pvc legacy cspline`. |
| `showme`, `showmeb`, `showmed`, and related plotting scripts | `pvc fn plot` | Plots a control/data file as a terminal sparkline. No `gnuplot` dependency. |

## Utilities

| Legacy tool/script | New command | Notes |
|---|---|---|
| `amptodB`, `dBtoamp`, `Hztopitch`, `pitchtoHz` | `pvc convert-units` | One command, `--from`/`--to` unit pair (`amp`, `db`, `hz`, `oppc`). |
| `readheader` | `pvc info` | Also reads audio file headers, not only `.pva` files. |
| `aiffs`, `aiffd`, `nexts`, `nextd`, `nextfloats` | Not needed | `pvc` reads and writes WAV/AIFF/FLAC directly. No separate conversion step is needed. |
| `showspect` | Not ported | Use `pvc legacy showspect`, or `pvc fn plot` on the equivalent binary output. |

## Long tail (Phase 5)

These tools have no legacy shell-script wrapper of their own in the original release. Each now has a native `pvc` command with the same name: `impulseresponse`, `irconvolver`, `irconvolvesequencer`, `peakformant`, `specflattracker`, `spectralextractor`, `delayfilter`, `ratechanger`, `formantsmapper`, `spectrummapper`.

## Not planned as direct ports

| Legacy tool | Status |
|---|---|
| `channelmix`, `channelcollect`, `mixfiles` | Use `sox`, or a future `pvc mix`. |
| `readheader`, `short_to_float`, `sndcompare`, `rms` | Diagnostics only, no synthesis behavior to port. |
| `roomresponsemaker`, `roomresponsesequencer` | In progress in `pvc-core`/`pvc-io`, not yet reachable as a `pvc` command. Use `pvc legacy roomresponsemaker` for now. |

## Deprecated

SuperCollider scripts under `legacy/supercollider_scripts/` are deprecated. Use a [preset](presets.md) with `pvc run --set` instead. See [legacy/supercollider_scripts/DEPRECATED.md](../legacy/supercollider_scripts/DEPRECATED.md) for the full replacement guide.
