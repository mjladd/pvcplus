//! Runs one already-parsed [`Command`], regardless of whether it came
//! from `std::env::args()` (`main`) or from a preset resolved by
//! `pvc run` (`commands::run`). Extracted out of `main` so both paths
//! share the exact same tool dispatch instead of `pvc run` needing its
//! own, separately-maintained copy.

use crate::cli::{Command, PresetAction};
use crate::commands;

pub fn execute(command: Command, json: bool, dry_run: bool, quiet: bool) -> anyhow::Result<()> {
    match command {
        Command::Info { path } => commands::info::run(&path, json),

        // `exec` replaces this process on success and only returns here
        // on failure.
        Command::Legacy { tool, args } => Err(commands::legacy::run(&tool, &args)),

        Command::Preset { action } => match action {
            PresetAction::Init { tool } => commands::preset::init(&tool),
            PresetAction::List => commands::preset::list(json),
        },

        Command::Run { preset, set } => commands::run::run(&preset, &set, json, dry_run, quiet),

        Command::Fn { generator } => commands::gen::run(generator),

        Command::Completions { shell } => {
            commands::completions::run(shell);
            Ok(())
        }

        Command::Pv(args) => commands::pv::run(&args, json, quiet),

        Command::Stretch {
            factor,
            input,
            output,
        } => commands::pv::run_stretch(factor, &input, &output, json, quiet),

        Command::Pitch {
            semitones,
            input,
            output,
        } => commands::pv::run_pitch(semitones, &input, &output, json, quiet),

        Command::Analyze(args) => commands::analyze::run(&args),

        Command::Tvfilter(args) => commands::tvfilter::run(&args, json, quiet),

        Command::Twarp(args) => commands::twarp::run(&args, json, quiet),

        Command::Freqresponse(args) => commands::freqresponse::run(&args),

        Command::Filter(args) => commands::filter::run(&args, json, quiet),

        Command::Denoise(args) => commands::denoise::run(&args, json, quiet),

        Command::Compand(args) => commands::compand::run(&args, json, quiet),

        Command::Spectwarp(args) => commands::spectwarp::run(&args, json, quiet),

        Command::Harmonize(args) => commands::harmonize::run(&args, json, quiet),

        Command::Envelope(args) => commands::envelope::run(&args),

        Command::Centroid(args) => commands::centroid::run(&args),

        Command::Flux(args) => commands::flux::run(&args),

        Command::Pitchtrack(args) => commands::pitchtrack::run(&args),

        Command::ConvertUnits {
            from,
            to,
            norm,
            values,
        } => commands::convert_units::run(from, to, norm, &values, json),

        Command::Impulseresponse(args) => commands::impulseresponse::run(&args),

        Command::Irconvolver(args) => commands::irconvolver::run(&args, json, quiet),

        Command::Ring(args) => commands::ring::run(&args, json, quiet),

        Command::Ringfilter(args) => commands::ringfilter::run(&args, json, quiet),

        Command::Ringtvfilter(args) => commands::ringtvfilter::run(&args, json, quiet),

        Command::Irconvolvesequencer(args) => {
            commands::irconvolvesequencer::run(&args, json, quiet)
        }

        Command::Spectralextractor(args) => commands::spectralextractor::run(&args, json, quiet),

        Command::Peakformant(args) => commands::peakformant::run(&args),

        Command::Specflattracker(args) => commands::specflattracker::run(&args),

        Command::Convolver(args) => commands::convolver::run(&args, json, quiet),

        Command::Delayfilter(args) => commands::delayfilter::run(&args, json, quiet),

        Command::Filtdeviator(args) => commands::filtdeviator::run(&args, json, quiet),

        Command::Tvfiltdeviator(args) => commands::tvfiltdeviator::run(&args, json, quiet),

        Command::Ratechanger(args) => commands::ratechanger::run(&args, json, quiet),

        Command::Inharmonator(args) => commands::inharmonator::run(&args, json, quiet),

        Command::Formantsmapper(args) => commands::formantsmapper::run(&args, json, quiet),

        Command::Spectrummapper(args) => commands::spectrummapper::run(&args),
        Command::Chordmapperplus(args) => commands::chordmapperplus::run(&args, json, quiet),
    }
}
