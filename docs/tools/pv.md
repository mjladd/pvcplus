# pvc pv

Phase vocoder: analyze, then resynthesize. This is the core transform. It handles time-stretch, pitch transposition, frequency shift, gain, spectrum warp, shelf EQ, and a brickwall frequency-window filter. It ports `plainpv`'s audio path. See `rust/crates/pvc-core/src/tools/pv.rs` for exactly what is in and out of scope. Its debug and display-only flags are not ported.

## Usage

```
pvc pv [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--fft <FFT>` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis window length. This is not `2 * fft` by default, unlike what the flag name suggests. Pass `0` for that auto-scaling rule instead | 2048 |
| `--window <WINDOW>` | Analysis and synthesis window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Analysis frames per second. This sets the hop size | 200 |
| `--stretch <STRETCH>` | Time-stretch factor. `1.0` leaves duration unchanged | 1 |
| `--pitch <PITCH>` | Pitch shift in semitones. Give a plain number, or `@path` to a control file for a time-varying shift | 0 |
| `--freq-shift <FREQ_SHIFT>` | Frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--gain <GAIN>` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--attack <ATTACK>` | Amplitude envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | Amplitude envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--warp <WARP>` | Spectrum magnitude warp index. `0` applies no warp. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | Low shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | High shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | Low shelf EQ frequency in Hz. Give a plain number, or `@path` | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | High shelf EQ frequency in Hz. Give a plain number, or `@path` | 2000 |
| `--threshold <THRESHOLD>` | Oscillator resynthesis threshold in dB. The tool skips bins quieter than this, relative to the frame's own peak | -96 |
| `--filter-type <FILTER_TYPE>` | Brickwall frequency-window filter mode | bandpass |
| `--filter-low <FILTER_LOW>` | Brickwall filter low frequency bound in Hz | 0 |
| `--filter-high <FILTER_HIGH>` | Brickwall filter high frequency bound in Hz. `-1` means Nyquist | -1 |

## Example

```
pvc pv --stretch 2 --pitch 7 input.wav output.wav
```

This stretches the input to twice its length and transposes it up a perfect fifth.
