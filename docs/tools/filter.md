# pvc filter

Fixed-spectrum phase-vocoder filter. This command multiplies each frame's amplitude by a shaped copy of a `.fr` response. It then mixes the filtered signal with the original source, unless disabled. It ports `filter`'s audio path. See `rust/crates/pvc-core/src/tools/filter.rs` for what is in and out of scope. Oscillator-bank resynthesis, needed only for pitch or frequency-shifted output, is not ported yet.

## Usage

```
pvc filter [OPTIONS] --response <RESPONSE> <INPUT> <OUTPUT>
```

## Options

### Analysis and response

| Flag | Description | Default |
|---|---|---|
| `--response <RESPONSE>` | Path to the `.fr` response file. Its own size sets the FFT size | required |
| `--window-size <WINDOW_SIZE>` | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Frames per second. This sets the hop size | 200 |
| `--stretch <STRETCH>` | Time-stretch factor. `1.0` leaves duration unchanged | 1 |
| `--response-pitch <RESPONSE_PITCH>` | Response pitch shift in semitones. Give a plain number, or `@path` | 0 |
| `--response-freq-shift <RESPONSE_FREQ_SHIFT>` | Response frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--response-warp <RESPONSE_WARP>` | Response magnitude warp index. Give a plain number, or `@path` | 0 |
| `--response-smoothing <RESPONSE_SMOOTHING>` | Response smoothing bandwidth. A positive value is Hz, a negative value is octaves. Give a plain number, or `@path` | 0 |
| `--band-reject` | Invert the response, so it band-rejects instead of band-passes | off |
| `--pitch-shift-source-only` | Apply the filter output's own pitch or frequency shift only to the source, not to how the response is positioned. Off by default, so the response tracks the filter output's shift too | off |

### Filter output

| Flag | Description | Default |
|---|---|---|
| `--filter-pitch <FILTER_PITCH>` | Filter-output pitch shift in semitones. Give a plain number, or `@path` | 0 |
| `--filter-freq-shift <FILTER_FREQ_SHIFT>` | Filter-output frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--filter-gain <FILTER_GAIN>` | Filter-output gain in dB. Give a plain number, or `@path` | 0 |
| `--filter-time-delay <FILTER_TIME_DELAY>` | Filter-output time delay in seconds. Give a plain number, or `@path` | 0 |
| `--delay-time-scaler <DELAY_TIME_SCALER>` | Scales the effective delay this command uses to resolve time-varying parameters against a delayed frame. Give a plain number, or `@path` | 1 |

### Source mixing

| Flag | Description | Default |
|---|---|---|
| `--source <SOURCE_ENABLED>` | Mix the, possibly delayed and shifted, original source in alongside the filtered signal | true |
| `--source-gain <SOURCE_GAIN>` | Source gain in dB. Give a plain number, or `@path` | 0 |
| `--source-pitch <SOURCE_PITCH>` | Source pitch shift in semitones. Give a plain number, or `@path` | 0 |
| `--source-freq-shift <SOURCE_FREQ_SHIFT>` | Source frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--source-time-delay <SOURCE_TIME_DELAY>` | Source time delay in seconds. Give a plain number, or `@path` | 0 |
| `--source-floor <SOURCE_FLOOR>` | Source signal floor in dB. This sets how much of the source passes through unfiltered at the response's quietest points. Give a plain number, or `@path` | -96 |

### Shelf EQ

| Flag | Description | Default |
|---|---|---|
| `--shelf-low-gain <SHELF_LOW_GAIN>` | Low shelf EQ gain in dB | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | High shelf EQ gain in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | Low shelf EQ frequency in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | High shelf EQ frequency in Hz | 2000 |

### Dynamics and envelope

| Flag | Description | Default |
|---|---|---|
| `--comp-threshold <COMP_THRESHOLD>` | Compression threshold in dB | 0 |
| `--comp-amount <COMP_AMOUNT>` | Compression amount in dB | 0 |
| `--expand-threshold <EXPAND_THRESHOLD>` | Expansion threshold in dB | -96 |
| `--expand-amount <EXPAND_AMOUNT>` | Expansion amount in dB | 0 |
| `--normalization-limit <NORMALIZATION_LIMIT>` | Per-frame amplitude normalization limit in dB. Give a plain number, or `@path` | 0 |
| `--normalize-to-response` | Normalize each frame to match the filter response's own loudness, instead of the, possibly delayed, input sound's loudness | off |
| `--attack <ATTACK>` | Envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | Envelope release time in seconds. Give a plain number, or `@path` | 0 |

## Example

```
pvc filter --response response.fr input.wav output.wav
```
