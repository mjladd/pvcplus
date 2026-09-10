# pvc impulseresponse

Impulse-response analysis. This command zero-pads and runs an FFT over a `[--begin, --end)` window of each input channel, then writes a peak-normalized `.ir` file. It is part of Phase 5's FFT-convolution family. See `rust/crates/pvc-core/src/tools/impulseresponse.rs` for what is out of scope. It ports `impulseresponse`'s analysis path.

## Usage

```
pvc impulseresponse [OPTIONS] <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--begin <BEGIN>` | Analysis window start, in seconds | 0 |
| `--end <END>` | Analysis window end, in seconds. `0` means the end of the file | 0 |
| `--normalization <NORMALIZATION>` | How the per-channel spectra get peak-normalized | together |
| `--normalization-db <NORMALIZATION_DB>` | Normalization target level, in dB | 0 |

## Example

```
pvc impulseresponse --begin 0 --end 2 room.wav room.ir
```
