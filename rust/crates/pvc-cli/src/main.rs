use clap::Parser;

use pvc_cli::cli::Cli;
use pvc_cli::dispatch;

fn main() -> anyhow::Result<()> {
    let cli = Cli::parse();
    dispatch::execute(cli.command, cli.json, cli.dry_run, cli.quiet)
}
