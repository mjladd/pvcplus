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

        Command::Fn { generator } => commands::gen::run(generator),

        Command::Pv(args) => commands::pv::run(&args),

        Command::Stretch {
            factor,
            input,
            output,
        } => commands::pv::run_stretch(factor, &input, &output),

        Command::Pitch {
            semitones,
            input,
            output,
        } => commands::pv::run_pitch(semitones, &input, &output),

        Command::Analyze(args) => commands::analyze::run(&args),

        Command::Tvfilter(args) => commands::tvfilter::run(&args),

        Command::Twarp(args) => commands::twarp::run(&args),

        Command::Freqresponse(args) => commands::freqresponse::run(&args),

        Command::Filter(args) => commands::filter::run(&args),

        Command::Denoise(args) => commands::denoise::run(&args),

        Command::Compand(args) => commands::compand::run(&args),

        Command::Spectwarp(args) => commands::spectwarp::run(&args),

        Command::Harmonize(args) => commands::harmonize::run(&args),

        Command::Envelope(args) => commands::envelope::run(&args),

        Command::Centroid(args) => commands::centroid::run(&args),

        Command::Flux(args) => commands::flux::run(&args),

        Command::Pitchtrack(args) => commands::pitchtrack::run(&args),

        Command::ConvertUnits {
            from,
            to,
            norm,
            values,
        } => commands::convert_units::run(from, to, norm, &values, cli.json),

        Command::Impulseresponse(args) => commands::impulseresponse::run(&args),

        Command::Irconvolver(args) => commands::irconvolver::run(&args),

        Command::Ring(args) => commands::ring::run(&args),

        Command::Ringfilter(args) => commands::ringfilter::run(&args),

        Command::Ringtvfilter(args) => commands::ringtvfilter::run(&args),

        Command::Irconvolvesequencer(args) => commands::irconvolvesequencer::run(&args),
    }
}
