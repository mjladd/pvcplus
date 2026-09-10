# pvc flux

Spectral flux over a frequency band. This measures frame-to-frame frequency change, optionally weighted by amplitude, as a time-series of scalar values, never audio. It ports `fluxoid`'s audio path. See `rust/crates/pvc-core/src/tools/fluxoid.rs` for its shared two-pass shape.

## Usage

```
pvc flux [OPTIONS] <INPUT> <OUTPUT>
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
| `--amplitude-weighting <AMPLITUDE_WEIGHTING>` | Weight each bin's frequency change by its own amplitude | true |
| `--compress-threshold <COMPRESS_THRESHOLD>` | Output compression threshold in dB | 0 |
| `--compress-amount <COMPRESS_AMOUNT>` | Output compression amount in dB | 0 |
| `--gate-threshold <GATE_THRESHOLD>` | Output gate threshold in dB | -96 |
| `--warp <WARP>` | Output warp index | 0 |
| `--output-rate <OUTPUT_RATE>` | Output values per second | 500 |
| `--output-scale <OUTPUT_SCALE>` | Output value scale | amp |
| `--output-type <OUTPUT_TYPE>` | Output file format | ascii |

## Example

```
pvc flux --band-low 200 --band-high 2000 input.wav flux.txt
```
