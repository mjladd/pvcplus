# Tutorial: a full walkthrough

[Getting started](getting-started.md) gets you a working `pvc` and one quick transform. This page goes further: chaining tools together, building and reusing presets, and scripting `pvc` from another program. Every command below was run for real while writing this page.

## Setup

```bash
mkdir -p ~/pvctut
sox -n -r 44100 -c 1 ~/pvctut/oboe.wav synth 2 sine 440
```

No `sox`? Any short mono WAV file works. Save it as `~/pvctut/oboe.wav`.

## Look before you transform

```bash
pvc info ~/pvctut/oboe.wav
```

This prints sample rate, channel count, and duration. Add `--json` before `info` for machine-readable output, covered later in this page.

## One transform at a time

Stretch the file to one and a half times its length:

```bash
pvc stretch --factor 1.5 ~/pvctut/oboe.wav ~/pvctut/stretched.wav
```

Every command that writes audio prints a one-line summary: duration, channel count, sample rate, and peak level. If any samples exceeded full scale, the summary also gives a clipped-sample count.

## Chaining tools

`pvc` has no pipe operator of its own. Chain tools the same way you chain any other command-line programs: write one tool's output to a file, then read that file as the next tool's input.

Add a phase-vocoder reverb tail to the stretched file:

```bash
pvc ring --feedback-decay 4 --feedback-gain -6 \
  --input-eq-low-gain 0 --loop-eq-low-gain 0 --output-eq-low-gain 0 \
  ~/pvctut/stretched.wav ~/pvctut/reverbed.wav
```

The three `--*-eq-low-gain 0` flags matter here. `ring`'s own low-shelf EQ gain defaults to `200` dB in all three of its EQ stages. This is a real bug in the original C tool, reproduced here on purpose (see [docs/tools/ring.md](tools/ring.md)). With the feedback loop on, that default compounds every time a frame recirculates and the output goes silent. Passing `0` for each avoids it.

## Presets: save a transform for reuse

Typing the same long flag list every time gets old. A preset is a TOML file that remembers it for you:

```bash
pvc preset init pv > ~/pvctut/my-pv.toml
```

Edit `~/pvctut/my-pv.toml`'s `input`/`output` fields and whichever `[params]` values you want, then run it:

```bash
pvc run ~/pvctut/my-pv.toml
```

Override one field without editing the file:

```bash
pvc run ~/pvctut/my-pv.toml --set stretch=2.0 --set output=~/pvctut/double.wav
```

`pvc preset list` shows the five example presets bundled with this project (`examples/presets/`). Run `make demo` in a checkout of this repository for a ready-to-hear tour of all five. See [Presets](presets.md) for the full field-mapping rules.

## Scripting `pvc` from another program

Every command that normally prints human-readable text accepts `--json` instead:

```bash
pvc --json info ~/pvctut/reverbed.wav
```

```json
{
  "kind": "audio",
  "path": "/home/you/pvctut/reverbed.wav",
  "sample_rate": 44100,
  "channels": 1,
  "frames": 312840,
  "duration_secs": 7.093877551020408
}
```

The same applies to the run summary every audio-writing command prints, and to `pvc preset list`. Parse this with any JSON-capable language instead of scraping human-readable text.

## Coming from an old `S.*` script

If you have an `S.plainpv`-style shell script from the original toolkit, migrate its variable block into a preset directly:

```bash
pvc migrate-script legacy/scripts/S.plainpv > ~/pvctut/migrated.toml
```

Only `S.plainpv`-shaped scripts are mapped today. Take the legacy tool's own display and auto-play options as an example: they have no equivalent in the new `pv` command, so the migrated preset lists them in a trailing comment instead of dropping them silently. See [Migration](migration.md) for every other legacy tool's own new equivalent.

## When a tool has no native `pvc` command yet

`pvc legacy <tool> <flags>` runs the original C tool directly, its own original flags unchanged:

```bash
pvc legacy plainpv -N1024 -P7 ~/pvctut/oboe.wav ~/pvctut/legacy-pitched.wav
```

Use this for anything [Migration](migration.md) marks as "not yet reachable as a `pvc` command" or "not planned."

## Where to go next

- [Concepts](concepts.md): the phase-vocoder ideas most commands share.
- `docs/tools/<name>.md`: one page per `pvc` subcommand's own flags.
- `pvc completions <shell>` and the man pages under `man/` for day-to-day use in a shell.
