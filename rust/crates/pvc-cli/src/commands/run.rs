//! `pvc run <preset.toml> [--set key=value ...]`.
//!
//! No tool is wired up to actually execute yet - that lands per-tool in
//! Phase 3, alongside each tool's port. Until then, `run` still does the
//! genuinely useful part: load the preset, apply overrides, and print the
//! fully resolved parameter set (this *is* `--dry-run`'s whole job, so
//! `run` without `--dry-run` does the same resolution and then reports
//! that execution isn't available yet, pointing at `pvc legacy` as the
//! transition path).

use std::path::Path;

use anyhow::Result;

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

    anyhow::bail!(
        "running tool {:?} isn't wired up yet - `pvc run` gains real execution in Phase 3 as each tool is ported; use `pvc legacy {}` in the meantime",
        preset.tool,
        preset.tool
    );
}
