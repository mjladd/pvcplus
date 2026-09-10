//! `pvc migrate-script <path>`.

use std::path::Path;

use anyhow::{Context, Result};

pub fn run(script: &Path) -> Result<()> {
    let text =
        std::fs::read_to_string(script).with_context(|| format!("reading {}", script.display()))?;
    let preset = crate::migrate::migrate_script(&text, &script.display().to_string())
        .map_err(|e| anyhow::anyhow!(e))?;
    print!("{preset}");
    Ok(())
}
