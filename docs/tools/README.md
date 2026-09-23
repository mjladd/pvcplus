# Tool pages

One page per `pvc` command: what it does, every flag it takes, and an example. The headings below are the same groups `pvc --help` prints, in the same order.

## Time and pitch

| Command | What it does |
|---|---|
| [`pvc pv`](pv.md) | Stretch, transpose, filter, and warp a sound |
| [`pvc stretch`](stretch.md) | Change duration without changing pitch |
| [`pvc pitch`](pitch.md) | Transpose in semitones, keeping the duration |
| [`pvc analyze`](analyze.md) | Write a .pva analysis file, without resynthesis |
| [`pvc twarp`](twarp.md) | Resynthesize along a moving point in a .pva file |
| [`pvc ratechanger`](ratechanger.md) | Resample at a variable rate (varispeed) |

## Filtering and convolution

| Command | What it does |
|---|---|
| [`pvc freqresponse`](freqresponse.md) | Analyze a sound into a .fr response file |
| [`pvc filter`](filter.md) | Filter with a fixed .fr response file |
| [`pvc tvfilter`](tvfilter.md) | Filter with a time-varying .pva analysis file |
| [`pvc denoise`](denoise.md) | Gate out noise learned from part of the input |
| [`pvc convolver`](convolver.md) | Multiply a sound's spectrum by a .pva file |
| [`pvc impulseresponse`](impulseresponse.md) | Write a .ir file from a window of a sound |
| [`pvc irconvolver`](irconvolver.md) | Convolve or deconvolve a sound with a .ir file |
| [`pvc irconvolvesequencer`](irconvolvesequencer.md) | Convolve through a morphing series of .ir files |
| [`pvc spectralextractor`](spectralextractor.md) | Split a sound into its tonal and noise parts |

## Amplitude warping

| Command | What it does |
|---|---|
| [`pvc compand`](compand.md) | Compand each bin against a fixed reference file |
| [`pvc spectwarp`](spectwarp.md) | Compand each bin against its own frame's peak |

## Additive synthesis

| Command | What it does |
|---|---|
| [`pvc harmonize`](harmonize.md) | Add transposed copies of frequency bands |
| [`pvc inharmonator`](inharmonator.md) | Remap partials onto a table of target partials |
| [`pvc chordmapperplus`](chordmapperplus.md) | Synthesize chords from a table of tones |
| [`pvc formantsmapper`](formantsmapper.md) | Remap source formants onto target formants |

## Resonance and reverb

| Command | What it does |
|---|---|
| [`pvc ring`](ring.md) | Resonate through a spectral feedback loop |
| [`pvc ringfilter`](ringfilter.md) | Resonate through a loop with a fixed .fr filter |
| [`pvc ringtvfilter`](ringtvfilter.md) | Resonate through a loop with a .pva filter |

## Frequency deviation

| Command | What it does |
|---|---|
| [`pvc delayfilter`](delayfilter.md) | Delay each bin by its own response-driven time |
| [`pvc filtdeviator`](filtdeviator.md) | Filter, plus per-bin delay and frequency deviation |
| [`pvc tvfiltdeviator`](tvfiltdeviator.md) | Time-varying filter, plus delay and deviation |

## Feature extraction

| Command | What it does |
|---|---|
| [`pvc envelope`](envelope.md) | Track amplitude over a frequency band |
| [`pvc centroid`](centroid.md) | Track the weighted mean frequency of a band |
| [`pvc flux`](flux.md) | Track frame-to-frame frequency change in a band |
| [`pvc pitchtrack`](pitchtrack.md) | Track the fundamental frequency over time |
| [`pvc peakformant`](peakformant.md) | Track the loudest bin's frequency over time |
| [`pvc specflattracker`](specflattracker.md) | Track how noise-like or tonal each frame is |
| [`pvc spectrummapper`](spectrummapper.md) | Track formant peaks into continuous tracks |

## Control functions

| Command | What it does |
|---|---|
| [`pvc fn`](fn.md) | Generate control functions and response files |

## Utilities

| Command | What it does |
|---|---|
| [`pvc convert-units`](convert-units.md) | Convert amplitude, decibel, Hz, and pitch units |

## Commands with no page here

These commands run the toolkit rather than transform sound, so other pages cover them:

- `pvc info` and `pvc legacy`: [Getting started](../getting-started.md) and the [Tutorial](../tutorial.md).
- `pvc preset` and `pvc run`: [Presets](../presets.md).
- `pvc migrate-script`: [Migration](../migration.md) and the [Tutorial](../tutorial.md).
- `pvc completions`: [Getting started](../getting-started.md).

Run `pvc <command> --help` for any command's own full description and flags. The man pages under `man/` hold the same text.
