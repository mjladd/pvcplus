# pvc filtdeviator

Fixed-spectrum, additive source-plus-filter phase-vocoder filter, like `pvc filter`. It adds a per-bin, response-shaped time delay into the filter's own delay line. It also adds a self-referential decay and feedback accumulator on that same delay line. Finally, it adds a per-bin frequency deviation shaped by the response. The original tool calls this last part "response-correlated frequency deviation." It ports `filtdeviator`'s audio path. See `rust/crates/pvc-core/src/tools/filtdeviator.rs` for what is in and out of scope. That file also documents a real dead computation, and every `randf()`-based mode this project defers by established precedent.

## Usage

```
pvc filtdeviator [OPTIONS] --response <RESPONSE> <INPUT> <OUTPUT>
```

## Options

### Core

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--response <RESPONSE>` | `-F` | Path to the `.fr` response file. Its own size sets the FFT size | required |
| `--window-size <WINDOW_SIZE>` | `-M` | Analysis and resynthesis window length. `0` means auto (`2 * fft`), using the response file's own FFT size | 0 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | `-D` | Frames per second. This sets the hop size. Any value under `32` resets to `200` | 200 |
| `--time-factor <TIME_FACTOR>` | `-I` | Time expansion or contraction factor. `1.0` leaves duration unchanged. Any value at or below `0` resets to `1.0` | 1 |
| `--begin <BEGIN>` | `-b` | Begin time in seconds. This trims real samples | 0 |
| `--end <END>` | `-e` | End time in seconds. `0` means the end of the file | 0 |
| `--channel <CHANNEL>` | `-C` | Which input channel to resynthesize. `0` processes every channel independently. A number of `1` or higher resynthesizes only that one channel, giving mono output | 0 |

### Source and filter output

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--source-gain <SOURCE_GAIN>` | `-h` | Source gain in dB. Give a plain number, or `@path`. Any value above `-96`, or a time-varying one, turns source mixing on entirely | 0 |
| `--source-freq-shift <SOURCE_FREQ_SHIFT>` | `-r` | Source frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--source-delay <SOURCE_DELAY>` | `-s` | Source delay time in seconds. Give a plain number, or `@path` | 0 |
| `--source-pitch <SOURCE_PITCH>` | `-y` | Source pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--filter-gain <FILTER_GAIN>` | `-A` | Filter output gain in dB. Give a plain number, or `@path` | 0 |
| `--filter-pitch <FILTER_PITCH>` | `-P` | Filter output pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--filter-freq-shift <FILTER_FREQ_SHIFT>` | `-a` | Filter output frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--response-pitch <RESPONSE_PITCH>` | `-T` | Response pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--response-freq-shift <RESPONSE_FREQ_SHIFT>` | `-V` | Response frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--source-floor <SOURCE_FLOOR>` | `-S` | Source signal floor in dB. Give a plain number, or `@path` | -96 |
| `--band-reject` | `-G` | Invert the response, so it band-rejects instead of band-passes | off |

### Envelope, warp, and shelf EQ

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--attack <ATTACK>` | `-l` | Amplitude attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | `-L` | Amplitude release time in seconds. Give a plain number, or `@path` | 0 |
| `--amp-warp <AMP_WARP>` | `-W` | Amplitude response warp index. Give a plain number, or `@path` | 0 |
| `--freq-warp <FREQ_WARP>` | `-v` | Frequency response warp index. Give a plain number, or `@path` | 0 |
| `--delay-warp <DELAY_WARP>` | `-o` | Time delay response warp index. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | | Low shelf EQ gain in dB | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | | High shelf EQ gain in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | | Low shelf EQ frequency in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | | High shelf EQ frequency in Hz | 2000 |

### Delay line and decay

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--time-delay-base <TIME_DELAY_BASE>` | `-j` | Time delay base, in seconds. Give a plain number, or `@path` | 0 |
| `--time-delay-peak <TIME_DELAY_PEAK>` | `-J` | Time delay peak, in seconds. Give a plain number, or `@path` | 0 |
| `--time-delay-control <TIME_DELAY_CONTROL>` | `-q` | Time delay master control, between `0` and `1`. Give a plain number, or `@path`. This default of `1`, fully engaged, matches the original tool's real initializer, not its usage text | 1 |
| `--delay-time-scaler <DELAY_TIME_SCALER>` | `-@` | Scales the effective delay this command uses to resolve time-varying parameters against a delayed bin. Give a plain number, or `@path` | 1 |
| `--decay-time-peak <DECAY_TIME_PEAK>` | `-/` | Peak decay time in seconds. Give a plain number, or `@path` | 0 |
| `--decay-time-base <DECAY_TIME_BASE>` | `-:` | Base decay time in seconds. Give a plain number, or `@path` | 0 |
| `--decay-time-control <DECAY_TIME_CONTROL>` | `-B` | Decay time master control, between `0` and `1`. Give a plain number, or `@path`. Same real-default-of-`1` situation as `--time-delay-control` | 1 |

### Frequency deviation

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--freq-dev-base <FREQ_DEV_BASE>` | `-k` | Base frequency deviation in semitones. Give a plain number, or `@path` | 0 |
| `--freq-dev-peak <FREQ_DEV_PEAK>` | `-K` | Peak frequency deviation in semitones. Give a plain number, or `@path` | 0 |
| `--freq-shift-dev-base <FREQ_SHIFT_DEV_BASE>` | `-u` | Base frequency deviation shift in Hz. Give a plain number, or `@path` | 0 |
| `--freq-shift-dev-peak <FREQ_SHIFT_DEV_PEAK>` | `-U` | Peak frequency deviation shift in Hz. Give a plain number, or `@path` | 0 |
| `--freq-dev-control <FREQ_DEV_CONTROL>` | `-O` | Frequency deviation master control, between `0` and `1`. Give a plain number, or `@path` | 1 |
| `--freq-dev-mode <FREQ_DEV_MODE>` | `-E` | Frequency deviation mode: `0` for response-driven, the default, or `@path` to a table for file mode. The original tool's random mode is not supported here | 0 |

### Output normalization

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--normalization-limit <NORMALIZATION_LIMIT>` | `-n` | Per-frame amplitude normalization limit in dB. Give a plain number, or `@path` | 0 |
| `--normalize-to-response` | `-x` | Normalize each frame to match the filter response's own loudness, instead of the, possibly delayed, input sound's loudness | off |
| `--threshold <THRESHOLD>` | `-t` | Oscillator resynthesis threshold in dB | -96 |

## Example

```
pvc filtdeviator --response response.fr --time-delay-peak 0.05 input.wav output.wav
```
