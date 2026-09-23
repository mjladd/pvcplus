//! `pvc --help` snapshot test (Task 2.7's explicit acceptance check),
//! plus the check that every subcommand is listed under one of
//! `cli::COMMAND_GROUPS`'s own headings. Regenerate the snapshot with
//! `INSTA_UPDATE=always cargo test -p pvc-cli` after a deliberate CLI
//! change, then review the diff in the `.snap` file before committing
//! it.

use std::fs;
use std::path::Path;

use clap::CommandFactory;
use pvc_cli::cli::{self, Cli, COMMAND_GROUPS};

#[test]
fn help_output_snapshot() {
    let help = cli::command().render_long_help().to_string();
    insta::assert_snapshot!(help);
}

/// A command that no group names still appears in `pvc --help`, under
/// "Other", so this test is the reminder to file it rather than a
/// guard against losing it.
#[test]
fn command_groups_cover_every_subcommand() {
    let mut built = Cli::command();
    built.build();

    let grouped: Vec<&str> = COMMAND_GROUPS
        .iter()
        .flat_map(|(_, names)| names.iter().copied())
        .collect();

    for sub in built.get_subcommands() {
        assert!(
            grouped.contains(&sub.get_name()),
            "`{}` is in no COMMAND_GROUPS heading in cli.rs",
            sub.get_name()
        );
    }

    for name in &grouped {
        assert!(
            built.find_subcommand(name).is_some(),
            "COMMAND_GROUPS names `{name}`, which is not a pvc subcommand"
        );
        assert_eq!(
            grouped.iter().filter(|other| *other == name).count(),
            1,
            "COMMAND_GROUPS names `{name}` under more than one heading"
        );
    }
}

/// Every command's short description is one line, and short enough to
/// sit in the grouped list's own second column without wrapping an
/// 80-column terminal.
#[test]
fn command_about_lines_are_short_and_single_line() {
    let mut built = Cli::command();
    built.build();

    let width = built
        .get_subcommands()
        .map(|sub| sub.get_name().len())
        .max()
        .unwrap_or(0);

    for sub in built.get_subcommands() {
        let about = sub
            .get_about()
            .unwrap_or_else(|| panic!("`{}` has no about text", sub.get_name()))
            .to_string();
        assert!(
            !about.contains('\n'),
            "`{}`'s about text is more than one line",
            sub.get_name()
        );
        assert!(
            about.len() + width + 4 <= 80,
            "`{}`'s about text is too long for the grouped list: {} chars",
            sub.get_name(),
            about.len()
        );
    }
}

/// The tool-page index (`docs/tools/README.md`) lists every command
/// that has a page of its own, with the same one-line description the
/// CLI prints. This test fails when a command's description changes
/// and the index keeps the old wording.
#[test]
fn tool_page_index_matches_the_command_list() {
    let docs = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../../docs/tools");
    let index = fs::read_to_string(docs.join("README.md")).expect("docs/tools/README.md");

    let mut built = Cli::command();
    built.build();

    for sub in built.get_subcommands() {
        let name = sub.get_name();
        if !docs.join(format!("{name}.md")).exists() {
            continue;
        }
        let about = sub.get_about().expect("about text").to_string();
        let row = format!("| [`pvc {name}`]({name}.md) | {about} |");
        assert!(
            index.contains(&row),
            "docs/tools/README.md has no row for `{name}`:\n{row}"
        );
    }
}
