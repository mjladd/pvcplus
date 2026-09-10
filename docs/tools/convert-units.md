# pvc convert-units

Converts between amplitude/decibel and Hz/octave.pitchclass units. This replaces four standalone legacy utilities: `amptodB`, `dBtoamp`, `Hztopitch`, and `pitchtoHz`. It takes one `--from`/`--to` unit pair instead.

## Usage

```
pvc convert-units [OPTIONS] --from <FROM> --to <TO> --value <VALUES>
```

## Options

| Flag | Description | Default |
|---|---|---|
| `--from <FROM>` | Unit to convert from: `amp`, `db`, `hz`, or `oppc` | required |
| `--to <TO>` | Unit to convert to: `amp`, `db`, `hz`, or `oppc` | required |
| `--norm <NORM>` | Normalization amplitude for `amp` to `db` conversions. Divides the amplitude by this before converting to decibels. Ignored for any other conversion | 1 |
| `--value <VALUES>` | A value to convert. Repeat this flag for more than one | required |

## Example

```
pvc convert-units --from db --to amp --value -6 --value -12
```
