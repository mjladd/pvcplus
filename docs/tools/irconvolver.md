# pvc irconvolver

Fast FFT convolution, or deconvolution, of each input channel against a `.ir` file's spectrum. It is part of Phase 5's FFT-convolution family. See `rust/crates/pvc-core/src/tools/irconvolver.rs` for what is out of scope, and a real usage-text bug in the original C, corrected here. It ports `irconvolver`'s resynthesis path.

## Usage

```
pvc irconvolver [OPTIONS] --ir <IR> <INPUT> <OUTPUT>
```

## Options

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--ir <IR>` | `-E` | Path to the `.ir` impulse-response file, from `pvc impulseresponse` | required |
| `--begin <BEGIN>` | `-b` | Analysis window start, in seconds | 0 |
| `--end <END>` | `-e` | Analysis window end, in seconds. `0` means the end of the file | 0 |
| `--ring-tail` | `-d` | Extend the processed window by one impulse-length of trailing silence, so the reverb tail does not get cut off | off |
| `--impulse-channel <IMPULSE_CHANNEL>` | `-J` | Which `.ir` channel to use. `0` auto round-robins across channels | 0 |
| `--mode <MODE>` | `-a` | Convolve or deconvolve. The original C's own usage text mislabels this flag, corrected here | convolve |
| `--ir-low-freq <IR_LOW_FREQ>` | `-s` | Impulse response bandpass low rolloff point, in Hz | 0 |
| `--ir-high-freq <IR_HIGH_FREQ>` | `-t` | Impulse response bandpass high rolloff point, in Hz. `0` means Nyquist | 0 |
| `--ir-low-rolloff <IR_LOW_ROLLOFF>` | `-g` | Impulse response low rolloff width | 0 |
| `--ir-high-rolloff <IR_HIGH_ROLLOFF>` | `-G` | Impulse response high rolloff width | 0 |
| `--source-low-freq <SOURCE_LOW_FREQ>` | `-D` | Input sound bandpass low rolloff point, in Hz | 0 |
| `--source-high-freq <SOURCE_HIGH_FREQ>` | `-f` | Input sound bandpass high rolloff point, in Hz. `0` means Nyquist | 0 |
| `--source-low-rolloff <SOURCE_LOW_ROLLOFF>` | `-h` | Input sound low rolloff width | 0 |
| `--source-high-rolloff <SOURCE_HIGH_ROLLOFF>` | `-H` | Input sound high rolloff width | 0 |
| `--source-gain <SOURCE_GAIN>` | `-A` | Dry-signal gain mixed back in after convolution, in dB. Give a plain number, or `@path` | 0 |
| `--input-gain <INPUT_GAIN>` | `-q` | Gain applied before convolution, in dB. Give a plain number, or `@path` | 0 |
| `--output-gain <OUTPUT_GAIN>` | `-r` | Gain applied to the convolution output, in dB. Give a plain number, or `@path` | 0 |

## Example

```
pvc impulseresponse room.wav room.ir
pvc irconvolver --ir room.ir input.wav output.wav
```
