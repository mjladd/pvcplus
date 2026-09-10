# Presets

A preset is a TOML file that `pvc run` reads and executes. It replaces
the old `S.*` shell scripts. It also replaces SuperCollider scripts,
which are now deprecated (see [migration](migration.md)).

## Creating a preset

`pvc preset init <tool>` prints an annotated starting point for a
tool:

```
pvc preset init pv > my-preset.toml
```

Today only `pv` has a bundled template. You can still write a preset
for any other tool by hand. Give it a `tool` field with the tool's own
`pvc` subcommand name, and a `[params]` entry for each flag you want
to set. Each `[params]` key must be the flag's own name in snake_case:
`frames_per_sec` for `--frames-per-sec`, `shelf_low_gain` for
`--shelf-low-gain`. Check the tool's own page under `tools/` for its
real flag names.

A preset looks like this:

```toml
tool = "pv"
input = "in.wav"
output = "out.wav"

[params]
fft = 2048
window = "kaiser8"
frames_per_sec = 400
stretch = 1.0
pitch = 0.0
shelf_low_gain = 0
shelf_low_freq = 500
```

## Running a preset

```
pvc run my-preset.toml
```

This prints the resolved preset, then runs the named tool with those
parameters. `pvc run` reuses the exact same argument parser as `pvc
<tool> ...` typed directly. A preset can only ever set flags that
tool's own command already accepts.

Two flags change this behavior:

- `--dry-run` prints the resolved preset and stops. It never runs the
  tool.
- `--quiet` skips the printed preset and only runs the tool.

## Overriding fields without editing the file

Use `--set key=value` to override one field at a time:

```
pvc run my-preset.toml --set stretch=2.0 --set output=stretched.wav
```

An unqualified key like `stretch` sets `[params].stretch`. A
`section.key` form, like `shelf_eq.low_gain`, targets a different
table. The bare names `tool`, `input`, and `output` set the preset's
own top-level fields, not a table entry. Repeat `--set` as many times
as you need.

## Boolean flags

A `true` value in `[params]` becomes a bare flag, for a switch-style
option. A `false` value omits the flag entirely. This only gives the
right result for a switch whose own default is already `false`. A
preset cannot force a default-`true` switch back off today.

## Bundled example presets

`pvc preset list` shows the example presets bundled with `pvc`, under
`examples/presets/`. That directory does not exist yet in this
project. A later change adds a first batch of demo presets there.
