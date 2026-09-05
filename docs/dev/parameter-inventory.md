# Parameter inventory (Task 1.4)

Source of truth for the new `pvc` CLI's flag names and the TOML preset
schema (§2.1/§2.2 of the modernization plan). Derived from two sources per
tool: its recorded `tests/golden/usage/<tool>.txt` (the authoritative text
a user actually sees) and, where usage() turned out to be incomplete or
wrong (found while building the golden harness - noted inline below), the
`case '<flag>':` switch in `legacy/pvc_src/<tool>.c` itself.

**Columns:**
- **Legacy flag** — the single-letter flag from `crack()`.
- **Proposed long name** — the new `pvc` CLI's `--long-name`. Follows the
  plan's §2.1 rules: descriptive, no abbreviations where avoidable, grouped
  by concept (e.g. every shelf-EQ flag becomes `--shelf-*`).
- **Type/range** — as documented; `0-N` for enumerated integer modes (the
  new CLI should take a name, e.g. `--window hamming`, not the integer).
- **Func?** — whether the legacy flag accepts `(func)`: a constant *or* a
  path to a control-function file (`crackstring`/`crackfloat`). The new
  CLI's equivalent takes `<number>` or `@path`, no sniffing (plan §2.1).
- **Default** — the legacy default, verbatim from usage() unless noted.

Flags shared across nearly every tool (`-N`, `-M`, `-w`, `-D`, `-b`/`-e`,
`-t`, `-p`/`-i`, `-_`, `-=`, `-C`, shelf EQ) are listed once in §1 and not
repeated in every per-tool table - they carry the same meaning everywhere
they appear (confirmed identical across every usage.txt reviewed).

---

## 1. Common flags (appear identically across most/all analysis-driven tools)

| Legacy | Proposed long name | Type/range | Func? | Default |
|---|---|---|---|---|
| `-N` | `--fft` | int, power of 2 | no | `1024` |
| `-M` | `--window-size` | int, power of 2 (0 = auto: `2*fft`) | no | `2*fft` |
| `-w` | `--window` | enum: `hamming\|rect\|blackman\|bartlett\|kaiser4`..`kaiser12\|blackman-harris\|nuttal\|blackman-nuttal\|flattop` | no | `hamming` |
| `-D` | `--frames-per-sec` | float | no | `200` |
| `-I` | `--stretch` (time expansion/contraction factor) | float | no | `1.0` |
| `-b` | `--begin` | seconds | no | `0.0` |
| `-e` | `--end` | seconds (`0`=EOF; some tools use `-1`=EOF - noted per tool) | no | `0.0` |
| `-C` | `--channel` | int (`0`=all/average) | no | `0` |
| `-A` | `--gain` | dB | yes | `0.0` |
| `-t` | `--osc-threshold` | dB | no | `-96` |
| `-p`/`-i` | `--report-interval` (on/off + interval combined into one option: `--report <seconds>`, absent = off) | float seconds | no | off / `.25` |
| `-_` | `--play` | enum: `off\|once\|prompt-once\|prompt-each\|N` (repeat count) | no | `off` |
| `-=` | `--rescale` | enum: `dB-level\|match-input\|bypass\|only-if-clipping` (0-96 numeric or the three special codes) | no | `1` (match input) |
| `-H`/`-X`/`-m`/`-R` (shelf EQ) | `--shelf-low-gain`/`--shelf-high-gain`/`--shelf-low-freq`/`--shelf-high-freq` | dB / dB / Hz / Hz | yes | `0`/`0`/`200`/`2000` |
| `-l`/`-L` | `--attack`/`--release` | seconds | yes | `0.0` |
| `-P` | `--pitch` (semitones) | float | yes | `0` |
| `-a` | `--freq-shift` (bin frequency adder, pre-`-P`) | Hz | yes | `0.0` |
| `-W` | `--warp` (magnitude/frequency response reshape index) | float (`>0` expand, `<0` compress) | yes | `0.0` |
| `-f`/`-F` (as a frequency window pair) | `--freq-low`/`--freq-high` | Hz | yes | `0` / Nyquist |

