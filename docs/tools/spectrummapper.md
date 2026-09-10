# pvc spectrummapper

Formant-tracking analysis tool. This command extracts formant peaks frame by frame from raw audio. It greedily assembles them into time-continuous segments, called tracks, then links separate segments into longer chains. It writes the result to ASCII or binary formant-track files, and writes no audio output at all. It ports `spectrummapper`. See `rust/crates/pvc-core/src/tools/spectrummapper.rs` for what is in and out of scope. That file documents a real amplitude-rescale bug. It double-applies a frame's own peak amplitude once that peak reaches `1.0` or above. It also documents a real copy-paste bug in the "impose" onset envelope.

## Usage

```
pvc spectrummapper [OPTIONS] --segments-file <SEGMENTS_FILE> <INPUT>
```

## Options

### Output files

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--segments-file <SEGMENTS_FILE>` | `-S` | Path to write the binary formant-track file | required |
| `--scatter-file <SCATTER_FILE>` | `-f` | Path to write an ASCII scatter plot of every raw per-frame formant point, as time and frequency. Omit to skip writing it | none |
| `--ascii-segments-file <ASCII_SEGMENTS_FILE>` | `-a` | Path to write an ASCII listing of the final linked segments. Omit to skip writing it | none |

### Core analysis

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--fft <FFT>` | `-N` | FFT size | 1024 |
| `--window-size <WINDOW_SIZE>` | `-M` | Analysis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | `-D` | Frames per second. This sets the hop size. Any value under `32` resets to `200` | 200 |
| `--begin <BEGIN>` | `-b` | Begin time in seconds. This trims real samples | 0 |
| `--end <END>` | `-e` | End time in seconds. `0` means the end of the file | 0 |
| `--channel <CHANNEL>` | `-C` | Which input channel to analyze. `0` processes every channel independently. A number of `1` or higher analyzes only that one channel | 0 |
| `--eq-bypass` | `-B` | Bypass the shelf EQ entirely, instead of applying it | off |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | `-H` | Low shelf EQ gain in dB | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | `-X` | High shelf EQ gain in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | `-m` | Low shelf EQ frequency in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | `-R` | High shelf EQ frequency in Hz | 2000 |

### Formant selection

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--low-freq-limit <LOW_FREQ_LIMIT>` | `-L` | Low frequency limit in Hz for formant selection. Give a plain number, or `@path` | 0 |
| `--high-freq-limit <HIGH_FREQ_LIMIT>` | `-j` | High frequency limit in Hz for formant selection. `0` or below means Nyquist. Give a plain number, or `@path` | 0 |
| `--min-formant-db <MINIMUM_FORMANT_DB>` | `-A` | Minimum formant peak amplitude in dB | -96 |
| `--formant-threshold <FORMANT_SELECTION_THRESHOLD>` | `-g` | Formant selection threshold, between `0` and `1`. Higher values select fewer, stronger formants | 0.5 |

### Segment construction

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--min-decibels <MINIMUM_DECIBELS>` | `-c` | Minimum decibel level for a formant to stay eligible for segment construction | -200 |
| `--max-decibels <MAXIMUM_DECIBELS>` | `-E` | Maximum decibel level for a formant to stay eligible | 0 |
| `--min-segment-length <MINIMUM_SEGMENT_LENGTH>` | `-o` | Minimum final segment length in frames | 1 |
| `--max-segment-length <MAXIMUM_SEGMENT_LENGTH>` | `-O` | Maximum final segment length in frames. `0` or below means unlimited | 0 |
| `--min-segment-duration <MINIMUM_SEGMENT_DURATION>` | `-q` | Minimum final segment duration in seconds | 0 |
| `--max-segment-duration <MAXIMUM_SEGMENT_DURATION>` | `-Q` | Maximum final segment duration in seconds. `0` or below means twice the analysis duration | 0 |
| `--max-freq-change <MAX_FREQUENCY_CHANGE_PER_MS>` | `-p` | Maximum frequency change per millisecond allowed while a segment grows | 12 |
| `--max-db-rise <MAX_DECIBEL_RISE_PER_MS>` | `-V` | Maximum decibel rise per millisecond allowed while a segment grows | 90 |
| `--max-db-fall <MAX_DECIBEL_FALL_PER_MS>` | `-v` | Maximum decibel fall per millisecond allowed while a segment grows | 90 |

### Linking and onset shaping

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--linkage-time <LINKAGE_TIME>` | `-r` | Maximum time gap in seconds this command allows between two segments it links together | 0.02 |
| `--max-freq-linkage <MAXIMUM_FREQUENCY_LINKAGE>` | `-W` | Maximum frequency difference in Hz this command allows between two segments it links together | 100 |
| `--onset-release-mode <ONSET_RELEASE_MODE>` | `-F` | How onset and release points get added to a segment | none |
| `--onset-duration <ONSET_DURATION>` | `-u` | Onset duration in seconds. Only used once `--onset-release-mode` is not `none` | 0 |
| `--release-duration <RELEASE_DURATION>` | `-U` | Release duration in seconds. Only used once `--onset-release-mode` is not `none` | 0 |
| `--time-shift <TIME_SHIFT>` | `-n` | Constant time shift in seconds, applied to every written point's own timestamp | 0 |

## Example

```
pvc spectrummapper --segments-file tracks.bin --scatter-file scatter.txt input.wav
```
