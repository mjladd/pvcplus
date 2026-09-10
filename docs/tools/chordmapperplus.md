# pvc chordmapperplus

Data-file-driven multi-tone additive chord and harmony synthesizer. This is the largest tool in this project's Phase 5 port. Each tone in the `--tones` data file picks a source point out of the `--analysis` `.pva` file. It builds a set of partials around that point, then resynthesizes them. The result blends the live per-frame analysis with a static spectral-morph average of the whole file. It ports `chordmapperplus`. See `rust/crates/pvc-core/src/tools/chordmapperplus.rs` for a full, phase-by-phase account of what is in and out of scope. That file documents a real per-band clamping bug, and a real naming-versus-behavior mismatch in `--pitch-change-expansion`. Source-signal mixing and the auto-adjust center-frequency and bandwidth refinement are not implemented, and have no flag here.

## Usage

```
pvc chordmapperplus [OPTIONS] --analysis <ANALYSIS> --tones <TONES> <OUTPUT>
```

## Options

### Core

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--analysis <ANALYSIS>` | `-f` | Path to the `.pva` analysis file every tone's own source point draws from | required |
| `--tones <TONES>` | `-F` | Path to the tone data file. Each tone is 23 whitespace-separated fields: source point, transpose point, partial spacing, count, bandwidth, spectral stretch, tone and noise levels, and more | required |
| `--window-size <WINDOW_SIZE>` | `-M` | Resynthesis window length. `0` means auto, twice the analysis file's own FFT size | 0 |
| `--frames-per-sec <FRAMES_PER_SEC>` | `-I` | Frames per second. This sets the hop size. Any value under `32` resets to `200` | 200 |
| `--duration <DURATION>` | | Output duration in seconds. `0` means to use the analysis file's own duration | 0 |
| `--channel <CHANNEL>` | `-C` | Which output channel to resynthesize. `0` writes every channel any tone's own routing uses, each independently. A number of `1` or higher resynthesizes only that one channel | 0 |
| `--threshold <THRESHOLD>` | `-t` | Oscillator-bank resynthesis threshold in decibels. Bins quieter than this are skipped entirely | -96 |

### Analysis navigation

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--time-origin <TIME_ORIGIN>` | `-x` | Time-position origin in seconds into the analysis data. Give a plain number, or `@path` | 0 |
| `--rate <RATE>` | `-Y` | Analysis-data playback rate multiplier. Give a plain number, or `@path` | 1 |
| `--window-low <WINDOW_LOW>` | `-g` | Analysis time window low boundary in seconds. Give a plain number, or `@path` | 0 |
| `--window-high <WINDOW_HIGH>` | `-k` | Analysis time window high boundary in seconds. A negative value means the end of the analysis data. Give a plain number, or `@path` | -1 |
| `--loop-smooth <LOOP_SMOOTH>` | `-/` | Sampler-loop boundary smoothing time in seconds. Give a plain number, or `@path` | 0.2 |
| `--window-mode <WINDOW_MODE>` | `-z` | Time-window behavior. `autostop` stops once time exits the window. `loop` wraps, folds, or clips at its edges forever | loop |
| `--loop-mode <LOOP_MODE>` | `-Q` | Sampler-loop boundary behavior. Only used in `loop` window mode | wrap |
| `--onset-release <ONSET_RELEASE>` | `-@` | Onset and release segment mode | off |
| `--loop-normalize` | `-e` | Crossfade the analysis channel's own overall amplitude toward a reference level at the loop window's two boundaries. Only in `loop` window mode | off |

### Tone-level gain and transposition

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--master-gain <MASTER_GAIN>` | `-A` | Master gain in decibels. Give a plain number, or `@path` | 0 |
| `--tones-gain <TONES_GAIN>` | `-m` | Master gain applied to every tone, in decibels. Give a plain number, or `@path` | 0 |
| `--tones-freq-shift <TONES_FREQ_SHIFT>` | `-q` | Frequency shift in Hz, applied to every tone whose own master transposition switch is on. Give a plain number, or `@path` | 0 |
| `--tones-pitch <TONES_PITCH>` | `-X` | Pitch transposition in semitones, applied to every tone whose own master transposition switch is on. Give a plain number, or `@path` | 1 |

### Rate-correlated dynamics

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--rate-correlated-tone <RATE_CORRELATED_TONE>` | `-T` | Rate-correlated tone-level control, in decibels. Give a plain number, or `@path`. This has no audible effect unless `--rate` varies away from `1` | 0 |
| `--rate-correlated-noise <RATE_CORRELATED_NOISE>` | `-E` | Rate-correlated noise-level control, in decibels. Give a plain number, or `@path` | 0 |
| `--rate-correlated-force-suppression` | `-B` | Scale the harmony and noise stasis medians down as `--rate` approaches `0` | off |

### Noise bands

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--noise-limit <NOISE_LIMIT>` | `-L` | Noise band decibel-limiter offset. Give a plain number, or `@path` | 0 |
| `--noise-limit-rolloff <NOISE_LIMIT_ROLLOFF>` | `-b` | Noise band decibel-limiter rolloff. Give a plain number, or `@path` | 0 |
| `--pitch-change-expansion <PITCH_CHANGE_EXPANSION>` | `-S` | Despite the original C's own "expansion" naming, a negative value here quiets a noise bin once its own frequency stays stable for a while. Give a plain number, or `@path` | -50 |
| `--frequency-change-threshold <FREQUENCY_CHANGE_THRESHOLD>` | `-c` | A noise bin only gets that suppression once its own smoothed frequency-change metric drops below this. Give a plain number, or `@path` | 0.1 |
| `--frequency-change-response <FREQUENCY_CHANGE_RESPONSE>` | `-h` | Seconds the frequency-change metric above takes to rise. Its own fall is always instant. Give a plain number, or `@path` | 0.1 |

### Vibrato

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--vibrato-rate <VIBRATO_RATE>` | `-r` | Synthetic vibrato rate in Hz, for tones whose own data-file switch enables it. Give a plain number, or `@path` | 6 |
| `--vibrato-period-mechanical <VIBRATO_PERIOD_MECHANICAL>` | `-U` | `1`, mechanical, uses the current natural-vibrato segment's own period length. `0`, natural, uses the detected average throughout. Only consulted with `--natural-vibrato` | 1 |
| `--natural-vibrato` | `-R1` | Detect natural vibrato periods from `--original-audio`, and use the detected loop window in place of `--window-low` and `--window-high`. Requires `--original-audio` and `--vibrato-reference` | off |
| `--original-audio <ORIGINAL_AUDIO>` | | The original sound file `--natural-vibrato` runs pitch tracking against. The original C never actually assigns its own equivalent global, so this port takes an explicit flag instead of reproducing that unset state | none |
| `--vibrato-reference <VIBRATO_REFERENCE>` | `-u` | The reference fundamental frequency vibrato gets detected around, in Hz or octave.pitchclass at `12` or below. Required with `--natural-vibrato` | none |
| `--vibrato-deviation <VIBRATO_DEVIATION>` | `-V` | How far a period's own length can drift from the running median, as a proportion, and still count as in-vibrato. Past this limit, the search widens its own threshold and retries | 0.05 |

## Example

```
pvc analyze input.wav input.pva
pvc chordmapperplus --analysis input.pva --tones chord.txt output.wav
```
