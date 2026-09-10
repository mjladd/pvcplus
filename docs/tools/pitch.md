# pvc pitch

Pitch transposition only. This is a shortcut for `pvc pv --pitch <semitones>` with every other option at its default.

## Usage

```
pvc pitch [OPTIONS] --semitones <SEMITONES> <INPUT> <OUTPUT>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--semitones <SEMITONES>` | Pitch shift in semitones. Positive goes up, negative goes down | required |

## Example

```
pvc pitch --semitones -12 input.wav output.wav
```
