# pvc analyze

Phase vocoder analysis only. This writes a `.pva` frame file, with no resynthesis. It ports `pvanalysis`'s audio path. See `rust/crates/pvc-core/src/tools/analyze.rs` for what is in and out of scope. This command always writes the new `PVA1` format, described in `rust/crates/pvc-io/src/pva.rs`. It never writes the legacy layout, which this project only reads back, for comparing against the C oracle.

## Usage

```
pvc analyze [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--fft <FFT>` | FFT size. Must be a power of two | 1024 |
| `--window-size <WINDOW_SIZE>` | Analysis window length. This is not `2 * fft` by default. Pass `0` for that auto-scaling rule instead | 4096 |
| `--window <WINDOW>` | Analysis window shape | hamming |
| `--frames-per-sec <FRAMES_PER_SEC>` | Analysis frames per second. This sets the hop size | 200 |
| `--gain <GAIN>` | Gain in dB | 0 |
| `--shelf-low-gain <SHELF_LOW_GAIN>` | Low shelf EQ gain in dB | 0 |
| `--shelf-high-gain <SHELF_HIGH_GAIN>` | High shelf EQ gain in dB | 0 |
| `--shelf-low-freq <SHELF_LOW_FREQ>` | Low shelf EQ frequency in Hz | 200 |
| `--shelf-high-freq <SHELF_HIGH_FREQ>` | High shelf EQ frequency in Hz | 2000 |
| `--warp <WARP>` | Spectrum magnitude warp index. `0` applies no warp | 0 |

## Example

```
pvc analyze input.wav input.pva
```
