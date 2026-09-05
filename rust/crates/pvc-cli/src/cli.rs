//! Top-level argument definitions (Task 2.7: the `pvc` skeleton). Only
//! `info`, `legacy`, `preset init/list`, and `run` exist here - the real
//! DSP subcommands (`pv`, `stretch`, `analyze`, ...) arrive one at a time
//! in Phase 3 as their tools are ported, per the plan's §2.1 CLI sketch.

use std::path::PathBuf;

use clap::{Parser, Subcommand};

#[derive(Parser, Debug)]
#[command(
    name = "pvc",
    version,
    about = "A modern CLI for the PVCplus phase-vocoder toolkit."
)]
pub struct Cli {
    /// Suppress non-essential output.
    #[arg(long, global = true)]
    pub quiet: bool,

    /// Emit machine-readable JSON instead of human-readable text.
    #[arg(long, global = true)]
    pub json: bool,

    /// Print the resolved parameters/actions without doing anything.
    #[arg(long, global = true)]
    pub dry_run: bool,

    #[command(subcommand)]
    pub command: Command,
}

#[derive(Subcommand, Debug)]
pub enum Command {
    /// Print information about an audio file or a `.pva` analysis file.
    Info {
        /// Path to an audio file (wav/aiff/flac/...) or a `.pva` file.
        path: PathBuf,
    },

    /// Run a legacy C tool directly (transition path).
    ///
    /// Executes `$PVC_LEGACY_BIN/<tool>` if that variable is set, else
    /// `/opt/pvc-legacy/bin/<tool>` (where the Docker image installs the
    /// legacy build) - for any tool that hasn't been ported to a native
    /// `pvc` subcommand yet. Put `--` before the legacy tool's own flags
    /// (`pvc legacy plainpv -- -N 2048`) to guarantee they reach the
    /// legacy tool rather than being read as one of pvc's own global
    /// flags - none of the legacy tools' single-dash flags collide with
    /// `--quiet`/`--json`/`--dry-run` today, but `--` makes that certain
    /// regardless.
    #[command(disable_help_flag = true)]
    Legacy {
        /// Legacy tool name, e.g. `plainpv`, `pvanalysis`.
        tool: String,

        /// Arguments passed through unchanged to the legacy tool.
        #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
        args: Vec<String>,
    },

    /// Manage presets (TOML files consumed by `pvc run`).
    Preset {
        #[command(subcommand)]
        action: PresetAction,
    },

    /// Run a preset, optionally overriding fields with `--set key=value`.
    Run {
        /// Path to a preset TOML file.
        preset: PathBuf,

        /// Override a field before running, e.g. `--set stretch=1.5`.
        /// Unqualified keys apply under `[params]`; `section.key` targets
        /// another table; `tool`/`input`/`output` are top-level fields.
        #[arg(long = "set", value_name = "KEY=VALUE")]
        set: Vec<String>,
    },

    /// Control-function generators (the CARL/cmusic "GEN" family).
    Fn {
        #[command(subcommand)]
        generator: FnCommand,
    },
}

/// A breakpoint's raw `time,value` argument, e.g. `50,1.0`.
fn parse_point2(s: &str) -> Result<(f32, f32), String> {
    let (t, v) = s
        .split_once(',')
        .ok_or_else(|| format!("expected \"time,value\", got {s:?}"))?;
    Ok((
        t.parse().map_err(|_| format!("bad time in {s:?}"))?,
        v.parse().map_err(|_| format!("bad value in {s:?}"))?,
    ))
}

/// A `gen4` breakpoint's raw `time,value,alpha` argument.
fn parse_point3(s: &str) -> Result<(f32, f32, f32), String> {
    let mut parts = s.split(',');
    let (Some(t), Some(v), Some(a), None) =
        (parts.next(), parts.next(), parts.next(), parts.next())
    else {
        return Err(format!("expected \"time,value,alpha\", got {s:?}"));
    };
    Ok((
        t.parse().map_err(|_| format!("bad time in {s:?}"))?,
        v.parse().map_err(|_| format!("bad value in {s:?}"))?,
        a.parse().map_err(|_| format!("bad alpha in {s:?}"))?,
    ))
}

/// A `gen5` partial's raw `harmonic,amplitude,phase` argument.
fn parse_partial(s: &str) -> Result<(f32, f32, f32), String> {
    let mut parts = s.split(',');
    let (Some(h), Some(a), Some(p), None) =
        (parts.next(), parts.next(), parts.next(), parts.next())
    else {
        return Err(format!("expected \"harmonic,amplitude,phase\", got {s:?}"));
    };
    Ok((
        h.parse().map_err(|_| format!("bad harmonic in {s:?}"))?,
        a.parse().map_err(|_| format!("bad amplitude in {s:?}"))?,
        p.parse().map_err(|_| format!("bad phase in {s:?}"))?,
    ))
}

