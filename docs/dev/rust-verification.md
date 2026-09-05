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
