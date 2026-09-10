# pvc irconvolvesequencer

Crossfades a signal through a sequence of impulse responses, morphing from one to the next. It is part of Phase 5's FFT-convolution family. See `rust/crates/pvc-core/src/tools/irconvolvesequencer.rs` for the original C's dead `-a` flag and what is out of scope. It ports `irconvolvesequencer`.

## Usage

```
pvc irconvolvesequencer [OPTIONS] --impulse-list-dir <IMPULSE_LIST_DIR> <INPUT> <OUTPUT>
```

## Options

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--impulse-list-dir <IMPULSE_LIST_DIR>` | `-I` | Directory containing an `impulseFileNames` list file. Its first whitespace-separated token is the impulse count, followed by that many impulse-response sound file paths, one per sequence point, in order | required |
| `--begin <BEGIN>` | `-b` | Sequence window start, in seconds | 0 |
| `--end <END>` | `-e` | Sequence window end, in seconds. `0` means the end of the file | 0 |
| `--ring-tail` | `-d` | Extend every segment's processed window by one impulse-length of trailing silence | off |
| `--impulse-channel <IMPULSE_CHANNEL>` | `-J` | Which impulse channel to use. `0` auto round-robins across channels | 0 |
| `--normalization <NORMALIZATION>` | `-v` | How the final mixed output gets peak-normalized | off |
| `--ir-low-freq <IR_LOW_FREQ>` | `-s` | Impulse response bandpass low rolloff point, in Hz, evaluated once per impulse at its sequence position. Give a plain number, or `@path` | 0 |
| `--ir-high-freq <IR_HIGH_FREQ>` | `-t` | Impulse response bandpass high rolloff point, in Hz. `0` means Nyquist. Give a plain number, or `@path` | 0 |
| `--ir-low-rolloff <IR_LOW_ROLLOFF>` | `-g` | Impulse response low rolloff width. Give a plain number, or `@path` | 0 |
| `--ir-high-rolloff <IR_HIGH_ROLLOFF>` | `-G` | Impulse response high rolloff width. Give a plain number, or `@path` | 0 |
| `--source-low-freq <SOURCE_LOW_FREQ>` | `-D` | Input sound bandpass low rolloff point, in Hz. Give a plain number, or `@path` | 0 |
| `--source-high-freq <SOURCE_HIGH_FREQ>` | `-f` | Input sound bandpass high rolloff point, in Hz. `0` means Nyquist. Give a plain number, or `@path` | 0 |
| `--source-low-rolloff <SOURCE_LOW_ROLLOFF>` | `-h` | Input sound low rolloff width. Give a plain number, or `@path` | 0 |
| `--source-high-rolloff <SOURCE_HIGH_ROLLOFF>` | `-H` | Input sound high rolloff width. Give a plain number, or `@path` | 0 |
| `--source-gain <SOURCE_GAIN>` | `-A` | Dry-signal gain mixed back in after convolution, in dB, evaluated once per impulse at its sequence position. Give a plain number, or `@path` | 0 |
| `--output-gain <OUTPUT_GAIN>` | `-r` | Gain applied to each segment's convolution output, in dB. Give a plain number, or `@path` | 0 |

## Example

```
pvc irconvolvesequencer --impulse-list-dir ./impulses input.wav output.wav
```
