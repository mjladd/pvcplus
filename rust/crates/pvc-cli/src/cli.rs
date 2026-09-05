//! Top-level argument definitions (Task 2.7: the `pvc` skeleton). Only
//! `info`, `legacy`, `preset init/list`, and `run` exist here - the real
//! DSP subcommands (`pv`, `stretch`, `analyze`, ...) arrive one at a time
//! in Phase 3 as their tools are ported, per the plan's §2.1 CLI sketch.

use std::path::PathBuf;

use clap::{Parser, Subcommand};
use pvc_core::{ControlFn, Window};

#[derive(Parser, Debug)]
#[command(
    name = "pvc",
    version,
    about = "A modern CLI for the PVCplus phase-vocoder toolkit."
)]
pub struct Cli {
    /// Suppress non-essential output.
    #[arg(long, global = true)]
    pub quiet: bool,

    /// Emit machine-readable JSON instead of human-readable text.
    #[arg(long, global = true)]
    pub json: bool,

    /// Print the resolved parameters/actions without doing anything.
    #[arg(long, global = true)]
    pub dry_run: bool,

    #[command(subcommand)]
    pub command: Command,
}

#[derive(Subcommand, Debug)]
pub enum Command {
    /// Print information about an audio file or a `.pva` analysis file.
    Info {
        /// Path to an audio file (wav/aiff/flac/...) or a `.pva` file.
        path: PathBuf,
    },

    /// Run a legacy C tool directly (transition path).
    ///
    /// Executes `$PVC_LEGACY_BIN/<tool>` if that variable is set, else
    /// `/opt/pvc-legacy/bin/<tool>` (where the Docker image installs the
    /// legacy build) - for any tool that hasn't been ported to a native
    /// `pvc` subcommand yet. Put `--` before the legacy tool's own flags
    /// (`pvc legacy plainpv -- -N 2048`) to guarantee they reach the
    /// legacy tool rather than being read as one of pvc's own global
    /// flags - none of the legacy tools' single-dash flags collide with
    /// `--quiet`/`--json`/`--dry-run` today, but `--` makes that certain
    /// regardless.
    #[command(disable_help_flag = true)]
    Legacy {
        /// Legacy tool name, e.g. `plainpv`, `pvanalysis`.
        tool: String,

        /// Arguments passed through unchanged to the legacy tool.
        #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
        args: Vec<String>,
    },

    /// Manage presets (TOML files consumed by `pvc run`).
    Preset {
        #[command(subcommand)]
        action: PresetAction,
    },

    /// Run a preset, optionally overriding fields with `--set key=value`.
    Run {
        /// Path to a preset TOML file.
        preset: PathBuf,

        /// Override a field before running, e.g. `--set stretch=1.5`.
        /// Unqualified keys apply under `[params]`; `section.key` targets
        /// another table; `tool`/`input`/`output` are top-level fields.
        #[arg(long = "set", value_name = "KEY=VALUE")]
        set: Vec<String>,
    },

    /// Control-function generators (the CARL/cmusic "GEN" family).
    Fn {
        #[command(subcommand)]
        generator: FnCommand,
    },

    /// Phase vocoder: analyze then resynthesize.
    ///
    /// Time-stretch, pitch transposition, frequency shift, gain,
    /// spectrum warp, shelf EQ, and a brickwall frequency-window filter.
    /// Ports `plainpv`'s audio-processing path (`legacy/pvc_src/
    /// plainpv.c`) - see `pvc-core::tools::pv`'s doc comment for exactly
    /// what's in and out of scope (its debug/display-only flags aren't
    /// ported).
    Pv(Box<PvArgs>),

    /// Time-stretch only - `pvc pv --stretch <factor>` with everything
    /// else at its default.
    Stretch {
        /// Time-stretch factor (`1.0` = unchanged, `2.0` = twice as long).
        #[arg(long)]
        factor: f32,
        input: PathBuf,
        output: PathBuf,
    },

    /// Pitch transposition only - `pvc pv --pitch <semitones>` with
    /// everything else at its default.
    Pitch {
        /// Pitch shift in semitones (positive = up, negative = down).
        #[arg(long)]
        semitones: f32,
        input: PathBuf,
        output: PathBuf,
    },
}

