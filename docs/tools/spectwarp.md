# pvc spectwarp

Per-bin dynamics processor. This command compresses or expands each bin's amplitude within an adjustable frequency band. Unlike `pvc compand`, it measures each bin against its own frame's live spectral peak, a self-referential envelope-follower. It ports `spectwarper`'s audio path. See `rust/crates/pvc-core/src/tools/spectwarper.rs` for what is in and out of scope, including a real bound bug in the original tool's sliding-window mode, reproduced faithfully.

## Usage

```
pvc spectwarp [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--fft <FFT>` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Frames per second. This sets the hop size | 200 |
| `--stretch <STRETCH>` | Time-stretch factor. `1.0` leaves duration unchanged | 1 |
| `--pitch <PITCH>` | Pitch shift in semitones. Give a plain number, or `@path` | 0 |
| `--freq-shift <FREQ_SHIFT>` | Frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--gain <GAIN>` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--compress-threshold <COMPRESS_THRESHOLD>` | Compression threshold in dB, relative to the live peak. Give a plain number, or `@path` | 0 |
| `--compress-amount <COMPRESS_AMOUNT>` | Compression amount in dB. This value must be negative to actually compress. A non-negative value is a no-op. Give a plain number, or `@path` | 0 |
| `--expand-threshold <EXPAND_THRESHOLD>` | Expansion threshold in dB, clamped to `-95` or above. Give a plain number, or `@path` | -96 |
| `--expand-amount <EXPAND_AMOUNT>` | Expansion amount in dB. This value must be negative to actually expand. Give a plain number, or `@path` | 0 |
| `--band-low <BAND_LOW>` | Companding band low edge in Hz. Give a plain number, or `@path` | 0 |
| `--band-high <BAND_HIGH>` | Companding band high edge in Hz. A negative value means Nyquist. Give a plain number, or `@path` | -1 |
| `--band-rolloff <BAND_ROLLOFF>` | Companding band rolloff width in octaves. Give a plain number, or `@path` | 0 |
| `--warp-curve <WARP_CURVE>` | Shape exponent for the compress and expand gradient curve. `0` is linear. Give a plain number, or `@path` | 0 |
| `--response-time <RESPONSE_TIME>` | Smoothing time in seconds for the per-bin amplitude-change multiplier. Give a plain number, or `@path` | 0 |
| `--complement <COMPLEMENT>` | Proportion, between `0` and `1`, of the unmodified source blended back into the companded result. Give a plain number, or `@path` | 0 |
| `--compress-window <COMPRESS_WINDOW>` | Sliding compression window size. `0` or lower means one global peak for the whole band. Values up to `8` are octaves, above `8` are Hz. Give a plain number, or `@path`. See `rust/crates/pvc-core/src/tools/spectwarper.rs` for a real bound bug in the original tool this can trigger | 0 |
| `--attack <ATTACK>` | Peak-reference attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | Peak-reference release time in seconds. Give a plain number, or `@path` | 0 |
| `--normalize-limit <NORMALIZE_LIMIT>` | Per-frame amplitude normalization limit in dB. `0` disables normalization. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | Low shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | High shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | Low shelf EQ frequency in Hz. Give a plain number, or `@path` | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | High shelf EQ frequency in Hz. Give a plain number, or `@path` | 2000 |
| `--threshold <THRESHOLD>` | Oscillator resynthesis threshold in dB. The tool skips bins quieter than this, relative to the frame's own peak | -96 |

## Example

```
pvc spectwarp --compress-threshold -6 --compress-amount -3 input.wav output.wav
```
