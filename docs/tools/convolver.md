# pvc convolver

Short-term FFT spectral multiplier. This command convolves a live input, called "Sound A", against a pre-analyzed `.pva` filter file, called "Sound B". It moves through Sound B over time. This works the same way `pvc tvfilter` and `pvc twarp` move through their own sources. It then pans between the dry sounds and their convolution. It ports `convolver`'s audio path. See `rust/crates/pvc-core/src/tools/convolver.rs` for a real "spectral multiplication" bug and a real uninitialized-memory bug, not exposed here.

## Usage

```
pvc convolver [OPTIONS] --filter-response <FILTER_RESPONSE> <INPUT> <OUTPUT>
```

## Options

### Core

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--window-size <WINDOW_SIZE>` | `-M` | Analysis and resynthesis window length. `0` means auto (`2 * fft`), using Sound B's own analysis file's FFT size. There is no independent FFT-size flag | 0 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--time-factor <TIME_FACTOR>` | `-I` | Time expansion or contraction factor. `1.0` leaves duration unchanged | 1 |
| `--begin <BEGIN>` | `-b` | Begin time in seconds. This trims real samples, not just bookkeeping | 0 |
| `--end <END>` | `-e` | End time in seconds. `0` means the end of the file | 0 |
| `--pitch <PITCH>` | `-P` | Pitch transposition of the output spectrum, in semitones. Give a plain number, or `@path`. Any nonzero value, together with `--freq-shift`, selects oscillator-bank resynthesis | 0 |
| `--freq-shift <FREQ_SHIFT>` | `-a` | Frequency shift of the output spectrum, in Hz. Give a plain number, or `@path` | 0 |
| `--gain <GAIN>` | `-A` | Output gain in dB. Give a plain number, or `@path` | 0 |
| `--threshold <THRESHOLD>` | `-t` | Oscillator-bank resynthesis threshold in dB | -96 |

### Sound B navigation

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--filter-response <FILTER_RESPONSE>` | `-F` | Path to Sound B's analysis file, the filter to convolve against. This is a `.pva` file, in either the legacy layout or `pvc analyze`'s own format | required |
| `--analysis-channel <ANALYSIS_CHANNEL>` | `-K` | Which filter-file channel to use. `0` pairs by channel index with the input sound file | 0 |
| `--filter-time-origin <FILTER_TIME_ORIGIN>` | `-Q` | Sound B time point origin, in seconds. Give a plain number, or `@path` | 0 |
| `--filter-rate <FILTER_RATE>` | `-Y` | Sound B rate multiplier. Give a plain number, or `@path` | 1 |
| `--filter-window-low <FILTER_WINDOW_LOW>` | `-g` | Sound B time window lower boundary, in seconds. Give a plain number, or `@path` | 0 |
| `--filter-window-high <FILTER_WINDOW_HIGH>` | `-G` | Sound B time window upper boundary, in seconds. A negative value means the end of the filter file. Give a plain number, or `@path` | -1 |
| `--loop-mode <LOOP_MODE>` | `-o` | Sound B time window out-of-bounds behavior. Only used outside autostop mode | wrap |
| `--onset-release` | `-r` | Trigger the time window only once it is first entered | off |
| `--autostop` | `-y` | Stop synthesis once Sound B's time position exits its window, instead of looping | off |

### Mixing and panning

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--sound-a-gain <SOUND_A_GAIN>` | `-q` | Sound A's own gain, in dB. Give a plain number, or `@path` | 0 |
| `--sound-b-gain <SOUND_B_GAIN>` | `-B` | Sound B's own gain, in dB. Give a plain number, or `@path` | 0 |
| `--convolve-gain <CONVOLVE_GAIN>` | `-Z` | Convolution gain, in dB. Give a plain number, or `@path` | 0 |
| `--pan <PAN>` | `-S` | Pan position between Sound A, Sound B, and their convolution. `-1` is Sound A, `1` is Sound B, `0` is the convolution. Give a plain number, or `@path` | 0 |
| `--panwarp-a <PANWARP_A>` | `-j` | Pan-position domain warp on the Sound A side, for `pan < 0` | 0 |
| `--panwarp-b <PANWARP_B>` | `-J` | Pan-position domain warp on the Sound B side, for `pan >= 0` | 0 |

### Output shaping

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--shelf-low-gain <SHELF_LOW_GAIN>` | `-H` | Convolution output low shelf gain, in dB. Fixed for the whole run, not a control-file path | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | `-X` | Convolution output high shelf gain, in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | `-m` | Convolution output low shelf frequency, in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | `-R` | Convolution output high shelf frequency, in Hz | 2000 |
| `--frame-norm-limit <FRAME_NORM_LIMIT>` | `-n` | Frame normalization decibel limit. `0` disables normalization. Give a plain number, or `@path` | 0 |
| `--normalize-to <NORMALIZE_TO_FILTER>` | `-v` | What frame normalization scales toward | input |

## Example

```
pvc analyze filter_source.wav filter.pva
pvc convolver --filter-response filter.pva --pan 0 input.wav output.wav
```
