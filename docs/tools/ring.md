# pvc ring

Phase-vocoder feedback reverberator and resonator. This is a delay network built from spectral frames instead of time-domain samples. It combines an independently pitch or frequency-shiftable copy of the dry source with a recirculating, equalized, envelope-gated feedback path. It ports `ring`'s audio path. See `rust/crates/pvc-core/src/tools/ring.rs` for real dead flags, a real swapped-default bug reproduced faithfully, and what is out of scope.

## Usage

```
pvc ring [OPTIONS] <INPUT> <OUTPUT>
```

## Options

### Core

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--fft <FFT>` | | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 0 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | | Frames per second. This sets the hop size | 200 |
| `--time-factor <TIME_FACTOR>` | | Time expansion or contraction factor. `1.0` leaves duration unchanged | 1 |
| `--begin <BEGIN>` | `-b` | Begin time in seconds | 0 |
| `--end <END>` | `-e` | End time in seconds. `0` means the end of the file | 0 |
| `--oscbank-threshold <OSCBANK_THRESHOLD>` | `-t` | Oscillator-bank resynthesis threshold, in dB | -96 |

### Source and master gain

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--master-gain <MASTER_GAIN>` | `-A` | Master gain, source plus reverb, in dB. Give a plain number, or `@path` | 0 |
| `--source-gain <SOURCE_GAIN>` | `-S` | Source gain in dB. Give a plain number, or `@path` | 0 |
| `--source-freq-shift <SOURCE_FREQ_SHIFT>` | `-f` | Source frequency shift adder, in Hz. Give a plain number, or `@path` | 0 |
| `--source-pitch <SOURCE_PITCH>` | `-p` | Source pitch transposition in semitones. Give a plain number, or `@path` | 0 |

### Feedback path

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--feedback-gain <FEEDBACK_GAIN>` | `-F` | Reverb (feedback) gain in dB. Give a plain number, or `@path` | 0 |
| `--feedback-freq-shift <FEEDBACK_FREQ_SHIFT>` | `-H` | Reverb frequency shift adder, in Hz. Give a plain number, or `@path` | 0 |
| `--feedback-pitch <FEEDBACK_PITCH>` | `-P` | Reverb pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--feedback-decay <FEEDBACK_DECAY>` | `-Z` | Reverb feedback delay line decay time, in seconds. `0` disables the loop entirely. Give a plain number, or `@path` | 0 |
| `--feedback-threshold <FEEDBACK_THRESHOLD>` | `-z` | Reverb input envelope-follower gate threshold, in dB. Give a plain number, or `@path` | -96 |
| `--feedback-threshold-mode <FEEDBACK_THRESHOLD_MODE>` | `-V` | Reverb threshold pass mode | above |
| `--attack <ATTACK>` | `-l` | Reverb input envelope attack time, in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | `-L` | Reverb input envelope release time, in seconds. Give a plain number, or `@path` | 0 |

### Input, loop, and output EQ

Each of these three stages shares the same shape: a low shelf gain and frequency, and a high shelf gain and frequency.

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--input-eq-low-gain <INPUT_EQ_LOW_GAIN>` | `-O` | Reverb input EQ low shelf gain, in dB. Give a plain number, or `@path` | 200 |
| `--input-eq-high-gain <INPUT_EQ_HIGH_GAIN>` | `-Y` | Reverb input EQ high shelf gain, in dB. Give a plain number, or `@path` | 0 |
| `--input-eq-low-freq <INPUT_EQ_LOW_FREQ>` | `-d` | Reverb input EQ low shelf frequency, in Hz. Give a plain number, or `@path` | 0 |
| `--input-eq-high-freq <INPUT_EQ_HIGH_FREQ>` | `-n` | Reverb input EQ high shelf frequency, in Hz. Give a plain number, or `@path` | 2000 |
| `--loop-eq-decay <LOOP_EQ_DECAY>` | `-T` | Reverb in-loop feedback EQ decay time, in seconds. Give a plain number, or `@path` | 1 |
| `--loop-balance-limit <LOOP_BALANCE_LIMIT>` | `-E` | Reverb feedback signal balance gain limiter level, from `0` to `96` dB. `0` disables balancing entirely | 0 |
| `--loop-eq-low-gain <LOOP_EQ_LOW_GAIN>` | `-X` | Reverb in-loop feedback EQ low shelf gain, in dB. Give a plain number, or `@path` | 200 |
| `--loop-eq-high-gain <LOOP_EQ_HIGH_GAIN>` | `-Q` | Reverb in-loop feedback EQ high shelf gain, in dB. Give a plain number, or `@path` | 0 |
| `--loop-eq-low-freq <LOOP_EQ_LOW_FREQ>` | `-U` | Reverb in-loop feedback EQ low shelf frequency, in Hz. Give a plain number, or `@path` | 0 |
| `--loop-eq-high-freq <LOOP_EQ_HIGH_FREQ>` | `-m` | Reverb in-loop feedback EQ high shelf frequency, in Hz. Give a plain number, or `@path` | 2000 |
| `--output-eq-low-gain <OUTPUT_EQ_LOW_GAIN>` | `-k` | Reverb output EQ low shelf gain, in dB. Give a plain number, or `@path` | 200 |
| `--output-eq-high-gain <OUTPUT_EQ_HIGH_GAIN>` | `-c` | Reverb output EQ high shelf gain, in dB. Give a plain number, or `@path` | 0 |
| `--output-eq-low-freq <OUTPUT_EQ_LOW_FREQ>` | `-s` | Reverb output EQ low shelf frequency, in Hz. Give a plain number, or `@path` | 0 |
| `--output-eq-high-freq <OUTPUT_EQ_HIGH_FREQ>` | `-G` | Reverb output EQ high shelf frequency, in Hz. Give a plain number, or `@path` | 2000 |

## Notes

Every low-shelf-gain default above reads `200`, and every low-shelf-frequency default reads `0`. That is not a typo in this table. It is a real bug in the original C, reproduced here on purpose. `rust/crates/pvc-core/src/tools/ring.rs` documents it as a swapped default across all three EQ stages.

## Example

```
pvc ring --feedback-decay 4 --feedback-gain -6 input.wav output.wav
```
