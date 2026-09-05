//! `pvc legacy <tool> ...`: the transition path for tools not yet ported
//! to a native subcommand. Replaces the current process with the legacy
//! binary (`exec`, not spawn-and-wait) so exit codes, signals, and
//! stdin/stdout behavior (including TTY detection for the legacy tools'
//! own progress banners) pass through exactly as if the legacy binary had
//! been invoked directly.

use std::env;
use std::os::unix::process::CommandExt;
use std::process::Command;

const DEFAULT_LEGACY_BIN_DIR: &str = "/opt/pvc-legacy/bin";

pub fn run(tool: &str, args: &[String]) -> anyhow::Error {
    let bin_dir = env::var("PVC_LEGACY_BIN").unwrap_or_else(|_| DEFAULT_LEGACY_BIN_DIR.to_string());
    let bin_path = format!("{bin_dir}/{tool}");

    // `exec` only returns on failure - a successful exec never comes back
    // here at all.
    let err = Command::new(&bin_path).args(args).exec();
    anyhow::anyhow!("failed to exec legacy tool at {bin_path}: {err}")
}
