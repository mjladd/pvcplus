# pvc pitchtrack

Fundamental-frequency tracker. This produces a time-series of scalar pitch values, never audio. It ports `pitchtracker`'s audio path. See `rust/crates/pvc-core/src/tools/pitchtracker.rs` for two doc-corrected defaults and several real bugs, reproduced faithfully.

## Usage

```
pvc pitchtrack [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--fft <FFT>` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Frames per second. This sets the hop size | 200 |
| `--band-low <BAND_LOW>` | Detection band low edge | 0 |
| `--band-high <BAND_HIGH>` | Detection band high edge. A negative value means Nyquist | -1 |
| `--method <METHOD>` | Pitch-detection method | optimal-comb |
| `--window-min <WINDOW_MIN>` | Beginning note-stabilization buffer size in seconds | 0.05 |
| `--window-max <WINDOW_MAX>` | Maximum note-stabilization buffer size in seconds | 0.3 |
| `--detect-threshold <DETECT_THRESHOLD>` | Detection amplitude threshold in dB | -40 |
| `--mode-filter-window <MODE_FILTER_WINDOW>` | Temporal mode-filter window in seconds. `0` disables it | 0 |
| `--oversample <OVERSAMPLE>` | Amplitude-weighted oversampling factor. `0` disables it | 0 |
| `--smooth-response <SMOOTH_RESPONSE>` | One-pole lowpass smoothing applied to the output frequency alone | 0 |
| `--channel-method <CHANNEL_METHOD>` | How multiple input channels combine | average |
| `--compress-threshold <COMPRESS_THRESHOLD>` | Output compression threshold in dB | 0 |
| `--compress-amount <COMPRESS_AMOUNT>` | Output compression amount in dB | 0 |
| `--gate-threshold <GATE_THRESHOLD>` | Output gate threshold in dB | -96 |
| `--warp <WARP>` | Output warp index | 0 |
| `--attack <ATTACK>` | Output attack time in seconds | 0 |
| `--release <RELEASE>` | Output release time in seconds | 0 |
| `--output-rate <OUTPUT_RATE>` | Output values per second | 500 |
| `--output-format <OUTPUT_FORMAT>` | Output value format | freq |
| `--reference <REFERENCE>` | Reference pitch, in Hz or octave.pitchclass (`12` or below reads as octave.pitchclass). Only affects the `semitones-deviation` and `neg-semitones-deviation` output formats | 440 |
| `--output-type <OUTPUT_TYPE>` | ASCII, one value per line, or a headerless raw f32 stream | ascii |

## Example

```
pvc pitchtrack input.wav pitch.txt
```
