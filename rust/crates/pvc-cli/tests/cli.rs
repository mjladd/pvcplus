//! `pvc --help` snapshot test (Task 2.7's explicit acceptance check).
//! Regenerate with `INSTA_UPDATE=always cargo test -p pvc-cli` after a
//! deliberate CLI change, then review the diff in the `.snap` file before
//! committing it.

use clap::CommandFactory;
use pvc_cli::cli::Cli;

#[test]
fn help_output_snapshot() {
    let help = Cli::command().render_long_help().to_string();
    insta::assert_snapshot!(help);
}
