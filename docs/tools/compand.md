# pvc compand

Per-bin dynamics processor. This command compresses or expands each bin's amplitude within an adjustable frequency band. It measures each bin against a static peaks or reference file, such as one from `pvc freqresponse`. It ports `compander`'s audio path. See `rust/crates/pvc-core/src/tools/compander.rs` for what is in and out of scope. The original `-L`/release flag is a real no-op in the source tool, so it is not exposed here.

## Usage

```
pvc compand [OPTIONS] --peaks <PEAKS> <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--peaks <PEAKS>` | Path to the peaks or reference file, `.fr`-layout raw `N+2` binary floats, such as one from `pvc freqresponse`. Its size must match `--fft` exactly. This command exits with an error on a mismatch, rather than adjusting either value | required |
| `--fft <FFT>` | FFT size. Must be a power of two, and must match `--peaks`'s own size | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Frames per second. This sets the hop size | 200 |
| `--stretch <STRETCH>` | Time-stretch factor. `1.0` leaves duration unchanged | 1 |
| `--pitch <PITCH>` | Pitch shift in semitones. Give a plain number, or `@path` | 0 |
| `--freq-shift <FREQ_SHIFT>` | Frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--gain <GAIN>` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--compress-threshold <COMPRESS_THRESHOLD>` | Compression threshold in dB. Bins louder than this, relative to the peaks file, get compressed. Give a plain number, or `@path` | 0 |
| `--compress-amount <COMPRESS_AMOUNT>` | Compression amount in dB. This value must be negative to actually compress. A non-negative value is a no-op. Give a plain number, or `@path` | 0 |
| `--expand-threshold <EXPAND_THRESHOLD>` | Expansion threshold in dB. Give a plain number, or `@path` | -96 |
| `--expand-amount <EXPAND_AMOUNT>` | Expansion amount in dB. This value must be negative to actually expand. Give a plain number, or `@path` | 0 |
| `--band-low <BAND_LOW>` | Companding band low edge in Hz. Give a plain number, or `@path` | 0 |
| `--band-high <BAND_HIGH>` | Companding band high edge in Hz. A negative value means Nyquist. Give a plain number, or `@path` | -1 |
| `--band-rolloff <BAND_ROLLOFF>` | Companding band rolloff width in octaves. Give a plain number, or `@path` | 0 |
| `--peaks-smoothing <PEAKS_SMOOTHING>` | Peaks-file smoothing bandwidth, applied once at load time. A negative value is octaves, a non-negative value is Hz, and `0` applies no smoothing | 0 |
| `--attack <ATTACK>` | Per-bin amplitude-change attack time in seconds. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | Low shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | High shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | Low shelf EQ frequency in Hz. Give a plain number, or `@path` | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | High shelf EQ frequency in Hz. Give a plain number, or `@path` | 2000 |
| `--threshold <THRESHOLD>` | Oscillator resynthesis threshold in dB. The tool skips bins quieter than this, relative to the frame's own peak | -96 |

## Example

```
pvc freqresponse input.wav peaks.fr
pvc compand --peaks peaks.fr --compress-threshold -12 --compress-amount -6 input.wav output.wav
```
