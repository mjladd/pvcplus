# pvc delayfilter

Per-bin time-delay resynthesis. This command reads a source `.pva` file. For each frequency bin, it fetches that bin's source frame from a bin-specific point earlier in the source's own timeline. The per-bin delay times and amp multipliers come from a `groupdelaymaker`-produced response file. It ports `delayfilter`'s audio path. See `rust/crates/pvc-core/src/tools/delayfilter.rs` for what is in and out of scope, including a real `-C` flag whose own numeric value the original tool never actually uses.

## Usage

```
pvc delayfilter [OPTIONS] --delay-filter <DELAY_FILTER> <ANALYSIS> <OUTPUT>
```

## Options

### Core

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `<ANALYSIS>` | `-F` | Path to the source `.pva` analysis file to resynthesize from | required |
| `--delay-filter <DELAY_FILTER>` | `-B` | Path to the group-delay response file, a `groupdelaymaker`-shaped `.fr` file of per-bin `(amp, delay-seconds)` pairs | required |
| `--window-size <WINDOW_SIZE>` | `-M` | Analysis and resynthesis window length. `0` means auto (`2 * fft`), using the source file's own FFT size. There is no independent FFT-size flag | 0 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | `-D` | Output frames per second. This sets the hop size. Any value under `32` resets to `200`, matching the original tool's own safety clamp | 200 |
| `--duration <DURATION>` | `-d` | Output duration in seconds. `0` means to use the source file's own analysis duration, before this command adds the delay tail | 0 |
| `--delay-time-scaler <DELAY_TIME_SCALER>` | `-x` | Scales how much of each bin's own delay this command applies to that bin's control-function lookups in time. `0` applies no shift, `1` applies the full shift. Give a plain number, or `@path` | 0 |

### Source navigation

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--pitch <PITCH>` | `-P` | Pitch shift in semitones. Give a plain number, or `@path`. A nonzero or time-varying value, together with `--freq-shift`, picks oscillator-bank resynthesis. Leaving both at `0` picks overlap-add instead | 0 |
| `--freq-shift <FREQ_SHIFT>` | `-a` | Frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--gain <GAIN>` | `-A` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--time-origin <TIME_ORIGIN>` | `-Q` | Source time-position origin in seconds. Give a plain number, or `@path` | 0 |
| `--rate <RATE>` | `-Y` | Source navigation rate multiplier. Give a plain number, or `@path` | 1 |

### Group-delay response shaping

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--delay-transpose <DELAY_TRANSPOSE>` | `-b` | Pitch transposition of the group-delay response, in semitones. Give a plain number, or `@path` | 0 |
| `--delay-shift <DELAY_SHIFT>` | `-e` | Frequency shift of the group-delay response, in Hz. Give a plain number, or `@path` | 0 |
| `--delay-warpshape <DELAY_WARPSHAPE>` | `-w` | Warp index for reshaping the group-delay response. Truncated to an integer, matching the original tool's own cast on this one flag | 0 |
| `--zero-delay-gain <ZERO_DELAY_GAIN>` | `-V` | Decibel gain applied to a bin with zero delay. Give a plain number, or `@path` | 0 |
| `--max-delay-gain <MAX_DELAY_GAIN>` | `-y` | Decibel gain applied to a bin at the maximum delay. Give a plain number, or `@path` | 0 |
| `--delay-gain-curve <DELAY_GAIN_CURVE>` | `-z` | Curve shape, a warp index, for the zero-to-max delay gain interpolation. Give a plain number, or `@path` | 0 |
| `--delay-window <DELAY_WINDOW>` | `-T` | Multiplier applied to every bin's own delay time. Also sizes the output's extra ring-out tail. Give a plain number, or `@path`. The original tool's own usage text claims a default of `1`, but its real initializer sets `0`. This command matches the real initializer | 0 |

### Dynamics, EQ, and threshold

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--comp-threshold <COMP_THRESHOLD>` | `-E` | Input spectrum compression threshold in dB, `0` or lower | 0 |
| `--comp-db <COMP_DB>` | `-c` | Input spectrum decibels of compression, `0` or lower | 0 |
| `--release <RELEASE>` | `-L` | Output envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--attack <ATTACK>` | `-l` | Output envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--freq-response-time <FREQ_RESPONSE_TIME>` | `-f` | Output frequency-change response time in seconds. Give a plain number, or `@path` | 0 |
| `--warp <WARP>` | `-W` | Input spectrum magnitude warp index. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | `-H` | Low shelf EQ gain in dB | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | `-X` | High shelf EQ gain in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | `-m` | Low shelf EQ frequency in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | `-R` | High shelf EQ frequency in Hz | 2000 |
| `--threshold <THRESHOLD>` | `-t` | Oscillator resynthesis threshold in dB | -60 |

## Example

```
pvc analyze input.wav input.pva
pvc fn response groupdelaymaker --analysis input.pva --partials delays.txt delays.fr
pvc delayfilter --delay-filter delays.fr input.pva output.wav
```
