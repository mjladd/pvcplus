# Performance: `pvc pv --stretch 2` versus legacy `plainpv`

The plan's own validation section asks for this comparison: `pvc pv --stretch 2` on a 60-second stereo file, checked against legacy `plainpv`, and recorded here.

## Method

Both tools ran inside the same `runtime` Docker image (`docker build --target runtime`), on the same host, in the same container, one after another. Running both this way removes container startup and image-pull overhead from the comparison entirely. Only the two already-running binaries are timed.

Test file: 60 seconds, 44100 Hz, stereo, two sine tones (440 Hz and 554.37 Hz), generated with Python's standard `wave` module.

Commands compared:

```bash
plainpv -I2 bench60s.wav legacy_out.wav
pvc stretch --factor 2.0 bench60s.wav pvc_out.wav
```

`plainpv -I2` and `pvc stretch --factor 2.0` use the same real defaults on both sides: a 1024-point FFT, a Hamming window, and 200 frames per second. This page checked that by reading `pvc pv`'s own `--help` output next to `plainpv`'s own usage text. Both commands produced an output file with the same frame count and the same duration, `120.047` seconds. That means the two runs did equivalent work.

Ten runs of each command, after one untimed warm-up run of each. Times are wall-clock seconds around each single invocation (`date +%s%N` before and after).

Host: `x86_64`, Linux, Docker image built for `linux/amd64`.

## Result

| | min | max | mean |
|---|---|---|---|
| `plainpv -I2` | 1.041s | 1.064s | 1.056s |
| `pvc stretch --factor 2.0` | 1.089s | 1.112s | 1.103s |

`pvc` is about 4% slower than legacy `plainpv` on this file. That does not meet the plan's own target of "at least as fast." The gap held steady across ten runs each. It also held in both run orders: once with `pvc` first, then `plainpv`, and once the other way around. This is a real, repeatable difference, not measurement noise.

## Why, and whether to chase it

Nobody has run a profiler on this gap yet. Both tools do the same oscillator-bank resynthesis. Both also do the same whole-file peak rescale pass at the end. The difference likely comes from lower-level implementation choices instead: allocation patterns, `Vec<f32>` bounds checks, or trigonometric function calls. Neither side is missing an algorithmic optimization the other one has. A 4% gap on a real workload is small. Closing it is a judgment call, not something this page decides. Whoever picks this up next: profile a real run first (`cargo flamegraph` or `perf`), rather than guessing at a fix.
