# pvc ratechanger

Varispeed resampler. This command reads raw audio directly, with no `.pva` analysis file first, and resynthesizes it at a possibly time-varying playback rate, semitone shift, or both. It offers windowed-sinc summation, sample-and-hold or decimation, or linear interpolation. It ports `ratechanger`. See `rust/crates/pvc-core/src/tools/ratechanger.rs` for what is in and out of scope, including a real, verified heap-buffer-overflow in the original C's own default sinc-table lookup. This command clamps that lookup safely instead of reproducing the overflow.

## Usage

```
pvc ratechanger [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--synthesis-mode <SYNTHESIS_MODE>` | `-X` | Per-sample synthesis method | sinc |
| `--table-lookup <TABLE_LOOKUP>` | `-t` | Use the Blackman-windowed sinc lookup table, which is fast, instead of computing sine and cosine directly, which is slow. Only affects `--synthesis-mode sinc` | on |
| `--pi-interpolation-points <PI_INTERPOLATION_POINTS>` | `-L` | Lookup-table interpolation points per sinc period | 100 |
| `--truncation-db <TRUNCATION_DB>` | `-B` | Sinc window truncation level in dB, below `0`. Typically `-30` to `-96` | -60 |
| `--duration <DURATION>` | `-d` | Output duration in seconds. `0` triggers output-duration synthesis regardless of `--synthesize-duration` | 0 |
| `--synthesize-duration` | `-D` | Search for an output duration that lines up the end of the rate-change control functions with the end of the output. This uses an iterative convergence algorithm. It is always on once `--duration` is `0` or unset, no matter this flag's own value | off |
| `--channel <CHANNEL>` | `-C` | Input channel to process, 1-based. `0` processes every input channel | 0 |
| `--normalize <NORMALIZE>` | `-n` | Post-synthesis normalization | input |
| `--time-origin <TIME_ORIGIN>` | `-O` | Source time-position origin in seconds. Give a plain number, or `@path` | 0 |
| `--rate-in <RATE_IN>` | `-r` | Rate multiplier, as a function of input sound time. Give a plain number, or `@path` | 1 |
| `--rate-out <RATE_OUT>` | `-R` | Rate multiplier, as a function of output sound time. Give a plain number, or `@path` | 1 |
| `--semitones-in <SEMITONES_IN>` | `-s` | Semitone pitch shift, as a function of input sound time. Give a plain number, or `@path` | 0 |
| `--semitones-out <SEMITONES_OUT>` | `-S` | Semitone pitch shift, as a function of output sound time. Give a plain number, or `@path` | 0 |
| `--gain-in <GAIN_IN>` | `-a` | Amplitude envelope in dB, as a function of input sound time. Give a plain number, or `@path` | 0 |
| `--gain-out <GAIN_OUT>` | `-A` | Amplitude envelope in dB, as a function of output sound time. Give a plain number, or `@path` | 0 |
| `--window-low <WINDOW_LOW>` | `-b` | Input time window low boundary in seconds. Give a plain number, or `@path` | 0 |
| `--window-high <WINDOW_HIGH>` | `-e` | Input time window high boundary in seconds. `0` or below means the end of the input. Give a plain number, or `@path` | -1 |
| `--new-sample-rate <NEW_SAMPLE_RATE>` | `-m` | Impose a new sample rate on the output header. `0` or below keeps the input's own sample rate. This does not resample. It only changes the output file's declared rate | 0 |

## Example

```
pvc ratechanger --rate-in 0.5 input.wav output.wav
```