---

## 2. plainpv → `pvc pv`

843 lines, the template every other tool is structured like. `main()` picks
overlap-add resynthesis unless `-P`/`-a` are non-default, in which case it
switches to the oscillator bank (`obank = ptrans.n != 1. \|\| ptrans.A[0] != 0.
\|\| harmadd...`, `legacy/pvc_src/plainpv.c` around the `DETERMINE OVERLAP/ADD
OR OSCIL BANK RESYNTHESIS` comment) - this is the tolerance-kind split used
throughout `tests/golden/cases/`.

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-T` | `--filter-mode` | enum: `bandpass\|reject` | no | `bandpass` (0) |
| `-f`/`-F` | `--freq-low`/`--freq-high` (brickwall, pre `-P`/`-a`) | Hz | no | `0` / Nyquist |
| `-n` | `--display-frames` | int | no | `0` |
| `-u`/`-U` | `--display-bin-low`/`--display-bin-high` | Hz (`-1`=nyquist for high) | no | `-1` / Nyquist |
| `-S` | `--display` | enum: `off\|phase\|amp\|both` | no | `off` |
| `-c` | `--graph` | enum: `off\|freq\|db\|db-waterfall` | no | `off` |
| `-d` | `--graph-file` | path | no | `./ascii.out` |

All other flags are the §1 common set. `--pitch @path` is the plan's
worked example of a func-able parameter (§1.2's `-P@ramp` case).

## 3. pvanalysis → `pvc analyze`

Writes the analysis file `twarp`/`tvfilter`/etc. consume (§2.4's `.pva`
format decision: new format only, no legacy write path).

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-p` | `--print-elapsed` | bool | no | off |
| `-P` | `--print-spectrum` | bool, or int > 1 = hi cutoff Hz for the printout | no | off |
| `-W` | `--warp` | float | yes | `0.0` |

