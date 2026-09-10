# pvc ringfilter

`pvc ring` plus a switchable fixed-spectrum filter. The filter reads a `.fr` response file, the same format `pvc filter` reads. It sits either on the feedback path's input, called "prefilter," or inside the loop, called "postfilter." It ports `ringfilter`'s audio path. See `rust/crates/pvc-core/src/tools/ringfilter.rs` for what it shares with `pvc ring` and what differs.

## Usage

```
pvc ringfilter [OPTIONS] --filter-response <FILTER_RESPONSE> <INPUT> <OUTPUT>
```

## Options shared with `pvc ring`

Every flag on the [`pvc ring`](ring.md) page applies here unchanged, with one exception: `--feedback-threshold-mode`'s legacy flag letter is `-g` here, not `-V`. `--fft`'s size must also match the filter response file's own size, or this command exits with an error.

The same low-shelf-gain/low-shelf-frequency swapped-default bug documented on the `pvc ring` page applies to all three EQ stages here too.

## Filter-specific options

| Flag | Legacy | Description | Default |
|---|---|---|---|
| `--filter-response <FILTER_RESPONSE>` | `-y` | Path to a `.fr` frequency response file, the same format `pvc filter` reads | required |
| `--filter-warpshape <FILTER_WARPSHAPE>` | `-q` | Reshapes the filter response curve. Give a plain number, or `@path` | 0 |
| `--filter-transpose <FILTER_TRANSPOSE>` | `-u` | Filter response transposition in semitones. Give a plain number, or `@path` | 0 |
| `--filter-shift <FILTER_SHIFT>` | `-V` | Filter response shift in Hz, applied before `--filter-transpose`. Give a plain number, or `@path` | 0 |
| `--filter-source-gain <FILTER_SOURCE_GAIN>` | `-x` | Blend between the filtered signal and the dry source, in dB. `-96` is fully filtered. `0` bypasses the filter entirely. Give a plain number, or `@path` | -96 |
| `--filter-decay <FILTER_DECAY>` | `-r` | Filter decay time in seconds. Only used with `--filter-placement postfilter`. Give a plain number, or `@path` | 1 |
| `--filter-placement <FILTER_PLACEMENT>` | `-o` | Where the filter sits: `prefilter` or `postfilter` | prefilter |
| `--filter-pitch-mode <FILTER_PITCH_MODE>` | `-B` | Whether the filter's own transpose or shift compensates for the reverb's own `--feedback-pitch` and `--feedback-freq-shift` | source-only |

## Example

```
pvc ringfilter --filter-response response.fr --filter-placement postfilter input.wav output.wav
```
