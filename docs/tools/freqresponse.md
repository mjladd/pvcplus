# pvc freqresponse

Analysis-driven `.fr` frequency response. This command builds a response file from a sound file's spectrum, accumulated by average or peak amplitude across all channels combined. It also detects formants and can normalize or compand formant bands. It ports `freqresponse`'s audio path. See `rust/crates/pvc-core/src/tools/freqresponse.rs` for what is in and out of scope. Plot, ASCII, and binary formant report files are not ported, since those are pure reporting.

## Usage

```
pvc freqresponse [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--fft <FFT>` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis window length. `0` means auto (`2 * fft`) | 2048 |
| `--window <WINDOW>` | Analysis window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Analysis frames per second. This sets the hop size | 200 |
| `--spectrum-type <SPECTRUM_TYPE>` | How frames accumulate into the response spectrum | average |
| `--weight-average` | Weight the average toward louder frames, using each frame's amplitude sum raised to the 5th power. Only meaningful with `--spectrum-type average` | off |
| `--formant-normalize` | Normalize the response by its formant peaks. Bins between formants get cross-faded gain from the bounding formants | off |
| `--formant-warp <FORMANT_WARP>` | Warp index for mid-formant amplitude compression or expansion. A positive value expands the dynamic range between formants. A negative value compresses it. This applies even with `--formant-normalize` off | 0 |
| `--freq-low <FREQ_LOW>` | Low frequency limit for formant detection, in Hz | 0 |
| `--freq-high <FREQ_HIGH>` | High frequency limit for formant detection, in Hz. `0` means Nyquist | 0 |
| `--formant-floor <FORMANT_FLOOR>` | Minimum formant peak amplitude, in dB | -96 |
| `--formant-threshold <FORMANT_THRESHOLD>` | Formant selection threshold, between `0` and `1`. Higher values select fewer, stronger formants | 0.5 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | Low shelf EQ gain in dB | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | High shelf EQ gain in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | Low shelf EQ frequency in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | High shelf EQ frequency in Hz | 2000 |
| `--no-normalize` | Skip EQ and its peak normalization entirely | off |

## Example

```
pvc freqresponse --formant-normalize input.wav response.fr
```
