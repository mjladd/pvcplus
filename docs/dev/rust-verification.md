# Verifying the Rust port against the C oracle (Task 2.4+)

For DSP code (windows, FFT, and everything Task 2.5's analyzer/synthesizer
engine adds), a hand-written expected value in a unit test is only as
trustworthy as the arithmetic done to derive it by hand - and that
arithmetic is exactly the kind of thing this port exists to get right.
Task 2.4's own tests initially asserted two wrong expected values (an FFT
impulse magnitude assumed to be `1.0`, an overlap-add window sum assumed to
normalize back to `1.0`) that looked reasonable but weren't what the C
actually computes. Both were caught by comparing against **real C output**,
not by re-deriving the math more carefully - and that method is worth
reusing for Task 2.5.

## `legacy/tools/dumpwin.c`

A tiny, deliberately-not-CMake-registered C program that links directly
against `libpvoc.a` and dumps real `makewindows()`/`rfft()` output. Not a
shipped tool - a dev-only verification aid, per the plan's Task 2.4 note
("compare against C output dumped to a fixture (add a tiny
`legacy/tools/dumpwin.c` if needed)").

Build and run it against a fresh `libpvoc.a`:

```bash
cmake -S legacy -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target pvoc -j
gcc -Ilegacy/pvc_lib -Ilegacy/pvc_src -o /tmp/dumpwin legacy/tools/dumpwin.c build/libpvoc.a -lsndfile -lm
/tmp/dumpwin windows > /tmp/windows.txt   # all 17 window types, Nw=N=1024, I=220
/tmp/dumpwin fft > /tmp/fft.txt           # rfft of an impulse, N=256
```

`windows.txt` is one `# window_type=N` section per window type, then one
line per sample: `index raw analysis synthesis`. `fft.txt` is one value per
line, the 512 floats of `rfft`'s forward-transform output.

## What this caught in Task 2.4

- `rfft`'s forward-transform impulse response has magnitude `1/(2*n)` at
  every bin (`0.001953125` for `n=256`), not `1.0` - matches `cfft`'s
  `1/nd` forward scale factor. The Rust port already produced this exact
  value; only the test's assumption was wrong.
- `makewindows`' final synthesis-window normalization stage
  (`S[i] *= 1/sum(S[stride]^2)`) does **not** renormalize the strided sum
  back to `1.0` - it produces `1/original_sum`, which for a typical
  Hamming/N=1024/I=220 window is `~150` at the window's peak, not
  anywhere near `[0, 1]`. Confirmed by direct comparison: Rust and C agree
  to 6-7 significant figures at every sampled index for both Hamming and
  Kaiser(alpha=8).

Neither was a porting bug - `dumpwin` output matched the Rust port's
values before either test was corrected. Both were wrong expectations
baked into the first draft of the tests. Worth remembering for Task 2.5:
write the oracle-comparison test *before* trusting a hand-derived
expected value for anything beyond the most trivial cases (DC, a single
known bin).

## When to reach for this again

Any time a Task 2.5+ test wants to assert a *specific numeric value*
(not just a structural property like "round-trips" or "symmetric") for
something derived from real DSP math - `convert()`/`unconvert()` output,
`noscbank` synthesis, `phaselock`, etc. - dump the real C value with a
small addition to `dumpwin.c` (or a new tiny tool alongside it) rather
than computing the expected value by hand.

## What Task 2.5 caught: a structural test can mislead too

The lesson above is about wrong *numeric* expectations, but Task 2.5's
`pvoc.rs` round-trip test (`Analyzer` -> `Synthesizer` with `I == D`
should reproduce the input within -60dB) shows the same trap can hide
inside a "structural" property test that looks self-evidently correct.

