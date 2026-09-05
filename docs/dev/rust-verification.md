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
