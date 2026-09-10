# pvc formantsmapper

Formant mapper. This command reads source and target formant lists, binary files produced externally, not by any tool in this project. It pairs each formant with its nearest counterpart in the other list. It then remaps each source formant's own bin band onto the paired target's frequency and amplitude, using one or two independently-controllable oscillator banks. It can optionally pass unclaimed bins through as a separate residue bank. It ports `formantsmapper`. See `rust/crates/pvc-core/src/tools/formantsmapper.rs` for what is in and out of scope. That file documents an entirely dead correction subsystem, and a real bug: turning on residue bins together with dual-bank mode silently drops bank B.

## Usage

```
pvc formantsmapper [OPTIONS] --source-formants <SOURCE_FORMANTS> --target-formants <TARGET_FORMANTS> <INPUT> <OUTPUT>
```

## Options

### Core

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--source-formants <SOURCE_FORMANTS>` | `-E` | Path to the source formants file. The original tool's own usage text mislabels this "Target Formants File." It is the source file | required |
| `--target-formants <TARGET_FORMANTS>` | `-g` | Path to the target formants file | required |
| `--fft <FFT>` | `-N` | FFT size | 1024 |
| `--window-size <WINDOW_SIZE>` | `-M` | Analysis and resynthesis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | `-D` | Frames per second. This sets the hop size. Any value under `32` resets to `200` | 200 |
| `--time-factor <TIME_FACTOR>` | `-I` | Time expansion or contraction factor. Any value at or below `0` resets to `1.0` | 1 |
| `--begin <BEGIN>` | `-b` | Begin time in seconds. This trims real samples | 0 |
| `--end <END>` | `-e` | End time in seconds. `0` means the end of the file | 0 |
| `--channel <CHANNEL>` | `-C` | Which input channel to resynthesize. `0` processes every channel independently. A number of `1` or higher resynthesizes only that one channel | 0 |
| `--gain <GAIN>` | `-A` | Gain in decibels. Give a plain number, or `@path` | 0 |
| `--pitch <PITCH>` | `-P` | Pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--freq-shift <FREQ_SHIFT>` | `-a` | Frequency shift in Hz, applied before `--pitch`. Give a plain number, or `@path` | 0 |
| `--attack <ATTACK>` | `-l` | Envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | `-L` | Envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | `-H` | Low shelf EQ gain in dB, applied after transpose and shift. Give a plain number, or `@path` | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | `-X` | High shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | `-m` | Low shelf EQ frequency in Hz. Give a plain number, or `@path` | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | `-R` | High shelf EQ frequency in Hz. Give a plain number, or `@path` | 2000 |
| `--threshold <THRESHOLD>` | `-t` | Oscillator resynthesis threshold in dB | -96 |

### Source formants

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--source-low-freq <SOURCE_LOW_FREQ>` | `-z` | Source formants low frequency boundary in Hz | 20 |
| `--source-high-freq <SOURCE_HIGH_FREQ>` | `-Z` | Source formants high frequency boundary in Hz. `0` means Nyquist | 0 |
| `--source-db-threshold <SOURCE_DB_THRESHOLD>` | `-@` | Source formants amplitude threshold in dB | -200 |
| `--bandwidth-extension-factor <BANDWIDTH_EXTENSION_FACTOR>` | `-S` | Pre-synthesis formant bandwidth extension factor, source formants only. `0` reduces every formant to a single bin. `1` bypasses this step | 1 |
| `--extend-source` | `-n` | Extend source formants with synthetic harmonic-partial formants | off |
| `--source-extend-db-threshold <SOURCE_EXTEND_DB_THRESHOLD>` | `-U` | Source formant extension amplitude threshold in dB | -96 |
| `--source-extend-peak-partial <SOURCE_EXTEND_PEAK_PARTIAL>` | `-V` | Highest partial to extend source formants to. `0` means every partial below Nyquist | 0 |
| `--source-extend-octaves` | `-y` | Also add upper octaves of each extended source partial | off |

### Target formants

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--target-transpose <TARGET_TRANSPOSE>` | `-T` | Target formants transposition in semitones, applied before filtering and extension | 0 |
| `--target-db-threshold <TARGET_DB_THRESHOLD>` | `-K` | Target formants amplitude threshold in dB | -96 |
| `--target-low-freq <TARGET_LOW_FREQ>` | `-o` | Target formants low frequency boundary in Hz | 0 |
| `--target-high-freq <TARGET_HIGH_FREQ>` | `-O` | Target formants high frequency boundary in Hz. `0` means Nyquist | 0 |
| `--extend-target` | `-c` | Extend target formants with synthetic harmonic-partial formants | off |
| `--target-extend-db-threshold <TARGET_EXTEND_DB_THRESHOLD>` | `-d` | Target formant extension amplitude threshold in dB | -96 |
| `--target-extend-peak-partial <TARGET_EXTEND_PEAK_PARTIAL>` | `-j` | Highest partial to extend target formants to. `0` means every partial below Nyquist | 0 |
| `--target-extend-octaves` | `-k` | Also add upper octaves of each extended target partial | off |
| `--extend-partial-rolloff <EXTEND_PARTIAL_ROLLOFF>` | `-Y` | Added-formant-partial decibel rolloff per partial, shared between source and target extension | 0 |
| `--amp-gain-limit <AMP_GAIN_LIMIT>` | `-~` | Amplitude-scaler clamp limit. The original C has a real unit-mismatch bug here, reproduced exactly | 200 |

### Oscillator banks A and B

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--bank-a-rolloff <BANK_A_ROLLOFF>` | `-h` | Bank A decibel rolloff per octave, measured from a mapped formant's own center bin. Give a plain number, or `@path` | 0 |
| `--bank-a-freq-interp <BANK_A_FREQ_INTERP>` | `-r` | Bank A frequency interpolation control, between `0` and `1`. Give a plain number, or `@path` | 0 |
| `--bank-a-amp-interp <BANK_A_AMP_INTERP>` | `-v` | Bank A amplitude interpolation control, between `0` and `1`. Give a plain number, or `@path` | 0 |
| `--bank-a-gain <BANK_A_GAIN>` | `-G` | Bank A gain in dB. Give a plain number, or `@path` | 0 |
| `--bank-a-pitch <BANK_A_PITCH>` | `-f` | Bank A pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--dual-bank` | `-/` | Also resynthesize bank B, a second, independently-controlled mapping of the same formant pairing | off |
| `--bank-b-rolloff <BANK_B_ROLLOFF>` | `-B` | Bank B decibel rolloff per octave. Give a plain number, or `@path` | 0 |
| `--bank-b-freq-interp <BANK_B_FREQ_INTERP>` | `-:` | Bank B frequency interpolation control, between `0` and `1`. Give a plain number, or `@path` | 0 |
| `--bank-b-amp-interp <BANK_B_AMP_INTERP>` | `-J` | Bank B amplitude interpolation control, between `0` and `1`. Give a plain number, or `@path` | 0 |
| `--bank-b-gain <BANK_B_GAIN>` | `-F` | Bank B gain in dB. Give a plain number, or `@path` | 0 |
| `--bank-b-pitch <BANK_B_PITCH>` | `-W` | Bank B pitch transposition in semitones. Give a plain number, or `@path` | 0 |
| `--residue-bins` | `-x` | Pass bins outside every formant's own band through as a separate residue oscillator bank | off |
| `--residue-gain <RESIDUE_GAIN>` | `-s` | Residue bins gain in dB. Give a plain number, or `@path` | 0 |

## Example

```
pvc formantsmapper --source-formants source.fmt --target-formants target.fmt input.wav output.wav
```
