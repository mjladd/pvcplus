# pvc envelope

Amplitude envelope over a frequency band. This produces a time-series of scalar values, as ASCII or raw float, never audio. It ports `envelope`'s audio path. See `rust/crates/pvc-core/src/tools/envelope.rs` for its two-pass design and a real pass-1/pass-2 state-carryover quirk, reproduced faithfully.

## Usage

```
pvc envelope [OPTIONS] <INPUT> <OUTPUT>
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
| `--attack <ATTACK>` | Main envelope attack time in seconds | 0 |
| `--release <RELEASE>` | Main envelope release time in seconds | 0 |
| `--filtered-attack <FILTERED_ATTACK>` | Attack time for a second, subtractable "filtered envelope" | 0 |
| `--filtered-release <FILTERED_RELEASE>` | Release time for that same filtered envelope | 0 |
| `--filtered-cut <FILTERED_CUT>` | Proportion of the filtered envelope subtracted from the raw band sum before the main attack and release stage. `0` subtracts none, `1` subtracts the full amount | 0 |
| `--compress-threshold <COMPRESS_THRESHOLD>` | Output compression threshold in dB | 0 |
| `--compress-amount <COMPRESS_AMOUNT>` | Output compression amount in dB | 0 |
| `--gate-threshold <GATE_THRESHOLD>` | Output gate threshold in dB | -96 |
| `--warp <WARP>` | Output warp index | 0 |
| `--output-rate <OUTPUT_RATE>` | Output values per second | 500 |
| `--output-scale <OUTPUT_SCALE>` | Output value scale | amp |
| `--output-type <OUTPUT_TYPE>` | Output file format | ascii |

## Example

```
pvc envelope --band-low 200 --band-high 2000 input.wav envelope.txt
```
