# pvc harmonize

Builds one or more "voices" from a data table of frequency bands. Each voice is a pitch or frequency-shifted copy of a triangular-windowed slice of the input spectrum. The command can also sum in a delayed and shifted copy of the source signal. It ports `harmonizer`'s audio path. See `rust/crates/pvc-core/src/tools/harmonizer.rs` for the data-table format and what is in and out of scope. That file also documents a real crash bug this command validates against, instead of reproducing it, and a real cross-band bug it does reproduce faithfully.

## Usage

```
pvc harmonize [OPTIONS] --table <TABLE> <INPUT> <OUTPUT>
```

## Options

### Core and table

| Flag | Description | Default |
|---|---|---|
| `--fft <FFT>` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Frames per second. This sets the hop size | 200 |
| `--stretch <STRETCH>` | Time-stretch factor. `1.0` leaves duration unchanged | 1 |
| `--table <TABLE>` | Path to the 8-column data table. This is a whitespace-delimited ASCII file, one row per band. Each row holds a shift factor, then low, high, and center frequency in Hz or octave.pitchclass. It also holds peak dB, stopband dB, a Q-index, and a delay in seconds | required |
| `--shift-format <SHIFT_FORMAT>` | How this command reads the table's shift-factor column | multiplier |

### Table column scaling

Each of these applies a scale or shift to one table column, across every band at once.

| Flag | Description | Default |
|---|---|---|
| `--table-freq-scale <TABLE_FREQ_SCALE>` | Scaler for the low, high, and center frequency columns, applied after octave.pitchclass conversion | 1 |
| `--table-freq-shift <TABLE_FREQ_SHIFT>` | Shifter for those same frequency columns, in Hz | 0 |
| `--table-peak-scale <TABLE_PEAK_SCALE>` | Scaler for the peak dB column | 1 |
| `--table-stopband-scale <TABLE_STOPBAND_SCALE>` | Scaler for the stopband dB column | 1 |
| `--table-stopband-shift <TABLE_STOPBAND_SHIFT>` | Shifter for the stopband dB column, in dB | 0 |
| `--table-q-shift <TABLE_Q_SHIFT>` | Shifter for the Q-index column | 0 |
| `--table-delay-scale <TABLE_DELAY_SCALE>` | Scaler for the delay column | 1 |
| `--table-delay-shift <TABLE_DELAY_SHIFT>` | Shifter for the delay column, in seconds | 0 |
| `--table-shift-scale <TABLE_SHIFT_SCALE>` | Scaler for the shift-factor column | 1 |
| `--table-shift-shift <TABLE_SHIFT_SHIFT>` | Shifter for the shift-factor column | 0 |

### Source and voice mixing

| Flag | Description | Default |
|---|---|---|
| `--gain <GAIN>` | Master gain in dB. Give a plain number, or `@path` | 0 |
| `--source-freq-shift <SOURCE_FREQ_SHIFT>` | Source frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--source-pitch <SOURCE_PITCH>` | Source pitch shift in semitones. Give a plain number, or `@path`. The default of `1` semitone matches the original tool's own real initializer, not the `0` some older documentation lists | 1 |
| `--source-gain <SOURCE_GAIN>` | Source gain in dB. Give a plain number, or `@path` | 0 |
| `--source-delay <SOURCE_DELAY>` | Source delay in seconds. Give a plain number, or `@path` | 0 |
| `--voice-freq-shift <VOICE_FREQ_SHIFT>` | Voice, or harmony, frequency shift in Hz, added after the per-band shift. Give a plain number, or `@path` | 0 |
| `--voice-pitch <VOICE_PITCH>` | Voice, or harmony, pitch shift in semitones, applied after the per-band shift. Give a plain number, or `@path`. Defaults to `1` semitone for the same reason as `--source-pitch` | 1 |
| `--voice-gain <VOICE_GAIN>` | Voice, or harmony, gain in dB, per band. Give a plain number, or `@path` | 0 |
| `--voice-warp <VOICE_WARP>` | Voice, or harmony, spectrum warp index. Give a plain number, or `@path` | 0 |

### Interpolation and threshold

| Flag | Description | Default |
|---|---|---|
| `--freq-interp <FREQ_INTERP>` | Frequency interpolation control, per band. `0` means unshifted, `1` means fully shifted. Give a plain number, or `@path`. See `rust/crates/pvc-core/src/tools/harmonizer.rs` for a real bug affecting this flag's per-band granularity in `multiplier` and `semitones` shift-format modes | 1 |
| `--time-interp <TIME_INTERP>` | Time interpolation control, per band. `0` applies no delay, `1` applies the table's full delay. Give a plain number, or `@path` | 1 |
| `--threshold <THRESHOLD>` | Oscillator resynthesis threshold in dB. The tool skips bins quieter than this, relative to the frame's own peak | -96 |

## Example

```
pvc harmonize --table bands.txt input.wav output.wav
```
