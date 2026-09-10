# pvc spectralextractor

Periodic/noise spectrum separator. This command tracks each bin's frame-to-frame frequency deviation. It gates each bin on or off depending on whether that deviation stays under a threshold. The result extracts either the tonal part of a sound or its noise residue. It ports `spectralextractor`'s audio path. See `rust/crates/pvc-core/src/tools/spectralextractor.rs` for a real dead pitch/frequency-shift computation, reproduced faithfully, and what is out of scope.

## Usage

```
pvc spectralextractor [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--fft <FFT>` | `-N` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | `-M` | Analysis and resynthesis window length. `0` means auto (`2 * fft`). If the resynthesis hop needs more room, this command grows the window further | 0 |
| `--window <WINDOW>` | | Window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | `-D` | Frames per second. This sets the hop size | 200 |
| `--time-factor <TIME_FACTOR>` | `-I` | Time expansion or contraction factor. `1.0` leaves duration unchanged | 1 |
| `--pitch <PITCH>` | `-P` | Pitch transposition in semitones. Give a plain number, or `@path`. This has a real, narrow effect: it selects oscillator-bank resynthesis and shifts the shelf-EQ banding frequency, but never the output's actual pitch | 0 |
| `--freq-shift <FREQ_SHIFT>` | `-a` | Frequency shift adder in Hz. Give a plain number, or `@path`. Same narrow effect as `--pitch` | 0 |
| `--gain <GAIN>` | `-A` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--begin <BEGIN>` | `-b` | Begin time in seconds | 0 |
| `--end <END>` | `-e` | End time in seconds. `0` means the end of the file | 0 |
| `--spectral-type <SPECTRAL_TYPE>` | `-q` | Which part of the spectrum to keep | periodic |
| `--freq-change-threshold <FREQ_CHANGE_THRESHOLD>` | `-Q` | Maximum, in periodic mode, or minimum, in noise mode, frequency change allowed every 5 milliseconds, in Hz. Give a plain number, or `@path` | 0 |
| `--freq-change-response <FREQ_CHANGE_RESPONSE>` | `-g` | Response time of the frequency-change threshold accumulator, in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | `-L` | Amplitude-gate release time in seconds. Give a plain number, or `@path` | 0 |
| `--complement <COMPLEMENT>` | `-c` | Complement amplitude spectrum proportion, between `0` and `1`. Give a plain number, or `@path` | 0 |
| `--frame-norm-limit <FRAME_NORM_LIMIT>` | `-E` | Frame normalization decibel limit. `0` disables normalization. Give a plain number, or `@path` | 0 |
| `--warp <WARP>` | `-W` | Spectrum magnitude warp index. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | `-H` | Low shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | `-X` | High shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | `-m` | Low shelf EQ frequency in Hz. Give a plain number, or `@path` | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | `-R` | High shelf EQ frequency in Hz. Give a plain number, or `@path` | 2000 |
| `--threshold <THRESHOLD>` | `-t` | Oscillator-bank resynthesis threshold in dB. This only applies once `--pitch` or `--freq-shift` selects that resynthesis path | -96 |

## Example

```
pvc spectralextractor --spectral-type periodic input.wav tonal.wav
```
