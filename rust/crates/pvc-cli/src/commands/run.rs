//! `pvc run <preset.toml> [--set key=value ...]`.
//!
//! Loads the preset, applies any `--set` overrides, prints the fully
//! resolved parameter set (unless `--quiet`), and then - unless
//! `--dry-run` - actually runs the resolved tool.
//!
//! Running reuses the exact same argument parser and dispatch table as
//! `pvc <tool> ...` typed directly: [`Preset::to_cli_args`] turns the
//! resolved preset into the argument list `Cli::try_parse_from` would see
//! from a real command line (`pvc <tool> --flag value ... <input>
//! <output>`), and [`crate::dispatch::execute`] then runs it exactly the
//! way `main` does. This means a preset can only ever drive a tool
//! through the flags that tool's own `clap` struct already accepts - no
//! separate, easy-to-drift mapping table between preset field names and
//! real flags to maintain.

use std::path::Path;

use anyhow::{Context, Result};
use clap::Parser;

use crate::cli::Cli;
use crate::dispatch;
use crate::preset::Preset;

pub fn run(
    preset_path: &Path,
    sets: &[String],
    json: bool,
    dry_run: bool,
    quiet: bool,
) -> Result<()> {
    let mut preset = Preset::load(preset_path)?;
    for set in sets {
        preset.apply_set(set)?;
    }

    if !quiet {
        if json {
            println!("{}", serde_json::to_string_pretty(&preset)?);
        } else {
            println!("{}", toml::to_string_pretty(&preset)?);
        }
    }

    if dry_run {
        return Ok(());
    }

    let args = preset.to_cli_args("pvc");
    let cli = Cli::try_parse_from(&args).with_context(|| {
        format!(
            "preset {} resolved to invalid arguments for tool {:?}: {}",
            preset_path.display(),
            preset.tool,
            args.join(" ")
        )
    })?;
    dispatch::execute(cli.command, json, dry_run, quiet)
}