/// `pvc pv`'s full flag surface. Long names follow
/// `docs/dev/parameter-inventory.md` §12/§14's proposed mapping from
/// `plainpv`'s single-letter flags.
#[derive(clap::Args, Debug)]
pub struct PvArgs {
    /// FFT size (must be a power of two).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis window length. Defaults to `2048` - matching `plainpv`'s
    /// own hardcoded default, which is *not* `2 * fft` despite looking
    /// that way for the common `--fft 1024` case (confirmed against the
    /// real tool's own startup banner at `--fft 2048`, which still
    /// reports window size 2048, not 4096). Pass `0` explicitly for the
    /// C's actual auto-scaling rule (`2 * fft`, or larger still if
    /// needed to fit the resynthesis hop) - that only ever triggers in
    /// the C via an explicit `-M0` or negative override, never by
    /// default.
    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    /// Analysis/synthesis window shape.
    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    /// Analysis frames per second (sets the hop size).
    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time-stretch factor (`1.0` = unchanged).
    #[arg(long, default_value_t = 1.0)]
    pub stretch: f32,

    /// Pitch shift in semitones - a plain number, or `@path` to a
    /// control file for a time-varying shift.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub pitch: ControlFn,

    /// Frequency shift in Hz - a plain number, or `@path`.
    #[arg(long = "freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub freq_shift: ControlFn,

    /// Gain in dB - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub gain: ControlFn,

    /// Amplitude envelope attack time in seconds - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// Amplitude envelope release time in seconds - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    /// Spectrum magnitude warp index (`0` = no warp) - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp: ControlFn,

    /// Low shelf EQ gain in dB - a plain number, or `@path`.
    #[arg(long = "shelf-low-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub shelf_low_gain: ControlFn,

    /// High shelf EQ gain in dB - a plain number, or `@path`.
    #[arg(long = "shelf-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub shelf_high_gain: ControlFn,

    /// Low shelf EQ frequency in Hz - a plain number, or `@path`.
    #[arg(long = "shelf-low-freq", value_parser = parse_control_fn, default_value = "200")]
    pub shelf_low_freq: ControlFn,

    /// High shelf EQ frequency in Hz - a plain number, or `@path`.
    #[arg(long = "shelf-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub shelf_high_freq: ControlFn,

    /// Oscillator resynthesis threshold in dB (bins quieter than this,
    /// relative to the frame's own peak, are skipped).
    #[arg(long, default_value_t = -96.0, allow_hyphen_values = true)]
    pub threshold: f32,

    /// Brickwall frequency-window filter mode.
    #[arg(long = "filter-type", value_parser = parse_filter_type, default_value = "bandpass")]
    pub filter_type: pvc_core::tools::pv::FilterType,

    /// Brickwall filter low frequency bound in Hz.
    #[arg(long = "filter-low", default_value_t = 0.0)]
    pub filter_low: f32,

