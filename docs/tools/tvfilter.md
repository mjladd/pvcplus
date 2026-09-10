# pvc tvfilter

Like `pvc filter`, but the response is a time-varying sequence of frames from a legacy `.pva` file. This command moves through that file over time the same way `pvc twarp` moves through its own resynthesis source, rather than reading one static `.fr` file. It ports `tvfilter`'s audio path. See `rust/crates/pvc-core/src/tools/tvfilter.rs` for what is in and out of scope, including a dead `-u` flag from the original C.

## Usage

```
pvc tvfilter [OPTIONS] --filter-response <FILTER_RESPONSE> <INPUT> <OUTPUT>
```

## Options

Each row shows the matching legacy `tvfilter` flag letter, where one exists.

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--fft <FFT>` | | FFT size. Must be a power of two. Independent of the filter response file's own FFT size | 1024 |
| `--window-size <WINDOW_SIZE>` | | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 0 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | | Frames per second. This sets the hop size | 200 |
| `--time-factor <TIME_FACTOR>` | | Time expansion or contraction factor. `1.0` leaves duration unchanged | 1 |
| `--pitch <PITCH>` | `-P` | Pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--freq-shift <FREQ_SHIFT>` | `-a` | Frequency shift adder in Hz. Give a plain number, or `@path` | 0 |
| `--gain <GAIN>` | `-A` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--filter-pitch-mode <FILTER_PITCH_MODE>` | `-B` | Whether the filter's own transpose or shift compensates for `--pitch` and `--freq-shift` | source-only |
| `--filter-response <FILTER_RESPONSE>` | `-F` | Path to the time-varying filter response file, a `.pva` analysis file in either the legacy layout or `pvc analyze`'s own format | required |
| `--analysis-channel <ANALYSIS_CHANNEL>` | `-K` | Which filter-file channel to use. `0` pairs by channel index with the input sound file | 0 |
| `--invert-mode <INVERT_MODE>` | `-q` | Filtering method | pass |
| `--time-origin <TIME_ORIGIN>` | `-Q` | Filter time point origin in seconds. Give a plain number, or `@path` | 0 |
| `--rate <RATE>` | `-Y` | Filter rate multiplier. Give a plain number, or `@path` | 1 |
| `--window-low <WINDOW_LOW>` | `-g` | Filter time window low boundary in seconds. Give a plain number, or `@path` | 0 |
| `--window-high <WINDOW_HIGH>` | `-G` | Filter time window high boundary in seconds. A negative value means the end of the filter file. Give a plain number, or `@path` | -1 |
| `--loop-mode <LOOP_MODE>` | `-o` | Sampler-loop boundary behavior. Only used outside autostop mode | wrap |
| `--onset-release` | `-r` | Trigger the time window only once it is first entered | off |
| `--autostop` | `-d` | Stop synthesis once the filter's time position exits its window, instead of looping | off |
| `--loop-normalization` | `-j` | Keep the filter's own amplitude roughly continuous across a sampler loop's seam | off |
| `--loop-smooth <LOOP_SMOOTH>` | `-k` | Peak loop-seam smoothing time in seconds. Give a plain number, or `@path` | 0.2 |
| `--comp-threshold <COMP_THRESHOLD>` | `-E` | Filter-spectrum compression threshold in dB, `0` or lower. Give a plain number, or `@path` | 0 |
| `--comp-db <COMP_DB>` | `-c` | Filter-spectrum decibels of compression, `0` or lower. Give a plain number, or `@path` | 0 |
| `--filter-transpose <FILTER_TRANSPOSE>` | `-T` | Filter response transposition in semitones. Give a plain number, or `@path` | 0 |
| `--filter-shift <FILTER_SHIFT>` | `-V` | Filter response shift in Hz. Give a plain number, or `@path` | 0 |
| `--filter-release <FILTER_RELEASE>` | `-Z` | Filter envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--filter-attack <FILTER_ATTACK>` | `-z` | Filter envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--filter-source-gain <FILTER_SOURCE_GAIN>` | `-S` | Blend between the filtered signal and the dry source, in dB. `-96` is fully filtered. `0` bypasses the filter entirely. Give a plain number, or `@path` | -96 |
| `--filter-warpshape <FILTER_WARPSHAPE>` | `-W` | Filter response warp index. Give a plain number, or `@path` | 0 |
| `--filter-smoothing <FILTER_SMOOTHING>` | `-f` | Filter response smoothing bandwidth. Negative values are octaves, positive values are Hz. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | `-H` | Filter response low shelf gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | `-X` | Filter response high shelf gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | `-m` | Filter response low shelf frequency in Hz. Give a plain number, or `@path` | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | `-R` | Filter response high shelf frequency in Hz. Give a plain number, or `@path` | 2000 |
| `--frame-norm-limit <FRAME_NORM_LIMIT>` | `-n` | Frame normalization decibel limit. `0` disables it. Give a plain number, or `@path` | 0 |
| `--normalize-to <NORMALIZE_TO_FILTER>` | `-v` | What frame normalization scales toward | input |
| `--attack <ATTACK>` | `-l` | Envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | `-L` | Envelope release time in seconds. Give a plain number, or `@path` | 0 |

## Example

```
pvc tvfilter --filter-response filter.pva input.wav output.wav
```