`-H`/`-X`/`-m`/`-R` here are **not** func-able (no `(func)` tag, unlike
every other tool's shelf EQ), and `-A` (gain) also isn't - worth flagging
in the CLI design rather than silently making them func-able for
consistency with the rest of the shelf-EQ/gain family.

## 4. twarp → `pvc twarp`

Time-varying resynthesis driven by a `.pva` analysis file; always
oscillator-bank (no overlap-add path exists). **Output file must already
exist** with a valid header (`outfile_setup` opens it `SFM_READ` first
purely to read sample rate/channels - it never creates one; found via the
golden harness, see `tests/golden/cases/twarp/*.toml`). The new CLI should
just create the output file itself rather than reproducing this quirk.

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-d` | `--duration` | seconds | no | (required) |
| `-F` | `--analysis` | path (`.pva`) | no | (required) |
| `-Q` | `--time-origin` | seconds | yes | `0.0` |
| `-Y` | `--rate` (multiplier: `1`=original, `2`=2x, negative=reverse, `0`=stationary) | float | yes | `1.0` |
| `-T` | `--time-dither` | seconds | yes | `0.0` |
| `-K` | `--time-response` | seconds | yes | `0.0` |
| `-g`/`-G` | `--window-low`/`--window-high` | seconds | yes | `0.0` / EOF |
| `-S` | `--window-mode` | enum: `autostop\|loop` | no | `autostop` |
| `-o` | `--loop-mode` | enum: `wrap\|fold\|clip` | no | `wrap` |
| `-r` | `--onset-release` | bool | no | off |
| `-b` | `--loop-smooth` (peak-seam lowpass smoothing time) | seconds | yes | `0` |
| `-v` | `--loop-normalize` | bool | no | off |
| `-B`/`-e` | `--shimmer-db`/`--shimmer-response` (random amplitude variation) | dB / seconds | yes | `0.0` |
| `-j`/`-k`/`-n` | `--freq-variation`/`--freq-variation-response`/`--freq-variation-curve` | proportion / seconds / index | yes | `0.0` |
| `-I`/`-J`/`-N` | `--variation-rolloff`/`--variation-cutoff`/`--variation-shape` | Hz / Hz / index | yes | `22050` / `0` / `0` |
| `-f` | `--freq-response-time` | seconds | yes | `0.0` |

Note `-b` is reused for two unrelated things across the codebase (begin
time in most tools, loop-smooth time here) - the long names disambiguate
this; keep an eye out for other such collisions when designing the shared
flag-name table.

## 5. freqresponse → `pvc freqresponse`

Analysis-driven `.fr` file writer (as opposed to filtresponsemaker/
chordresponsemaker's synthesis).

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-c` | `--spectrum-type` | enum: `average\|peak` | no | `average` |
| `-W` | `--weight-average` | bool | no | off |
| `-p` | `--formant-normalize` | bool | no | off |
| `-E` | `--formant-warp` | float | no | `0` |
| `-L`/`-j` | `--freq-low`/`--freq-high` | Hz | no | `0` / Nyquist |
| `-A` | `--formant-floor` (min formant peak amplitude) | dB | no | `-96` |
| `-g` | `--formant-threshold` | `0-1` | no | `.5` |
| `-B` | `--no-normalize` (inverted: "EQ with normalization, 0=yes 1=no") | bool | no | `0` (normalize) |
| `-a`/`-o`/`-i`/`-F` | `--plot-db`/`--plot-formants`/`--formants-ascii`/`--formants-file` | paths | no | none |
| `-P` | `--printout-cutoff` | Hz (`0`=off) | no | `0` |

`-A` here (gain, listed as `A: minimum formant peak amplitude in dB` in
usage) is **not** the common `-A`=gain flag - a naming collision worth
resolving in the new CLI (`--formant-floor`, not `--gain`, for this one).

## 6. filtresponsemaker → `pvc fn response filtresponsemaker` / chordresponsemaker → `pvc fn response chordresponsemaker`

Both synthesize a `.fr` file from a breakpoint/partial-table data file
rather than analyzing a soundfile; `filter` then applies whichever `.fr`
it's pointed at.

**filtresponsemaker:**

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-A` | `--auto-adjust-limit` (max FFT size in auto-adjust mode; `0`=off) | int | no | `0` |
| `-F` | `--breakpoints` | path (ASCII: `freq-or-octave.pc, dB` duples, unordered) | no | (required) |
| `-f` | `--target-sound-file` (sample rate source) | path | no | (required) |
| `-i` | `--mode` | enum: `bandpass\|reject` | no | `bandpass` |
| `-a`/`-v` | `--plot-db`/`--printout-cutoff` | path / Hz | no | none / `0` |

**chordresponsemaker:**

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-R` | `--sample-rate` | int | no | `44100` — **misleading**: chordresponsemaker.c unconditionally reads the sample rate from `-f`'s soundfile via `sf_open`, regardless of `-R`; found via the golden harness (`tests/golden/cases/filter/chordresponsemaker_driven.toml`). `-f` isn't even listed in chordresponsemaker's own usage() text despite being accepted by `crack()`. The new CLI should just take `--sample-rate` for real and not require a dummy input file. |
| `-f` | *(undocumented in legacy usage - see above)* `--sample-rate-file` | path | no | (effectively required) |
| `-F` | `--partials` | path (ASCII sextuples: pitch, numPartials, bandwidth-prop, dB, spacing-prop, rolloff/octave) | no | (required) |
| `-s` | `--accumulation` | enum: `peak\|sum` | no | `peak` |
| `-D` | `--band-window` | enum: `triangle\|rectangle` | no | `triangle` |
| `-i`/`-v`/`-a` | same as filtresponsemaker | | | |

## 7. noisefilter → `pvc denoise`

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-B`/`-E` | `--noise-begin`/`--noise-end` | seconds | no | `0.0` |
| `-F` | `--noise-method` | enum: `average\|peak` | no | `average` |
| `-Z` | `--noise-printout-cutoff` | Hz (`0`=off) | no | `0` |
| `-S` | `--noise-threshold-gain` | dB | yes | `0.0` |
| `-x` | `--expansion-index` | float | no | `3.0` |
| `-Q` | `--noise-bypass-threshold` | dB | no | `1.0` |
| `-y` | `--analysis-printout-cutoff` | Hz | no | `22050` |

## 8. compander → `pvc compand`

Always requires `-F` (a peaks/frequency-response file) - not optional
despite usage() not marking it required; confirmed via the golden harness
(`compander -o-30 -O15 <in> <out>` with no `-F` exits with "YOU MUST
PROVIDE A PEAKS FILE. BYE.").

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-F` | `--peaks-file` | path | no | **(required)** |
| `-S` | `--peaks-smoothing` (bandwidth; sign selects octave vs. Hz) | float | no | `0` |
| `-Z` | `--peaks-printout-cutoff` | Hz | no | `0` |
| `-o`/`-O` | `--compress-threshold`/`--compress-amount` | dB / dB | yes | `0` / `0` |
| `-q`/`-Q` | `--expand-threshold`/`--expand-amount` | dB / dB | yes | `-96` / `0` |
| `-c`/`-d` | `--band-low`/`--band-high` | Hz | yes | `0` / Nyquist |
| `-f` | `--band-rolloff` | octaves | yes | `0.0` |

## 9. spectwarper → `pvc spectwarp`

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-o`/`-O` | `--compress-threshold`/`--compress-amount` | dB / dB | yes | `-0` / `-0` |
| `-q`/`-Q` | `--expand-threshold`/`--expand-amount` | dB / dB | yes | `-96` / `-0` |
| `-r` | `--response-time` | seconds (sign selects smooth-into vs sharpen) | yes | `0.0` |
| `-c`/`-d`/`-f` | `--band-low`/`--band-high`/`--band-rolloff` | Hz / Hz / octaves | yes | `0` / Nyquist / `0` |
| `-S` | `--compress-window` | octaves (`0`=off, one peak for all) | yes | `0.0` |
| `-g` | `--complement` | `0-1` | yes | `0.0` |
| `-n` | `--normalize-limit` | dB | no | `0` |
| `-Z` | `--print` | bool | no | off |

## 10. filter → `pvc filter`

The biggest per-tool flag surface among the core set (~35 flags) — source
and filter response paths each get their own transposition/shift/gain, and
the filter's own response gets a full EQ→compand→warp→invert→smooth→
normalize chain.

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-B` | `--transpose-target` | enum: `source\|both` (apply `-P`/`-a` to source only, or source+filter) | no | `source` |
| `-r`/`-u`/`-J`/`-U` | `--source-pitch`/`--source-freq-shift`/`--source-gain`/`--source-delay` | semitones / Hz / dB / seconds | yes | `0` |
| `-F` | `--response-file` | path (`.fr`) | no | (required) |
| `-G` | `--response-mode` | enum: `pass\|reject` | no | `pass` |
| `-Z` | `--response-printout-cutoff` | Hz | no | `0` |
| `-T`/`-V` | `--response-pitch`/`--response-shift` | semitones / Hz | yes | `0` |
| `-S` | `--source-floor` | dB | yes | `0.0` |
| filter shelf EQ (`-H`/`-X`/`-m`/`-R`) | `--response-shelf-*` | (same as §1, but scoped to the filter response, not the source) | yes | `0`/`0`/`200`/`2000` |
| `-c`/`-d`/`-E`/`-g` | `--response-compress-threshold`/`--response-compress-amount`/`--response-expand-threshold`/`--response-expand-amount` | dB | no | `0`/`0`/`-96`/`0` |
| `-W` | `--response-warp` | float | yes | `0.0` |
| `-Q` | `--response-smoothing` | octaves/Hz | yes | `0` |
| `-h`/`-x` | `--response-delay`/`--delay-scaler` | seconds / float | yes | `0` |
| `-n`/`-v` | `--normalize-limit`/`--normalize-reference` | dB / enum(`input\|filter`) | no | `0` / `input` |
| `-q` | `--compare-file` (playback comparison only) | path | no | none |

## 11. harmonizer → `pvc harmonize`

Data-table format (§2.1's `pvc harmonize --table harm.toml`): the plan's
example already anticipates converting harmonizer's ASCII table into TOML
`[[band]]` entries. The table's 8 columns (from usage(), confirmed against
source at `legacy/pvc_src/harmonizer.c` since usage() undersells one of
them):

| Column | Legacy usage() name | Notes |
|---|---|---|
| 1 | shift factor | multiplier / adder / semitones, per `-z` |
| 2 | window low boundary | Hz or octave.pitchclass; `-1`=0Hz |
| 3 | window high boundary | Hz or octave.pitchclass; `-1`=Nyquist |
| 4 | **not documented in usage()** — "Center" per the printed table headers | a center frequency for the band's triangular window; **must differ from both boundaries** — `harmonizer.c` divides by `(centerChannel - lowChannel)` and `(highChannel - centerChannel)` with no zero-width-window guard, so `center == low` (or `== high`) segfaults. Found via the golden harness; documented in `tests/golden/fixtures/gen.sh`'s comment on `harmonizer_table.txt`, not fixed in the tool (degenerate input, not a bad default). The new CLI should validate this and error clearly instead of reproducing the crash. |
| 5 | peak decibels | |
| 6 | stopband/boundary decibels | |
| 7 | Q-index | `0`=linear, `+`=sharper/thinner, `-`=smoother/wider |
| 8 | time delay (seconds) | |

Also found via the harness: five "data file macro modifier" scalers
(`-Y`/`-U`/`-o`/`-O`/`-j`, i.e. shift-factor/frequency/peak-dB/stopband-dB/
time-delay scalers) are documented as defaulting to `1` but were actually
uninitialized globals defaulting to `0`, silently zeroing every table
column unless all five flags were passed explicitly - **fixed** in
`legacy/pvc_src/harmonizer.c` (see git history) rather than just noted,
since it broke the tool's basic documented default behavior, not an edge
case. The new CLI doesn't need an equivalent bug, but should note in its
own docs/tests that a data-table-driven case with defaults must produce
non-zero, pass-through-scaled values - that's exactly the regression this
bug would have reproduced if left unfixed and silently ported.

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-z` | `--shift-format` | enum: `multiplier\|adder\|semitones` | no | `multiplier` |
| `-U`/`-y` | `--table-freq-scale`/`--table-freq-shift` | float / Hz | no | `1` / `0` |
| `-o`/`-O` | `--table-peak-scale`/`--table-stopband-scale` | float | no | `1` / `1` |
| `-Q`/`-r` | `--table-stopband-shift`/`--table-q-shift` | dB / float | no | `0` / `0` |
| `-j`/`-n` | `--table-delay-scale`/`--table-delay-shift` | float / seconds | no | `1` / `0` |
| `-@`/`-Y` | `--table-shift-shift`/`--table-shift-scale` | float / float | no | `0` / `1` |
| `-a`/`-P`/`-G`/`-g` | `--source-freq-shift`/`--source-pitch`/`--source-gain`/`--source-delay` | Hz / semitones / dB / seconds | yes | `0` |
| `-q`/`-X`/`-m`/`-W` | `--voice-freq-shift`/`--voice-pitch`/`--voice-gain`/`--voice-warp` | Hz / semitones / dB / float | yes | `0` |
| `-J`/`-K`/`-Z` | `--amp-interp`/`--freq-interp`/`--time-interp` | `0-1` | yes | `1` |

## 12. envelope → `pvc envelope`, centroid → `pvc centroid`, fluxoid → `pvc flux`

These three (plus pitchtracker below) share one shape: FFT analysis over a
frequency detection band, a compressor/gate, distribution warp, and a
choice of ASCII vs. float output at an interpolated output rate. Their
usage() texts are near-identical; differences noted below.

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-X` | `--channel-method` | enum: `average\|peak` | no | `average` |
| `-Q` | `--band-type` | enum: `freq\|octave-pitchclass` | no | `freq` |
| `-f`/`-F` | `--band-low`/`--band-high` | Hz or octave.pc | yes | `0` / Nyquist |
| `-T`/`-G` | `--compress-threshold`/`--compress-amount` | dB | yes | `0` / `0` |
| `-S` | `--gate-threshold` | dB | yes | `-96` |
| `-W` | `--distribution-warp` | float | no | `0` |
| `-p` | `--print-elapsed` | bool | no | off |
| `-r` | `--output-rate` | samples/sec (interpolated) | no | `500` |
| `-g` | `--output-format` | enum: `ascii\|float` | no | `ascii` (`envelope`/`fluxoid`) or `float` (`pitchtracker`, see below) |
| `-P` | `--plot` | bool | no | off |

**envelope-only:** `-j`/`-k`/`-m` (`--filtered-attack`/`--filtered-release`/
`--filtered-cut`, the "envelope that can be subtracted out" feature) and
`-q` (`--output-scale`: `amp\|db\|inverted-amp\|inverted-db`).

**centroid-only:** `-H` (`--warp`, magnitude response reshape - a second,
different warp from `-W`'s distribution warp), `-q` (`--output-format`:
`freq\|octave\|octave-pitchclass\|semitones-deviation`), `-G`
(`--reference-pitch`, for the semitones-deviation format — **note this
collides with the common `-G`=compress-amount flag**; the long names
disambiguate).

**fluxoid-only:** `-A` (`--amplitude-weighting`, bool, default on) —
fluxoid's job (tracking bin-frequency change weighted by amplitude) makes
this the one flag that's genuinely fluxoid-specific.

## 13. pitchtracker → `pvc pitchtrack`

Same shape as §12 plus pitch-specific detection parameters.

| Legacy | Proposed | Type/range | Func? | Default |
|---|---|---|---|---|
| `-m` | `--method` | enum: `optimal-comb\|strongest\|centroid` | no | `optimal-comb` |
| `-f`/`-F` | `--band-low`/`--band-high` | Hz or octave.pc (`<=12` treated as octave.pc) | yes | `13` / Nyquist |
| `-j`/`-J` | `--window-min`/`--window-max` | seconds | no | `.05` / `.3` |
| `-d` | `--detect-threshold` | dB | no | `-40` |
| `-H` | `--mode-filter-window` | seconds | no | `0` |
| `-E` | `--oversample` | float | no | `0` |
| `-O` | `--output-format` | enum: `freq\|octave-decimal\|semitones\|neg-semitones\|midi\|octave-pitchclass` | no | `freq` |
| `-o` | `--reference` | Hz or octave.pc | yes | `440` |
| `-a` | `--smooth-response` | seconds | no | `0.0` |
| `-g` | `--output-format` (data type: ascii/float) | enum: `ascii\|float` | no | **`float`** (the one tool in this family that defaults to float, not ascii) |

## 14. reshape → `pvc fn reshape`

**Scope note, matching the plan's own §Phase-3.1 guidance** ("port the
modes used by `utilities/*` and `S.*` scripts first... document rest as
TODO"): `reshape.c` is 2489 lines with a `crack()` flag string covering
nearly the entire alphabet (`A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|
W|X|Y|Z|a|b|c|d|e|f||h|j|k|l|m|n|o|p|r|s|t|u|w|x|y|z`). No `S.reshape`
script exists in this repo to derive real-world usage from, and its bare
`reshape` invocation doesn't print a flag-by-flag usage() the way every
other tool does (it just starts reading stdin) - `tests/golden/usage/
reshape.txt` reflects that: it's a stdin-read transcript, not a help
banner. `tests/golden/cases/reshape/stdin_default.toml` covers only the
default (`-A0`) stdin-pipe path. **A full flag inventory for reshape is
deferred** to whoever actually ports it in Phase 3.1 - reading through
~2500 lines of an unfamiliar, densely-flagged file speculatively, with no
real caller to validate the reading against, isn't a good use of a
parameter-inventory pass; it belongs next to the porting work where each
flag's behavior can be verified against a real case as it's added.

One real bug found and fixed while getting reshape working for the
harness at all: it segfaulted on *every* invocation that reads from stdin
(including a bare no-args run) — two redundant `fclose(NULL)` calls in its
"find a unique /tmp scratch filename" loops (see git history, Task 1.2).

## 15. gen1..gen6 → `pvc fn gen1`..`pvc fn gen6`

CARL/cmusic control-function generators; output is a headerless native f32
stream (or ASCII to a tty). None take the common flags from §1 - each has
its own tiny, positional-argument grammar.

| Tool | Legacy usage | Notes |
|---|---|---|
| `gen1` | `-Llength t1 v1 ... tN vN` | Linear breakpoint envelope: `-L`=output length, then time/value pairs. |
| `gen2` | `-Llength [-o \| -c] a1...aN b0...bM N` | Sine/cosine sum. **Trap found via the golden harness**: the trailing `N` is a count of how many leading coefficients to actually use as sine terms, not the output length (`-L` already covers that) - passing a value larger than the actual coefficient count reads uninitialized stack floats as extra coefficients (UB), which usually manifests as "GEN2: all-zero function" rather than a crash. The new CLI should take separate, named `--sine-coeffs`/`--cosine-coeffs` lists so this class of mistake isn't representable. `-o`/`-c` select open vs. closed waveform. |
| `gen3` | `-Llength v1 v2 ... vN` | Constant-value table, one value per output sample position (`-L` sets both the read count and, implicitly, the table size). |
| `gen4` | `-Llength t1 v1 a1 ... tN vN` | Exponential-segment envelope: time/value/shape triples. |
| `gen5` | `-Llength h1 a1 p1 ... hN aN pN` | Harmonic sum from (harmonic-number, amplitude, phase) triples. |
| `gen6` | `-Llength` | No further arguments - a fixed/default table shape. |

---

## Cross-cutting naming collisions found while compiling this table

Worth resolving explicitly in the shared flag-name table rather than
per-tool, since the same legacy letter means different things depending on
which tool it's attached to:

- **`-A`**: gain (dB) almost everywhere, but "minimum formant peak
  amplitude" in `freqresponse` and "auto-adjust FFT limit" in
  `filtresponsemaker`.
- **`-G`**: compression amount (dB) in the envelope/centroid/fluxoid/
  pitchtracker family, but "reference pitch" in `centroid`'s `-q2` output
  format and "source gain" in `harmonizer`.
- **`-b`**: begin time almost everywhere, but "loop smooth time" in `twarp`.
- **`-B`**: noise-analysis begin time in `noisefilter`, but "transpose
  target" in `filter` and "EQ normalization flag" in `freqresponse`.
- **`-Q`**: detection-band data type in the envelope family, expansion
  amount in `compander`/`spectwarper`, response smoothing in `filter`,
  and a data-table stopband shifter in `harmonizer`.

None of these are bugs - each tool's `crack()` flag string is independent
- but they're exactly the kind of thing that makes memorizing the legacy
CLI hard, and exactly what moving to long names fixes. Listed here so the
shared-flag-name table (§1) doesn't accidentally reuse one name for two
unrelated legacy flags that happen to share a letter.