    /// Brickwall filter high frequency bound in Hz (`-1` = Nyquist).
    #[arg(long = "filter-high", default_value_t = -1.0, allow_hyphen_values = true)]
    pub filter_high: f32,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_window(s: &str) -> Result<Window, String> {
    match s {
        "hamming" => Ok(Window::Hamming),
        "rectangular" => Ok(Window::Rectangular),
        "blackman" => Ok(Window::Blackman),
        "bartlett" => Ok(Window::Bartlett),
        "blackman_harris" => Ok(Window::BlackmanHarris),
        "nuttall" => Ok(Window::Nuttall),
        "blackman_nuttall" => Ok(Window::BlackmanNuttall),
        "flat_top" => Ok(Window::FlatTop),
        _ => {
            if let Some(alpha) = s.strip_prefix("kaiser") {
                let alpha: f32 = alpha
                    .parse()
                    .map_err(|_| format!("bad kaiser alpha in {s:?}"))?;
                Ok(Window::Kaiser(alpha))
            } else {
                Err(format!(
                    "unknown window {s:?} (expected hamming, rectangular, blackman, bartlett, \
                     kaiser<4-12>, blackman_harris, nuttall, blackman_nuttall, or flat_top)"
                ))
            }
        }
    }
}

fn parse_filter_type(s: &str) -> Result<pvc_core::tools::pv::FilterType, String> {
    match s {
        "bandpass" => Ok(pvc_core::tools::pv::FilterType::Bandpass),
        "reject" => Ok(pvc_core::tools::pv::FilterType::BandReject),
        _ => Err(format!("expected \"bandpass\" or \"reject\", got {s:?}")),
    }
}

/// A `pv`-family control-function argument: a plain number, or `@path`
/// to a control file for a time-varying value (plan §2.1: "func-able
/// parameters accept `<number>` or `@path`, explicit, no sniffing").
fn parse_control_fn(s: &str) -> Result<ControlFn, String> {
    if let Some(path) = s.strip_prefix('@') {
        let data = pvc_io::read_control_file(std::path::Path::new(path))
            .map_err(|e| format!("reading control file {path:?}: {e}"))?;
        Ok(ControlFn::Table(data.values))
    } else {
        s.parse::<f32>()
            .map(ControlFn::Const)
            .map_err(|_| format!("expected a number or @path, got {s:?}"))
    }
}

/// A breakpoint's raw `time,value` argument, e.g. `50,1.0`.
fn parse_point2(s: &str) -> Result<(f32, f32), String> {
    let (t, v) = s
        .split_once(',')
        .ok_or_else(|| format!("expected \"time,value\", got {s:?}"))?;
    Ok((
        t.parse().map_err(|_| format!("bad time in {s:?}"))?,
        v.parse().map_err(|_| format!("bad value in {s:?}"))?,
    ))
}

/// A `gen4` breakpoint's raw `time,value,alpha` argument.
fn parse_point3(s: &str) -> Result<(f32, f32, f32), String> {
    let mut parts = s.split(',');
    let (Some(t), Some(v), Some(a), None) =
        (parts.next(), parts.next(), parts.next(), parts.next())
    else {
        return Err(format!("expected \"time,value,alpha\", got {s:?}"));
    };
    Ok((
        t.parse().map_err(|_| format!("bad time in {s:?}"))?,
        v.parse().map_err(|_| format!("bad value in {s:?}"))?,
        a.parse().map_err(|_| format!("bad alpha in {s:?}"))?,
    ))
}

/// A `gen5` partial's raw `harmonic,amplitude,phase` argument.
fn parse_partial(s: &str) -> Result<(f32, f32, f32), String> {
    let mut parts = s.split(',');
    let (Some(h), Some(a), Some(p), None) =
        (parts.next(), parts.next(), parts.next(), parts.next())
    else {
        return Err(format!("expected \"harmonic,amplitude,phase\", got {s:?}"));
    };
    Ok((
        h.parse().map_err(|_| format!("bad harmonic in {s:?}"))?,
        a.parse().map_err(|_| format!("bad amplitude in {s:?}"))?,
        p.parse().map_err(|_| format!("bad phase in {s:?}"))?,
    ))
}

#[derive(Subcommand, Debug)]
pub enum FnCommand {
    /// gen1: piecewise-linear envelope from explicit `(time, value)`
    /// breakpoints (`legacy/cmusic_gen/gen/gen1.c`).
    Gen1 {
        /// Output table length in samples.
        #[arg(short = 'L', long)]
        length: usize,

        /// Open curve (the default closes it - see `gen1.c`'s `-o` flag).
        #[arg(long)]
        open: bool,

        /// A breakpoint as `time,value`; repeat for each point (need at
        /// least two), e.g. `--point 0,0 --point 50,1 --point 100,0`.
        #[arg(long = "point", value_parser = parse_point2, required = true, allow_hyphen_values = true)]
        points: Vec<(f32, f32)>,

        /// Where to write the resulting table - `.txt` for ASCII (one
        /// value per line), anything else for little-endian f32 binary.
        output: PathBuf,
    },

