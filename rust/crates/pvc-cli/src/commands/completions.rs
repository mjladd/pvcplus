//! `pvc completions <shell>`: prints a shell completion script to stdout.

use clap_complete::{generate, Shell};

use crate::cli;

pub fn run(shell: Shell) {
    let mut cmd = cli::command();
    let name = cmd.get_name().to_string();
    generate(shell, &mut cmd, name, &mut std::io::stdout());
}
