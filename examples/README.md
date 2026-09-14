# Examples

`examples/presets/` holds five ready-to-run [presets](../docs/presets.md), each showing a different kind of transform.

| Preset | Tool | What it does |
|---|---|---|
| `stretch.toml` | `pv` | Doubles the duration, same pitch. |
| `pitch.toml` | `pv` | Transposes up a perfect fifth, same duration. |
| `ring-reverb.toml` | `ring` | Adds a phase-vocoder feedback reverb tail. |
| `denoise.toml` | `denoise` | Gates out noise, using the first half second as the noise profile. |
| `spectwarp-compress.toml` | `spectwarp` | Compresses loud bins, per frequency bin. |

## Running the demo

```bash
make demo
```

This builds `pvc` and generates a short 440 Hz test tone with `sox`. It then runs all five presets against that tone, and writes the results to `examples/output/`. Play the files there and compare them against `examples/output/input.wav`.

`make demo` requires `sox`. Without it, run any preset by hand against your own audio file:

```bash
pvc run examples/presets/ring-reverb.toml --set input=my-file.wav --set output=result.wav
```

## Listing presets from `pvc` itself

```bash
pvc preset list
```
