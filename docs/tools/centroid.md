# pvc centroid

Spectral centroid over a frequency band. This is the amplitude-squared-weighted mean frequency, produced as a time-series of scalar values, never audio. It ports `centroid`'s audio path. See `rust/crates/pvc-core/src/tools/centroid.rs` for two provably-dead flags this port does not expose.

## Usage

```
pvc centroid [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--fft <FFT>` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Frames per second. This sets the hop size | 200 |
| `--band-octave-pitchclass` | Read the detection-band boundaries as octave.pitchclass values instead of Hz | off |
| `--band-low <BAND_LOW>` | Detection band low edge | 0 |
| `--band-high <BAND_HIGH>` | Detection band high edge. A negative value means Nyquist | -1 |
| `--channel-method <CHANNEL_METHOD>` | How multiple input channels combine | average |
| `--attack <ATTACK>` | Output attack time in seconds | 0 |
| `--release <RELEASE>` | Output release time in seconds | 0 |
| `--warp <WARP>` | Output warp index | 0 |
| `--output-rate <OUTPUT_RATE>` | Output values per second | 500 |
| `--output-format <OUTPUT_FORMAT>` | Output value format | freq |
| `--reference-pitch <REFERENCE_PITCH>` | Reference pitch, in octave.pitchclass notation. Only affects the `semitones-deviation` and `neg-semitones-deviation` output formats | 8 |
| `--output-type <OUTPUT_TYPE>` | Output file format | ascii |

## Example

```
pvc centroid --band-low 200 --band-high 2000 input.wav centroid.txt
```