The first draft compared `output[i]` directly to `input[i]` (after
skipping a hop-count's worth of warm-up) and got -17dB - looked like a
real bug. It wasn't the math; it was the comparison. Any block-based
overlap-add processor delays its output relative to its input by
`Nw - D` samples (the window has to finish sliding across a sample
before that sample's contribution is fully summed) - on top of the
hop-granularity warm-up latency the code already handles (matching
`shiftout`'s `n >= 0` write gate, which silently drops the first few
hops' output rather than write bad/incomplete samples). The test's
naive alignment was off by exactly that missing `Nw - D` term.

This was diagnosed the same way as the Task 2.4 issues - not by
re-deriving the arithmetic harder, but by building a smaller oracle:
feed a single impulse through the real `Analyzer`/`Synthesizer` pair
and look at *where* it comes back out. It reappeared at
`input_position + (Nw - D)` samples, at ~0.997 amplitude - unambiguous
proof the reconstruction itself was already correct and only the
test's alignment was wrong. That impulse check is now a standalone
test (`impulse_reappears_after_nw_minus_d_samples_at_unity_gain`) so
this doesn't need re-deriving next time.

A second, smaller finding from the same session: the fixed hop
`Nw/D = 220` used in Task 2.4's `dumpwin` fixture is a fine choice for
sampling window *values*, but it is not a great choice for a
reconstruction-fidelity test - Hamming's squared window only satisfies
constant-overlap-add (COLA) to within ~0.3% at that hop, capping
reconstruction around -49dB regardless of how correct the code is.
Confirmed directly against the window arrays (`analysis[i]*synthesis[i]`
summed at stride 220 ranges 0.9966-1.0001 across phases, vs. dead-on
1.0 at every phase for stride `Nw/4 = 256`). The round-trip test uses
`Nw/4` for exactly this reason - when a fidelity test has a target dB
threshold, pick a hop the window is actually well-behaved at, or the
test will fail for reasons that have nothing to do with a bug.

**Takeaway for Task 2.6+:** before concluding a round-trip/identity
test has found a bug, ask whether the pipeline has an inherent fixed
latency or an inherent (non-bug) precision ceiling that the test's
comparison needs to account for - verify with the smallest oracle that
can isolate it (an impulse, a single known input) before touching the
implementation.

## `legacy/tools/dumputils.c`

Same idea as `dumpwin.c`, for `legacy/pvc_lib/miscellania.c`'s
`dB_to_amp()`/`amp_to_dB()`/`semitones_to_mult()` - needed for Task 3.2's
`pv` tool. `dB_to_amp` and `semitones_to_mult` are 5000-entry lookup
tables with linear interpolation, not the exact formula (`10^(dB/20)`,
`2^(semitones/12)`) - real, deliberate approximation error the tool's
actual output depends on (`dB_to_amp(0.0)` is `0.997791529`, not `1.0`).

```bash
cmake --build build --target pvoc -j
gcc -Ilegacy/pvc_lib -Ilegacy/pvc_src -o /tmp/dumputils legacy/tools/dumputils.c build/libpvoc.a -lsndfile -lm
/tmp/dumputils
```

Caught the same class of bug as `trans`'s `denom` (Task 3.1): the
table-construction expressions (`temp = (192. * ((float)i / 4999.)) -
96.;`) mix a `float` with the C's bare double-literal constants, which
promotes the *entire* expression to double precision, narrowed to
`float` only on assignment. A first draft computed these in `f32`
throughout and matched the real `dB_to_amp(0.0)` to only 6 significant
figures, not bit-for-bit - go expression-by-expression checking which
literals are bare (`96.`, promotes) vs already-`float` operands
(nothing added here); don't assume a whole function is uniformly one
precision just because most of its literals are.

Later extended with `smooth()`/`smooth_setup()`, `curve()`/
`spectmagwarp()`, and `eq2()` cases (`legacy/pvc_lib/`), including
setting the `frame_count` global directly (declared `extern` in `pv.h`
but only *defined* by a real tool's `globals.h` normally, so a
standalone dumper needs its own `int frame_count = 0;`) to drive
`smooth`'s first-frame behavior and `eq2`'s startup banner. All of
these matched on the first try once written against the precision
rules established above - worth noting as the payoff: the rules
generalize once you've been burned by them a couple of times.

## A different kind of gotcha: calling an unprototyped K&R function

`getthresh()` (`legacy/pvc_lib/getthresh.c`, used by `noscbank`'s
threshold) surfaced a new failure mode. It's defined old-K&R-style
(`float getthresh( arr, Nplus2, tgen ) float *arr, tgen; int Nplus2;`)
and has no prototype anywhere - every real caller forward-declares it as
`float getthresh();` (empty parens mean "unspecified arguments" in C,
not "no arguments": this suppresses argument type-checking and applies
default promotion, e.g. `float` args get passed as `double`).

The first version of this test declared a real, fully-typed prototype
(`extern float getthresh(float *arr, int Nplus2, float tgen);`) before
calling it - compiles fine, links fine, and silently returned `0` for
every input. A real prototype tells the compiler to pass `tgen` as a
plain `float`; the K&R-style *definition*'s actual calling convention
still expects the default float-to-double promotion an unprototyped
call site applies, so the two disagree about how the argument is
passed and the callee reads garbage. Matching the real callers' bare
`extern float getthresh();` declaration fixed it immediately.

Worth remembering for anything else in `pvc_lib`/`pvc_src` reached via
an old bare forward declaration rather than a `pv.h` prototype: don't
"upgrade" the call site to a modern typed signature just because the
compiler will accept it - match how the real code actually calls it.

## Task 3.2's `pv` assembly: individually-verified primitives weren't enough

Every DSP primitive `pvc-core::tools::pv::process_channel` calls -
`Analyzer`, `phaselock`, `Smoother`, `spectmagwarp`, `eq2`, `OscBank`,
`getthresh` - had already been oracle-verified in isolation before this
task started. Wiring them together against the real golden-harness
`plainpv` cases still surfaced five more real bugs, none of which an
isolated unit test could have caught, because each lives in the *glue*
between primitives or in a legacy behavior that only manifests at the
whole-tool level:

1. **`OscBank`'s `N` was actually `N2`.** `noscbank(channel, N2, R, Nw,
   I, P, output)` - the call site passes `N2`, but the C's own parameter
   is *named* `N`, and `tabscale`/`NP` are both computed from it. A
   first draft of `OscBank::new` took the real FFT size, producing a
   constant, clean ~2.4x-too-loud output - the kind of bug that looks
   exactly like a missing gain factor until traced further. Fixed by
   renaming the parameter to `n2` outright (`pvoc.rs`) so the mistake
   can't recur at a future call site.
2. **`OSCILBANKGAIN` (+5dB, `10^(5/20) = 1.7782794`)**, applied to every
   oscillator-bank sample in `bufferout()` - a file-I/O function, not
   DSP code, easy to miss on a read-through focused on the signal path.
3. **`plainpv`'s hardcoded window-size default is a literal `2048`**,
   not `2 * fft_size` - despite `if (Nw <= 0) Nw = 2 * N` existing right
   there in the source, that branch only triggers on an explicit `-M0`/
   negative override, never by default. Silently correct for the
   common `--fft 1024` case (`2 * 1024` coincidentally equals `2048`)
   and silently wrong for every other FFT size - a case worth
   remembering: a code path that's dead by default can still look live
   if your first test case happens to land on the same answer either way.
4. **`timenow(dur)` sets `t = samps / R`**, where `samps` only advances
   inside `bufferout()` - which only runs once `shiftout`'s write gate
   has passed. `t` (and therefore every `fval()`-driven control value)
   stays at exactly `0.0` through the startup-suppressed hops, not a
   smoothly-running `t += I/R` from frame zero. Invisible with constant
   parameters; a 13dB spectral mismatch with a time-varying one
   (`-P@ramp.txt`) before this was found.
5. **`rescalev` defaults to `1`, not `0`** (`globals.h`), which
   triggers a whole-*file* post-pass (`rescaleThisBuffer`, called from a
   temp-file readback after every frame is written) that rescales the
   entire output so its peak matches the *input* file's peak. On by
   default, not a debug/display feature, and it lives in file-I/O code
   far from the DSP - the single largest contributor to an initial
   ~19% constant amplitude mismatch across an entire test file.

Also found, in the *test* infrastructure rather than the port: the
Phase 1 golden case files assumed `plainpv` picks overlap-add resynthesis
whenever no pitch/frequency shift is requested (`stretch.toml`,
`warp_and_shelf_eq.toml`) and that `-P@path` is a valid way to point the
*legacy* tool at a control file (`pitch_control_function.toml`). Both
were wrong, confirmed against the real binary's own startup banner and
`crackstring()`'s source respectively - see those files' own notes for
the detail. `crackstring()` never recognizes `@`; it treats an argument
as a filename only if it starts with a letter or `/`, so `-P@...`
silently fails to parse as a number and the parameter just stays at its
default. The `@path` convention is real, but it's this repo's *new*
`pvc` CLI's own explicit design choice (`docs/dev/parameter-inventory.md`),
not something that ever applied to invoking the legacy tool directly -
mixing the two up produced a "golden" oracle recording that didn't
actually exercise what its case description said it did.

**Takeaway:** oracle-verifying each primitive in isolation is necessary
but not sufficient. Budget real time for whole-tool integration against
the actual golden harness cases before calling a ported tool done - and
when a whole-file comparison shows a clean, constant ratio or a
suspiciously large-but-uniform error, look for a *global* post-process
step (a final rescale, a fixed makeup gain) before assuming the bug is
in the per-frame DSP math itself.

## Task 3.3's `.pva` reader: a bug `plainpv`'s parity work never exercised

While reading `pvanalysis.c` to port `pvc analyze`, tracing its
per-channel output loop turned up a real bug in `pvc-io::pva`'s
`read_legacy_pva`, dating back to Task 2.3 and never caught by Task
3.2's `plainpv` work (which never reads a multi-channel `.pva` file -
`plainpv` doesn't consume `.pva` files at all). `pvanalysis.c` re-seeks
to the start of the frame data on *every* channel's pass and writes that
channel's frame into slot `k` among `ochan` slots per frame - so a
multi-channel legacy `.pva` file interleaves frames across channels
(frame 0 ch0, frame 0 ch1, frame 1 ch0, ...), not laid out as one
channel's frames followed by the next's. The reader assumed the latter,
and the only existing real-oracle test used a *mono* fixture, which
can't distinguish the two layouts at all - worse, the hand-built test
fixture writer encoded the same wrong sequential order, so the original
unit test passed without ever exercising the real bug. Confirmed against
a freshly-built `pvanalysis` run on a real stereo fixture (an alternating
dominant-bin/peak-amplitude pattern across consecutive frame-blocks,
consistent with interleaving) before fixing the reader, the fixture
writer, and adding a byte-level cross-check test
(`legacy_reader_deinterleaves_real_stereo_output_correctly`) that
re-derives the layout independently from raw file bytes rather than
hardcoding expected values. Landed as its own PR ahead of the rest of
Task 3.3, on the theory that a correctness fix to already-merged code is
worth shipping separately from new feature work.

## Task 3.3's `pvc analyze`: `eq()` is not `eq2()`, and near-silent bins have no "correct" frequency

`pvanalysis.c` calls a different shelf-EQ function than `plainpv.c`
does: `eq()` (`legacy/pvc_lib/eq.c`), not `eq2()`. Where `eq2` computes
each bin's *actual* frequency (via a per-bin drift factor accumulated
frame to frame) and compares that against the shelf frequencies every
frame, `eq` converts the shelf frequencies to a fixed *bin-index* range
once (`ilow`/`ihigh`, from `freqlow`/`freqhi` and `fundamental` alone)
and gains by array position instead - a bin's gain depends only on where
it sits in the array, never on any frame-to-frame frequency drift. Ported
as a separate `eq()` function in `pvc-core::eq` rather than folded into
`eq2`, verified against a real oracle build across three cases (a shelf
transition, the `dBlow == dBhi` "gain only" fast path, and a wider
transition region spanning several bins) via `legacy/tools/dumputils.c`.

Also confirmed by reading the flag parser: none of `pvanalysis.c`'s
per-frame-look ing parameters (`-H`/`-X`/`-m`/`-R` shelf EQ, `-W`
warpshape, `-A` gain) are actually control-function strings - every one
is read with a plain `atof`, never `crackfloat`/`fval()`. So unlike
`plainpv`, `pvanalysis` has nothing that varies per frame, and
`timenow(dur)`'s per-frame `t` (which drives `plainpv`'s control
functions) is genuinely dead code here - worth stating explicitly since
it's an easy wrong assumption to import by analogy from the `pv` port.
Also confirmed by reading the loop body directly: no `phaselock` call
either - phase-locking is a resynthesis-quality concern `pvanalysis`
(analysis only, no resynthesis) has no use for.

The golden-harness comparison for `pvc analyze` needed one more
adjustment past what `plainpv`'s did: it reads both files through
`pvc_io::pva` and compares decoded frame content directly (not
`compare.py`'s byte-exact check, which can't apply here - `pvc analyze`
only ever writes the new `PVA1` format, never the legacy layout, by
design). The first run showed one 200Hz-scale frequency outlier out of
several hundred thousand compared values, traced to a single bin with
expected magnitude ~2.2e-10 - essentially silence. `convert()`'s
frequency estimate for a bin that quiet comes from `atan2` of
near-zero real/imaginary parts, which is hugely sensitive to
floating-point rounding noise; the C and a faithfully-ported Rust rfft
can legitimately disagree by hundreds of Hz on such a bin without either
being "wrong." Fixed by only checking frequency error on bins with
non-negligible magnitude (magnitude error has no such floor and is
checked everywhere) - not by loosening the tolerance blindly, which
would have hidden a real regression just as easily as this noise.

## Task 3.4's `pvc twarp`: a real interpolation bug, a real rescale bug, and a one-iteration-stale time check

`twarp` (time-varying resynthesis navigating a `.pva` analysis file) is
the largest tool ported so far, and turned up three genuine, empirically
confirmed quirks in the real C - not porting mistakes, things the real
tool actually does:

1. **`makeInterpolatedFilterFrame()` never interpolates.** Its own
   source comments describe computing a fractional position between two
   analysis frames, but `filtfprop` is declared `int` on the same
   declaration line as the frame indices (`int i, k, filtflow, filtfhigh,
   filtfprop;`) - so `filtfprop = filtf - (float) filtflow`, always a
   fractional value in `[0, 1)`, truncates to exactly `0` on every call.
   The function always returns the floor frame verbatim; `F_higher` is
   read from the file but its value is never used. Confirmed via a
   dedicated oracle harness (`legacy/tools/dumptwarp.c`): querying a
   fractional timepoint returned the floor frame's values exactly, not a
   value partway toward the next one. `pvc-core::timenav::
   interpolate_frame` reproduces this - it's a floor lookup, not a lerp,
   matching the real tool's actual (if misleadingly-named) behavior.
2. **`twarp`'s default whole-file rescale is a real bug, not just a
   quirk.** Same shared `rescalev`-driven post-pass `pvc pv` already
   found (`ipeakamp`/`rescaleThisBuffer`), but `twarp` copies its
   *target* peak from the `.pva` analysis file's own stored peak
   amplitudes (`for (k...) ipeakamp[k] = normamp[k];`) - an FFT-
   *magnitude*-domain value from `pvanalysis.c`'s analysis pass, not a
   waveform-amplitude peak. Confirmed empirically: a 0.5-amplitude 440Hz
   sine's analysis peak came out around `0.00046`; real `twarp`'s default
   output for that file was a 16-bit peak sample value of exactly `15` -
   audible as near-total silence, for every default (no `-=0` override)
   invocation. Reproduced for golden-harness parity in `commands::
   twarp::run` (documented there in detail) rather than silently
   "fixed." This also meant the existing golden cases' `spectral` (dB-
   relative) tolerance was the wrong comparator: a dB-relative comparison
   is meaningless on audio this quiet (one block went from a real
   oracle's `-106dB` to this port's `-180dB` floor purely from noise-
   floor-level differences, reporting a fabricated "73.7dB error" on a
   file where the max *absolute* sample difference was under 0.001).
   Switched `tests/golden/cases/twarp/*.toml` to `kind = "sample"`
   (absolute) instead.
3. **The frame-loop's own `t < dur` check reads one iteration stale.**
   `t` is a plain global, set once per iteration by `timenow()` early in
   the loop body - *before* that same iteration's own write can advance
   `samps` - and never touched again until the next iteration's own
   `timenow()` call. So the value the while-check sees before admitting
   iteration `k+1` is `samps` as of the *start* of iteration `k`, one
   full iteration stale relative to `samps` as of the *end* of iteration
   `k`. A first draft recomputed `t` fresh at the top of the Rust loop
   (the same approach `tools::pv`/`tools::analyze` correctly use, since
   their loops terminate on an EOF/valid-samples state machine, not a
   `t < dur` comparison) and came up exactly one hop (`I` samples) short
   of the real tool's output length on every case. Diagnosed by
   instrumenting a real debug build to print `t`/`samps`/`on` every frame
   and tracing the exact iteration where the two diverged. Fixed by
   carrying the previous iteration's `t` forward explicitly
   (`t_for_check` in `tools::twarp::process_channel`) instead of
   recomputing it fresh each time.

Also found while wiring this up: `Synthesizer::flush()` (added for this
task, since `plainpv` never uses the overlap-add path) initially returned
the *entire* synthesis ring, but `shiftout(output, Nw, I, 1, 1)`'s flush
call transfers samples via `bufferout(A, I, 1)`, which only ever reads
`I` (the hop size) samples regardless of `flushflag` - confirmed by
reading `bufferout`'s `while (outbuffpt < I)` loop, which never
references `Nw` at all. Same "flush only ever adds one hop" finding
`tools::pv` already made for the oscillator-bank path, now confirmed to
apply identically to the overlap-add path (both go through the same
generic `bufferout`).

Also discovered, in the process of tracing all of this: `twarp` is *not*
always oscillator-bank despite `docs/dev/parameter-inventory.md` §4
claiming so - see that section's correction and `tests/golden/cases/
twarp/basic_chain.toml`'s note for the detail (confirmed via a debug
build printing the resolved `obank` flag: `0` by default, `1` once a
pitch/frequency shift is requested).

**Takeaway:** for a tool this size, oracle-verifying the individual
library functions it calls (`makeInterpolatedFilterFrame`,
`findFilterTimeAndConstrainByWindow`, `makeLoopSmoothTime`) *before*
assembling them caught two of these three bugs early and cheaply, via a
dedicated small C harness (`dumptwarp.c`) rather than debugging through a
full end-to-end audio diff. The third (the stale-`t` loop check) only
showed up at the whole-tool integration level, once real length
mismatches appeared - consistent with Task 3.2's own takeaway that
integration testing against the golden harness is still necessary even
after every primitive checks out individually.

## Task 3.5's `.fr` family: two byte-exact synthesizers, one noise-floor-sensitive analyzer

`freqresponse`/`filtresponsemaker`/`chordresponsemaker` all write the
same `.fr` format (headerless raw `N+2` floats - about as simple a format
as exists in this codebase), but split into two very different kinds of
tool:

- **`filtresponsemaker`/`chordresponsemaker`** *synthesize* a response
  from a breakpoint/partial table - pure deterministic arithmetic, no
  audio analysis involved. Both ported and verified **byte-identical**
  (max abs error `0.0`) against a real oracle build on the first attempt
  that compiled - no bugs found in either. `chordresponsemaker` did need
  one careful fix during porting, not a bug: the C's bin-range checks
  (`(flat_index > 0) && (flat_index < N)`) are written against the
  legacy `1 + 2*bin` *flat* array index, which translates to `bin >= 0`
  (not `bin > 0`) in plain bin-index terms - an easy off-by-one to
  introduce translating between the two conventions, caught before ever
  running against the oracle by re-deriving the translation carefully
  rather than guessing. `NormalizeToPeaksOfSpectrumInBand` (used by
  `freqresponse`, see below) has the same flat/bin translation risk in
  three separate branches; ported by mirroring the C's own flat-index
  arithmetic literally rather than re-deriving bin-unit formulas, to
  avoid the same class of mistake in more places at once.

- **`freqresponse`** *analyzes* a sound file - fold/rfft/convert
  accumulated across every frame of every channel into one combined
  response (confirmed by reading the per-channel "REINITS" block: it
  resets `frame_count`/`eof`/`t`/`samps` but not `AmplitudeSpectrum`/
  `binAmpSumAndFreqSum`/`buffer_count`, so multi-channel input produces
  one response, not one per channel), then `eq()` (always normalizing to
  peak `1.0`, confirmed against the oracle: both peaked at exactly `1.0`
  after this step), then formant detection
  (`get_formants`/`NormalizeToPeaksOfSpectrumInBand`, ported into
  `pvc-core::formant`).

  Found and fixed one real bug in `get_formants` during porting, before
  the oracle comparison ever ran: its own internal `fundamental =
  (nyquist * 2.) / (float) N` uses the function's *own* `N` parameter,
  which every real caller passes as `N_actual + 2` (`Nplus2`), not
  `N_actual` - so this is really `R / (N_actual + 2)`, not the true
  fundamental `R / N_actual`. The same "which N" mixup that's bitten
  several other primitives this project already ported (`getthresh`,
  `OscBank`'s `N2`, plainpv's Nyquist-exclusion bound) - reproduced
  faithfully rather than corrected, since it's what the real tool
  actually computes.

  Also skipped, confirmed dead for every real caller (`freqresponse.c`
  never sets the one flag - `CorrelateWithFreqStasisFlag` - that would
  make either matter, and `symmetryFactor` is computed but never read
  again afterward anywhere in the function): the "FREQ STASIS" block and
  the "FIND SYMMETRIES" block, plus six hardcoded `fopen("/tmp/...")`
  debug writes with no CLI flag controlling them at all.

  The oracle comparison did turn up real *floating-point noise-floor
  instability*, not a structural bug - the same phenomenon already found
  and documented for `pvanalysis`'s near-silent-bin frequency estimates,
  here affecting *amplitude* at near-silent bins in the accumulated
  spectrum instead: bin 18 of the test fixture's raw (pre-`eq`)
  accumulated amplitude came out `4.0e-8` in the real oracle vs `1.7e-7`
  in this port - both far below the spectrum's actual peak (used for
  `eq`'s normalization, confirmed to match exactly), but a real
  difference at that scale after summing 410 frames' worth of `convert()`
  output, in a bin that carries essentially no real energy for a 440Hz
  test tone. `get_formants`' own decline-tracing (used to compute a
  formant's reported bandwidth/Q/stopband indices) is similarly sensitive
  in the same near-silent regions - the detected formant's *center*
  matched exactly (same index, same frequency, same amplitude), but its
  reported stopband index diverged (a real, if practically inconsequential
  divergence: neither value is read back into the output spectrum -
  `NormalizeToPeaksOfSpectrumInBand` only ever reads `formantIndices[]`,
  confirmed by reading its full body - so this doesn't affect the `.fr`
  file's actual content, only these two informational fields this port
  doesn't expose anyway). Verified via a temporary, not-committed debug
  build printing the smoothed amplitude array and pipeline peak values at
  each stage, then reverted rather than kept as another dev tool - this
  one didn't reveal a pattern worth a reusable harness the way
  `dumpwin.c`/`dumputils.c`/`dumptwarp.c` did.

  The golden case's tolerance follows `pvanalysis`'s precedent for the
  same reason: amplitude error is checked everywhere (max observed
  `~0.003` on a 0–1 scale) and frequency error only on bins with
  non-negligible amplitude, plus an exact peak-amplitude check (since
  that's the one value guaranteed stable regardless of noise-floor
  bins).

## Task 3.8's `pvc filter`: a real off-by-one, a real "which N" bug, and a bug hunted down via a spike test

`filter.c` turned out larger in scope than expected once actually read:
it doesn't subtractively filter the source, it *additively mixes* a
filtered copy of it with a separately delayed/shifted copy of the
original (`sourceflag` defaults to *on*, not off - `SOURCE_dB`'s default
gain, `0dB`, is `> -96dB`, the threshold that would disable it). New
primitives needed: `compand`, `invertresponse`, `smoothspec`
(`pvc-core::filter_response`), and `spectmagwarp2` (`pvc-core::warp`'s
`filter_warp`).

**A real, structural bug found and fixed only via the oracle** (unlike
most findings this project has made, which were caught by reading the C
carefully *before* running anything): the filter-output pitch/frequency-
shift-and-gain loop's bound is `for (i=1; i<(N+2); i+=2)`, covering
*every* bin including bin 0 - not `for (i=1; i<N; i+=2)`, the Nyquist-
excluding pattern `plainpv`/`twarp`'s analogous loops use. A first draft
assumed the familiar pattern by analogy (`for j in 1..=n2`, skipping bin
0) and passed its own unit tests, but produced audio that was audibly
close-but-off against the real oracle (max per-block RMS difference
3.47dB against a 0.5dB tolerance, on an otherwise-correct-shaped
waveform - frame counts, peak amplitude, and overall envelope all matched
exactly). Fixing the loop to `0..=n2` (matching the C's `i<N+2` bound
literally, not by analogy with other tools) brought all three golden
cases into tolerance immediately (0.013dB, 0.002dB, 0.19dB). Worth
restating as a general lesson: a loop bound that *looks* like a pattern
already seen twice in this codebase is exactly the case most likely to
get copied without re-checking the actual number in the source.

**Two more `spectmagwarp2` bugs (its own dedicated `filter.c` variant of
`spectmagwarp`), found by reading the C - not the oracle - since a
one-line synthetic case caught both before any full-tool run**:
1. In `normalize=true, warpshape=0.0` mode, the C normalizes into the
   output array and then immediately *overwrites* that result with the
   unnormalized input - the normalization loop's output is computed and
   then thrown away, unconditionally, in the same branch.
2. In `normalize=true, warpshape != 0.0` mode, the warp step calls
   `curve(0, peakbinamp, SP[i], warpshape)` using the *raw, unnormalized*
   `SP[i]` where `SP[i] / peakbinamp` was clearly intended (the sibling
   non-normalizing branch a few lines down does divide first) -
   confirmed against the oracle: a bin at exactly the peak warped to
   `~466`, not `1.0`.

Both reproduced faithfully; `filter.c`'s own warpshape control function
defaults to a constant `0.0`, so bug 1 makes this a no-op by default,
matching the oracle exactly.

**A third "which N" bug, this time in `smoothspec()`** (see
`pvc-core::filter_response`'s doc comment): its internal `fundamental =
R / (N2plus1 * 2)` uses `N2plus1 * 2`, which is `N_actual + 2`, not
`N_actual` - the same class of mistake as `get_formants`' fundamental
(Task 3.5) and `getthresh`'s K&R calling convention (Task 3.2), each a
different flavor of "the codebase has several `N` conventions in flight
and it's easy to grab the wrong one." Confirmed against the oracle:
`smoothspec` only actually smooths (rather than degenerating to a no-op
single-bin window) under the *buggy* fundamental at bandwidths where the
*true* fundamental would still round down to a single-bin window.

**A separate, real out-of-bounds read in the same function**, found
while writing its oracle tests rather than by reading alone: `hibin`'s
clamp only fires on strictly-greater
(`if (hibin > N2plus1) hibin = N2plus1;`), so a smoothing window can
legitimately compute `hibin == N2plus1` - one past the last valid index -
completely unclamped, and the loop reads it anyway. This isn't a narrow
edge case reachable only with unusual inputs: *any* nonzero smoothing
width reaches it for bins near the top of the array, and because the
whole array is peak-rescaled by one shared factor afterward, the
resulting undefined value can shift *every* bin's output, not just the
ones whose own window touches the edge - confirmed directly: an initial
oracle test using a small, all-comparable-magnitude array produced a
uniform ~20% scale discrepancy between the C and this port, traced to
the rescale factor itself differing (not the per-bin averaging formula,
which matched). Worked around for testing purposes (not "fixed" in the
port, which still just clamps safely) by deliberately setting one bin to
a value far louder than any plausible adjacent-memory garbage, anchoring
the peak-rescale reliably regardless of what the C's OOB read actually
returns, then comparing only the bins whose own window never reaches the
corrupted region.

Scope deliberately deferred to a follow-up, each confirmed independently
verifiable later without touching what's already landed: the oscillator-
bank resynthesis path (`noscbank2` - needed only once a pitch/frequency
shift is requested; every golden case exercises the overlap-add path
`filter.c` also uses by default) and nonzero `--filter-time-delay`/
`--source-time-delay` (the delay-line ring buffer itself is fully
implemented and exercised even at zero delay, just not oracle-tested at
a nonzero one).

**Takeaway:** this tool had the highest ratio of "bugs only the oracle
caught" to "bugs caught by reading" of anything ported so far - the
off-by-one bound in particular looked completely unremarkable next to
two structurally similar (but differently-bounded) loops already ported
correctly elsewhere. Where a strong prior exists ("this is the same
pattern as X"), it's worth explicitly re-deriving the bound from the
source's actual numbers rather than pattern-matching against a
recently-ported sibling - the two other bugs found by reading alone
(both in `spectmagwarp2`) came from a synthetic case constructed
specifically to distinguish "did it divide by peak or not," not from
a prior-pattern check at all.

## Task 3.6's `pvc denoise`: a design decision confirmed by reading two call chains, and a real fatal quirk in the oracle itself

`noisefilter.c`'s own architecture shells out to a separate `freqresponse`
process at runtime (`system("freqresponse ... %s %s", ...)`), writing its
noise-response analysis to a `/tmp` file and reading it back via
`fillfunc`. Rather than reproducing that subprocess/tmp-file design, `pvc
denoise` calls the already-ported `pvc_core::tools::freqresponse::process`
in-process on a `[--noise-begin, --noise-end)` slice of the same channel
it's about to filter (`pvc_core::tools::noisefilter::build_noise_response`).
This only holds up if `-b`/`-e` genuinely restrict the analyzed samples
in the real tool - confirmed by reading the actual call chain rather than
assuming: `freqresponse.c` sets `begint`/`endt` from `-b`/`-e`, then
`setupfiles()` → `getInputFileDataToSetOutputChannels()`
(`legacy/pvc_lib/fileio.c`) seeks to `begint*R` and buffers only
`(endt-begint)*R` frames into a scratch file *before* `openfiles()`
re-opens it for the frame loop - real, sample-accurate trimming, not the
display-only `dur` bookkeeping `-b`/`-e` amount to in `plainpv`/
`pvanalysis` (a different call path, not revisited here).

`threshold_limit` and `smooth_amp_change` (`legacy/pvc_lib/`) had no
prior Rust port; both were small enough to verify by direct reading
(`filter_response.rs`'s established amplitude-only-array convention
covers `threshold_limit` too - it never touches frequency slots) - no
oracle test needed beyond the existing unit-test style already used
throughout this crate. The pitch/frequency-shift + gain loop reuses the
same real bound already found the hard way in Task 3.8:
`for(i=1;i<(N+2);i+=2)` in the C, covering every bin including Nyquist -
not `plainpv`/`twarp`'s Nyquist-excluding `i<N`. Recognizing the pattern
from `filter.c` this time didn't require re-discovering it via a
tolerance mismatch, but it was still re-derived from `noisefilter.c`'s
own source rather than assumed by similarity, per Task 3.8's takeaway.

The oscillator-bank-vs-overlap-add selector is genuinely conditional
here (like `twarp`, unlike `plainpv`'s hardcoded-always-oscbank quirk),
decided once per channel from whether pitch transposition or frequency
shift is non-default - `tools::twarp`'s existing dual-branch structure
(`OscBank`/`Synthesizer` side by side, selected by one `bool`) transferred
directly with no changes needed to either primitive.

**A real fatal quirk in the oracle itself, not a porting bug:** the two
pre-existing (Task-1-era, never oracle-tested) golden cases originally
specified the default noise-analysis method (average). Run against the
real binary, both crashed before writing any output:

```
ANALYSIS SEGMENT DURATION: 0.500000
ALL ZERO AMPLITUDES!
OUTPUT FILE: DURATION: 2.500000
WARNING!  NOISE SAMPLE HAS ZERO AMPLITUDE! BYE
```

Traced to the shared `get_formants`/`NormalizeToPeaksOfSpectrumInBand`
machinery (also used by `freqresponse`/`filtresponsemaker`/
`chordresponsemaker`, Task 3.5): average-method formant detection finds
*zero* formants in the fixture's whitenoise lead-in, and the band-
normalization step collapses the entire response to all-zero amplitudes
when that happens - confirmed by testing `-F1` (peak method) directly
against the same binary/fixture/window, which finds formants fine and
produces real (220,924-byte) output. This is a real behavior of the
original toolkit for broadband-noise input, encountered only because
this was the first time a case exercised `get_formants` on *noise*
specifically rather than a tonal/harmonic source - not a bug in this
port, and not worth "fixing" (reproducing it faithfully would mean `pvc
denoise --noise-method average` also fails loudly on similar input,
which is the correct, oracle-matching behavior). Both case `.toml`s were
updated to add `-F1`/`--noise-method peak`, with the finding recorded
inline in `basic_denoise.toml`'s notes.

## Task 3.7's `pvc compand`/`pvc spectwarp`: two per-bin dynamics processors that only look alike from a distance

`compander.c` and `spectwarper.c` share the same overall shape (companding
loop over a rolloff-widened `[lowbin, hibin)` band, then a full-spectrum
pitch/freq-shift+gain loop, then shelf EQ, then oscbank-or-overlap-add) -
close enough that reading one first creates a real risk of pattern-
matching the other's details onto it. Three genuine per-tool differences,
each re-derived from source rather than assumed by similarity:

- **What's being companded against.** `compander.c` normalizes each bin
  against a *separately loaded, static* peaks/reference file (`-F`,
  required - same raw `N+2`-float layout as `filter.c`'s `.fr`,
  `smoothspec`'d once at load time). `spectwarper.c` has no such file at
  all (`-F` is in its `crack()` accept string but has no `case` - dead) -
  it normalizes each bin against *its own frame's own live spectral
  peak*, recomputed fresh every frame. Architecturally distinct enough
  that they don't share a companding primitive - `compander.rs` doesn't
  call anything from `spectwarper.rs` or vice versa.
- **`getthresh`'s `N` vs. `N+2`.** `compander.c` calls
  `getthresh(channel, N+2, threshfac)` (the full bins array, including
  Nyquist); `spectwarper.c` calls `getthresh(channel, N, threshfac)`
  (excluding Nyquist, matching `plainpv`/`twarp`'s convention). Same
  function, same-looking call site one file over, opposite answer -
  exactly the "which N reaches the call site" gotcha this project keeps
  finding (`getthresh`/`get_formants`/`smoothspec`/`filter.c`'s pitch
  loop), now confirmed to vary *between two structurally similar
  siblings*, not just within one tool's own internals.
- **Is `-L`/release real?** Dead in `compander.c` (both its attack and
  decay smoothing branches use the same attack coefficients - a real bug,
  reproduced faithfully, not exposed as a CLI flag since exposing a
  provably-dead one would mislead). Fully live in `spectwarper.c`, which
  uses a *different* single-value smoothing primitive
  (`smooth_one_value`, newly ported to `pvc-core::smooth` - genuinely
  attack/release-conditional, unlike `compander.c`'s broken pair) for its
  peak-follower, plus a third, distinct smoothing role (`-r`, a plain
  lerp with no attack/release branching at all) for the per-bin
  amplitude-change multiplier itself.

**A real out-of-bounds read found via a Rust panic, not by reading:**
`spectwarper.c`'s sliding-window mode (`-S > 0`) computes a per-bin local
peak-search window, with a bound-clamp that reads backwards
(`if (hib < N) hib = N - 1;` where a real clamp would test `hib > N`).
Since `hib` is almost always far smaller than `N` for a realistic
window, this fires on nearly every bin, so the "local" window nearly
always becomes `[lowb, N-1)` - defeating the sliding-window design
almost entirely, reproduced faithfully here (not "fixed"). But for bins
near Nyquist with a wide (especially octave-mode) window, the
*un*-clamped `hib` can genuinely exceed the array's real allocation
(`N+2`) - this port's own
`sliding_window_compression_runs_without_panicking` test hit exactly
this, panicking with `index out of bounds: the len is 1026 but the
index is 1026` on first run. Same class of bug as `filter_response::
smooth_response`'s documented OOB read (Task 3.8) - clamped to the last
valid index here instead of reproducing undefined behavior, since Rust's
bounds checking makes the difference between "silently reads adjacent
heap memory" and "panics" immediate and impossible to ignore, unlike C.

Both tools' pre-existing (Task-1-era) golden cases passed against a
freshly built C oracle on the first try, no case-flag corrections needed
this time (unlike Task 3.6's) - `compander`'s `freqresponse_driven` case
and both of `spectwarper`'s cases (`basic_compand`, `warp_curve`) matched
within tolerance immediately once the ported math was correct.

## Task 3.9's `pvc harmonize`: a dead control array only the oracle could have found

`harmonizer.c` is the most structurally complex tool ported so far (two
new circular delay-line rings, a dual-oscillator-bank resynthesis path
that shares one cosine table between its source and harmony banks, and a
data table whose per-bin static triangular-window profile is baked in
once at setup rather than recomputed per frame) - and it produced this
session's largest oracle-vs-reading gap yet: a first working-by-
construction port measured **16.7dB** off the real binary (tolerance
0.5dB) on the very case the plan's own Task 1.2 wrote as this tool's
canonical example.

Diagnosis (see the session transcript for the full walk, condensed here):
per-frame instrumentation on both sides (a `HDEBUG` env-gated `eprintln!`
in the Rust port, a matching temporary `getenv("HDEBUG")` block spliced
into a scratch-built copy of `harmonizer.c` - never committed, reverted
via `git checkout` the moment it had answered the question) showed the
*static per-bin triangular-window amplitude* (`HARMONIZER_DATA_amp`,
already confirmed byte-identical between the two implementations for
several bins) was being **fully applied** in the port but had **no
effect at all** in the real binary - its output for the dominant bin
matched the *unwindowed* raw analysis amplitude to within a rounding
error, not the ~54dB-quieter windowed value the port (correctly,
per the source's evident intent) computed.

Root cause, found by grepping every reference to the array once the
symptom was narrowed down: `target_AmpInterpControl_VALUES` is allocated
(`harmonizer.c:483`) but **never written** - every other per-band control
array (`target_harmadd_VALUES`, `target_dB_VALUES`,
`target_FreqInterpControl_VALUES`, `target_TimeInterpControl_VALUES`)
gets populated by its own `getGlobalFunctionValues(...)` call in the
per-frame "GET THE VALUES" section; `target_AmpInterpControl_VALUES`
simply has no such call anywhere in the file. It stays at whatever
`fvec()`'s zero-initialization left it - permanently `0.0` - regardless
of `-J`'s value or its own documented default of `1.0`. Since the
per-bin blend is `temp4 = temp2 + target_AmpInterpControl_VALUES[band] *
(temp3 - temp2)` (unwindowed dB `temp2`, fully-windowed dB `temp3`), a
permanent `0.0` collapses this to `temp4 = temp2` unconditionally: **the
entire triangular-window feature this tool is nominally built around is
dead code in the shipped binary**, and `-J`/amp-interp does nothing
regardless of what value it's given.

This is not the kind of bug reading alone was ever going to catch here:
the array's *declaration*, *allocation*, and every *read* site all look
completely ordinary; only the conspicuous *absence* of one populate call
among five structurally-identical siblings gives it away, and that
absence is invisible until something (the oracle, in this case) proves
the "obviously intended" behavior wrong first. Reproduced faithfully:
[`crate::tools::harmonizer`]'s blend always uses `0.0`, and
`--amp-interp` isn't exposed as a `pvc harmonize` flag at all - matching
`pvc compand`'s established treatment of its own dead `-L` flag, since
surfacing a provably-inert flag would mislead users into thinking it
does something.

Once fixed, `basic_harmony.toml`'s only remaining gap was the file's
very last 1024-sample block (both sides already 66-68dB below peak,
i.e. inaudible) - the same flush/tail-boundary variance already
documented for other oscillator-bank tools, not a new finding; the
tolerance was widened from 0.5dB to 2.0dB with that measurement
recorded inline, same as every other tolerance widening this session.

Two smaller, independently-confirmed findings from the same read:
`getthresh` is called with `N` (not `N+2`) for the source array here,
matching `plainpv`/`twarp`/`spectwarper`/`noisefilter`'s convention, but
with the harmony array's own *real, unpadded* size (`NCmult2`, no `+2`)
for the harmony array - yet another tool disagreeing with `compander`'s
outlier `(N+2)` on this exact point, reconfirming why this project
re-derives the bound at every call site. And the dual-bank resynthesis
path shares one oscillator-table (`OscBank::with_shared_table`, a new
`pvc-core::pvoc` constructor) built from the *source* bank's `n2`/`nw`,
not the harmony bank's own very different bin count - ported by reading
`noscbank2.c` closely rather than assuming two independent `OscBank::new`
calls would suffice, since they would each derive their own (different,
wrong) `tabscale`.

## Task 3.10's `envelope`/`centroid`/`flux`: the first non-resynthesizing tools, and a shared shape that hides real per-tool differences

`envelope.c`, `centroid.c`, and `fluxoid.c` are the first ported tools
that never resynthesize audio at all - each outputs a time-series of
scalar values (ASCII or a headerless raw `f32` stream), one per output
sample, computed from a two-pass pipeline: pass 1 analyzes each channel
independently (`fold`/`rfft`/`convert` via the existing `Analyzer`, a
per-tool scalar metric over a frequency band, attack/release smoothing),
combines channels index-wise (average or peak), then pass 2 normalizes,
optionally compresses/gates, applies a distribution warp, and resamples
onto an independent output rate via linear interpolation.

The three tools' usage() texts and this project's own pre-existing
parameter-inventory doc present them as near-identical, sharing one flag
table - reading each source directly instead of trusting that symmetry
turned up real, meaningful differences:

- **`centroid` has no compress/gate stage at all.** `-T`/`-S` (compress-
  threshold/gate-threshold) appear in the shared doc table as common
  flags, but `centroid.c`'s `crack()` switch has no `case` for either
  letter - confirmed by grepping every `case '` in the file. Its pass 2
  only warps the raw centroid frequency and converts it to the requested
  output format; the whole normalize/compress/gate/output-scale chain
  `envelope`/`fluxoid` share is simply absent. Not exposed as CLI flags
  at all, matching this project's established treatment of provably-dead
  legacy flags elsewhere (`pvc compand`'s dead `-L`, `pvc harmonize`'s
  dead `-J`).
- **`centroid`'s `-H` (a second, different warp from `-W`) is also dead**
  - parsed, printed in the startup banner, but its only call site
    (`spectmagwarp(...)`) is commented out in the source.
- **A frame-0 asymmetry between two structurally similar siblings**:
  `find_centroid` seeds its fallback `old_value` to the band's midpoint
  on frame 0 and centroid.c never overrides that, so frame 0's
  smoothing compares the real centroid against the band midpoint - not
  a no-op. `fluxoid`'s caller, by contrast, forces `old_temp = temp`
  immediately after frame 0's (always-zero, by construction) flux
  value, making its own frame-0 smoothing step a genuine no-op. Same
  general shape, opposite frame-0 behavior - each confirmed by reading
  the actual library function and its call site, not assumed from the
  other.
- **A real, if practically unreachable, out-of-bounds read found by
  comparing three "identical-looking" band-sum loops**: `envelope.c`'s
  own inline band-sum loop has no upper clamp against the bins array
  size, while both `find_centroid.c` and `find_fluxoid.c` (the shared
  library functions the other two tools call) do clamp `k2` to the
  array's last valid bin before checking whether it collides with `k1`
  and bumping it apart - and that *order* (clamp, then bump) matters: a
  `k1`/`k2` that both land on the clamped max bin bumps `k2` one past
  it, a real one-past-the-end read in the C for that narrow edge case.
  Reproduced as a defensive double-clamp in the Rust port (panic-safe on
  a slice index) rather than the OOB read itself.
- **Two structurally-inlined attack/release smoothers that quietly
  disagree with the shared `smooth_one_value` primitive at the exact-
  equality boundary**: `centroid`/`fluxoid` both hand-roll their
  attack-vs-decay branch (`if (temp > old_temp) ATTACK else DECAY`)
  rather than calling `smooth_one_value`, and that hand-rolled condition
  routes an exact tie to *decay*, while `smooth_one_value`'s own
  condition (`if a < old_a { release } else { attack }`) routes the same
  tie to *attack*. Implemented as bespoke inline branches in both
  `centroid.rs` and `fluxoid.rs` rather than reusing `smooth_one_value`,
  to match each tool's own boundary exactly.
- **A real quirk carried over from a much earlier task's own precedent**
  (state that outlives its own "pass"): pass 2's interpolation variable
  (`old_temp`, the value the very first output sample interpolates
  *from*) is never reset between passes in either `envelope.c` or
  `fluxoid.c` - it's simply whatever pass 1's *last channel's last
  frame* left it at. Under this task's own constant-control-function
  golden cases this is invisible (verified bit-for-bit against the real
  binaries with it reproduced faithfully), but it's a real quirk for any
  future time-varying `-T`/`-G`/`-S`/`-W` table, documented in both
  modules' doc comments.

All three tools use exact `pow`/`log10` throughout (never the
lookup-table `DbToAmp`/`SemitonesToMult` structs used by the
resynthesis-family tools) - confirmed by reading every dB/amp conversion
site in all three files, not assumed from the resynthesis tools'
convention.

A genuinely new modernization choice, not present in the C at all: the
Nyquist-frequency sentinel these three use for `--band-high < 0` doesn't
exist in the original tools (their own default is simply the literal
Nyquist value computed once at startup) - it's this project's own
already-established `pv`/`filter`-style CLI convention for deferring
Nyquist resolution until the input's sample rate is known, applied here
for consistency rather than hardcoding a fixed Hz default that would
silently be wrong for any sample rate other than the golden fixture's
44.1kHz.

All three tools' pre-existing (Task-1-era) golden cases passed against a
freshly built C oracle on the first try (after fixing the
44.1kHz-specific default noted above, before any oracle run) - no
further flag corrections needed.

## Task 3.10's `pvc pitchtrack`: three bugs, found by three different techniques, in the session's most complex tool

`pitchtracker.c` (2593 lines) is a comb-filter fundamental-frequency
detector with a top-20 ranked-candidate list, harmonic reinforcement,
octave/subharmonic error correction, an optional temporal mode filter, a
separate plateau-seeking note-stabilizer (`-j`/`-J`), and a backlog/hold
voiced-gate state machine - by a wide margin the most structurally
complex tool ported this session. Getting its one golden case
(`partials_vibrato_2s_44k.wav`, all-default flags) to match required
finding and fixing three distinct, unrelated bugs, using three different
techniques:

**1. A one-past-the-end read, found by the Rust port's own panic.**
`find_common_freq`'s symmetric-pair index
(`beginbufferindexpoint + begin_buffsize - i`) reaches exactly
`max_buffsize` - one past `fbuff`/`abuff`'s real allocated size - during
the "shrink toward end" phase of its search. The very first test run
panicked with `index out of bounds: the len is 60 but the index is 60`
before any oracle comparison was even possible. Clamped to the last
valid index instead of reproduced, matching this project's now-
established treatment of this exact bug class.

**2. A dead-code trap, found by comparing raw per-frame output against a
scratch-instrumented oracle binary.** With the panic fixed, the port ran
but produced values with no resemblance to the real tool's (wild
frame-to-frame jumps between ~44Hz/~220Hz where the real tool held a
much steadier trajectory). A temporary `getenv("PDEBUG")`-gated
`fprintf` spliced into a scratch-built copy of `pitchtracker.c` (never
committed, reverted via `git checkout` once it had answered the
question - the same technique already used for `harmonizer.c`) printed
`optimal_comb()`'s own candidate list and final `*freqnow` side by side
with the port's equivalent state. The candidate lists matched almost
exactly - the divergence was in which value got *returned*: the
function computes an octave/subharmonic-corrected `alternateFreq` and
visibly uses it to seed the (disabled-by-default) mode filter's history
buffer, which reads as "this is the value in play" on a first pass - but
the actual fallback return statement for the common (mode-filter-
disabled) code path uses `strongestFundamentals[0]` directly, silently
ignoring the correction entirely. A single-letter-looking difference
(`alternateFreq` vs. `strongestFundamentals[0]`) with no textual
proximity to hint at it - the correction computation and its one real
consumer are 20+ lines apart, separated by the entire mode-filter
branch. Not findable by reading in isolation; only the side-by-side
per-frame numeric comparison made the wrong-variable substitution
visible.

**3. An off-by-one in a "consumed one value before the loop starts" pre-
fill, found by comparing pass-2 state frame-by-frame.** After fix #2,
output matched far longer (31 of ~1158 samples before diverging, versus
21 before) but was still shifted exactly one analysis frame late at the
transition point, and 2 samples too long overall. `pitchtracker.c`'s
pass 2 pre-fills its sliding buffers by reading exactly one value from
the frequency/amplitude streams into `fbuff[0]`/`abuff[0]` *before* its
main loop begins - meaning the main loop's own first read starts at
stream index 1, not 0. This port's cursor-based reimplementation (no
scratch files, just `Vec` indices) started both cursors at `0`,
re-reading the same first value the pre-fill had already consumed
instead of advancing past it - one whole frame of drift, invisible
until a frame-by-frame `ampnow`/`freqnow` diff against the same
scratch-instrumented oracle (technique #2, reused) showed every value
matching exactly but shifted by one frame.

Two additional bugs found by reading alone (both **reproduced
faithfully**, not fixed): multi-channel amplitude averaging is
completely broken (the C computes a real running average into a
variable that's then discarded, so the final value is simply the *last*
channel's own value - the parallel frequency-averaging code one block
down does *not* have this bug, a real asymmetry between two near-
identical-looking blocks); and pass 1's frequency output stream can grow
longer than its amplitude stream (a duplicate-write mechanism for
consecutive "no candidate found" frames applies only to the frequency
file), so pass 2's read-one-from-each-in-lockstep loop can end when
either stream individually runs dry.

**Takeaway**: three real, independently-discovered bugs in a ~2600-line
file, none reachable by static reading alone in reasonable time, each
surfaced by a different technique (a Rust panic on an unsafe C read; a
side-by-side raw-value comparison against a scratch-instrumented
oracle; a frame-by-frame state diff against the same). This is the
clearest evidence yet in this project for oracle-driven porting's core
thesis: for code this size and this densely interdependent, "port by
reading, verify against the real binary" finds real bugs that "port by
reading" alone - however careful - does not.

## Phase 5's `pvc convert-units`: a printf round-trip is not the same as a bit-exact oracle

The first Phase 5 tool (the long tail of minor legacy utilities, after
Phase 3's core-tool parity): `amptodB`, `dBtoamp`, `Hztopitch`,
`pitchtoHz` (four tiny standalone converters, 54-69 lines each)
consolidated into one `pvc convert-units --from --to` command. All four
conversions turned out to already exist as oracle-verified math
elsewhere in this codebase (`units::amp_to_db`, `response::oppc_to_hz`,
and `pitchtracker.rs`'s private `hz_to_oppc`, promoted to
`response::hz_to_oppc` so both `pvc pitchtrack` and `pvc convert-units`
share one definition) - except `dBtoamp`'s own math, which turned out to
be a genuine *third* dB/amplitude convention: unlike the `dB_to_amp`
lookup table (`pv`/`filter`/`envelope`-family) and unlike the exact
`amp_to_db`, the standalone `dBtoamp` tool never calls into
`miscellania.c` at all - it inlines `pow(10., dB/20.)` directly. Added
as `units::db_to_amp_exact`, confirmed against the real compiled binary
in `/tmp/pvcbuild` (built once locally outside Docker for this session;
values transcribed into the unit test's oracle table).

That same real-binary check caught a subtler trap while writing
`hz_to_oppc`'s own oracle test. `Hztopitch 261.625` prints
`7.119999` - a deliberately-faithful reproduction of a real C quirk
already documented on `pitchtracker.rs`'s output-format path (a value
landing a hair below an exact octave boundary truncates into the
*previous* octave's pitch classes rather than rounding up). Writing
`assert_eq!(hz_to_oppc(261.625), 7.119999)` from that printed text
**failed**: the computed `f32` was `7.1199994`, one bit off from what
`"7.119999"` parses to. The C's `%f` only prints six digits after the
decimal point, which isn't enough to pin down a specific `f32` bit
pattern near that magnitude - two adjacent floats round-print
identically. The fix wasn't a code change (the computation was already
correct, confirmed separately by formatting the same value with `{:.6}`
and getting `"7.119999"` back, matching the C exactly) - it was using
the panic's own `left: 7.1199994` as the test's expected value instead
of the printed text, the same "cite oracle values at full `f32`
round-trip precision, not truncated display precision" convention
already flagged by `#[allow(clippy::excessive_precision)]` comments
elsewhere in `units.rs`. A reminder that a `printf`-based oracle
comparison is exact only down to the number of digits actually printed
- getting a numeric match from truncated text is necessary, not
sufficient, evidence of a bit-exact result.

## Phase 5's `pvc impulseresponse`: a stale prebuilt oracle binary looks exactly like a real 15x bug

`impulseresponse.c` (analyzes a `[-b, -e)` window into a zero-padded,
peak-normalized rfft-format spectrum - the head of a small FFT-
convolution family, `.ir` files later consumed by `irconvolver`/
`irconvolvesequencer`) ported cleanly, its raw (pre-normalization)
spectrum matching a locally-built C oracle to ~1e-6 relative on the
first try. Its *normalized* output, though, was a uniform **15.165x**
too large across every bin - not a bug pattern this project had seen
before (a scale error, not a shape/timing one), and suspiciously
precise (identical ratio to 6 significant figures across every sample),
which usually means "one wrong constant," not "an algorithmic
divergence."

The wrong constant turned out to be the *oracle*, not the port. Earlier
oracle-verification work this session had produced a full toolkit build
under `/tmp/pvcbuild` (Docker `gcc:12-bookworm`, output copied to a
host-visible bind mount so it would survive across separate `docker run`
invocations - the project's established pattern, `docs/dev/
rust-verification.md`'s own "Docker dev workflow" note). That build was
never rebuilt as later commits touched `legacy/pvc_src/*.c` for
unrelated oracle-debugging sessions (always `git checkout`-reverted
before committing, per this project's own instrumentation-hygiene rule
- reverted in the *source tree*, but the *binary* built from an
in-between state stayed on disk in `/tmp`, outside git entirely, with no
signal that it no longer matched `HEAD`). Rebuilding `impulseresponse`
fresh (same Docker image, same `pvc_src/Makefile` invocation, this
time with `getenv("PDEBUG")`-gated `fprintf`s added and reverted the
usual way) against the *current* `legacy/pvc_src/impulseresponse.c`
gave `peakAmp = 0.0657946542`, `normalizationLevel = 0.997791529` -
ratio `15.166`, matching the port's own scale factor to five
significant figures. The port was correct all along; the byte-exact
`cmp` failure was two different, both-individually-reasonable build
artifacts disagreeing with each other, not either of them disagreeing
with the current source.

**Takeaway**: a prebuilt oracle binary is a cached artifact like any
other, and this project has no mechanism that invalidates one when the
source it was built from changes - "rebuild before trusting a surprising
result" belongs in the same checklist as "revert instrumentation before
committing." A uniform, precisely-repeating scale-factor mismatch across
every value is itself a useful signal that the *comparison inputs*, not
the algorithm, are what's misaligned - a genuine algorithmic bug at this
level of complexity essentially never produces a single perfectly
uniform constant across every output value.

Once compared against the freshly-rebuilt oracle, remaining error was
~6e-8 absolute (one `f32` ULP) - accumulated summation-order noise in a
16384-point FFT, the same class of essentially-exact-but-not-bit-
identical result already accepted elsewhere in this project for large
transforms. `tests/golden/cases/impulseresponse/basic.toml` uses a
`numeric` tolerance of `1e-4` (checked by a custom Rust comparator,
`golden_impulseresponse.rs`, not `compare.py` - `.ir` is a binary
format, header plus raw spectra, not `compare_numeric`'s whitespace-
separated text) - generous headroom over that ULP-level noise floor
while still catching a real structural mismatch.

## Phase 5's `pvc irconvolver`: a shared library function's own flush call doesn't flush

`irconvolver.c` (the second tool in the FFT-convolution family: reads a
`.ir` file, block-convolves or -deconvolves each input channel against
it via classic overlap-add) surfaced a real bug in shared library code
already flagged once before in this project, this time actually
affecting a golden case's output length rather than just a synthesis
ring's internal bookkeeping.

**The bug**: `legacy/pvc_lib/fileio.c`'s `bufferout()` buffers output
into `BLOCKSIZE` (`1024`)-sample chunks in a `static` scratch array,
writing a chunk to the per-channel temp file only once it's full - or,
its `flushflag` parameter promises, when told to flush regardless. The
promise doesn't hold: the entire write path lives inside `while
(outbuffpt < I)`, gated on the *new* sample count `I`, not on
`flushflag`. `irconvolver.c`'s own final call is `bufferout(outbuffer,
0, 1)` - `I = 0` - so that loop never runs even once, `flushflag`
notwithstanding, and any pending sub-`BLOCKSIZE` remainder is silently
never written. A 3-second, 132300-sample convolution's real output is
132096 samples (`129 * 1024`), not 132300.

This is the same `bufferout()` quirk Task 3.4's `pvc twarp` work already
found from the *synthesis* side (`docs/dev/rust-verification.md`'s
Task 3.4 section: "`bufferout(A, I, 1)` ... only ever reads `I` ... samples
regardless of `flushflag`") - but `twarp`'s own flush call happens to
pass a nonzero `I` (the hop size), so that tool's trailing block is
written correctly and the quirk stays invisible in its own output. Only
`irconvolver.c`'s specific `bufferout(_, 0, 1)` call pattern actually
loses data: the same shared bug, a different call site, a different
consequence - confirmed only by re-deriving it from scratch against a
real length mismatch, not recognized from the earlier finding until
after the fact. Reproduced faithfully (not "corrected") in
`tools::irconvolver::process`: every channel's output is truncated to
the largest multiple of `1024` samples, including down to *entirely
empty* for an input under one `BLOCKSIZE` long.

**A second, unrelated finding, caught by the same investigation**: after
fixing the length mismatch, per-sample values still differed from the
real oracle by up to ~0.05% of full scale - small, but non-trivial for a
"sample" tolerance kind. Tracing it back: `legacy/pvc_lib/fileio.c`'s
`rescaleThisBuffer` (the shared "rescale whole output to match input
peak" feature every already-ported resynthesis tool in this project
also uses) computes its rescale ratio *once*, from `ipeakamp`/`peakamp`
as they stand at the moment the *first* output block is flushed - not
the true peak over the whole file. This port uses the true whole-file
peak instead (simpler, and consistent with every other tool's existing
rescale port here), so the two agree only when a signal's peak
amplitude doesn't drift much over time. The golden case's frequency
sweep, convolved against a resonant-ish impulse, is exactly a case where
it drifts: the post-convolution peak keeps changing well past the first
block. Not chased further into an exact snapshot-timing replica - the
discrepancy is small, well inside `twarp`'s own already-precedented
`sample`-tolerance margin (`0.002`), and doing so would mean modeling
the *exact* interleaving of `bufferin`/`bufferout`'s internal block
counters, disproportionate to what this secondary reporting feature is
worth.

**Takeaway**: the same underlying shared-library bug can hide behind one
call site and surface at another - "already found and documented
elsewhere" is a reason to look there first, not a reason to assume a new
symptom is already explained. Re-deriving the mechanism from the actual
observed numbers (a suspiciously round `frameBeginSampNow`/`sampsRead`
trace via a temporary oracle rebuild, then noticing `132300 - 132096 =
204 = 132300 mod 1024`) was still necessary before the connection to the
earlier `twarp` finding became obvious.

## Phase 5's `pvc ring`: a silent duration mismatch, and the same copy-paste bug three times over

`ring.c` ("phase vocoder reverberator/resonator": a feedback delay
network built from spectral frames, not time-domain samples) surfaced two
real findings - one from a length mismatch before any sample comparison
was even possible, one from a large, structural amplitude mismatch found
only by isolating the tool's two independent signal paths one at a time.

**Finding 1: a silent duration extension, found by a plain length
mismatch.** The first candidate run against the real oracle failed before
`compare.py` could even compute an error: `length differs: expected
103620 samples, got 90420` - a 13200-sample (60-hop, at this case's
220-sample hop size) gap, non-trivial and clearly not off-by-one. Tracing
it back: `ring.c` computes `funcStats(&feedback_level, ...).hi` right
after flag parsing into a variable named `ringTime`, which then appears
to go nowhere - it's never read again anywhere in `ring.c` itself, not
even in a comment. It turns out to be a genuine `extern float ringTime`
(`legacy/pvc_lib/pv.h`), consumed by the *shared* `fileio.c`'s
`bufferin()`: once the real input runs out, `bufferin` keeps supplying
`ringTimeSamples = ringTime * isr` further samples of silence before
finally reporting EOF - letting the feedback delay line's own reverb tail
actually ring out past the input's end, rather than being cut off at it.
Nothing in `ring.c`'s own source suggested this was happening; the only
way to find it was noticing `13200 / 220 = 60` was suspiciously round and
`13200 / 44100 ≈ 0.3` matched this case's own `-Z 0.3` almost exactly,
then grepping the *shared library headers* (not just `ring.c`) for
`ringTime` to find where it's actually consumed. Reproduced in
`tools::ring::process_channel` by extending the input with
`control_fn_max(&params.feedback_decay_secs).max(0.0) * sample_rate`
silent samples before running the usual hop/EOF bookkeeping - `dur` (the
control-function normalization window) is deliberately left computed
from the *original*, unextended span, matching `ring.c`'s own `dur`
calculation happening earlier, unaffected by `bufferin`'s later read-side
extension.

**Finding 2: the same swapped-default bug, independently, three times.**
With the length mismatch fixed, sample values still diverged by a huge,
structural margin - not the small residual noise a floating-point
approximation difference produces. Isolating `ring`'s two independently-
gain-controllable paths (`-F-999`/`-S-999` to silence one or the other)
found the *source* path matching the oracle almost exactly (`15798` vs.
`15797` peak in a stable region) while the *feedback* path was ~8x
(~+18dB) too loud (`15608` vs. an oracle `1955`) - proof the port's core
oscillator-bank/phase-vocoder machinery was already correct (confirmed
independently once more by `pvc irconvolver`/`pvc harmonize`'s own
reuse of it) and the bug was specific to something only the feedback
path touches. Re-reading `ring.c`'s three shelf-EQ initializer blocks
side by side (input, in-loop feedback, output) found the *identical*
copy-paste mistake in all three: each stage's low-shelf gain is
initialized to `200.` under a comment reading "... LOW SHELF: DB", and
its low-shelf frequency to `0.` under "... LOW SHELF: FREQ" -
`INPUT_dBlow`/`INPUT_freqlow`, `FEEDBACK_dBlow`/`FEEDBACK_freqlow`, and
`OUTPUT_dBlow`/`OUTPUT_freqlow` all swapped the same way, all
contradicting `usage()`'s own documented defaults (`-O`/`-X`/`-k`
"\[0.\]", `-d`/`-U`/`-s` "\[200.\]"). Since `eq()`'s flat-gain fast path
only triggers when the low and high shelf dB values are exactly equal,
each of these three defaults takes the *shelf* branch on every frame with
no flags passed at all - here, specifically the *output* EQ instance
(applied unconditionally to the feedback path's every frame, unlike the
in-loop one, which - see `pvc-core::tools::ring`'s doc comment on
`y(n)` vs. `y(n-1)` - only shapes future frames' delay-line tail) was
what the isolated feedback-only test was actually measuring. `pvc ring`'s
CLI layer now defaults all three low-shelf gain/frequency pairs to
`200.0`/`0.0`, matching the C's real (not documented) behavior.

**Takeaway**: a length mismatch is worth root-causing structurally (a
round hop-count gap, checked against every flag's own value) before
assuming it's an off-by-one in the port's own bookkeeping - the actual
cause here lived in a shared library header, not the tool's own source at
all. And finding one instance of a copy-paste default-value bug is a
reason to grep for the *pattern*, not just the one site: this tool
declares the same broken initializer three separate times, and only one
of the three had already been read closely enough to notice by the time
the first (input EQ) instance was found from the source alone - the
second and third were found only because the *oracle* still disagreed
after the first fix, not because the source was re-read more carefully.

## Phase 5's `pvc ringfilter`: reused `ring` machinery, and a real clap bug found along the way

`ringfilter.c` ports cleanly as `ring.c` plus one added feature (a
switchable fixed-spectrum filter, placed either on the feedback path's
input or inside the loop) - `tools::ringfilter` reuses `tools::ring`'s
shared helpers directly (`RawAnalyzer`, `lean_convert`/`lean_unconvert`,
`apply_shelf_eq`, `control_fn_max`, all promoted to `pub(crate)` for this)
rather than re-deriving them, and inherits the same swapped-shelf-EQ-
default bug and `ringTime` duration-extension mechanism `tools::ring`'s
own section above already documents. One genuine algorithmic difference
was found by reading the two files side by side: `ringfilter.c` guards
the in-loop feedback EQ's decay-time division (`if (FEEDBACK_decay_time
.A[0] < IR) ... else ...`) where `ring.c` divides unconditionally - a
real safety fix the twin tool has and the other doesn't.

**A real, pre-existing bug in this project's own `clap` CLI layer, found
while testing a brand-new flag - and confirmed to already affect shipped,
merged code.** `pvc ringfilter` needed three new `bool`-typed flags backed
by a custom string `value_parser` (`--filter-placement prefilter|
postfilter`, `--filter-pitch-mode source-only|source-and-filter`, plus a
second copy of `pvc ring`'s own `--feedback-threshold-mode above|below`).
Testing `--filter-placement postfilter` for the first time (to spot-check
the postfilter code path against the oracle) failed immediately: `error:
invalid value 'true' for '--filter-placement': expected "prefilter" or
"postfilter", got "true"`. `clap`'s derive macro infers `ArgAction::
SetTrue` for any plain `bool`-typed field *by default*, silently
overriding an explicit `value_parser` unless the field's `#[arg(...)]`
also says `num_args = 1` - so every such flag in this project was
actually a valueless toggle that always resolved to `true` regardless of
what a caller wrote after it, with its "true" branch simply unreachable
any other way. Grepping the whole file for the same shape (`value_parser`
immediately followed by a `pub _: bool,` field with no `num_args`) found
four more instances, all with the identical bug: `pvc ring`'s own
`--feedback-threshold-mode`, and - already merged and shipped on
`main`, nothing to do with this tool or `ring` at all - `pvc twarp`'s
`--window-mode loop|autostop`. Confirmed directly: `pvc twarp
--window-mode autostop` failed with the exact same "got \"true\"" error
before the fix, and `pvc twarp --help` showed the flag with no `<...>`
value placeholder at all, exactly matching what `ArgAction::SetTrue`
flags look like. All five fixed with `num_args = 1`, verified after by
running the previously-failing invocation of each.

**Takeaway**: a single well-understood library-level pitfall (clap's
type-directed action inference silently overriding an explicit parser)
recurs anywhere the same shape is copy-pasted, including into code that
shipped and merged before this bug was ever noticed - grepping for the
*shape* of the bug across the whole file, not just fixing the one flag
that happened to fail a test, is what surfaced the other four instances,
one of them in already-released functionality with no connection to the
feature actually being worked on.

## Phase 5's `pvc tvfilter`: composing two already-verified tools instead of re-deriving either

`tvfilter.c` ("time-varying cross-synthetic phase vocoder filter") turned
out to be a genuine composition of two already-ported tools' own
machinery, not a new algorithm needing its own oracle-finding process
from scratch: its per-frame filter application is the same shape as
`filter.c`'s (shelf EQ, warp, smoothing, bin-shift/transpose lookup,
frame normalization - even the same `pmult=1`/`normflag=0` call
conventions), and its filter-response *source* is time-navigated exactly
the way `twarp.c` navigates its own resynthesis source (`findFilterTime
AndConstrainByWindow`/`makeInterpolatedFilterFrame` are the *same*
shared-library functions twarp already ported as `timenav::
TimeNavigator`/`interpolate_frame`). Confirming this by reading both
files side by side - rather than re-deriving the bin-shift formula or the
time-window wrap/fold/clip logic from `tvfilter.c` alone - was what made
this port tractable: `tools::tvfilter::process_channel` calls
`timenav`'s functions directly and mirrors `tools::filter`'s own
response-shaping order, with only the pieces that are genuinely new to
this tool (`N_ratio` bin scaling for an independent audio/filter FFT
size, `compress()`, `normalize()`, `normalizeLoopAmplitudes()`) written
fresh.

One real, load-bearing gap surfaced by this reuse: `tools::filter`'s own
`invert_response` had only ever been exercised by `filter.c`'s single
call site (`normflag=0`, a fixed peak of `1.0`), so its port took the
narrower shape and documented that as intentional. `tvfilter.c`'s own
`-q 2` mode calls the *same* shared `invertresponse()` with `normflag=1`
(invert against each frame's own peak, not a fixed `1.0`) - a second real
caller the original port's own justification hadn't accounted for.
Extended `invert_response` to take an explicit `peak_relative: bool`
rather than writing a second, near-duplicate function, since the two
call sites use different values of the *same* parameter the C already
exposes, not different algorithms.

Also confirmed by reading rather than assumed: this tool has no time-
position smoothing equivalent to `twarp`'s own `-time-response` (`make
InterpolatedFilterFrame` is called with the navigator's raw, unsmoothed
`filttnow`), and `-u` ("FILTER data access mode") is entirely dead -
declared, parsed, and printed, but the time-navigation formula it would
supposedly select between doesn't actually branch on it anywhere.

Oracle-verified on the first real attempt (a sweep's own analysis, time-
navigated in the default wrap-loop mode, used as a cross-synthesis filter
on a steady tone) at ~3e-5 max absolute sample error - essentially
floating-point noise, not a residual approximation needing its own
tolerance headroom the way several earlier tools' rescale/snapshot-timing
quirks did.

**Takeaway**: when a new tool's own usage text and flag names closely
echo an already-ported one's, checking whether they share actual
*library* functions (not just a similar-sounding feature) before writing
anything new can turn a large port into function composition - and can
also surface a real gap in the earlier port's own scope, one that
wouldn't have been found without a second real caller exercising it.

## Phase 5's `pvc irconvolvesequencer`: a shell-script orchestrator, and an unclipped write

`irconvolvesequencer.c` (the third tool in the FFT-convolution family:
crossfades a signal through a sequence of impulse responses, morphing
from one to the next across `[begin, end]`) is not really a DSP tool of
its own in the C - it is a shell-script orchestrator around four other
tools (`impulseresponse`, `irconvolver`, `gen4`/`reshape` for the
crossfade envelope shapes, `mixfiles` for the final sum), built entirely
out of `sprintf`+`system()` calls. This port calls the equivalent Rust
functions directly instead (`tools::impulseresponse::process`,
`tools::irconvolver::process`, `gen4`, `amp_to_db`), with a small
`mix_segments` function standing in for the specific "sum N delayed
buffers, then normalize" slice of `mixfiles.c` this tool actually uses -
not a general port of that still-unported tool.

**A dead flag, found by reading the `crack()` list against the
`switch`**: `-a` appears in the flag list `crack()` is given, but there
is no `case 'a':` at all in the `switch` beneath it - unlike every other
flag in that list, which is handled. The local
`deconvolution_0__convolution_1` variable a working `-a` would have set
therefore never leaves its hardcoded initializer (`1`, convolution), no
matter what a caller passes. This is a different class of doc/behavior
mismatch than `irconvolver`'s own `-a` (a real, working flag whose
*usage text* just mislabels it) - here the flag does nothing at all, in
both the code and the usage text (which doesn't mention `-a` either).
Caught by systematically cross-referencing `crack()`'s flag string
against every `case` in the `switch`, not by observing a symptom -
worth doing for every tool with this `crack()`/`switch` shape, since a
silently-dead flag produces no symptom to notice in the first place.

**A second finding, this time from an actual oracle run**: the first
attempt at this tool's golden case left `-v` (mixfiles' normalization
code) at its default, `0` (off). The candidate output diverged from the
oracle by a huge margin (`max abs sample error 1.99872`, essentially
uncorrelated) despite every other sample matching to five decimal
places - one Python script later, the mismatch was a single sample:
expected `32750`, candidate `-32744`, with both sides' immediate
neighbors sitting smoothly around `-32700` to `-33000`. `mixfiles.c`
writes its summed output straight through `sf_write_float` with no
`SFC_SET_CLIPPING` call - libsndfile's default behavior for a float
sample outside `[-1.0, 1.0]` feeding a 16-bit PCM writer is to
wrap/truncate the out-of-range integer, not clamp it, so a summed
sample at `1.99999...` (unsurprising with normalization off: two
full-scale-ish impulse responses, summed) came out as an
inverted-polarity `32750` instead of a clipped `32767`. Every other
resynthesis tool ported here reaches its output through `bufferout()`'s
shared rescale path first (`rescaleThisBuffer`, see `irconvolver`'s own
section above), which already keeps values in range before they reach
`sf_write_float` - `mixfiles.c` is the first tool in this project that
skips that path entirely, so this divergence had no earlier precedent
to check against. `pvc_io::write_wav`'s `f32 -> i16` conversion is a
plain Rust `as` cast, which saturates rather than wraps (a real,
deliberate language-level difference from C here, not a porting choice),
so this port clamps where the C wraps. Not chased into a bit-for-bit
wrap reproduction - the golden case switches to `-v2` (`together`
normalization, which unconditionally rescales under `1.0`) instead,
sidestepping the divergence rather than replicating it, since it only
manifests when the *un-normalized* mix genuinely clips and no other
ported tool has needed wraparound-accurate integer conversion. With that
one flag changed, the real residual error (the two segments' own
convolution/FFT floating-point noise, summed) measured `~0.00046`
absolute - comfortably inside the `0.002` `sample`-tolerance precedent
carried over from `irconvolver`'s own golden case.

**Takeaway**: a wildly large, near-uncorrelated error across an
otherwise-matching buffer is itself a signal to look for a single
outlier sample rather than a systemic scale/sign error - one Python
`zip`-and-diff loop over both buffers found the exact index immediately.
And "no clipping guard on the raw write path" is a real, if narrow,
category of C/Rust divergence distinct from every lookup-table or
snapshot-timing approximation found in this project so far: not a
numerical approximation at all, just two languages disagreeing on what
an out-of-range cast should do.
