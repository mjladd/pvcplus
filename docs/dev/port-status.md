# Port status

This document tracks progress against
`.hermes/plans/2026-09-02_183500-pvcplus-modernization.md`. Update it
after each tool lands, not before. Do not restate the plan here. Link to
the plan section instead.

## Phases 0 to 4

Phases 0 to 4 are complete. The legacy C build runs on CMake inside a
pinned Docker image. The golden-file harness records C oracle output and
compares every Rust port against it. The Rust workspace (`pvc-core`,
`pvc-io`, `pvc-cli`) exists. All ten core tools from the Phase 3 table are
ported. CI runs fmt, clippy, the full test suite, and the golden harness
on every push.

## Phase 5 (long tail)

The plan lists Phase 5 tools with no required order. It says:
re-prioritize by user demand. Every tool below stays reachable through
`pvc legacy`, no matter its Rust-port status.

| Tool | Status | PR |
|---|---|---|
| irconvolvesequencer | Ported | merged |
| ring | Ported | merged |
| ringfilter | Ported | merged |
| tvfilter | Ported | merged |
| ringtvfilter | Ported | merged |
| groupdelaymaker | Ported | merged |
| spectralextractor | Ported | #35 |
| peakformant | Ported | #36 |
| specflattracker | Ported | #37 |
| convolver | Ported | #38 |
| delayfilter | Ported | #39 |
| filtdeviator | Ported | #40 |
| tvfiltdeviator | Ported | #41 |
| ratechanger | Ported | #42 |
| inharmonator | Ported | #43 |
| formantsmapper | Ported | #44 |
| spectrummapper | Ported | #45 |
| chordmapperplus | Ported | #46 |
| roomresponsemaker | In progress. See below. | not yet opened |
| roomresponsesequencer | Not started | |
| channelmix, channelcollect, mixfiles | Not planned as direct ports. The plan calls for `sox` or a future `pvc mix`. | |
| amptodB, dBtoamp, Hztopitch, pitchtoHz | Not needed. `pvc convert-units` already covers this. | |
| readheader, short_to_float, sndcompare, rms | Not planned. These are diagnostics with no synthesis behavior to port. | |

The plan flags `roomresponsemaker` and `roomresponsesequencer` as
"consider leaving legacy-only." The plan also says, in a separate
section: do not port `roomresponsemaker` unless someone asks. The repo
owner asked. Work on it continues by that explicit request, not because
the plan requires it.

## roomresponsemaker sub-phases

`roomresponsemaker.c` has 9,318 lines. It is the largest tool in the
legacy toolkit, larger than `chordmapperplus`, the previous largest port.
It does not fit the plan's one-PR-per-tool template. The port instead
runs in checkpointed phases on branch `feat/pvc-roomresponsemaker`. Each
phase is its own commit. No pull request exists yet. Every commit passes
`cargo fmt`, `cargo clippy --all-targets -- -D warnings`, and
`cargo test --workspace`.

Real C bugs found during the port are numbered inside
`rust/crates/pvc-core/src/tools/roomresponsemaker.rs`, in that module's
own doc comment (findings 1 onward). Do not copy that list into this
file. Read the module doc comment for the current findings.

| Phase | Scope | Commit |
|---|---|---|
| 1 | Room, speaker, and listener geometry. Polygon construction, coordinate transforms, position resolution. | `658444d` |
| 2 | Recursive image-source reflection-path search | `63007de` |
| 3 | Per-reflection amplitude and delay-time math | `928df9a` |
| 4 | Impulse-response convolution and filtering math | `8971113` |
| 5 | Direct-sound pulse path and speaker-dispersion math | `91faa8d` |
| 6 | Wall/reflection-order impulse-response cache bookkeeping (the longest-cached-prefix search a new reflection reuses instead of recomputing a shared convolution chain) | `c4859e3` |

Phase 6 also settled an open question from Phase 4: the wall
channel-assignment and gainscale-level readers turned out to hold no pure
math worth a separate port. Both are pure file parsing, so they stay
deferred to `pvc-cli`/`pvc-io`. The C's own manual cache memory-growth
code has no Rust equivalent to write at all, since a `Vec`-backed cache
grows on its own.

Work still not started, after Phase 6 lands:

- The actual file I/O that reads and writes per-wall and
  per-reflection-order impulse-response files.
- The filter-and-normalize and truncate-envelope-and-normalize
  orchestration functions. These mostly call already-ported math in
  sequence, deferred to their own phase rather than folded into Phase 6.
- Presence-level bookkeeping.
- `main()`'s CLI control flow.
- A golden test against the real oracle binary.

The golden test is likely the hardest of these. This tool depends on
temporary impulse-response cache files and global state more than any
tool ported so far.

Open a pull request for this branch only once the tool reaches a state
the user wants merged. The other option is to defer it as a
`pvc legacy`-only tool.
