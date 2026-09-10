# pvc fn

Control-function generators. This is the CARL/cmusic "GEN" family. It builds breakpoint tables, harmonic-sum tables, and noise tables. It also includes a terminal plotter and a `.fr` frequency-response synthesizer. `pvc fn` has no behavior of its own. Every actual generator is one of its subcommands below.

## pvc fn gen1

Piecewise-linear envelope from explicit `(time, value)` breakpoints.

### Usage

```
pvc fn gen1 [OPTIONS] --length <LENGTH> --point <POINTS> <OUTPUT>
```

### Options

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Where to write the table. `.txt` writes ASCII, one value per line. Anything else writes little-endian f32 binary | required |
| `-L, --length <LENGTH>` | Output table length in samples | required |
| `--point <POINTS>` | A breakpoint as `time,value`. Repeat this flag for each point. You need at least two, for example `--point 0,0 --point 50,1 --point 100,0` | required |
| `--open` | Open curve (the default closes it) | closed |

### Example

```
pvc fn gen1 --length 1000 --point 0,0 --point 500,1 --point 1000,0 envelope.txt
```

## pvc fn gen2

Sum of sine and cosine harmonics. This command uses named `--sine`/`--cosine` lists instead of the original C's positional-count argument. The original had a real bug. A count larger than the actual number of coefficients given read uninitialized memory. This CLI cannot reproduce that bug.

### Usage

```
pvc fn gen2 [OPTIONS] --length <LENGTH> <OUTPUT>
```

