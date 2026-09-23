use clap::FromArgMatches;

use pvc_cli::cli::{self, Cli};
use pvc_cli::dispatch;

fn main() -> anyhow::Result<()> {
    // `cli::command()` is `Cli::parse()`'s own command plus the grouped
    // help listing, so `pvc --help` and `pvc help` show the groups.
    let matches = cli::command().get_matches();
    let cli = Cli::from_arg_matches(&matches).unwrap_or_else(|err| err.exit());
    dispatch::execute(cli.command, cli.json, cli.dry_run, cli.quiet)
}