    /// gen2: sum of sine and cosine harmonics (`legacy/cmusic_gen/gen/
    /// gen2.c`). Named `--sine`/`--cosine` lists rather than the C's
    /// positional-count argument, which has a real footgun (see
    /// `docs/dev/parameter-inventory.md` §15): passing a count larger
    /// than the actual number of coefficients given reads uninitialized
    /// memory in the C. That's not representable here.
    Gen2 {
        #[arg(short = 'L', long)]
        length: usize,

        /// Closed curve (gen2's default is open, unlike gen1/gen3/gen4/gen5).
        #[arg(long)]
        closed: bool,

        /// Sine harmonic amplitudes, lowest harmonic first (harmonic 1, 2, ...).
        #[arg(long = "sine", allow_hyphen_values = true)]
        sine: Vec<f32>,

        /// Cosine harmonic amplitudes, `--cosine` for harmonic 0 (DC) first.
        #[arg(long = "cosine", allow_hyphen_values = true)]
        cosine: Vec<f32>,

        output: PathBuf,
    },

    /// gen3: piecewise-linear envelope at evenly spaced breakpoints, given
    /// only their values (`legacy/cmusic_gen/gen/gen3.c`).
    Gen3 {
        #[arg(short = 'L', long)]
        length: usize,

        #[arg(long)]
        open: bool,

        /// A breakpoint value; repeat for each point (need at least two).
        #[arg(long = "value", required = true, allow_hyphen_values = true)]
        values: Vec<f32>,

        output: PathBuf,
    },

    /// gen4: like gen1, but each breakpoint carries its own transition
    /// shape (`legacy/cmusic_gen/gen/gen4.c`): `alpha = 0` linear,
    /// negative exponential, positive logarithmic.
    Gen4 {
        #[arg(short = 'L', long)]
        length: usize,

        #[arg(long)]
        open: bool,

        /// A breakpoint as `time,value,alpha`; repeat for each point
        /// (need at least two - the last point's alpha is unused, since
        /// there's no segment after it).
        #[arg(long = "point", value_parser = parse_point3, required = true, allow_hyphen_values = true)]
        points: Vec<(f32, f32, f32)>,

        output: PathBuf,
    },

    /// gen5: sum of arbitrary `(harmonic, amplitude, phase)` partials
    /// (`legacy/cmusic_gen/gen/gen5.c`).
    Gen5 {
        #[arg(short = 'L', long)]
        length: usize,

        #[arg(long)]
        closed: bool,

        /// A partial as `harmonic,amplitude,phase`; repeat for each one.
        #[arg(long = "partial", value_parser = parse_partial, required = true, allow_hyphen_values = true)]
        partials: Vec<(f32, f32, f32)>,

        output: PathBuf,
    },

    /// gen6: uniform noise in `[-1.0, 1.0)` (`legacy/cmusic_gen/gen/
    /// gen6.c`), using the exact `rand()` sequence the C gets by never
    /// seeding one (glibc's default state, as if `srandom(1)` had been
    /// called) - deterministic, not a fresh random table on every run.
    Gen6 {
        #[arg(short = 'L', long)]
        length: usize,

        output: PathBuf,
    },

    /// Plot a control/data file as a terminal sparkline - replaces the
    /// legacy `showme`/`showmeb`/`showmed`/... family, whose actual job
    /// (confirmed by reading them: `reshape -A1 $1 > /tmp/$USER`, then
    /// `gnuplot` on that file) is exactly "convert a binary or ASCII
    /// float file to something plottable, then plot it" - not any of
    /// `reshape.c`'s ~40 other transformation flags, none of which any
    /// script in this repo actually uses. No `gnuplot` dependency needed.
    Plot {
        /// An ASCII (.txt) or binary f32 control/data file.
        input: PathBuf,

        /// Number of columns to downsample to (default: 120, or the
        /// file's own length if shorter).
        #[arg(long)]
        width: Option<usize>,
    },
}

#[derive(Subcommand, Debug)]
pub enum PresetAction {
    /// Emit an annotated default preset for a tool.
    Init {
        /// Tool name, e.g. `pv`.
        tool: String,
    },
    /// List example presets bundled with `pvc`.
    List,
}
