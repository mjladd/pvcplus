# Port status

This document tracks progress against
`.hermes/plans/2026-09-02_183500-pvcplus-modernization.md`. Update it
after each tool lands, not before. Do not restate the plan here. Link to
the plan section instead.

## Phases 0 to 3

Phases 0 to 3 are complete. The legacy C build runs on CMake inside a
pinned Docker image. The golden-file harness records C oracle output and
compares every Rust port against it. The Rust workspace (`pvc-core`,
`pvc-io`, `pvc-cli`) exists. All ten core tools from the Phase 3 table are
ported. CI runs fmt, clippy, the full test suite, and the golden harness
on every push.

## Phase 4 (user-friendliness layer)

All seven items in the plan's own Phase 4 list are complete and merged,
as of 2026-09-10.

| Item | Scope | PR |
|---|---|---|
| 4.1 Docs site | Split the README into `docs/`: getting started, concepts, migration table, presets guide, and one page per tool | #49 |
| 4.2 Examples | `examples/presets/` and `make demo` | #50 |
| 4.3 Run summary and `--json` | Duration, peak level, and clipping printed after every run | #51 |
| 4.4 Distribution | `pvc` now ships in the Docker image. A GHCR release workflow builds and pushes it on a version tag | #52 |
| 4.5 Completions and man pages | `pvc completions <shell>`. Man pages for every subcommand live under `man/` | #53 |
| 4.6 Migration helper | `pvc migrate-script` turns an old `S.*` script into a preset | #54 |
| 4.7 Tutorial | `docs/tutorial.md`, a full walkthrough | #55 |

Two real bugs surfaced during this phase, found by running every
example before writing it down, not by a dedicated bug hunt. First,
`pvc run <preset.toml>` never executed anything at all before PR #49.
It always failed outside `--dry-run`, even for a fully-working tool
like `pv`. Second, `ring`'s own low-shelf EQ gain default (`200` dB) is
a known, deliberately-reproduced C bug. Turn its feedback loop on, and
the tool's own output goes completely silent, not just too loud as the
earlier finding said. This second bug was already
shipped in two places, the `ring` doc page's own example and the
`make demo` preset, and PR #55 fixed both.

The plan's own Phase 4 scope included a progress bar alongside the run
summary. That part is deliberately not done. A real progress bar needs
callback hooks threaded through every tool's own processing loop in
`pvc-core`. That code is already verified against the C oracle. The
repo owner chose to skip the progress bar, given its small benefit
against that risk.

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
the plan requires it. As of 2026-09-10, work is paused on purpose, in
favor of Phase 4. Phases 1 to 10 below are merged to `main`. An
eleventh phase (the sound-file side of the wall/reflection-order
impulse-response readers) exists on a local branch, not yet pushed or
opened as a pull request.

## Remaining work

Everything below is what is left against the plan, as of 2026-09-10.

- **`roomresponsemaker`**: paused mid-port. All the pure math and file
  parsing are done. Still missing: the external shell-out
  pre-convolution pipeline, `main()`'s own CLI control flow, and a
  golden test against the real oracle binary. See the phase list
  below.
- **`roomresponsesequencer`**: not started. Both tools are optional
  under the plan's own "consider leaving legacy-only" note.
- **Repo rename**: the plan's decision 7 approves renaming this
  repository from `docker-pvcplus` to `pvcplus`. Nobody did this yet.
- ~~**SuperCollider scripts**~~: done. `legacy/supercollider_scripts/DEPRECATED.md`
  points to `pvc run --set`/presets, and to `pvc migrate-script` for
  converting an `S.plainpv`-shaped script directly. The plan's own text
  also names a `[[batch]]` preset array as this layer's replacement for
  multi-file iteration. No such feature exists. `preset.rs`/`run.rs`
  only support `--set`, one field at a time, so the new file points to
  a plain shell loop over `pvc run` instead. This is now a real,
  separate open item: nobody built a `[[batch]]` array.
- **Performance benchmark**: the plan's own validation section asks
  for a documented comparison of `pvc pv --stretch 2` against legacy
  `plainpv` on a 60-second file, saved to `docs/dev/bench.md`. A
  `criterion` benchmark suite already exists
  (`rust/crates/pvc-core/benches/pvoc.rs`). Nobody wrote the actual
  comparison against the legacy binary, or the document, yet.
- **GHCR image**: the release workflow from Phase 4.4 is ready. Nobody
  pushed a version tag yet, so no image exists at
  `ghcr.io/mjladd/pvcplus`.

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
| 7 | Filter/normalize and truncate/envelope/normalize orchestration for wall and reflection-order impulse responses | `94fe45a` |
| 8 | Reflection-order convolution-sequence bookkeeping, presence-level balance math, and maximum speaker-to-listener/speaker-to-speaker distance search | `dd3ef9f` |
| 9 | Channel-assignment selection for wall/reflection-order impulse response reads, and the wall pulse-mode default | `31f9571` |
| 10 | Loop-assign, cast, and validation logic for the six channel-assignment/gainscale/presence-level data-file readers, plus a `pvc-io` reader for the on-disk file format they share | `b20f89d` |

