# pvc tvfiltdeviator

`pvc tvfilter`'s own time-varying cross-synthetic filter. It adds a per-bin, response-shaped time delay into the source's own delay line. It also adds a per-bin frequency deviation shaped by the response. That last part is the same "response-correlated frequency deviation" `pvc filtdeviator` has. It ports `tvfiltdeviator`'s audio path. See `rust/crates/pvc-core/src/tools/tvfiltdeviator.rs` for what is in and out of scope, including the same filter-fetch stride bug already found and reproduced in `pvc convolver`.

## Usage

```
pvc tvfiltdeviator [OPTIONS] --filter-response <FILTER_RESPONSE> <INPUT> <OUTPUT>
```

## Options

### Core

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--fft <FFT>` | `-N` | FFT size. Must be a power of two. Independent of the filter response file's own FFT size | 1024 |
| `--window-size <WINDOW_SIZE>` | `-M` | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 0 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | `-D` | Frames per second. This sets the hop size | 200 |
| `--time-factor <TIME_FACTOR>` | `-I` | Time expansion or contraction factor. `1.0` leaves duration unchanged | 1 |
| `--begin <BEGIN>` | `-b` | Begin time in seconds. This trims real samples | 0 |
| `--end <END>` | `-e` | End time in seconds. `0` means the end of the file | 0 |

### Time-varying filter navigation

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--filter-response <FILTER_RESPONSE>` | `-F` | Path to the time-varying filter response file, a `.pva` analysis file in either the legacy layout or `pvc analyze`'s own format | required |
| `--analysis-channel <ANALYSIS_CHANNEL>` | `-K` | Which filter-file channel to use. `0` pairs by channel index with the input sound file | 0 |
| `--pitch <PITCH>` | `-P` | Pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--freq-shift <FREQ_SHIFT>` | `-a` | Frequency shift adder in Hz. Give a plain number, or `@path` | 0 |
| `--gain <GAIN>` | `-A` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--filter-pitch-mode <FILTER_PITCH_MODE>` | `-B` | Whether the filter's own transpose or shift compensates for `--pitch` and `--freq-shift` | source-only |
| `--invert-mode <INVERT_MODE>` | `-q` | Filtering method | pass |
| `--time-origin <TIME_ORIGIN>` | `-Q` | Filter time point origin in seconds. Give a plain number, or `@path` | 0 |
| `--rate <RATE>` | `-Y` | Filter rate multiplier. Give a plain number, or `@path` | 1 |
| `--window-low <WINDOW_LOW>` | `-g` | Filter time window low boundary in seconds. Give a plain number, or `@path` | 0 |
| `--window-high <WINDOW_HIGH>` | `-G` | Filter time window high boundary in seconds. A negative value means the end of the filter file. Give a plain number, or `@path` | -1 |
| `--loop-mode <LOOP_MODE>` | `-x` | Sampler-loop boundary behavior. Only used outside autostop mode | wrap |
| `--onset-release` | `-v` | Trigger the time window only once it is first entered | off |
| `--autostop` | `-d` | Stop synthesis once the filter's time position exits its window, instead of looping | off |
| `--loop-normalization` | `-u` | Keep the filter's own amplitude roughly continuous across a sampler loop's seam | off |
| `--loop-smooth <LOOP_SMOOTH>` | `-~` | Peak loop-seam smoothing time in seconds. Give a plain number, or `@path` | 0.2 |

### Filter shaping

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--comp-threshold <COMP_THRESHOLD>` | `-E` | Filter-spectrum compression threshold in dB, below `0`. Give a plain number, or `@path` | 0 |
| `--comp-db <COMP_DB>` | `-c` | Filter-spectrum decibels of compression, below `0`. Give a plain number, or `@path` | 0 |
| `--filter-transpose <FILTER_TRANSPOSE>` | `-T` | Filter response transposition in semitones. Give a plain number, or `@path` | 0 |
| `--filter-shift <FILTER_SHIFT>` | `-V` | Filter response shift in Hz. Give a plain number, or `@path` | 0 |
| `--filter-release <FILTER_RELEASE>` | `-Z` | Filter envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--filter-attack <FILTER_ATTACK>` | `-z` | Filter envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--filter-source-gain <FILTER_SOURCE_GAIN>` | `-S` | Blend between the filtered signal and the dry source, in dB. `-96` is fully filtered. `0` bypasses the filter entirely. Give a plain number, or `@path` | -96 |
| `--filter-warpshape <FILTER_WARPSHAPE>` | `-W` | Filter response warp index. Give a plain number, or `@path` | 0 |
| `--filter-smoothing <FILTER_SMOOTHING>` | `-f` | Filter response smoothing bandwidth. Negative values are octaves, positive values are Hz. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | | Low shelf EQ gain in dB | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | | High shelf EQ gain in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | | Low shelf EQ frequency in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | | High shelf EQ frequency in Hz | 2000 |
| `--attack <ATTACK>` | `-l` | Amplitude attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | `-L` | Amplitude release time in seconds. Give a plain number, or `@path` | 0 |

### Delay line

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--time-delay-base <TIME_DELAY_BASE>` | `-/a` | Time delay base in seconds. Give a plain number, or `@path` | 0 |
| `--time-delay-peak <TIME_DELAY_PEAK>` | `-:` | Time delay peak in seconds. Give a plain number, or `@path` | 0 |
| `--time-delay-control <TIME_DELAY_CONTROL>` | `-@` | Time delay master control, between `0` and `1`. Give a plain number, or `@path` | 1 |
| `--time-delay-warp <TIME_DELAY_WARP>` | `-//` | Time delay response warp index. Give a plain number, or `@path` | 0 |

### Frequency deviation

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--freq-dev-base <FREQ_DEV_BASE>` | `-h` | Base frequency deviation in semitones. Give a plain number, or `@path` | 0 |
| `--freq-dev-peak <FREQ_DEV_PEAK>` | `-j` | Peak frequency deviation in semitones. Give a plain number, or `@path` | 0 |
| `--freq-shift-dev-base <FREQ_SHIFT_DEV_BASE>` | `-J` | Base frequency deviation shift in Hz. Give a plain number, or `@path` | 0 |
| `--freq-shift-dev-peak <FREQ_SHIFT_DEV_PEAK>` | `-k` | Peak frequency deviation shift in Hz. Give a plain number, or `@path` | 0 |
| `--freq-dev-control <FREQ_DEV_CONTROL>` | `-O` | Frequency deviation master control, between `0` and `1`. Give a plain number, or `@path` | 0 |
| `--freq-dev-mode <FREQ_DEV_MODE>` | `-U` | Frequency deviation mode: `0` for response-driven, the default, or `@path` to a table for file mode. The original tool's random mode is not supported here | 0 |
| `--freq-dev-response <FREQ_DEV_RESPONSE>` | `-y` | Frequency deviation response time in seconds, file mode only. Give a plain number, or `@path` | 0 |
| `--freq-dev-warp <FREQ_DEV_WARP>` | `-n` | Frequency deviation response warp index. Give a plain number, or `@path` | 0 |

### Threshold

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--threshold <THRESHOLD>` | `-t` | Oscillator resynthesis threshold in dB | -60 |

## Example

```
pvc tvfiltdeviator --filter-response filter.pva --time-delay-peak 0.03 input.wav output.wav
```
