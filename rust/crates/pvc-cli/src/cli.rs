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
