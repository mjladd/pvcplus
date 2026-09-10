# pvc specflattracker

Spectral flatness tracker. This produces a time-series of the geometric-to-arithmetic mean ratio of each frame's bins over a detection band, never audio. A value near `1.0` means a noise-like spectrum. A value near `0.0` means a tonal one. It ports `specflattracker`'s audio path. See `rust/crates/pvc-core/src/tools/specflattracker.rs` for why it reuses most of `pvc centroid`'s two-pass pipeline, plus a real pass-2 interpolation quirk, absent from `centroid` and reproduced faithfully here.

## Usage

```
pvc specflattracker [OPTIONS] <INPUT> <OUTPUT>
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
| `--method <METHOD>` | Which per-bin quantity feeds the flatness ratio | amplitude |
| `--amplitude-threshold <AMPLITUDE_THRESHOLD>` | Amplitude threshold in dB for excluding or flooring low-valued bins. The original tool's own usage text claims `-200` as the default, but its real initializer is `-96`. This command matches the code, not the usage text | -96 |
| `--attack <ATTACK>` | Trajectory attack time | 0 |
| `--release <RELEASE>` | Trajectory release time | 0 |
| `--warp <WARP>` | Output warp index | 0 |
| `--output-rate <OUTPUT_RATE>` | Output values per second | 500 |
| `--output-format <OUTPUT_FORMAT>` | Output value format | coefficient |
| `--output-type <OUTPUT_TYPE>` | Output file format | ascii |

## Example

```
pvc specflattracker input.wav flatness.txt
```
