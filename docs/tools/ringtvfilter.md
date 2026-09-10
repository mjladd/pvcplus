# pvc ringtvfilter

`pvc ring` plus a switchable time-varying filter. The filter reads a `.pva` filter-analysis file, moved through over time the same way `pvc tvfilter` moves through its own response. It sits either on the feedback path's input, called "prefilter," or inside the loop, called "postfilter." It ports `ringtvfilter`'s audio path. See `rust/crates/pvc-core/src/tools/ringtvfilter.rs` for what it shares with `pvc ringfilter` and `pvc tvfilter`, and what differs.

## Usage

```
pvc ringtvfilter [OPTIONS] --filter-response <FILTER_RESPONSE> <INPUT> <OUTPUT>
```

## Options shared with `pvc ring`

Most flags on the [`pvc ring`](ring.md) page apply here unchanged.

- Core. `--fft`, `--window-size`, `--window`, `--frames-per-sec`, `--time-factor`, `--begin`, `--end`, `--oscbank-threshold`.
- Source and feedback. `--source-gain`, `--source-freq-shift`, `--source-pitch`, `--feedback-gain`, `--feedback-freq-shift`, `--feedback-pitch`, `--feedback-decay`, `--feedback-threshold`, `--attack`, `--release`.
- EQ. The full input, loop, and output EQ block. The same swapped-default bug applies here too.

This command has no `--master-gain` flag. It also has no `--feedback-threshold-mode` flag, since it offers no threshold pass-mode option at all.

## Time-varying filter navigation

These flags move through the filter response file over time, the same way `pvc tvfilter` moves through its own response.

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--filter-response <FILTER_RESPONSE>` | `-y` | Path to the time-varying filter response file, a `.pva` analysis file in either the legacy layout or `pvc analyze`'s own format | required |
| `--analysis-channel <ANALYSIS_CHANNEL>` | | Which filter-file channel to use. `0` pairs by channel index with the input sound file | 0 |
| `--time-origin <TIME_ORIGIN>` | | Filter time point origin in seconds. Give a plain number, or `@path` | 0 |
| `--rate <RATE>` | | Filter rate multiplier. Give a plain number, or `@path` | 1 |
| `--window-low <WINDOW_LOW>` | | Filter time window low boundary in seconds. Give a plain number, or `@path` | 0 |
| `--window-high <WINDOW_HIGH>` | | Filter time window high boundary in seconds. A negative value means the end of the filter file. Give a plain number, or `@path` | -1 |
| `--loop-mode <LOOP_MODE>` | | Sampler-loop boundary behavior. Only used outside autostop mode | wrap |
| `--onset-release` | | Trigger the time window only once it is first entered | off |
| `--autostop` | | Stop synthesis once the filter's time position exits its window, instead of looping | off |
| `--loop-normalization` | | Keep the filter's own amplitude roughly continuous across a sampler loop's seam | off |
| `--loop-smooth <LOOP_SMOOTH>` | | Peak loop-seam smoothing time in seconds. Give a plain number, or `@path` | 0.2 |

## Filter shaping

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--filter-warpshape <FILTER_WARPSHAPE>` | | Reshapes the filter response curve. Give a plain number, or `@path` | 0 |
| `--comp-threshold <COMP_THRESHOLD>` | | Filter-spectrum compression threshold in dB, `0` or lower. A plain constant, not a control-file path | 0 |
| `--comp-db <COMP_DB>` | | Filter-spectrum decibels of compression, `0` or lower. A plain constant, not a control-file path | 0 |
| `--filter-transpose <FILTER_TRANSPOSE>` | | Filter response transposition in semitones. Give a plain number, or `@path` | 0 |
| `--filter-shift <FILTER_SHIFT>` | | Filter response shift in Hz. Give a plain number, or `@path` | 0 |
| `--filter-source-gain <FILTER_SOURCE_GAIN>` | | Blend between the filtered signal and the dry source, in dB. `-96` is fully filtered. `0` bypasses the filter entirely. Give a plain number, or `@path` | -96 |
| `--filter-decay <FILTER_DECAY>` | | Filter decay time in seconds. Only used with `--filter-placement postfilter`. Give a plain number, or `@path` | 1 |
| `--filter-placement <FILTER_PLACEMENT>` | | Where the filter sits: `prefilter` or `postfilter` | prefilter |
| `--filter-pitch-mode <FILTER_PITCH_MODE>` | | Whether the filter's own transpose or shift compensates for the reverb's own `--feedback-pitch` and `--feedback-freq-shift` | source-only |

## Example

```
pvc ringtvfilter --filter-response filter.pva --rate 0.5 input.wav output.wav
```
