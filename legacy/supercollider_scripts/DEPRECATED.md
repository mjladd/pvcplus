# Deprecated

The SuperCollider (SC) scripts in this directory are deprecated. They still run, unchanged, through `pvc legacy <tool> <flags>`. They get no further documentation updates and no port to the new `pvc` CLI.

## Use instead

For a single transform, use a preset with `pvc run`:

```bash
pvc preset init pv > my-preset.toml
pvc run my-preset.toml --set stretch=1.5 --set output=out2.wav
```

This replaces the SC scripts' own variable-setting layer (`setRoutineVariables`). A preset is the same idea as an `S.*` script's own variable block, just in TOML instead of shell or SuperCollider syntax. `pvc migrate-script` turns an existing `S.plainpv`-shaped script into a starting preset directly. See [docs/presets.md](../../docs/presets.md).

For batch processing over many files, the SC scripts' own `addCommand`/`runCommands` layer has no direct `pvc` equivalent yet. Run `pvc run` once per file from a shell loop instead:

```bash
for f in *.wav; do
  pvc run my-preset.toml --set input="$f" --set output="processed-$f"
done
```
