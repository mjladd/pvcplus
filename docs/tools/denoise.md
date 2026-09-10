# pvc denoise

Spectral noise gate. This command builds a noise-response profile by analyzing a `[--noise-begin, --noise-end)` window of the input itself. By default that window is the whole file, so point it at an actual noise-only stretch, such as leading silence. It then expands any bin quieter than its noise-response threshold toward silence. It ports `noisefilter`'s audio path. See `rust/crates/pvc-core/src/tools/noisefilter.rs` for what is in and out of scope.

## Usage

```
pvc denoise [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--fft <FFT>` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Frames per second. This sets the hop size | 200 |
| `--stretch <STRETCH>` | Time-stretch factor. `1.0` leaves duration unchanged | 1 |
| `--noise-begin <NOISE_BEGIN>` | Start of the noise-sample window to analyze, in seconds | 0 |
| `--noise-end <NOISE_END>` | End of the noise-sample window, in seconds. `0` means the end of the file | 0 |
| `--noise-method <NOISE_METHOD>` | How the noise-sample window's spectrum accumulates | average |
| `--noise-bypass-threshold <NOISE_BYPASS_THRESHOLD>` | Noise-response bypass threshold in dB. Bins in the peak-normalized noise response louder than this pass through unaffected | 0 |
| `--pitch <PITCH>` | Pitch shift in semitones. Give a plain number, or `@path`. Any nonzero or time-varying value selects oscillator-bank resynthesis | 0 |
| `--freq-shift <FREQ_SHIFT>` | Frequency shift in Hz. Give a plain number, or `@path`. Any nonzero or time-varying value selects oscillator-bank resynthesis | 0 |
| `--gain <GAIN>` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--noise-threshold-gain <NOISE_THRESHOLD_GAIN>` | Noise threshold adjustment in dB. A positive value increases noise reduction. A negative value reduces it. Give a plain number, or `@path` | 0 |
| `--expansion-index <EXPANSION_INDEX>` | Shape of the noise gate's expansion curve. Give a plain number, or `@path` | 3 |
| `--attack <ATTACK>` | Envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | Envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | Low shelf EQ gain in dB | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | High shelf EQ gain in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | Low shelf EQ frequency in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | High shelf EQ frequency in Hz | 2000 |
| `--threshold <THRESHOLD>` | Oscillator resynthesis threshold in dB. The tool skips bins quieter than this, relative to the frame's own peak | -96 |

## Example

```
pvc denoise --noise-begin 0 --noise-end 0.5 input.wav output.wav
```