Phase 6 also settled an open question from Phase 4: the wall
channel-assignment and gainscale-level readers turned out to hold no pure
math worth a separate port. Both are pure file parsing, so they stay
deferred to `pvc-cli`/`pvc-io`. The C's own manual cache memory-growth
code has no Rust equivalent to write at all, since a `Vec`-backed cache
grows on its own.

Phase 7 found one more real dead-code case (finding 27 in the module's own
doc comment): the C's own "did the filtered result come back longer than
the input" grow-and-reshuffle branch can never run, in all three
filter-and-normalize orchestration functions alike. The port always
returns data the same length as its input for this reason, not because of
a missing case.

Phase 8 read the last unread stretch of the file, close to 750 lines. It
ported every piece of that stretch that turned out to be pure math.

Phase 8 also closed an open question from Phase 3. Phase 3 had already
ported the formulas for the greatest speaker-to-listener and
speaker-to-speaker distances. It had not yet ported the two functions that
search for those distances. Phase 8 ports both search functions and shows
that Phase 3's own formula choice was already right (finding 30 in the
module's own doc comment).

`preConvolveReflectionOrderImpulseResponsesWithIrconvolver` turned out to
hold no math at all to port. It shells out to `cp`, this project's own
`impulseresponse`/`irconvolver` legacy binaries, and `channelcollect`.
It does this through `/tmp` files and `system()` calls (finding 31).

Real per-wall and per-reflection-order impulse-response file I/O still
needs a `pvc-io` reader. A presence-level file that feeds one of those
readers has a real inconsistency with its own sibling reader. A future
reader of that file needs to decide on that inconsistency on purpose
(finding 29).

Phase 9 read the two wall/reflection-order impulse-response sound-file
readers in full. It ported the two pieces of them that are not file I/O.
The first piece is the shared channel-assignment selection logic. The
second piece is a wall-only pulse-mode default. That default stands in
for a sound file whenever wall impulse responses are off.
`pvc-io::audio::read_audio` already decodes and de-interleaves a sound
file into per-channel data. So the new channel-assignment function only
needs to pick and reorder channels, not decode anything. This phase
closes out almost all of the pure math left in `roomresponsemaker.c`.
What remains is file I/O, subprocess orchestration, and CLI control flow,
not more math for `pvc-core` to hold.

Phase 6 had already looked at the wall channel-assignment and
gainscale-level readers once. It found no pure math worth a separate
port. Phase 10 looks again, at all six of these small readers together:
the wall and reflection-order versions of channel assignments,
gainscale levels, and presence levels. This time it finds a cleaner
split. A file is already read into a string, with its comments already
cut, before this arithmetic runs. Once that happens, the loop-assign
arithmetic each reader does separates cleanly from the actual disk
read. Phase 10 ports that arithmetic into `pvc-core`, along with each
reader's own value casts and limit tests. It also adds the disk-read
side in a new `pvc-io` module, `roomresponse_data`. That module reads a
file, cuts its comments with the `cut_data_lines` function Phase 1
already ported, and returns the raw numbers.

Reading all six readers side by side this phase, rather than one at a
time, turns up two more real bugs. First, the wall presence-level
reader has its own loop-assign bug, separate from finding 29's already
known truncation bug. It divides by a count already set to the number
of walls, not the file's real number of values. So it never actually
loops back to the start. It reads the file's real values once, then
repeats the last one for every wall left over. This is finding 32 in
the module's own doc comment. Second, the reflection-order gainscale
reader never pads a short file back up to the number of reflection
orders, unlike the wall version. A short file there gives a shorter
table instead. Both bugs are reproduced on purpose, not fixed. That
matches this project's usual choice for a real, confirmed, non-crashing
bug.

Work still not started, after Phase 10 lands:

- The actual sound-file reads inside the wall/reflection-order
  impulse-response readers themselves. `pvc-io::audio::read_audio`
  already covers the decode step. Only the channel-selection and
  wall-pulse-default math around it is ported, in Phase 9.
- The external shell-out pre-convolution pipeline in
  `preConvolveReflectionOrderImpulseResponsesWithIrconvolver` (finding 31).
  This is a subprocess-orchestration question, not a math one.
- `main()`'s CLI control flow: which reader gets which file path, and
  which default value applies for a missing path.
- A golden test against the real oracle binary.

The golden test is likely the hardest of these. This tool depends on
temporary impulse-response cache files and global state more than any
tool ported so far.

Open a pull request for this branch only once the tool reaches a state
the user wants merged. The other option is to defer it as a
`pvc legacy`-only tool.
