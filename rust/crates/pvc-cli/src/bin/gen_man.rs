//! Generates man pages for `pvc` and every one of its subcommands into a
//! directory (default `man/`, or the first CLI argument). Run by hand
//! (`cargo run -p pvc-cli --bin gen-man`, or `make man`) - not part of
//! the normal `cargo build`, since there is no packaging step yet
//! (plan §8.4 defers `cargo-dist`/native binaries) that would consume
//! these pages automatically. Checked-in output, regenerated whenever
//! `cli.rs`'s own flag surface changes.

use std::fs;
use std::path::{Path, PathBuf};

use clap::Command;
use pvc_cli::cli;

fn main() -> std::io::Result<()> {
    let out_dir: PathBuf = std::env::args().nth(1).map_or_else(
        || PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../../../man"),
        PathBuf::from,
    );
    fs::create_dir_all(&out_dir)?;

    let root = cli::command();
    let mut count = 0usize;
    write_man_page(&root, &[], &out_dir, &mut count)?;

    println!("Wrote {count} man page(s) to {}", out_dir.display());
    Ok(())
}

/// Writes `cmd`'s own man page (named `pvc[-<name>]*.1` from `path`),
/// then recurses into every real subcommand - skipping clap's own
/// auto-generated `help` subcommand, which has no useful man page of
/// its own.
fn write_man_page(
    cmd: &Command,
    path: &[&str],
    out_dir: &Path,
    count: &mut usize,
) -> std::io::Result<()> {
    let page_name = if path.is_empty() {
        "pvc".to_string()
    } else {
        format!("pvc-{}", path.join("-"))
    };

    // clap_mangen renders `cmd`'s own name as the man page title, so a
    // subcommand needs a clone named after its full dotted path (e.g.
    // "pv" alone would render every subcommand's page with the title
    // "pv") - `Command::name` only affects the rendered title, not this
    // function's own file-naming, which already uses the full path.
    // `Command::name` wants an owned/`'static` name; clap's `String ->
    // Str` conversion is gated behind a feature this workspace doesn't
    // enable elsewhere, so leak instead - a one-shot dev tool run by
    // hand, not something that stays resident.
    let leaked_name: &'static str = Box::leak(page_name.clone().into_boxed_str());
    let mut titled = cmd.clone();
    titled = titled.name(leaked_name);
    let man = clap_mangen::Man::new(titled);
    let mut buffer: Vec<u8> = Vec::new();
    man.render(&mut buffer)?;
    fs::write(out_dir.join(format!("{page_name}.1")), buffer)?;
    *count += 1;

    for sub in cmd.get_subcommands() {
        if sub.get_name() == "help" {
            continue;
        }
        let mut sub_path = path.to_vec();
        sub_path.push(sub.get_name());
        write_man_page(sub, &sub_path, out_dir, count)?;
    }
    Ok(())
}
