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
