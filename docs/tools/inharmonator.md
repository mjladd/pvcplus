# pvc inharmonator

Inharmonic partials remapper. This command builds a per-bin filter from a data table of discrete target partials, each a windowed band around its own bin. It resynthesizes the residual spectrum separately, then feeds both through a spectral feedback delay line. It can optionally mix in a delayed copy of the source. It ports `inharmonator`. See `rust/crates/pvc-core/src/tools/inharmonator.rs` for what is in and out of scope. That file documents a real, severe bug: seven data-modifier scalers are used uninitialized in the original C unless their own flag is passed. This command uses the original tool's documented defaults instead. It also documents a real finding that "master gain" only ever affects the source signal, never the resynthesized partials.

## Usage

```
pvc inharmonator [OPTIONS] --partials <PARTIALS> <INPUT> <OUTPUT>
```

## Options

### Core

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--partials <PARTIALS>` | `-F` | Path to the partials data table. Each row has 5 whitespace-separated columns: partial number, shift target, decibels, delay time, and feedback decay time | required |
| `--fft <FFT>` | `-N` | FFT size | 1024 |
| `--window-size <WINDOW_SIZE>` | `-M` | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | `-D` | Frames per second. This sets the hop size. Any value under `32` resets to `200` | 200 |
| `--time-factor <TIME_FACTOR>` | `-I` | Time expansion or contraction factor. Any value at or below `0` resets to `1.0` | 1 |
| `--begin <BEGIN>` | `-b` | Begin time in seconds. This trims real samples | 0 |
| `--end <END>` | `-e` | End time in seconds. `0` means the end of the file | 0 |
| `--channel <CHANNEL>` | `-C` | Which input channel to resynthesize. `0` processes every channel independently. A number of `1` or higher resynthesizes only that one channel | 0 |
| `--method <METHOD>` | `-Z` | How this command reads each partial's own shift column | multiplier |
| `--partial-window <PARTIAL_WINDOW>` | `-B` | How the bins between a partial's own bin and its band edges taper | welch |
| `--partial-bandwidth <PARTIAL_BANDWIDTH>` | `-x` | Each target partial's own bandwidth, in partial-number units. Give a plain number, or `@path` | 1 |
| `--fundamental <FUNDAMENTAL>` | `-f` | Fundamental frequency, in Hz above `12` or octave.pitchclass at `12` or below. Give a plain number, or `@path` | 60 |

### Master, target, and non-target

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--master-gain <MASTER_GAIN>` | `-A` | Master gain in decibels. Give a plain number, or `@path`. This only ever affects the source signal, never the resynthesized partials or non-targets | 0 |
| `--attack <ATTACK>` | `-v` | Envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | `-V` | Envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--warpshape <WARPSHAPE>` | `-W` | Target spectrum warpshape index. Give a plain number, or `@path` | 0 |
| `--target-freq-shift <TARGET_FREQ_SHIFT>` | `-q` | Target frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--target-pitch <TARGET_PITCH>` | `-X` | Target pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--target-gain <TARGET_GAIN>` | `-m` | Target gain in decibels. Give a plain number, or `@path` | 0 |
| `--target-amp-interp <TARGET_AMP_INTERP>` | `-U` | Target amplitude interpolation control, between `0` and `1`. Give a plain number, or `@path` | 1 |
| `--target-freq-interp <TARGET_FREQ_INTERP>` | `-S` | Target frequency interpolation control, between `0` and `1`. Give a plain number, or `@path` | 1 |
| `--target-time-interp <TARGET_TIME_INTERP>` | `-T` | Target time-delay interpolation control, between `0` and `1`. Give a plain number, or `@path` | 1 |
| `--non-target-freq-shift <NON_TARGET_FREQ_SHIFT>` | `-a` | Non-target, residual-spectrum, frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--non-target-pitch <NON_TARGET_PITCH>` | `-P` | Non-target pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--non-target-gain <NON_TARGET_GAIN>` | `-G` | Non-target gain in decibels. Give a plain number, or `@path` | 0 |
| `--non-target-delay <NON_TARGET_DELAY>` | `-j` | Non-target delay time in seconds. Give a plain number, or `@path` | 0 |
| `--non-target-decay <NON_TARGET_DECAY>` | `-E` | Non-target feedback decay time in seconds. Give a plain number, or `@path` | 0 |

### Source mixing

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--source-freq-shift <SOURCE_FREQ_SHIFT>` | `-Q` | Source frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--source-pitch <SOURCE_PITCH>` | `-u` | Source pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--source-gain <SOURCE_GAIN>` | `-r` | Source gain in decibels. Give a plain number, or `@path`. Any value above `--threshold`, or a time-varying one, turns source mixing on entirely | 0 |
| `--source-attack <SOURCE_ATTACK>` | `-l` | Source envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--source-release <SOURCE_RELEASE>` | `-L` | Source envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--source-delay <SOURCE_DELAY>` | `-J` | Source delay time in seconds. Give a plain number, or `@path` | 0 |

### Partials table scaling

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--partial-number-scale <PARTIAL_NUMBER_SCALE>` | `-z` | Scaler applied to the data table's own partial-number column | 1 |
| `--partial-number-shift <PARTIAL_NUMBER_SHIFT>` | `-R` | Shifter added to the partial-number column, after scaling | 0 |
| `--partial-db-scale <PARTIAL_DB_SCALE>` | `-y` | Scaler applied to the data table's own decibels column | 1 |
| `--partial-delay-scale <PARTIAL_DELAY_SCALE>` | `-o` | Scaler applied to the data table's own delay-time column | 1 |
| `--partial-delay-shift <PARTIAL_DELAY_SHIFT>` | `-O` | Shifter added to the delay-time column, after scaling | 0 |
| `--partial-decay-scale <PARTIAL_DECAY_SCALE>` | `-g` | Scaler applied to the data table's own feedback-decay-time column | 0 |
| `--partial-decay-shift <PARTIAL_DECAY_SHIFT>` | `-k` | Shifter added to the feedback-decay-time column, after scaling | 0 |

### Threshold

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--threshold <THRESHOLD>` | `-t` | Oscillator resynthesis threshold in dB. This also gates whether the source signal mixes in at all, together with `--source-gain` | -60 |

## Example

```
pvc inharmonator --partials partials.txt --fundamental 220 input.wav output.wav
```
