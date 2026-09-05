use clap::Parser;

use pvc_cli::cli::{Cli, Command, PresetAction};
use pvc_cli::commands;

fn main() -> anyhow::Result<()> {
    let cli = Cli::parse();

    match cli.command {
        Command::Info { path } => commands::info::run(&path, cli.json),

        // `exec` replaces this process on success and only returns here
        // on failure.
        Command::Legacy { tool, args } => Err(commands::legacy::run(&tool, &args)),

        Command::Preset { action } => match action {
            PresetAction::Init { tool } => commands::preset::init(&tool),
            PresetAction::List => commands::preset::list(cli.json),
        },

        Command::Run { preset, set } => {
            commands::run::run(&preset, &set, cli.json, cli.dry_run, cli.quiet)
        }
    }
}