### Options

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Output table path | required |
| `-L, --length <LENGTH>` | Output table length in samples | required |
| `--sine <SINE>` | Sine harmonic amplitudes, lowest harmonic first (harmonic 1, 2, ...) | none |
| `--cosine <COSINE>` | Cosine harmonic amplitudes, first value is harmonic 0 (DC) | none |
| `--closed` | Closed curve (gen2's own default is open, unlike gen1/gen3/gen4/gen5) | open |

### Example

```
pvc fn gen2 --length 1000 --sine 1.0,0.5,0.25 harmonics.txt
```

## pvc fn gen3

Piecewise-linear envelope at evenly spaced breakpoints, given only their values.

### Usage

```
pvc fn gen3 [OPTIONS] --length <LENGTH> --value <VALUES> <OUTPUT>
```

### Options

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Output table path | required |
| `-L, --length <LENGTH>` | Output table length in samples | required |
| `--value <VALUES>` | A breakpoint value. Repeat this flag for each point. You need at least two | required |
| `--open` | Open curve | closed |

### Example

```
pvc fn gen3 --length 1000 --value 0 --value 1 --value 0.5 --value 0 evenly-spaced.txt
```

## pvc fn gen4

Like `gen1`, but each breakpoint carries its own transition shape: `alpha = 0` is linear, negative is exponential, positive is logarithmic.

### Usage

```
pvc fn gen4 [OPTIONS] --length <LENGTH> --point <POINTS> <OUTPUT>
```

### Options

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Output table path | required |
| `-L, --length <LENGTH>` | Output table length in samples | required |
| `--point <POINTS>` | A breakpoint as `time,value,alpha`. Repeat this flag for each point. You need at least two. The last point's alpha goes unused, because no segment follows it | required |
| `--open` | Open curve | closed |

### Example

```
pvc fn gen4 --length 1000 --point 0,0,0 --point 500,1,-2 --point 1000,0,2 shaped.txt
```

## pvc fn gen5

Sum of arbitrary `(harmonic, amplitude, phase)` partials.

### Usage

```
pvc fn gen5 [OPTIONS] --length <LENGTH> --partial <PARTIALS> <OUTPUT>
```

### Options

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Output table path | required |
| `-L, --length <LENGTH>` | Output table length in samples | required |
| `--partial <PARTIALS>` | A partial as `harmonic,amplitude,phase`. Repeat this flag for each one | required |
| `--closed` | Closed curve | open |

### Example

```
pvc fn gen5 --length 1000 --partial 1,1.0,0 --partial 3,0.3,0 partials.txt
```

## pvc fn gen6

Uniform noise in `[-1.0, 1.0)`. This command uses the exact `rand()` sequence the original C gets by never seeding one. That is glibc's default state, the same state a call to `srandom(1)` produces. The result is deterministic, not a fresh random table on every run.

### Usage

```
pvc fn gen6 [OPTIONS] --length <LENGTH> <OUTPUT>
```

### Options

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Output table path | required |
| `-L, --length <LENGTH>` | Output table length in samples | required |

### Example

```
pvc fn gen6 --length 4410 noise.txt
```

## pvc fn plot

Plots a control/data file as a terminal sparkline. This replaces the legacy `showme`/`showmeb`/`showmed` family. Their actual job was to convert a binary or ASCII float file to something plottable, then hand it to `gnuplot`. This command needs no `gnuplot` dependency.

### Usage

```
pvc fn plot [OPTIONS] <INPUT>
```

### Options

| Flag | Description | Default |
|---|---|---|
| `<INPUT>` | An ASCII (`.txt`) or binary f32 control/data file | required |
| `--width <WIDTH>` | Number of columns to downsample to | 120. If the file is shorter, uses the file's own length instead |

### Example

```
pvc fn plot envelope.txt
```

## pvc fn response

Synthesizes a `.fr` frequency-response file from a breakpoint or partial table, rather than analyzing a sound file (`pvc freqresponse` does that instead). Has three subcommands.

### pvc fn response filtresponsemaker

A "frequency gradient" response, linearly interpolated in dB between unordered `(frequency-or-octave.pitchclass, decibels)` breakpoints.

```
pvc fn response filtresponsemaker [OPTIONS] --breakpoints <BREAKPOINTS> --target-sound-file <TARGET_SOUND_FILE> <OUTPUT>
```

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Output `.fr` path | required |
| `--breakpoints <BREAKPOINTS>` | ASCII data file of unordered breakpoint duples: each line (or whitespace-separated pair) is `freq-or-octave.pitchclass, decibels`. Values `<= 12` are read as octave.pitchclass, otherwise Hz | required |
| `--target-sound-file <TARGET_SOUND_FILE>` | Sound file to take the sample rate from | required |
| `--fft <FFT>` | FFT size (must be a power of two) | 1024 |
| `--mode` | Bandpass (keep the breakpoint shape) or band-reject (invert it: `1.0 - amplitude` at every bin) | bandpass |

Example:

```
pvc fn response filtresponsemaker --breakpoints gradient.txt --target-sound-file input.wav response.fr
```

### pvc fn response chordresponsemaker

A stack of harmonic-partial tones, each with a triangular- or rectangular-windowed dB rolloff around its center frequency, from unordered sextuples `(pitch-or-Hz, num_partials, bandwidth, decibels, partial_spacing, db_rolloff_per_octave)`.

```
pvc fn response chordresponsemaker [OPTIONS] --partials <PARTIALS> <OUTPUT>
```

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Output `.fr` path | required |
| `--partials <PARTIALS>` | ASCII data file of unordered sextuples: `pitch-or-Hz, num_partials, bandwidth, decibels, partial_spacing, db_rolloff_per_octave` (whitespace-separated) | required |
| `--fft <FFT>` | FFT size (must be a power of two) | 1024 |
| `--sample-rate <SAMPLE_RATE>` | Sample rate in Hz. The original tool took this from a sound file it opened only to read the sample rate, without documenting that requirement. This CLI takes the number directly instead | 44100 |
| `--accumulation <ACCUMULATION>` | How overlapping partial windows combine at a shared bin | peak |
| `--band-window <BAND_WINDOW>` | The dB rolloff shape around each partial | triangle |
| `--mode` | Bandpass (keep the tone shape) or band-reject (invert it) | bandpass |

Example:

```
pvc fn response chordresponsemaker --partials chord.txt response.fr
```

### pvc fn response groupdelaymaker

Like `chordresponsemaker`, but each bin's response is an `(amp, delay-time)` pair instead of `(amp, frequency)`, from unordered septuples `(pitch-or-Hz, num_partials, bandwidth, decibels, partial_spacing, db_rolloff_total, delay_secs)`.

```
pvc fn response groupdelaymaker [OPTIONS] --analysis <ANALYSIS> --partials <PARTIALS> <OUTPUT>
```

| Flag | Description | Default |
|---|---|---|
| `<OUTPUT>` | Output `.fr` path | required |
| `--analysis <ANALYSIS>` | A `.pva` analysis file (legacy or `pvc analyze`'s own format), used only to adopt its FFT size and sample rate | required |
| `--partials <PARTIALS>` | ASCII data file of unordered septuples: `pitch-or-Hz, num_partials, bandwidth, decibels, partial_spacing, db_rolloff_total, delay_secs` (whitespace-separated) | required |
| `--edge-db <EDGE_DB>` | dB offset, relative to each partial's own level, at the outer edge of its band. `0` is a flat, effectively rectangular band, not silence at the edges | 0 |
| `--default-db <DEFAULT_DB>` | Default decibel level for bins no tone ever touches | 0 |
| `--default-delay <DEFAULT_DELAY>` | Default delay time, in seconds, for bins no tone ever touches | 0 |
| `--method <METHOD>` | How overlapping partials resolve their `(amp, delay)` pair at a shared bin | average |

Example:

```
pvc fn response groupdelaymaker --analysis input.pva --partials delays.txt response.fr
```

## Notes

`rust/crates/pvc-core/src/tools/groupdelaymaker.rs` documents the real differences from `chordresponsemaker`'s own dB-rolloff and edge-falloff formulas. If you port a script that used both tools, read that file first.