#[derive(Subcommand, Debug)]
pub enum FnCommand {
    /// gen1: piecewise-linear envelope from explicit `(time, value)`
    /// breakpoints (`legacy/cmusic_gen/gen/gen1.c`).
    Gen1 {
        /// Output table length in samples.
        #[arg(short = 'L', long)]
        length: usize,

        /// Open curve (the default closes it - see `gen1.c`'s `-o` flag).
        #[arg(long)]
        open: bool,

        /// A breakpoint as `time,value`; repeat for each point (need at
        /// least two), e.g. `--point 0,0 --point 50,1 --point 100,0`.
        #[arg(long = "point", value_parser = parse_point2, required = true, allow_hyphen_values = true)]
        points: Vec<(f32, f32)>,

        /// Where to write the resulting table - `.txt` for ASCII (one
        /// value per line), anything else for little-endian f32 binary.
        output: PathBuf,
    },

    /// gen2: sum of sine and cosine harmonics (`legacy/cmusic_gen/gen/
    /// gen2.c`). Named `--sine`/`--cosine` lists rather than the C's
    /// positional-count argument, which has a real footgun (see
    /// `docs/dev/parameter-inventory.md` §15): passing a count larger
    /// than the actual number of coefficients given reads uninitialized
    /// memory in the C. That's not representable here.
    Gen2 {
        #[arg(short = 'L', long)]
        length: usize,

        /// Closed curve (gen2's default is open, unlike gen1/gen3/gen4/gen5).
        #[arg(long)]
        closed: bool,

        /// Sine harmonic amplitudes, lowest harmonic first (harmonic 1, 2, ...).
        #[arg(long = "sine", allow_hyphen_values = true)]
        sine: Vec<f32>,

        /// Cosine harmonic amplitudes, `--cosine` for harmonic 0 (DC) first.
        #[arg(long = "cosine", allow_hyphen_values = true)]
        cosine: Vec<f32>,

        output: PathBuf,
    },

    /// gen3: piecewise-linear envelope at evenly spaced breakpoints, given
    /// only their values (`legacy/cmusic_gen/gen/gen3.c`).
    Gen3 {
        #[arg(short = 'L', long)]
        length: usize,

        #[arg(long)]
        open: bool,

        /// A breakpoint value; repeat for each point (need at least two).
        #[arg(long = "value", required = true, allow_hyphen_values = true)]
        values: Vec<f32>,

        output: PathBuf,
    },

    /// gen4: like gen1, but each breakpoint carries its own transition
    /// shape (`legacy/cmusic_gen/gen/gen4.c`): `alpha = 0` linear,
    /// negative exponential, positive logarithmic.
    Gen4 {
        #[arg(short = 'L', long)]
        length: usize,

        #[arg(long)]
        open: bool,

        /// A breakpoint as `time,value,alpha`; repeat for each point
        /// (need at least two - the last point's alpha is unused, since
        /// there's no segment after it).
        #[arg(long = "point", value_parser = parse_point3, required = true, allow_hyphen_values = true)]
        points: Vec<(f32, f32, f32)>,

        output: PathBuf,
    },

    /// gen5: sum of arbitrary `(harmonic, amplitude, phase)` partials
    /// (`legacy/cmusic_gen/gen/gen5.c`).
    Gen5 {
        #[arg(short = 'L', long)]
        length: usize,

        #[arg(long)]
        closed: bool,

        /// A partial as `harmonic,amplitude,phase`; repeat for each one.
        #[arg(long = "partial", value_parser = parse_partial, required = true, allow_hyphen_values = true)]
        partials: Vec<(f32, f32, f32)>,

        output: PathBuf,
    },

    /// gen6: uniform noise in `[-1.0, 1.0)` (`legacy/cmusic_gen/gen/
    /// gen6.c`), using the exact `rand()` sequence the C gets by never
    /// seeding one (glibc's default state, as if `srandom(1)` had been
    /// called) - deterministic, not a fresh random table on every run.
    Gen6 {
        #[arg(short = 'L', long)]
        length: usize,

        output: PathBuf,
    },
}

#[derive(Subcommand, Debug)]
pub enum PresetAction {
    /// Emit an annotated default preset for a tool.
    Init {
        /// Tool name, e.g. `pv`.
        tool: String,
    },
    /// List example presets bundled with `pvc`.
    List,
}
