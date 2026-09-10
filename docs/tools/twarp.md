# pvc twarp

Time-varying resynthesis. This command moves a virtual time position through a `.pva` analysis file, driven by rate, origin, and window settings. It resynthesizes from whatever frame that position lands on. It ports `twarp`'s audio path. See `rust/crates/pvc-core/src/tools/twarp.rs` for what is in and out of scope. Time-point dither, loop-boundary amplitude normalization, and random amplitude and frequency "shimmer" are not ported yet.

## Usage

```
pvc twarp [OPTIONS] <ANALYSIS> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `<ANALYSIS>` | Path to the `.pva` analysis file to resynthesize from | required |
| `--duration <DURATION>` | Output duration in seconds. `0` means to use the analysis file's own duration | 0 |
| `--window-size <WINDOW_SIZE>` | Analysis window length. `0` means auto (`2 * fft`, using the analysis file's own FFT size) | 2048 |
| `--window <WINDOW>` | Resynthesis window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Resynthesis frames per second. This sets the hop size. There is no independent stretch factor, unlike `pv` | 200 |
| `--time-origin <TIME_ORIGIN>` | Time-position origin in seconds into the analysis data. Give a plain number, or `@path` | 0 |
| `--rate <RATE>` | Playback rate multiplier. `1` plays at original speed, `2` doubles speed, a negative value reverses, `0` holds still. Give a plain number, or `@path` | 1 |
| `--window-low <WINDOW_LOW>` | Analysis time window low boundary in seconds. Give a plain number, or `@path` | 0 |
| `--window-high <WINDOW_HIGH>` | Analysis time window high boundary in seconds. A negative value means the end of the analysis data. Give a plain number, or `@path` | -1 |
| `--time-response <TIME_RESPONSE>` | Smoothing time in seconds for time-position changes. Give a plain number, or `@path` | 0 |
| `--loop-smooth <LOOP_SMOOTH>` | Sampler-loop boundary smoothing time in seconds. Give a plain number, or `@path` | 0.2 |
| `--window-mode <WINDOW_MODE>` | Time-window behavior. `autostop` stops once time exits the window. `loop` wraps, folds, or clips at its edges forever | loop |
| `--loop-mode <LOOP_MODE>` | Sampler-loop boundary behavior. Only used in `loop` window mode | wrap |
| `--onset-release` | Trigger the time window only once it is first entered. Then extend the output duration on the way out, so playback always reaches the window's edge. Use this for percussive onset, sustain-loop, and release shaping | off |
| `--pitch <PITCH>` | Pitch shift in semitones. Give a plain number, or `@path`. A nonzero (or time-varying) value picks oscillator-bank resynthesis. Leaving this and `--freq-shift` both at `0` picks overlap-add instead | 0 |
| `--freq-shift <FREQ_SHIFT>` | Frequency shift in Hz. Give a plain number, or `@path` | 0 |
| `--gain <GAIN>` | Gain in dB. Give a plain number, or `@path` | 0 |
| `--attack <ATTACK>` | Amplitude envelope attack time in seconds. Give a plain number, or `@path` | 0 |
| `--release <RELEASE>` | Amplitude envelope release time in seconds. Give a plain number, or `@path` | 0 |
| `--freq-response-time <FREQ_RESPONSE_TIME>` | Smoothing time in seconds for frequency changes. Give a plain number, or `@path` | 0 |
| `--warp <WARP>` | Spectrum magnitude warp index. `0` applies no warp. Give a plain number, or `@path` | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | Low shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | High shelf EQ gain in dB. Give a plain number, or `@path` | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | Low shelf EQ frequency in Hz. Give a plain number, or `@path` | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | High shelf EQ frequency in Hz. Give a plain number, or `@path` | 2000 |
| `--threshold <THRESHOLD>` | Oscillator resynthesis threshold in dB | -60 |

## Example

```
pvc analyze input.wav input.pva
pvc twarp --rate 0.5 --duration 10 input.pva output.wav
```
