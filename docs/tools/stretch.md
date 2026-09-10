# pvc stretch

Time-stretch only. This is a shortcut for `pvc pv --stretch <factor>` with every other option at its default.

## Usage

```
pvc stretch [OPTIONS] --factor <FACTOR> <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--factor <FACTOR>` | Time-stretch factor. `1.0` leaves duration unchanged. `2.0` doubles it | required |

## Example

```
pvc stretch --factor 1.5 input.wav output.wav
```
