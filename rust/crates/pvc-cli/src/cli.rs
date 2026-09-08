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

    /// Phase vocoder analysis only: writes a `.pva` frame file, no
    /// resynthesis.
    ///
    /// Ports `pvanalysis`'s audio-processing path (`legacy/pvc_src/
    /// pvanalysis.c`) - see `pvc-core::tools::analyze`'s doc comment for
    /// what's in and out of scope. Always writes the new `PVA1` format
    /// (see `pvc_io::pva`'s module doc comment) - never the legacy
    /// layout, which is a read-only oracle-comparison format here.
    Analyze(Box<AnalyzeArgs>),

    /// Time-varying resynthesis: navigates a virtual time position
    /// through a `.pva` analysis file (rate/origin/window-driven) and
    /// resynthesizes from whatever frame it lands on.
    ///
    /// Ports `twarp`'s audio-processing path (`legacy/pvc_src/twarp.c`);
    /// see `pvc-core::tools::twarp`'s doc comment for what's in and out
    /// of scope (time-point dither, loop-boundary amplitude
    /// normalization, and random amplitude/frequency "shimmer" aren't
    /// ported yet).
    Twarp(Box<TwarpArgs>),

    /// Analysis-driven `.fr` frequency response: accumulates a sound
    /// file's spectrum (by average or peak amplitude, across all
    /// channels combined) into a response file, with formant detection
    /// and optional formant-band normalization/companding.
    ///
    /// Ports `freqresponse`'s audio-processing path (`legacy/pvc_src/
    /// freqresponse.c`); see `pvc-core::tools::freqresponse`'s doc
    /// comment for what's in and out of scope (plot/ASCII/binary formant
    /// report files aren't ported - pure reporting).
    Freqresponse(Box<FreqresponseArgs>),

    /// Fixed-spectrum phase-vocoder filter: multiplies each frame's
    /// amplitude by a shaped copy of a `.fr` response, then mixes the
    /// filtered signal with (unless disabled) the original source.
    ///
    /// Ports `filter`'s audio-processing path (`legacy/pvc_src/
    /// filter.c`); see `pvc-core::tools::filter`'s doc comment for what's
    /// in and out of scope (oscillator-bank resynthesis - needed only
    /// for pitch/frequency-shifted output - isn't ported yet).
    Filter(Box<FilterArgs>),

    /// Like `filter`, but the response is a time-varying sequence of
    /// frames (a legacy `.pva` file) navigated over time the same way
    /// `twarp` navigates its own resynthesis source, rather than one
    /// static `.fr` file.
    ///
    /// Ports `tvfilter`'s audio-processing path (`legacy/pvc_src/
    /// tvfilter.c`); see `pvc-core::tools::tvfilter`'s doc comment for
    /// what's in and out of scope (oscillator-bank resynthesis, and a
    /// dead `-u` flag).
    Tvfilter(Box<TvfilterArgs>),

    /// Spectral noise gate: builds a noise-response profile by analyzing
    /// a `[--noise-begin, --noise-end)` window of the input itself
    /// (default: the whole file - point these at an actual noise-only
    /// stretch, e.g. leading silence), then expands any bin quieter than
    /// its noise-response threshold toward silence.
    ///
    /// Ports `noisefilter`'s audio-processing path (`legacy/pvc_src/
    /// noisefilter.c`); see `pvc-core::tools::noisefilter`'s doc comment
    /// for what's in and out of scope.
    Denoise(Box<DenoiseArgs>),

    /// Per-bin dynamics processor: compresses or expands each bin's
    /// amplitude relative to a static peaks/reference file (e.g. from
    /// `pvc freqresponse`), within an adjustable frequency band.
    ///
    /// Ports `compander`'s audio-processing path (`legacy/pvc_src/
    /// compander.c`); see `pvc-core::tools::compander`'s doc comment for
    /// what's in and out of scope (`-L`/release is a real no-op in the
    /// original tool, not exposed here).
    Compand(Box<CompanderArgs>),

    /// Per-bin dynamics processor: compresses or expands each bin's
    /// amplitude relative to its *own frame's* live spectral peak (a
    /// self-referential envelope-follower), within an adjustable
    /// frequency band.
    ///
    /// Ports `spectwarper`'s audio-processing path (`legacy/pvc_src/
    /// spectwarper.c`); see `pvc-core::tools::spectwarper`'s doc comment
    /// for what's in and out of scope, including a real bound bug in the
    /// original tool's sliding-window mode, reproduced faithfully.
    Spectwarp(Box<SpectwarpArgs>),

    /// Builds one or more "voices" from a data table of frequency bands,
    /// each a pitch/frequency-shifted copy of a triangular-windowed
    /// slice of the input spectrum, optionally summed with a delayed/
    /// shifted copy of the source signal.
    ///
    /// Ports `harmonizer`'s audio-processing path (`legacy/pvc_src/
    /// harmonizer.c`); see `pvc-core::tools::harmonizer`'s doc comment
    /// for the data-table format and what's in and out of scope
    /// (including a real crash bug it validates against instead of
    /// reproducing, and a real cross-band bug it reproduces faithfully).
    Harmonize(Box<HarmonizeArgs>),

    /// Amplitude envelope over a frequency band: a time-series of
    /// scalar values (ASCII or raw float), never audio.
    ///
    /// Ports `envelope`'s audio-processing path (`legacy/pvc_src/
    /// envelope.c`); see `pvc-core::tools::envelope`'s doc comment for
    /// the two-pass design and a real pass-1/pass-2 state-carryover
    /// quirk reproduced faithfully.
    Envelope(Box<EnvelopeArgs>),

    /// Spectral centroid (amplitude²-weighted mean frequency) over a
    /// frequency band: a time-series of scalar values, never audio.
    ///
    /// Ports `centroid`'s audio-processing path (`legacy/pvc_src/
    /// centroid.c`); see `pvc-core::tools::centroid`'s doc comment for
    /// two provably-dead flags this port doesn't expose.
    Centroid(Box<CentroidArgs>),

    /// Spectral flux (frame-to-frame frequency change, optionally
    /// amplitude-weighted) over a frequency band: a time-series of
    /// scalar values, never audio.
    ///
    /// Ports `fluxoid`'s audio-processing path (`legacy/pvc_src/
    /// fluxoid.c`); see `pvc-core::tools::fluxoid`'s doc comment for the
    /// shared two-pass shape.
    Flux(Box<FluxArgs>),

    /// Fundamental-frequency tracker: a time-series of scalar pitch
    /// values, never audio.
    ///
    /// Ports `pitchtracker`'s audio-processing path (`legacy/pvc_src/
    /// pitchtracker.c`); see `pvc-core::tools::pitchtracker`'s doc
    /// comment for two doc-corrected defaults and several real bugs
    /// reproduced faithfully.
    Pitchtrack(Box<PitchtrackArgs>),

    /// Convert between amplitude/decibel and Hz/octave.pitchclass units.
    ///
    /// Replaces the four standalone unit-conversion utilities `amptodB`,
    /// `dBtoamp`, `Hztopitch`, `pitchtoHz` (`legacy/pvc_src/*.c`) with one
    /// command taking a `--from`/`--to` unit pair (`amp`, `db`, `hz`,
    /// `oppc`). `--norm` (`amptodB`'s `-n`) only applies to `amp -> db`.
    ConvertUnits {
        /// Unit to convert from: `amp`, `db`, `hz`, or `oppc`.
        #[arg(long, value_parser = parse_convert_unit)]
        from: ConvertUnit,

        /// Unit to convert to: `amp`, `db`, `hz`, or `oppc`.
        #[arg(long, value_parser = parse_convert_unit)]
        to: ConvertUnit,

        /// Normalization amplitude for `amp -> db`: divides the amplitude
        /// by this before converting to decibels. Ignored otherwise.
        #[arg(long, default_value_t = 1.0)]
        norm: f32,

        /// A value to convert; repeat for more than one.
        #[arg(long = "value", required = true, allow_hyphen_values = true)]
        values: Vec<f32>,
    },

    /// Impulse-response analysis: zero-pads and FFTs a `[--begin, --end)`
    /// window of each input channel into a peak-normalized `.ir` file
    /// (Phase 5's FFT-convolution family - see `pvc-core::tools::
    /// impulseresponse`'s doc comment for what's out of scope).
    ///
    /// Ports `impulseresponse`'s analysis path (`legacy/pvc_src/
    /// impulseresponse.c`).
    Impulseresponse(Box<ImpulseresponseArgs>),

    /// Fast FFT convolution (or deconvolution) of each input channel
    /// against a `.ir` file's spectrum (Phase 5's FFT-convolution family).
    /// See `pvc-core::tools::irconvolver`'s doc comment for what's out of
    /// scope, and a real usage-text bug in the C corrected here.
    ///
    /// Ports `irconvolver`'s resynthesis path (`legacy/pvc_src/
    /// irconvolver.c`).
    Irconvolver(Box<IrconvolverArgs>),

    /// Phase-vocoder feedback reverberator/resonator: a delay network
    /// built out of spectral frames rather than time-domain samples,
    /// combining an independently pitch/frequency-shiftable copy of the
    /// dry source with a recirculating, EQ'd and envelope-gated feedback
    /// path. See `pvc-core::tools::ring`'s doc comment for real dead
    /// flags, a real swapped-default bug reproduced faithfully, and
    /// what's out of scope (Phase 5's long tail).
    ///
    /// Ports `ring`'s audio-processing path (`legacy/pvc_src/ring.c`).
    Ring(Box<RingArgs>),

    /// `pvc ring` plus a switchable fixed-spectrum filter (a `.fr`
    /// response file, the same format `pvc filter` reads), placed either
    /// on the feedback path's input ("prefilter") or inside the loop
    /// ("postfilter"). See `pvc-core::tools::ringfilter`'s doc comment
    /// for what's shared with `pvc ring` and what's different.
    ///
    /// Ports `ringfilter`'s audio-processing path (`legacy/pvc_src/
    /// ringfilter.c`).
    Ringfilter(Box<RingfilterArgs>),

    /// `pvc ring` plus a switchable *time-varying* filter (a `.pva`
    /// filter-analysis file, navigated over time the same way `pvc
    /// tvfilter` navigates its own response), placed either on the
    /// feedback path's input ("prefilter") or inside the loop
    /// ("postfilter"). See `pvc-core::tools::ringtvfilter`'s doc comment
    /// for what's shared with `pvc ringfilter`/`pvc tvfilter` and what's
    /// different.
    ///
    /// Ports `ringtvfilter`'s audio-processing path (`legacy/pvc_src/
    /// ringtvfilter.c`).
    Ringtvfilter(Box<RingtvfilterArgs>),

    /// Crossfades a signal through a sequence of impulse responses,
    /// morphing from one to the next (Phase 5's FFT-convolution family).
    /// See `pvc-core::tools::irconvolvesequencer`'s doc comment for the
    /// C's dead `-a` flag and what's out of scope.
    ///
    /// Ports `irconvolvesequencer` (`legacy/pvc_src/
    /// irconvolvesequencer.c`).
    Irconvolvesequencer(Box<IrconvolvesequencerArgs>),

    /// Short-term FFT spectral multiplier: convolves a live input
    /// ("Sound A") against a pre-analyzed `.pva` filter file ("Sound
    /// B"), navigated over time the same way `pvc tvfilter`/`pvc twarp`
    /// navigate their own filter/resynthesis sources, then pans between
    /// the dry sounds and their convolution.
    ///
    /// Ports `convolver`'s audio-processing path (`legacy/pvc_src/
    /// convolver.c`); see `pvc-core::tools::convolver`'s doc comment for
    /// a real "spectral multiplication" bug (the tool's own complex-
    /// multiply code is commented out; the active code does a naive
    /// per-index multiply instead, reproduced faithfully) and a real
    /// uninitialized-memory bug in its `-l`/`-L` smoothing path, not
    /// exposed here.
    Convolver(Box<ConvolverArgs>),

    /// Peak formant tracker: a time-series of the loudest bin's
    /// frequency over a detection band, never audio.
    ///
    /// Ports `peakformant`'s audio-processing path (`legacy/pvc_src/
    /// peakformant.c`); see `pvc-core::tools::peakformant`'s doc comment
    /// for why it reuses `pvc centroid`'s whole two-pass pipeline
    /// (confirmed byte-for-byte identical apart from the one real
    /// per-frame analysis difference).
    Peakformant(Box<PeakformantArgs>),

    /// Spectral flatness tracker: a time-series of the geometric-to-
    /// arithmetic mean ratio of each frame's bins over a detection band
    /// (near `1.0` for noise-like spectra, near `0.0` for tonal ones),
    /// never audio.
    ///
    /// Ports `specflattracker`'s audio-processing path (`legacy/
    /// pvc_src/specflattracker.c`); see `pvc-core::tools::
    /// specflattracker`'s doc comment for why it reuses most of `pvc
    /// centroid`'s two-pass pipeline, plus a real pass-2 interpolation
    /// quirk absent from `centroid` and reproduced faithfully here.
    Specflattracker(Box<SpecflattrackerArgs>),

    /// Periodic/noise spectrum separator: tracks each bin's frame-to-
    /// frame frequency deviation and gates it on or off depending on
    /// whether that deviation stays under a threshold, extracting either
    /// the tonal part of a sound or its noise residue.
    ///
    /// Ports `spectralextractor`'s audio-processing path (`legacy/
    /// pvc_src/spectralextractor.c`); see `pvc-core::tools::
    /// spectralextractor`'s doc comment for a real dead pitch/frequency-
    /// shift computation reproduced faithfully, and what's out of scope.
    Spectralextractor(Box<SpectralExtractorArgs>),
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

/// `pvc analyze`'s flag surface. Long names follow the same convention as
/// [`PvArgs`]; see `pvc-core::tools::analyze`'s doc comment for why these
/// are plain numbers, not `@path`-capable control functions like `pv`'s -
/// none of `pvanalysis.c`'s equivalent flags are control-function strings
/// in the C either.
#[derive(clap::Args, Debug)]
pub struct AnalyzeArgs {
    /// FFT size (must be a power of two).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis window length. Defaults to `4096` - the C's own hardcoded
    /// default, *not* `2 * fft` (same gotcha as `pv`'s `--window-size`;
    /// see `pvc-core::tools::analyze`'s doc comment). Pass `0` for the
    /// `2 * fft` auto-scaling rule instead.
    #[arg(long, default_value_t = 4096)]
    pub window_size: usize,

    /// Analysis window shape.
    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    /// Analysis frames per second (sets the hop size).
    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Gain in dB.
    #[arg(long, default_value_t = 0.0, allow_hyphen_values = true)]
    pub gain: f32,

    /// Low shelf EQ gain in dB.
    #[arg(
        long = "shelf-low-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_low_gain: f32,

    /// High shelf EQ gain in dB.
    #[arg(
        long = "shelf-high-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_high_gain: f32,

    /// Low shelf EQ frequency in Hz.
    #[arg(long = "shelf-low-freq", default_value_t = 200.0)]
    pub shelf_low_freq: f32,

    /// High shelf EQ frequency in Hz.
    #[arg(long = "shelf-high-freq", default_value_t = 2000.0)]
    pub shelf_high_freq: f32,

    /// Spectrum magnitude warp index (`0` = no warp).
    #[arg(long, default_value_t = 0.0, allow_hyphen_values = true)]
    pub warp: f32,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc twarp`'s flag surface. Long names follow the same convention as
/// [`PvArgs`]/[`AnalyzeArgs`]. See `pvc-core::tools::twarp`'s doc comment
/// for the features deliberately not exposed here yet (time-point
/// dither, loop-boundary normalization, shimmer).
#[derive(clap::Args, Debug)]
pub struct TwarpArgs {
    /// Output duration in seconds. `0` (the default) means "use the
    /// analysis file's own duration".
    #[arg(long, default_value_t = 0.0)]
    pub duration: f32,

    /// Analysis window length. `0` means auto (`2 * fft`, where `fft` is
    /// always the analysis file's own FFT size - `twarp` has no
    /// independent `--fft`). The C's own hardcoded default is a literal
    /// `2048`, matching `pv`'s `--window-size` gotcha.
    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    /// Resynthesis window shape.
    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    /// Resynthesis frames per second (sets the hop size; there's no
    /// independent stretch factor - unlike `pv`, `twarp`'s hop always
    /// equals its own decimation).
    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time-position origin in seconds into the analysis data - a plain
    /// number, or `@path`.
    #[arg(long = "time-origin", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub time_origin: ControlFn,

    /// Playback rate multiplier (`1` = original speed, `2` = double
    /// speed, negative = reverse, `0` = stationary) - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "1", allow_hyphen_values = true)]
    pub rate: ControlFn,

    /// Analysis time window low boundary in seconds - a plain number, or
    /// `@path`.
    #[arg(long = "window-low", value_parser = parse_control_fn, default_value = "0")]
    pub window_low: ControlFn,

    /// Analysis time window high boundary in seconds (negative = end of
    /// analysis data) - a plain number, or `@path`.
    #[arg(long = "window-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub window_high: ControlFn,

    /// Time-position change response (smoothing) time in seconds - a
    /// plain number, or `@path`.
    #[arg(long = "time-response", value_parser = parse_control_fn, default_value = "0")]
    pub time_response: ControlFn,

    /// Sampler-loop boundary smoothing time in seconds - a plain number,
    /// or `@path`.
    #[arg(long = "loop-smooth", value_parser = parse_control_fn, default_value = "0.2")]
    pub loop_smooth: ControlFn,

    /// Time-window behavior: stop once time exits the window
    /// (`autostop`), or wrap/fold/clip at its edges forever (`loop`).
    #[arg(long = "window-mode", value_parser = parse_window_mode, default_value = "loop", num_args = 1)]
    pub window_mode: bool,

    /// Sampler-loop boundary behavior (only used in `loop` window mode).
    #[arg(long = "loop-mode", value_parser = parse_loop_mode, default_value = "wrap")]
    pub loop_mode: pvc_core::timenav::LoopMode,

    /// Trigger the time window only once it's first entered, and extend
    /// the output duration on the way out to guarantee a full pass back
    /// to the window's edge (used for percussive onset + sustain-loop +
    /// release shaping).
    #[arg(long = "onset-release")]
    pub onset_release: bool,

    /// Pitch shift in semitones - a plain number, or `@path`. Nonzero (or
    /// time-varying) picks oscillator-bank resynthesis; left at `0`
    /// (with `--freq-shift` also `0`) picks overlap-add instead - see
    /// `pvc-core::tools::twarp`'s doc comment.
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

    /// Frequency-change response (smoothing) time in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "freq-response-time", value_parser = parse_control_fn, default_value = "0")]
    pub freq_response_time: ControlFn,

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

    /// Oscillator resynthesis threshold in dB.
    #[arg(long, default_value_t = -60.0, allow_hyphen_values = true)]
    pub threshold: f32,

    /// Path to the `.pva` analysis file to resynthesize from.
    pub analysis: PathBuf,

    pub output: PathBuf,
}

#[derive(clap::Args, Debug)]
pub struct TvfilterArgs {
    /// FFT size (must be a power of two) - independent of the filter
    /// response file's own FFT size (see `pvc-core::tools::tvfilter`'s
    /// doc comment on `N_ratio`).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`).
    #[arg(long, default_value_t = 0)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time expansion/contraction factor (`1.0` = unchanged duration).
    #[arg(long, default_value_t = 1.0)]
    pub time_factor: f32,

    /// `-P`: pitch transposition, in semitones - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub pitch: ControlFn,

    /// `-a`: frequency shift adder, in Hz - a plain number, or `@path`.
    #[arg(long = "freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub freq_shift: ControlFn,

    /// `-A`: gain in dB - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub gain: ControlFn,

    /// `-B`: whether the filter's own transpose/shift compensates for
    /// `--pitch`/`--freq-shift`.
    #[arg(long = "filter-pitch-mode", value_parser = parse_filter_pitch_mode, default_value = "source-only", num_args = 1)]
    pub filter_pitch_mode: bool,

    /// `-F`: path to the time-varying filter response file - a `.pva`
    /// analysis file, either the legacy layout (a real `pvanalysis`
    /// output) or `pvc analyze`'s own `PVA1` format. Required.
    #[arg(long = "filter-response")]
    pub filter_response: PathBuf,

    /// `-K`: which filter-file channel to use (`0` = pair by channel
    /// index with the input sound file).
    #[arg(long = "analysis-channel", default_value_t = 0)]
    pub analysis_channel: usize,

    /// `-q`: filtering method.
    #[arg(long = "invert-mode", value_parser = parse_invert_mode, default_value = "pass")]
    pub invert_mode: pvc_core::tools::tvfilter::InvertMode,

    /// `-Q`: filter time point origin, in seconds - a plain number, or
    /// `@path`.
    #[arg(long = "time-origin", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub time_origin: ControlFn,

    /// `-Y`: filter rate multiplier - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "1", allow_hyphen_values = true)]
    pub rate: ControlFn,

    /// `-g`: filter time window low boundary, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "window-low", value_parser = parse_control_fn, default_value = "0")]
    pub window_low: ControlFn,

    /// `-G`: filter time window high boundary, in seconds (negative =
    /// end of the filter file) - a plain number, or `@path`.
    #[arg(long = "window-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub window_high: ControlFn,

    /// `-o`: sampler-loop boundary behavior (only used outside autostop
    /// mode).
    #[arg(long = "loop-mode", value_parser = parse_loop_mode, default_value = "wrap")]
    pub loop_mode: pvc_core::timenav::LoopMode,

    /// `-r`: trigger the time window only once it's first entered.
    #[arg(long = "onset-release")]
    pub onset_release: bool,

    /// `-d`: stop synthesis once the filter's time position exits its
    /// window, instead of looping.
    #[arg(long)]
    pub autostop: bool,

    /// `-j`: keep the filter's own amplitude roughly continuous across a
    /// sampler loop's seam.
    #[arg(long = "loop-normalization")]
    pub loop_normalization: bool,

    /// `-k`: peak loop-seam smoothing time, in seconds - a plain number,
    /// or `@path`.
    #[arg(long = "loop-smooth", value_parser = parse_control_fn, default_value = "0.2")]
    pub loop_smooth: ControlFn,

    /// `-E`: filter-spectrum compression threshold, in dB (`<= 0`) - a
    /// plain number, or `@path`.
    #[arg(long = "comp-threshold", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub comp_threshold: ControlFn,

    /// `-c`: filter-spectrum decibels of compression (`<= 0`) - a plain
    /// number, or `@path`.
    #[arg(long = "comp-db", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub comp_db: ControlFn,

    /// `-T`: filter response transposition, in semitones - a plain
    /// number, or `@path`.
    #[arg(long = "filter-transpose", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_transpose: ControlFn,

    /// `-V`: filter response shift, in Hz - a plain number, or `@path`.
    #[arg(long = "filter-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_shift: ControlFn,

    /// `-Z`: filter envelope release time, in seconds - a plain number,
    /// or `@path`.
    #[arg(long = "filter-release", value_parser = parse_control_fn, default_value = "0")]
    pub filter_release: ControlFn,

    /// `-z`: filter envelope attack time, in seconds - a plain number,
    /// or `@path`.
    #[arg(long = "filter-attack", value_parser = parse_control_fn, default_value = "0")]
    pub filter_attack: ControlFn,

    /// `-S`: blend between the filtered signal and the dry source, in
    /// dB. `-96` (the default) is fully filtered, `0` bypasses the
    /// filter entirely. A plain number, or `@path`.
    #[arg(long = "filter-source-gain", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub filter_source_gain: ControlFn,

    /// `-W`: filter response warp index - a plain number, or `@path`.
    #[arg(long = "filter-warpshape", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_warpshape: ControlFn,

    /// `-f`: filter response smoothing bandwidth, in octaves (negative)
    /// or Hz (positive) - a plain number, or `@path`.
    #[arg(long = "filter-smoothing", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_smoothing: ControlFn,

    /// `-H`: filter response low shelf gain, in dB - a plain number, or
    /// `@path`.
    #[arg(long = "shelf-low-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub shelf_low_gain: ControlFn,

    /// `-X`: filter response high shelf gain, in dB - a plain number, or
    /// `@path`.
    #[arg(long = "shelf-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub shelf_high_gain: ControlFn,

    /// `-m`: filter response low shelf frequency, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "shelf-low-freq", value_parser = parse_control_fn, default_value = "200")]
    pub shelf_low_freq: ControlFn,

    /// `-R`: filter response high shelf frequency, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "shelf-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub shelf_high_freq: ControlFn,

    /// `-n`: frame normalization decibel limit (`0` disables it) - a
    /// plain number, or `@path`.
    #[arg(long = "frame-norm-limit", default_value = "0", value_parser = parse_control_fn, allow_hyphen_values = true)]
    pub frame_norm_limit: ControlFn,

    /// `-v`: what frame normalization scales toward.
    #[arg(long = "normalize-to", value_parser = parse_normalize_to, default_value = "input", num_args = 1)]
    pub normalize_to_filter: bool,

    /// `-l`: envelope attack time, in seconds - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// `-L`: envelope release time, in seconds - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc convolver`'s flag surface. See `pvc-core::tools::convolver`'s
/// doc comment for what's out of scope (`-N` is dead in the real tool;
/// `-l`/`-L`/`-k` rely on uninitialized C memory and aren't exposed).
#[derive(clap::Args, Debug)]
pub struct ConvolverArgs {
    /// `-M`: analysis/resynthesis window length. `0` means auto
    /// (`2 * fft`, where `fft` is Sound B's own analysis file's FFT
    /// size - there is no independent `-N`).
    #[arg(long, default_value_t = 0)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    /// `-I`: time expansion/contraction factor (`1.0` = unchanged
    /// duration).
    #[arg(long, default_value_t = 1.0)]
    pub time_factor: f32,

    /// `-b`: begin time in seconds - real sample-accurate trimming, not
    /// `dur`-only bookkeeping (see `pvc-core::tools::convolver`'s doc
    /// comment).
    #[arg(long = "begin", default_value_t = 0.0)]
    pub begin: f32,

    /// `-e`: end time in seconds (`0` = end of file).
    #[arg(long = "end", default_value_t = 0.0)]
    pub end: f32,

    /// `-P`: pitch transposition of the output spectrum, in semitones -
    /// a plain number, or `@path`. Selects oscillator-bank resynthesis
    /// whenever nonzero (with `--freq-shift`), and, unlike `pvc
    /// spectralextractor`'s otherwise similar-looking flag, genuinely
    /// does transpose the output.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub pitch: ControlFn,

    /// `-a`: frequency shift of the output spectrum, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub freq_shift: ControlFn,

    /// `-A`: output gain in dB - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub gain: ControlFn,

    /// `-F`: path to Sound B's analysis file (the filter to convolve
    /// against) - a `.pva` file, either the legacy layout or `pvc
    /// analyze`'s own `PVA1` format. Required.
    #[arg(long = "filter-response")]
    pub filter_response: PathBuf,

    /// `-K`: which filter-file channel to use (`0` = pair by channel
    /// index with the input sound file).
    #[arg(long = "analysis-channel", default_value_t = 0)]
    pub analysis_channel: usize,

    /// `-Q`: Sound B time point origin, in seconds - a plain number, or
    /// `@path`.
    #[arg(long = "filter-time-origin", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_time_origin: ControlFn,

    /// `-Y`: Sound B rate multiplier - a plain number, or `@path`.
    #[arg(long = "filter-rate", value_parser = parse_control_fn, default_value = "1", allow_hyphen_values = true)]
    pub filter_rate: ControlFn,

    /// `-g`: Sound B time window lower boundary, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "filter-window-low", value_parser = parse_control_fn, default_value = "0")]
    pub filter_window_low: ControlFn,

    /// `-G`: Sound B time window upper boundary, in seconds (negative =
    /// end of the filter file) - a plain number, or `@path`.
    #[arg(long = "filter-window-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub filter_window_high: ControlFn,

    /// `-o`: Sound B time window out-of-bounds behavior (only used
    /// outside autostop mode).
    #[arg(long = "loop-mode", value_parser = parse_loop_mode, default_value = "wrap")]
    pub loop_mode: pvc_core::timenav::LoopMode,

    /// `-r`: trigger the time window only once it's first entered.
    #[arg(long = "onset-release")]
    pub onset_release: bool,

    /// `-y`: stop synthesis once Sound B's time position exits its
    /// window, instead of looping.
    #[arg(long)]
    pub autostop: bool,

    /// `-q`: Sound A's own gain, in dB - a plain number, or `@path`
    /// (confirmed by reading `convolver.c`'s own `crack()` switch - an
    /// unusual letter for this purpose, not a typo).
    #[arg(long = "sound-a-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub sound_a_gain: ControlFn,

    /// `-B`: Sound B's own gain, in dB - a plain number, or `@path`.
    #[arg(long = "sound-b-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub sound_b_gain: ControlFn,

    /// `-Z`: convolution gain, in dB - a plain number, or `@path`.
    #[arg(long = "convolve-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub convolve_gain: ControlFn,

    /// `-S`: pan position between Sound A, Sound B, and their
    /// convolution (`-1` = Sound A, `1` = Sound B, `0` = convolution) -
    /// a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub pan: ControlFn,

    /// `-j`: pan-position domain warp on the Sound A side (`pan < 0`).
    #[arg(long = "panwarp-a", default_value_t = 0.0, allow_hyphen_values = true)]
    pub panwarp_a: f32,

    /// `-J`: pan-position domain warp on the Sound B side (`pan >= 0`).
    #[arg(long = "panwarp-b", default_value_t = 0.0, allow_hyphen_values = true)]
    pub panwarp_b: f32,

    /// `-H`: convolution output low shelf gain, in dB. Fixed for the
    /// whole run, not a control function (matches the real tool).
    #[arg(
        long = "shelf-low-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_low_gain: f32,

    /// `-X`: convolution output high shelf gain, in dB.
    #[arg(
        long = "shelf-high-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_high_gain: f32,

    /// `-m`: convolution output low shelf frequency, in Hz.
    #[arg(long = "shelf-low-freq", default_value_t = 200.0)]
    pub shelf_low_freq: f32,

    /// `-R`: convolution output high shelf frequency, in Hz.
    #[arg(long = "shelf-high-freq", default_value_t = 2000.0)]
    pub shelf_high_freq: f32,

    /// `-n`: frame normalization decibel limit (`0` disables
    /// normalization) - a plain number, or `@path`.
    #[arg(long = "frame-norm-limit", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub frame_norm_limit: ControlFn,

    /// `-v`: what frame normalization scales toward.
    #[arg(long = "normalize-to", value_parser = parse_normalize_to, default_value = "input", num_args = 1)]
    pub normalize_to_filter: bool,

    /// `-t`: oscillator-bank resynthesis threshold in dB.
    #[arg(long, default_value_t = -96.0, allow_hyphen_values = true)]
    pub threshold: f32,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_filter_pitch_mode(s: &str) -> Result<bool, String> {
    match s {
        "source-only" => Ok(false),
        "source-and-filter" => Ok(true),
        _ => Err(format!(
            "expected \"source-only\" or \"source-and-filter\", got {s:?}"
        )),
    }
}

fn parse_invert_mode(s: &str) -> Result<pvc_core::tools::tvfilter::InvertMode, String> {
    use pvc_core::tools::tvfilter::InvertMode;
    match s {
        "pass" => Ok(InvertMode::Pass),
        "invert-fixed-peak" => Ok(InvertMode::InvertFixedPeak),
        "invert-frame-peak" => Ok(InvertMode::InvertFramePeak),
        _ => Err(format!(
            "expected \"pass\", \"invert-fixed-peak\", or \"invert-frame-peak\", got {s:?}"
        )),
    }
}

fn parse_normalize_to(s: &str) -> Result<bool, String> {
    match s {
        "input" => Ok(false),
        "filter" => Ok(true),
        _ => Err(format!("expected \"input\" or \"filter\", got {s:?}")),
    }
}

fn parse_window_mode(s: &str) -> Result<bool, String> {
    match s {
        "loop" => Ok(false),
        "autostop" => Ok(true),
        _ => Err(format!("expected \"loop\" or \"autostop\", got {s:?}")),
    }
}

fn parse_loop_mode(s: &str) -> Result<pvc_core::timenav::LoopMode, String> {
    match s {
        "wrap" => Ok(pvc_core::timenav::LoopMode::Wrap),
        "fold" => Ok(pvc_core::timenav::LoopMode::Fold),
        "clip" => Ok(pvc_core::timenav::LoopMode::Clip),
        _ => Err(format!(
            "expected \"wrap\", \"fold\", or \"clip\", got {s:?}"
        )),
    }
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

    /// Synthesize a `.fr` frequency-response file from a breakpoint or
    /// partial table, rather than analyzing a sound file (`pvc
    /// freqresponse` does that instead).
    Response {
        #[command(subcommand)]
        tool: ResponseCommand,
    },
}

#[derive(Subcommand, Debug)]
pub enum ResponseCommand {
    /// Ports `filtresponsemaker` (`legacy/pvc_src/filtresponsemaker.c`):
    /// a "frequency gradient" response, linearly interpolated in dB
    /// between unordered `(frequency-or-octave.pitchclass, decibels)`
    /// breakpoints.
    Filtresponsemaker {
        /// FFT size (must be a power of two).
        #[arg(long, default_value_t = 1024)]
        fft: usize,

        /// ASCII data file of unordered breakpoint duples: each line (or
        /// whitespace-separated pair) is `freq-or-octave.pitchclass,
        /// decibels`. Values `<= 12` are octave.pitchclass, otherwise Hz.
        #[arg(long)]
        breakpoints: PathBuf,

        /// Sound file to take the sample rate from.
        #[arg(long = "target-sound-file")]
        target_sound_file: PathBuf,

        /// Bandpass (keep the breakpoint shape) or band-reject (invert
        /// it: `1.0 - amplitude` at every bin).
        #[arg(long, value_parser = parse_response_mode, default_value = "bandpass")]
        mode: bool,

        output: PathBuf,
    },

    /// Ports `chordresponsemaker` (`legacy/pvc_src/
    /// chordresponsemaker.c`): a stack of harmonic-partial tones, each
    /// with a triangular- or rectangular-windowed dB rolloff around its
    /// center frequency, from unordered sextuples `(pitch-or-Hz,
    /// num_partials, bandwidth, decibels, partial_spacing,
    /// db_rolloff_per_octave)`.
    Chordresponsemaker {
        /// FFT size (must be a power of two).
        #[arg(long, default_value_t = 1024)]
        fft: usize,

        /// Sample rate in Hz. The real tool takes this from a `-f
        /// <soundfile>` it opens only to read the sample rate (and
        /// doesn't even mention in its own `usage()` text, despite
        /// requiring it) - this CLI just takes the number directly.
        #[arg(long = "sample-rate", default_value_t = 44100)]
        sample_rate: u32,

        /// ASCII data file of unordered sextuples: `pitch-or-Hz,
        /// num_partials, bandwidth, decibels, partial_spacing,
        /// db_rolloff_per_octave` (whitespace-separated).
        #[arg(long)]
        partials: PathBuf,

        /// How overlapping partial windows combine at a bin.
        #[arg(long, value_parser = parse_accumulation, default_value = "peak")]
        accumulation: pvc_core::tools::chordresponsemaker::Accumulation,

        /// The dB rolloff shape around each partial.
        #[arg(long = "band-window", value_parser = parse_band_window, default_value = "triangle")]
        band_window: pvc_core::tools::chordresponsemaker::BandWindow,

        /// Bandpass (keep the tone shape) or band-reject (invert it).
        #[arg(long, value_parser = parse_response_mode, default_value = "bandpass")]
        mode: bool,

        output: PathBuf,
    },

    /// Ports `groupdelaymaker` (`legacy/pvc_src/groupdelaymaker.c`): like
    /// `chordresponsemaker`, but each bin's response is an `(amp,
    /// delay-time)` pair instead of `(amp, frequency)`, from unordered
    /// septuples `(pitch-or-Hz, num_partials, bandwidth, decibels,
    /// partial_spacing, db_rolloff_total, delay_secs)`. See `pvc-core::
    /// tools::groupdelaymaker`'s doc comment for the real differences
    /// from `chordresponsemaker`'s own dB-rolloff/edge-falloff formulas.
    Groupdelaymaker {
        /// `-f`: a `.pva` analysis file (legacy or `pvc analyze`'s own
        /// `PVA1` format), used only to adopt its FFT size and sample
        /// rate - required.
        #[arg(long = "analysis")]
        analysis: PathBuf,

        /// `-F`: ASCII data file of unordered septuples: `pitch-or-Hz,
        /// num_partials, bandwidth, decibels, partial_spacing,
        /// db_rolloff_total, delay_secs` (whitespace-separated).
        #[arg(long)]
        partials: PathBuf,

        /// `-D`: dB offset (relative to each partial's own level) at the
        /// outer edge of its band. `0` (the default) is a flat,
        /// effectively rectangular band - not silence at the edges.
        #[arg(long = "edge-db", default_value_t = 0.0, allow_hyphen_values = true)]
        edge_db: f32,

        /// `-i`: default decibel level for bins no tone ever touches.
        #[arg(long = "default-db", default_value_t = 0.0, allow_hyphen_values = true)]
        default_db: f32,

        /// `-I`: default delay time, in seconds, for bins no tone ever
        /// touches.
        #[arg(long = "default-delay", default_value_t = 0.0)]
        default_delay: f32,

        /// `-s`: how overlapping partials resolve their `(amp, delay)`
        /// pair at a shared bin.
        #[arg(long, value_parser = parse_overlap_method, default_value = "average")]
        method: pvc_core::tools::groupdelaymaker::OverlapMethod,

        output: PathBuf,
    },
}

/// `pvc freqresponse`'s flag surface. Long names follow
/// `docs/dev/parameter-inventory.md` §5's proposed mapping.
#[derive(clap::Args, Debug)]
pub struct FreqresponseArgs {
    /// FFT size (must be a power of two).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis window length. `0` means auto (`2 * fft`); the C's own
    /// hardcoded default is a literal `2048`, matching `pv`'s
    /// `--window-size` gotcha.
    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    /// Analysis window shape.
    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    /// Analysis frames per second (sets the hop size).
    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// How frames accumulate into the response spectrum.
    #[arg(long = "spectrum-type", value_parser = parse_spectrum_type, default_value = "average")]
    pub spectrum_type: pvc_core::tools::freqresponse::Method,

    /// Weight the average toward louder frames (frame amplitude sum to
    /// the 5th power) - only meaningful with `--spectrum-type average`.
    #[arg(long = "weight-average")]
    pub weight_average: bool,

    /// Normalize the response according to its formant peaks; bins
    /// between formants get cross-faded gain from the bounding formants.
    #[arg(long = "formant-normalize")]
    pub formant_normalize: bool,

    /// Warp index for mid-formant amplitude compression/expansion
    /// (`> 0` expands the dynamic range between formants, `< 0`
    /// compresses it). Applies even with `--formant-normalize` off - see
    /// `pvc-core::tools::freqresponse`'s doc comment.
    #[arg(
        long = "formant-warp",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub formant_warp: f32,

    /// Low frequency limit for formant detection, in Hz.
    #[arg(long = "freq-low", default_value_t = 0.0)]
    pub freq_low: f32,

    /// High frequency limit for formant detection, in Hz (`0` = Nyquist).
    #[arg(long = "freq-high", default_value_t = 0.0)]
    pub freq_high: f32,

    /// Minimum formant peak amplitude, in dB.
    #[arg(long = "formant-floor", default_value_t = -96.0, allow_hyphen_values = true)]
    pub formant_floor: f32,

    /// Formant selection/rejection threshold, `0..1` - higher values
    /// select fewer, stronger formants.
    #[arg(long = "formant-threshold", default_value_t = 0.5)]
    pub formant_threshold: f32,

    /// Low shelf EQ gain in dB.
    #[arg(
        long = "shelf-low-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_low_gain: f32,

    /// High shelf EQ gain in dB.
    #[arg(
        long = "shelf-high-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_high_gain: f32,

    /// Low shelf EQ frequency in Hz.
    #[arg(long = "shelf-low-freq", default_value_t = 200.0)]
    pub shelf_low_freq: f32,

    /// High shelf EQ frequency in Hz.
    #[arg(long = "shelf-high-freq", default_value_t = 2000.0)]
    pub shelf_high_freq: f32,

    /// Skip EQ and its accompanying peak normalization entirely
    /// (inverted from the C's own `-B`: `0` there means "EQ with
    /// normalization", i.e. the opposite of this flag's name).
    #[arg(long = "no-normalize")]
    pub no_normalize: bool,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc filter`'s flag surface. Long names follow the same convention as
/// the other tools' - see `pvc-core::tools::filter`'s doc comment for
/// the source-mixing/response-file-size design decisions these flags
/// reflect.
#[derive(clap::Args, Debug)]
pub struct FilterArgs {
    /// Path to the `.fr` response file (its own size determines the FFT
    /// size - see `pvc-core::tools::filter`'s doc comment).
    #[arg(long)]
    pub response: PathBuf,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`);
    /// the C's own hardcoded default is a literal `2048`, matching
    /// `pv`'s `--window-size` gotcha.
    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time-stretch factor (`1.0` = unchanged).
    #[arg(long, default_value_t = 1.0)]
    pub stretch: f32,

    /// Filter-output pitch shift in semitones - a plain number, or `@path`.
    #[arg(long = "filter-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_pitch: ControlFn,

    /// Filter-output frequency shift in Hz - a plain number, or `@path`.
    #[arg(long = "filter-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_freq_shift: ControlFn,

    /// Filter-output gain in dB - a plain number, or `@path`.
    #[arg(long = "filter-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_gain: ControlFn,

    /// Filter-output time delay in seconds - a plain number, or `@path`.
    #[arg(long = "filter-time-delay", value_parser = parse_control_fn, default_value = "0")]
    pub filter_time_delay: ControlFn,

    /// Scales the effective delay used when resolving time-varying
    /// parameters against a delayed frame - a plain number, or `@path`.
    #[arg(long = "delay-time-scaler", value_parser = parse_control_fn, default_value = "1")]
    pub delay_time_scaler: ControlFn,

    /// Mix the (delayed/shifted) original source in alongside the
    /// filtered signal - on by default, matching the real tool (see
    /// `pvc-core::tools::filter`'s doc comment).
    #[arg(long = "source", default_value_t = true, action = clap::ArgAction::Set)]
    pub source_enabled: bool,

    /// Source gain in dB - a plain number, or `@path`.
    #[arg(long = "source-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_gain: ControlFn,

    /// Source pitch shift in semitones - a plain number, or `@path`.
    #[arg(long = "source-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_pitch: ControlFn,

    /// Source frequency shift in Hz - a plain number, or `@path`.
    #[arg(long = "source-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_freq_shift: ControlFn,

    /// Source time delay in seconds - a plain number, or `@path`.
    #[arg(long = "source-time-delay", value_parser = parse_control_fn, default_value = "0")]
    pub source_time_delay: ControlFn,

    /// Response pitch shift in semitones - a plain number, or `@path`.
    #[arg(long = "response-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub response_pitch: ControlFn,

    /// Response frequency shift in Hz - a plain number, or `@path`.
    #[arg(long = "response-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub response_freq_shift: ControlFn,

    /// Response magnitude warp index - a plain number, or `@path`.
    #[arg(long = "response-warp", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub response_warp: ControlFn,

    /// Response smoothing bandwidth: positive is Hz, negative is octaves
    /// - a plain number, or `@path`.
    #[arg(long = "response-smoothing", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub response_smoothing: ControlFn,

    /// Source signal floor in dB - how much of the source passes through
    /// unfiltered at the response's quietest points - a plain number, or
    /// `@path`.
    #[arg(long = "source-floor", default_value = "-96", value_parser = parse_control_fn, allow_hyphen_values = true)]
    pub source_floor: ControlFn,

    /// Apply the filter output's own pitch/frequency shift only to the
    /// source, not to how the response is positioned (off by default:
    /// the response tracks the filter output's shift too).
    #[arg(long = "pitch-shift-source-only")]
    pub pitch_shift_source_only: bool,

    /// Invert the response (band-reject instead of band-pass).
    #[arg(long = "band-reject")]
    pub band_reject: bool,

    #[arg(
        long = "shelf-low-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_low_gain: f32,

    #[arg(
        long = "shelf-high-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_high_gain: f32,

    #[arg(long = "shelf-low-freq", default_value_t = 200.0)]
    pub shelf_low_freq: f32,

    #[arg(long = "shelf-high-freq", default_value_t = 2000.0)]
    pub shelf_high_freq: f32,

    /// Compression threshold in dB.
    #[arg(
        long = "comp-threshold",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub comp_threshold: f32,

    /// Compression amount in dB.
    #[arg(
        long = "comp-amount",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub comp_amount: f32,

    /// Expansion threshold in dB.
    #[arg(long = "expand-threshold", default_value_t = -96.0, allow_hyphen_values = true)]
    pub expand_threshold: f32,

    /// Expansion amount in dB.
    #[arg(
        long = "expand-amount",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub expand_amount: f32,

    /// Per-frame amplitude normalization limit in dB - a plain number,
    /// or `@path`.
    #[arg(long = "normalization-limit", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub normalization_limit: ControlFn,

    /// Normalize each frame to match the filter response's own loudness
    /// instead of the (delayed) input sound's.
    #[arg(long = "normalize-to-response")]
    pub normalize_to_response: bool,

    /// Envelope attack time in seconds - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// Envelope release time in seconds - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc denoise`'s flag surface. Long names follow the same convention as
/// the other tools' - see `pvc-core::tools::noisefilter`'s doc comment
/// for the noise-window/gate design these flags reflect.
#[derive(clap::Args, Debug)]
pub struct DenoiseArgs {
    /// FFT size (must be a power of two).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`);
    /// the C's own hardcoded default is a literal `2048`, matching
    /// `pv`'s `--window-size` gotcha.
    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time-stretch factor (`1.0` = unchanged).
    #[arg(long, default_value_t = 1.0)]
    pub stretch: f32,

    /// Start of the noise-sample window to analyze, in seconds - point
    /// this at an actual noise-only stretch of the input (e.g. leading
    /// silence).
    #[arg(long = "noise-begin", default_value_t = 0.0)]
    pub noise_begin: f32,

    /// End of the noise-sample window, in seconds (`0` = end of file).
    #[arg(long = "noise-end", default_value_t = 0.0)]
    pub noise_end: f32,

    /// How the noise-sample window's spectrum is accumulated.
    #[arg(long = "noise-method", value_parser = parse_spectrum_type, default_value = "average")]
    pub noise_method: pvc_core::tools::freqresponse::Method,

    /// Noise-response bypass threshold in dB: bins in the (peak-
    /// normalized) noise response louder than this are excluded from the
    /// gate, letting those frequencies pass through unaffected.
    #[arg(
        long = "noise-bypass-threshold",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub noise_bypass_threshold: f32,

    /// Pitch shift in semitones - a plain number, or `@path`. Selects
    /// oscillator-bank resynthesis whenever nonzero/time-varying.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub pitch: ControlFn,

    /// Frequency shift in Hz - a plain number, or `@path`. Selects
    /// oscillator-bank resynthesis whenever nonzero/time-varying.
    #[arg(long = "freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub freq_shift: ControlFn,

    /// Gain in dB - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub gain: ControlFn,

    /// Noise threshold adjust in dB - a plain number, or `@path`.
    /// Positive values increase noise reduction, negative values reduce
    /// it.
    #[arg(long = "noise-threshold-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub noise_threshold_gain: ControlFn,

    /// Shape of the noise gate's expansion curve - a plain number, or
    /// `@path`.
    #[arg(long = "expansion-index", value_parser = parse_control_fn, default_value = "3")]
    pub expansion_index: ControlFn,

    /// Envelope attack time in seconds - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// Envelope release time in seconds - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    #[arg(
        long = "shelf-low-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_low_gain: f32,

    #[arg(
        long = "shelf-high-gain",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub shelf_high_gain: f32,

    #[arg(long = "shelf-low-freq", default_value_t = 200.0)]
    pub shelf_low_freq: f32,

    #[arg(long = "shelf-high-freq", default_value_t = 2000.0)]
    pub shelf_high_freq: f32,

    /// Oscillator resynthesis threshold in dB (bins quieter than this,
    /// relative to the frame's own peak, are skipped).
    #[arg(long, default_value_t = -96.0, allow_hyphen_values = true)]
    pub threshold: f32,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc compand`'s flag surface. Long names follow the same convention as
/// the other tools' - see `pvc-core::tools::compander`'s doc comment for
/// the static-peaks-file design these flags reflect.
#[derive(clap::Args, Debug)]
pub struct CompanderArgs {
    /// Path to the peaks/reference file (`.fr`-layout raw `N+2` binary
    /// floats, e.g. from `pvc freqresponse`). Its size must match `--fft`
    /// exactly - the real tool exits with an error on mismatch rather
    /// than adjusting either value.
    #[arg(long)]
    pub peaks: PathBuf,

    /// FFT size (must be a power of two, and must match `--peaks`'s own
    /// size).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`);
    /// the C's own hardcoded default is a literal `2048`, matching
    /// `pv`'s `--window-size` gotcha.
    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time-stretch factor (`1.0` = unchanged).
    #[arg(long, default_value_t = 1.0)]
    pub stretch: f32,

    /// Pitch shift in semitones - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub pitch: ControlFn,

    /// Frequency shift in Hz - a plain number, or `@path`.
    #[arg(long = "freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub freq_shift: ControlFn,

    /// Gain in dB - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub gain: ControlFn,

    /// Compression threshold in dB (bins louder than this, relative to
    /// the peaks file, are compressed) - a plain number, or `@path`.
    #[arg(long = "compress-threshold", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_threshold: ControlFn,

    /// Compression amount in dB (ratio; must be negative to actually
    /// compress - a non-negative value is a no-op in the real tool) - a
    /// plain number, or `@path`.
    #[arg(long = "compress-amount", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_amount: ControlFn,

    /// Expansion threshold in dB - a plain number, or `@path`.
    #[arg(long = "expand-threshold", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub expand_threshold: ControlFn,

    /// Expansion amount in dB (must be negative to actually expand) - a
    /// plain number, or `@path`.
    #[arg(long = "expand-amount", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub expand_amount: ControlFn,

    /// Companding band low edge in Hz - a plain number, or `@path`.
    #[arg(long = "band-low", value_parser = parse_control_fn, default_value = "0")]
    pub band_low: ControlFn,

    /// Companding band high edge in Hz (`< 0` = Nyquist) - a plain
    /// number, or `@path`.
    #[arg(long = "band-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub band_high: ControlFn,

    /// Companding band rolloff width in octaves - a plain number, or
    /// `@path`.
    #[arg(long = "band-rolloff", value_parser = parse_control_fn, default_value = "0")]
    pub band_rolloff: ControlFn,

    /// Peaks-file smoothing bandwidth, applied once at load time (`< 0`
    /// = octaves, `>= 0` = Hz; `0` = no smoothing).
    #[arg(
        long = "peaks-smoothing",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub peaks_smoothing: f32,

    /// Per-bin amplitude-change attack time in seconds - a plain number,
    /// or `@path`. (`-L`/release has no effect in the real tool - see
    /// `pvc-core::tools::compander`'s doc comment - so it isn't exposed
    /// here.)
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

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

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc spectwarp`'s flag surface. Long names follow the same convention
/// as the other tools' - see `pvc-core::tools::spectwarper`'s doc
/// comment for the self-referential (live-spectrum) companding design
/// these flags reflect, distinct from `pvc compand`'s static peaks file.
#[derive(clap::Args, Debug)]
pub struct SpectwarpArgs {
    /// FFT size (must be a power of two).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`);
    /// the C's own hardcoded default is a literal `2048`, matching
    /// `pv`'s `--window-size` gotcha.
    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time-stretch factor (`1.0` = unchanged).
    #[arg(long, default_value_t = 1.0)]
    pub stretch: f32,

    /// Pitch shift in semitones - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub pitch: ControlFn,

    /// Frequency shift in Hz - a plain number, or `@path`.
    #[arg(long = "freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub freq_shift: ControlFn,

    /// Gain in dB - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub gain: ControlFn,

    /// Compression threshold in dB, relative to the live peak - a plain
    /// number, or `@path`.
    #[arg(long = "compress-threshold", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_threshold: ControlFn,

    /// Compression amount in dB (must be negative to actually compress -
    /// a non-negative value is a no-op in the real tool) - a plain
    /// number, or `@path`.
    #[arg(long = "compress-amount", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_amount: ControlFn,

    /// Expansion threshold in dB (clamped to `>= -95` in the real tool)
    /// - a plain number, or `@path`.
    #[arg(long = "expand-threshold", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub expand_threshold: ControlFn,

    /// Expansion amount in dB (must be negative to actually expand) - a
    /// plain number, or `@path`.
    #[arg(long = "expand-amount", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub expand_amount: ControlFn,

    /// Companding band low edge in Hz - a plain number, or `@path`.
    #[arg(long = "band-low", value_parser = parse_control_fn, default_value = "0")]
    pub band_low: ControlFn,

    /// Companding band high edge in Hz (`< 0` = Nyquist) - a plain
    /// number, or `@path`.
    #[arg(long = "band-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub band_high: ControlFn,

    /// Companding band rolloff width in octaves - a plain number, or
    /// `@path`.
    #[arg(long = "band-rolloff", value_parser = parse_control_fn, default_value = "0")]
    pub band_rolloff: ControlFn,

    /// Shape exponent for the compress/expand gradient curve (`0` =
    /// linear) - a plain number, or `@path`.
    #[arg(long = "warp-curve", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp_curve: ControlFn,

    /// Smoothing time in seconds for the per-bin amplitude-change
    /// multiplier - a plain number, or `@path`.
    #[arg(long = "response-time", value_parser = parse_control_fn, default_value = "0")]
    pub response_time: ControlFn,

    /// Proportion (`0..1`) of the unmodified source blended back into
    /// the companded result - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub complement: ControlFn,

    /// Sliding compression window size (`<= 0` = one global peak for the
    /// whole band; `<= 8` = octaves, `> 8` = Hz) - a plain number, or
    /// `@path`. See `pvc-core::tools::spectwarper`'s doc comment for a
    /// real bound bug in the original tool this triggers.
    #[arg(long = "compress-window", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_window: ControlFn,

    /// Peak-reference attack time in seconds - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// Peak-reference release time in seconds - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    /// Per-frame amplitude normalization limit in dB (`0` disables
    /// normalization) - a plain number, or `@path`.
    #[arg(long = "normalize-limit", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub normalize_limit: ControlFn,

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

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc harmonize`'s flag surface. Long names follow
/// `docs/dev/parameter-inventory.md` §11's proposed mapping from
/// `harmonizer`'s single-letter flags - see
/// `pvc-core::tools::harmonizer`'s doc comment for the data-table
/// design, the crash bug it guards against instead of reproducing, and
/// the cross-band interpolation bug it reproduces faithfully.
#[derive(clap::Args, Debug)]
pub struct HarmonizeArgs {
    /// FFT size (must be a power of two).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`);
    /// the C's own hardcoded default is a literal `2048`, matching
    /// `pv`'s `--window-size` gotcha.
    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time-stretch factor (`1.0` = unchanged).
    #[arg(long, default_value_t = 1.0)]
    pub stretch: f32,

    /// Path to the 8-column data table (whitespace-delimited ASCII
    /// floats, one row per band: shift factor, low/high/center frequency
    /// in Hz or octave.pitchclass, peak dB, stopband dB, Q-index, delay
    /// in seconds - see `pvc-core::tools::harmonizer`'s doc comment).
    #[arg(long)]
    pub table: PathBuf,

    /// How the table's shift-factor column is interpreted.
    #[arg(long = "shift-format", value_parser = parse_shift_format, default_value = "multiplier")]
    pub shift_format: pvc_core::tools::harmonizer::ShiftFormat,

    /// Table frequency-column scaler (applied to low/high/center, after
    /// octave.pitchclass conversion).
    #[arg(long = "table-freq-scale", default_value_t = 1.0)]
    pub table_freq_scale: f32,

    /// Table frequency-column shifter, in Hz.
    #[arg(
        long = "table-freq-shift",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub table_freq_shift: f32,

    /// Table peak-dB-column scaler.
    #[arg(long = "table-peak-scale", default_value_t = 1.0)]
    pub table_peak_scale: f32,

    /// Table stopband-dB-column scaler.
    #[arg(long = "table-stopband-scale", default_value_t = 1.0)]
    pub table_stopband_scale: f32,

    /// Table stopband-dB-column shifter, in dB.
    #[arg(
        long = "table-stopband-shift",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub table_stopband_shift: f32,

    /// Table Q-index-column shifter.
    #[arg(
        long = "table-q-shift",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub table_q_shift: f32,

    /// Table delay-column scaler.
    #[arg(long = "table-delay-scale", default_value_t = 1.0)]
    pub table_delay_scale: f32,

    /// Table delay-column shifter, in seconds.
    #[arg(
        long = "table-delay-shift",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub table_delay_shift: f32,

    /// Table shift-factor-column scaler.
    #[arg(long = "table-shift-scale", default_value_t = 1.0)]
    pub table_shift_scale: f32,

    /// Table shift-factor-column shifter.
    #[arg(
        long = "table-shift-shift",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub table_shift_shift: f32,

    /// Master gain in dB - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub gain: ControlFn,

    /// Source frequency shift in Hz - a plain number, or `@path`.
    #[arg(long = "source-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_freq_shift: ControlFn,

    /// Source pitch shift in semitones - a plain number, or `@path`.
    /// Defaults to `1` semitone, matching the real tool's own default
    /// (`docs/dev/parameter-inventory.md`'s table lists `0` here, but
    /// the source's own initializer is `SOURCE_ptrans.A[0] = 1.` -
    /// confirmed by reading `harmonizer.c`).
    #[arg(long = "source-pitch", value_parser = parse_control_fn, default_value = "1", allow_hyphen_values = true)]
    pub source_pitch: ControlFn,

    /// Source gain in dB - a plain number, or `@path`.
    #[arg(long = "source-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_gain: ControlFn,

    /// Source delay in seconds - a plain number, or `@path`.
    #[arg(long = "source-delay", value_parser = parse_control_fn, default_value = "0")]
    pub source_delay: ControlFn,

    /// Voice (harmony) frequency shift in Hz, added post-shift - a plain
    /// number, or `@path`.
    #[arg(long = "voice-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub voice_freq_shift: ControlFn,

    /// Voice (harmony) pitch shift in semitones, applied post-shift - a
    /// plain number, or `@path`. Defaults to `1` semitone - see
    /// `--source-pitch`'s doc comment for why.
    #[arg(long = "voice-pitch", value_parser = parse_control_fn, default_value = "1", allow_hyphen_values = true)]
    pub voice_pitch: ControlFn,

    /// Voice (harmony) gain in dB, per band - a plain number, or `@path`.
    #[arg(long = "voice-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub voice_gain: ControlFn,

    /// Voice (harmony) spectrum warp index - a plain number, or `@path`.
    #[arg(long = "voice-warp", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub voice_warp: ControlFn,

    /// Frequency interpolation control, per band (`0` = unshifted, `1` =
    /// fully shifted) - a plain number, or `@path`. See this struct's
    /// doc comment for a real bug affecting this flag's per-band
    /// granularity in `multiplier`/`semitones` shift-format modes.
    #[arg(long = "freq-interp", value_parser = parse_control_fn, default_value = "1")]
    pub freq_interp: ControlFn,

    /// Time interpolation control, per band (`0` = no delay applied, `1`
    /// = the table's full delay) - a plain number, or `@path`.
    #[arg(long = "time-interp", value_parser = parse_control_fn, default_value = "1")]
    pub time_interp: ControlFn,

    /// Oscillator resynthesis threshold in dB (bins quieter than this,
    /// relative to the frame's own peak, are skipped).
    #[arg(long, default_value_t = -96.0, allow_hyphen_values = true)]
    pub threshold: f32,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc envelope`'s flag surface - see `pvc-core::tools::envelope`'s doc
/// comment for the two-pass (per-channel analysis, then combine/
/// compress/gate/warp/interpolate) design these flags reflect.
#[derive(clap::Args, Debug)]
pub struct EnvelopeArgs {
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Detection-band boundaries are octave.pitchclass values instead of Hz.
    #[arg(long = "band-octave-pitchclass")]
    pub band_octave_pitchclass: bool,

    #[arg(long = "band-low", value_parser = parse_control_fn, default_value = "0")]
    pub band_low: ControlFn,

    /// `< 0` means Nyquist.
    #[arg(long = "band-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub band_high: ControlFn,

    #[arg(long = "channel-method", value_parser = parse_channel_method, default_value = "average")]
    pub channel_method: pvc_core::tools::envelope::ChannelMethod,

    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    /// Attack time for the subtractable "filtered envelope" (`-j`).
    #[arg(long = "filtered-attack", value_parser = parse_control_fn, default_value = "0")]
    pub filtered_attack: ControlFn,

    /// Release time for the subtractable "filtered envelope" (`-k`).
    #[arg(long = "filtered-release", value_parser = parse_control_fn, default_value = "0")]
    pub filtered_release: ControlFn,

    /// Proportion of the filtered envelope subtracted from the raw band
    /// sum before the main attack/release (`-m`; `0` = none, `1` = full).
    #[arg(long = "filtered-cut", value_parser = parse_control_fn, default_value = "0")]
    pub filtered_cut: ControlFn,

    #[arg(long = "compress-threshold", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_threshold: ControlFn,

    #[arg(long = "compress-amount", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_amount: ControlFn,

    #[arg(long = "gate-threshold", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub gate_threshold: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp: ControlFn,

    #[arg(long = "output-rate", default_value_t = 500.0)]
    pub output_rate: f32,

    #[arg(long = "output-scale", value_parser = parse_output_scale, default_value = "amp")]
    pub output_scale: pvc_core::tools::envelope::OutputScale,

    #[arg(long = "output-type", value_parser = parse_output_type, default_value = "ascii")]
    pub output_type: OutputType,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `ascii` (one `%f\n` value per line) vs `float` (headerless native f32
/// stream) - shared by `envelope`/`centroid`/`fluxoid`/`pitchtrack`'s
/// `-g` flag.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OutputType {
    Ascii,
    Float,
}

fn parse_output_type(s: &str) -> Result<OutputType, String> {
    match s {
        "ascii" => Ok(OutputType::Ascii),
        "float" => Ok(OutputType::Float),
        _ => Err(format!("expected \"ascii\" or \"float\", got {s:?}")),
    }
}

fn parse_channel_method(s: &str) -> Result<pvc_core::tools::envelope::ChannelMethod, String> {
    use pvc_core::tools::envelope::ChannelMethod;
    match s {
        "average" => Ok(ChannelMethod::Average),
        "peak" => Ok(ChannelMethod::Peak),
        _ => Err(format!("expected \"average\" or \"peak\", got {s:?}")),
    }
}

fn parse_output_scale(s: &str) -> Result<pvc_core::tools::envelope::OutputScale, String> {
    use pvc_core::tools::envelope::OutputScale;
    match s {
        "amp" => Ok(OutputScale::Amp),
        "db" => Ok(OutputScale::Db),
        "inverted-amp" => Ok(OutputScale::InvertedAmp),
        "inverted-db" => Ok(OutputScale::InvertedDb),
        _ => Err(format!(
            "expected \"amp\", \"db\", \"inverted-amp\", or \"inverted-db\", got {s:?}"
        )),
    }
}

/// `pvc centroid`'s flag surface - see `pvc-core::tools::centroid`'s doc
/// comment for the two dead flags (`-T`/`-S` compress/gate, `-H` a
/// second warp) not exposed here.
#[derive(clap::Args, Debug)]
pub struct CentroidArgs {
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    #[arg(long = "band-octave-pitchclass")]
    pub band_octave_pitchclass: bool,

    #[arg(long = "band-low", value_parser = parse_control_fn, default_value = "0")]
    pub band_low: ControlFn,

    /// `< 0` means Nyquist.
    #[arg(long = "band-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub band_high: ControlFn,

    #[arg(long = "channel-method", value_parser = parse_channel_method, default_value = "average")]
    pub channel_method: pvc_core::tools::centroid::ChannelMethod,

    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp: ControlFn,

    #[arg(long = "output-rate", default_value_t = 500.0)]
    pub output_rate: f32,

    #[arg(long = "output-format", value_parser = parse_centroid_output_format, default_value = "freq")]
    pub output_format: pvc_core::tools::centroid::OutputFormat,

    /// Reference pitch in octave.pitchclass notation, used only by
    /// `semitones-deviation`/`neg-semitones-deviation` output formats.
    #[arg(long = "reference-pitch", default_value_t = 8.0)]
    pub reference_pitch: f32,

    #[arg(long = "output-type", value_parser = parse_output_type, default_value = "ascii")]
    pub output_type: OutputType,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_centroid_output_format(
    s: &str,
) -> Result<pvc_core::tools::centroid::OutputFormat, String> {
    use pvc_core::tools::centroid::OutputFormat;
    match s {
        "freq" => Ok(OutputFormat::Freq),
        "octave" => Ok(OutputFormat::Octave),
        "octave-pitchclass" => Ok(OutputFormat::OctavePitchclass),
        "semitones-deviation" => Ok(OutputFormat::SemitonesDeviation),
        "neg-semitones-deviation" => Ok(OutputFormat::NegSemitonesDeviation),
        _ => Err(format!(
            "expected \"freq\", \"octave\", \"octave-pitchclass\", \"semitones-deviation\", or \"neg-semitones-deviation\", got {s:?}"
        )),
    }
}

/// `pvc peakformant`'s flag surface - identical shape to [`CentroidArgs`]
/// (see `pvc-core::tools::peakformant`'s doc comment for why).
#[derive(clap::Args, Debug)]
pub struct PeakformantArgs {
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    #[arg(long = "band-octave-pitchclass")]
    pub band_octave_pitchclass: bool,

    #[arg(long = "band-low", value_parser = parse_control_fn, default_value = "0")]
    pub band_low: ControlFn,

    /// `< 0` means Nyquist.
    #[arg(long = "band-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub band_high: ControlFn,

    #[arg(long = "channel-method", value_parser = parse_channel_method, default_value = "average")]
    pub channel_method: pvc_core::tools::peakformant::ChannelMethod,

    /// `-l`: envelope ascent (attack) time.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// `-L`: envelope descent (release) time.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp: ControlFn,

    #[arg(long = "output-rate", default_value_t = 500.0)]
    pub output_rate: f32,

    #[arg(long = "output-format", value_parser = parse_centroid_output_format, default_value = "freq")]
    pub output_format: pvc_core::tools::peakformant::OutputFormat,

    /// Reference pitch in octave.pitchclass notation, used only by
    /// `semitones-deviation`/`neg-semitones-deviation` output formats.
    #[arg(long = "reference-pitch", default_value_t = 8.0)]
    pub reference_pitch: f32,

    #[arg(long = "output-type", value_parser = parse_output_type, default_value = "ascii")]
    pub output_type: OutputType,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc specflattracker`'s flag surface - mostly the same shape as
/// [`CentroidArgs`]/[`PeakformantArgs`], minus `--reference-pitch` (dead
/// in the real tool - see `pvc-core::tools::specflattracker`'s doc
/// comment), plus `--method` and `--amplitude-threshold`.
#[derive(clap::Args, Debug)]
pub struct SpecflattrackerArgs {
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    #[arg(long = "band-octave-pitchclass")]
    pub band_octave_pitchclass: bool,

    #[arg(long = "band-low", value_parser = parse_control_fn, default_value = "0")]
    pub band_low: ControlFn,

    /// `< 0` means Nyquist.
    #[arg(long = "band-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub band_high: ControlFn,

    #[arg(long = "channel-method", value_parser = parse_channel_method, default_value = "average")]
    pub channel_method: pvc_core::tools::specflattracker::ChannelMethod,

    /// `-m`: which per-bin quantity feeds the flatness ratio.
    #[arg(long, value_parser = parse_flatness_method, default_value = "amplitude")]
    pub method: pvc_core::tools::specflattracker::FlatnessMethod,

    /// `-c`: amplitude threshold in dB for excluding/flooring low-valued
    /// bins. The real tool's own `usage()` text claims `-200` as the
    /// default, but its actual variable initializer is `-96.` - this
    /// matches the code, not the usage text (see `pvc-core::tools::
    /// specflattracker`'s doc comment).
    #[arg(long = "amplitude-threshold", default_value_t = -96.0, allow_hyphen_values = true)]
    pub amplitude_threshold: f32,

    /// `-l`: trajectory ascent (attack) time.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// `-L`: trajectory descent (release) time.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp: ControlFn,

    #[arg(long = "output-rate", default_value_t = 500.0)]
    pub output_rate: f32,

    #[arg(long = "output-format", value_parser = parse_specflattracker_output_format, default_value = "coefficient")]
    pub output_format: pvc_core::tools::specflattracker::OutputFormat,

    #[arg(long = "output-type", value_parser = parse_output_type, default_value = "ascii")]
    pub output_type: OutputType,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_flatness_method(
    s: &str,
) -> Result<pvc_core::tools::specflattracker::FlatnessMethod, String> {
    use pvc_core::tools::specflattracker::FlatnessMethod;
    match s {
        "amplitude" => Ok(FlatnessMethod::Amplitude),
        "amplitude-change" => Ok(FlatnessMethod::AmplitudeChange),
        "frequency-change" => Ok(FlatnessMethod::FrequencyChange),
        _ => Err(format!(
            "expected \"amplitude\", \"amplitude-change\", or \"frequency-change\", got {s:?}"
        )),
    }
}

fn parse_specflattracker_output_format(
    s: &str,
) -> Result<pvc_core::tools::specflattracker::OutputFormat, String> {
    use pvc_core::tools::specflattracker::OutputFormat;
    match s {
        "coefficient" => Ok(OutputFormat::Coefficient),
        "decibels" => Ok(OutputFormat::Decibels),
        _ => Err(format!(
            "expected \"coefficient\" or \"decibels\", got {s:?}"
        )),
    }
}

/// `pvc flux`'s flag surface - see `pvc-core::tools::fluxoid`'s doc
/// comment for the shared two-pass shape.
#[derive(clap::Args, Debug)]
pub struct FluxArgs {
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    #[arg(long = "band-octave-pitchclass")]
    pub band_octave_pitchclass: bool,

    #[arg(long = "band-low", value_parser = parse_control_fn, default_value = "0")]
    pub band_low: ControlFn,

    /// `< 0` means Nyquist.
    #[arg(long = "band-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub band_high: ControlFn,

    #[arg(long = "channel-method", value_parser = parse_channel_method, default_value = "average")]
    pub channel_method: pvc_core::tools::fluxoid::ChannelMethod,

    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    /// Weight each bin's frequency change by its own amplitude.
    #[arg(
        long = "amplitude-weighting",
        default_value_t = true,
        action = clap::ArgAction::Set
    )]
    pub amplitude_weighting: bool,

    #[arg(long = "compress-threshold", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_threshold: ControlFn,

    #[arg(long = "compress-amount", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_amount: ControlFn,

    #[arg(long = "gate-threshold", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub gate_threshold: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp: ControlFn,

    #[arg(long = "output-rate", default_value_t = 500.0)]
    pub output_rate: f32,

    #[arg(long = "output-scale", value_parser = parse_output_scale, default_value = "amp")]
    pub output_scale: pvc_core::tools::fluxoid::OutputScale,

    #[arg(long = "output-type", value_parser = parse_output_type, default_value = "ascii")]
    pub output_type: OutputType,

    pub input: PathBuf,
    pub output: PathBuf,
}

/// `pvc pitchtrack`'s flag surface - see
/// `pvc-core::tools::pitchtracker`'s doc comment for two doc-corrected
/// defaults (`--band-low` really defaults to `0` Hz, and ASCII output
/// really is the default, not float) and several real bugs reproduced
/// faithfully.
#[derive(clap::Args, Debug)]
pub struct PitchtrackArgs {
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    #[arg(long, default_value_t = 2048)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    #[arg(long = "band-low", value_parser = parse_control_fn, default_value = "0")]
    pub band_low: ControlFn,

    /// `< 0` means Nyquist.
    #[arg(long = "band-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub band_high: ControlFn,

    #[arg(long, value_parser = parse_detect_method, default_value = "optimal-comb")]
    pub method: pvc_core::tools::pitchtracker::DetectMethod,

    /// `-j`: beginning note-stabilization buffer size in seconds.
    #[arg(long = "window-min", default_value_t = 0.05)]
    pub window_min: f32,

    /// `-J`: maximum note-stabilization buffer size in seconds.
    #[arg(long = "window-max", default_value_t = 0.3)]
    pub window_max: f32,

    #[arg(
        long = "detect-threshold",
        default_value_t = -40.0,
        allow_hyphen_values = true
    )]
    pub detect_threshold: f32,

    /// `-H`: temporal mode-filter window, in seconds (`0` disables it).
    #[arg(long = "mode-filter-window", default_value_t = 0.0)]
    pub mode_filter_window: f32,

    /// `-E`: amplitude-weighted oversampling factor (`0` disables it).
    #[arg(long = "oversample", default_value_t = 0.0)]
    pub oversample: f32,

    /// `-a`: one-pole lowpass on the output frequency alone.
    #[arg(long = "smooth-response", default_value_t = 0.0)]
    pub smooth_response: f32,

    #[arg(long = "channel-method", value_parser = parse_channel_method, default_value = "average")]
    pub channel_method: pvc_core::tools::pitchtracker::ChannelMethod,

    #[arg(long = "compress-threshold", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_threshold: ControlFn,

    #[arg(long = "compress-amount", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub compress_amount: ControlFn,

    #[arg(long = "gate-threshold", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub gate_threshold: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    #[arg(long = "output-rate", default_value_t = 500.0)]
    pub output_rate: f32,

    #[arg(long = "output-format", value_parser = parse_pitchtrack_output_format, default_value = "freq")]
    pub output_format: pvc_core::tools::pitchtracker::OutputFormat,

    /// `-o`: reference pitch, Hz or octave.pitchclass (`<= 12`) - only
    /// affects `semitones-deviation`/`neg-semitones-deviation` output.
    #[arg(long = "reference", value_parser = parse_control_fn, default_value = "440")]
    pub reference: ControlFn,

    /// ASCII (one value per line) or a headerless raw f32 stream.
    #[arg(long = "output-type", value_parser = parse_output_type, default_value = "ascii")]
    pub output_type: OutputType,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_detect_method(s: &str) -> Result<pvc_core::tools::pitchtracker::DetectMethod, String> {
    use pvc_core::tools::pitchtracker::DetectMethod;
    match s {
        "optimal-comb" => Ok(DetectMethod::OptimalComb),
        "strongest" => Ok(DetectMethod::Strongest),
        "centroid" => Ok(DetectMethod::Centroid),
        _ => Err(format!(
            "expected \"optimal-comb\", \"strongest\", or \"centroid\", got {s:?}"
        )),
    }
}

fn parse_pitchtrack_output_format(
    s: &str,
) -> Result<pvc_core::tools::pitchtracker::OutputFormat, String> {
    use pvc_core::tools::pitchtracker::OutputFormat;
    match s {
        "freq" => Ok(OutputFormat::Freq),
        "octave-decimal" => Ok(OutputFormat::OctaveDecimal),
        "semitones" => Ok(OutputFormat::SemitonesDeviation),
        "neg-semitones" => Ok(OutputFormat::NegSemitonesDeviation),
        "midi" => Ok(OutputFormat::Midi),
        "octave-pitchclass" => Ok(OutputFormat::OctavePitchclass),
        _ => Err(format!(
            "expected \"freq\", \"octave-decimal\", \"semitones\", \"neg-semitones\", \"midi\", or \"octave-pitchclass\", got {s:?}"
        )),
    }
}

fn parse_shift_format(s: &str) -> Result<pvc_core::tools::harmonizer::ShiftFormat, String> {
    use pvc_core::tools::harmonizer::ShiftFormat;
    match s {
        "multiplier" => Ok(ShiftFormat::Multiplier),
        "adder" => Ok(ShiftFormat::Adder),
        "semitones" => Ok(ShiftFormat::Semitones),
        _ => Err(format!(
            "expected \"multiplier\", \"adder\", or \"semitones\", got {s:?}"
        )),
    }
}

#[derive(clap::Args, Debug)]
pub struct RingArgs {
    /// FFT size (must be a power of two).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`).
    #[arg(long, default_value_t = 0)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time expansion/contraction factor (`1.0` = unchanged duration).
    #[arg(long, default_value_t = 1.0)]
    pub time_factor: f32,

    /// `-b`: begin time in seconds.
    #[arg(long, default_value_t = 0.0)]
    pub begin: f32,

    /// `-e`: end time in seconds (`0` = end of file).
    #[arg(long, default_value_t = 0.0)]
    pub end: f32,

    /// `-t`: oscillator-bank resynthesis threshold, in dB.
    #[arg(long, default_value_t = -96.0, allow_hyphen_values = true)]
    pub oscbank_threshold: f32,

    /// `-A`: master (source + reverb) gain in dB - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub master_gain: ControlFn,

    /// `-S`: source gain in dB - a plain number, or `@path`.
    #[arg(long = "source-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_gain: ControlFn,

    /// `-f`: source frequency shift adder, in Hz - a plain number, or
    /// `@path`.
    #[arg(long = "source-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_freq_shift: ControlFn,

    /// `-p`: source pitch transposition, in semitones - a plain number,
    /// or `@path`.
    #[arg(long = "source-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_pitch: ControlFn,

    /// `-F`: reverb (feedback) gain in dB - a plain number, or `@path`.
    #[arg(long = "feedback-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_gain: ControlFn,

    /// `-H`: reverb frequency shift adder, in Hz - a plain number, or
    /// `@path`.
    #[arg(long = "feedback-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_freq_shift: ControlFn,

    /// `-P`: reverb pitch transposition, in semitones - a plain number,
    /// or `@path`.
    #[arg(long = "feedback-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_pitch: ControlFn,

    /// `-Z`: reverb (feedback delay line) decay time, in seconds - `0`
    /// disables the loop entirely. A plain number, or `@path`.
    #[arg(long = "feedback-decay", value_parser = parse_control_fn, default_value = "0")]
    pub feedback_decay: ControlFn,

    /// `-z`: reverb (input) envelope-follower gate threshold, in dB - a
    /// plain number, or `@path`.
    #[arg(long = "feedback-threshold", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub feedback_threshold: ControlFn,

    /// `-V`: reverb threshold pass mode.
    #[arg(long = "feedback-threshold-mode", value_parser = parse_threshold_mode, default_value = "above", num_args = 1)]
    pub feedback_threshold_mode: bool,

    /// `-l`: reverb (input) envelope attack time, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "attack", value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// `-L`: reverb (input) envelope release time, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "release", value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    /// `-O`: reverb (input) EQ low shelf gain, in dB - a plain number, or
    /// `@path`. Defaults to `200`, not `0` - see `pvc-core::tools::
    /// ring`'s doc comment on the real swapped-default bug this
    /// reproduces.
    #[arg(long = "input-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub input_eq_low_gain: ControlFn,

    /// `-Y`: reverb (input) EQ high shelf gain, in dB - a plain number,
    /// or `@path`.
    #[arg(long = "input-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub input_eq_high_gain: ControlFn,

    /// `-d`: reverb (input) EQ low shelf frequency, in Hz - a plain
    /// number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ring`'s doc comment.
    #[arg(long = "input-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub input_eq_low_freq: ControlFn,

    /// `-n`: reverb (input) EQ high shelf frequency, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "input-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub input_eq_high_freq: ControlFn,

    /// `-T`: reverb (in-loop feedback) EQ decay time, in seconds - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-decay", value_parser = parse_control_fn, default_value = "1")]
    pub loop_eq_decay: ControlFn,

    /// `-E`: reverb (feedback) signal balance gain limiter level, `0` to
    /// `96` dB (`0` disables balancing entirely).
    #[arg(long = "loop-balance-limit", default_value_t = 0.0)]
    pub loop_balance_limit: f32,

    /// `-X`: reverb (in-loop feedback) EQ low shelf gain, in dB - a plain
    /// number, or `@path`. Defaults to `200`, not `0` - see
    /// `pvc-core::tools::ring`'s doc comment on the real swapped-default
    /// bug this reproduces (the same one as `-O`/`-d`, independently
    /// present here too).
    #[arg(long = "loop-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub loop_eq_low_gain: ControlFn,

    /// `-Q`: reverb (in-loop feedback) EQ high shelf gain, in dB - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub loop_eq_high_gain: ControlFn,

    /// `-U`: reverb (in-loop feedback) EQ low shelf frequency, in Hz - a
    /// plain number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ring`'s doc comment.
    #[arg(long = "loop-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub loop_eq_low_freq: ControlFn,

    /// `-m`: reverb (in-loop feedback) EQ high shelf frequency, in Hz - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub loop_eq_high_freq: ControlFn,

    /// `-k`: reverb (output) EQ low shelf gain, in dB - a plain number,
    /// or `@path`. Defaults to `200`, not `0` - see `pvc-core::tools::
    /// ring`'s doc comment on the real swapped-default bug this
    /// reproduces (the same one as `-O`/`-d`, independently present here
    /// too).
    #[arg(long = "output-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub output_eq_low_gain: ControlFn,

    /// `-c`: reverb (output) EQ high shelf gain, in dB - a plain number,
    /// or `@path`.
    #[arg(long = "output-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub output_eq_high_gain: ControlFn,

    /// `-s`: reverb (output) EQ low shelf frequency, in Hz - a plain
    /// number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ring`'s doc comment.
    #[arg(long = "output-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub output_eq_low_freq: ControlFn,

    /// `-G`: reverb (output) EQ high shelf frequency, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "output-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub output_eq_high_freq: ControlFn,

    pub input: PathBuf,
    pub output: PathBuf,
}

#[derive(clap::Args, Debug)]
pub struct RingfilterArgs {
    /// FFT size (must be a power of two) - must match the filter response
    /// file's own size (`fillfunc`'s "FILTER FILE SIZE DOES NOT MATCH
    /// FFT" exit).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`).
    #[arg(long, default_value_t = 0)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time expansion/contraction factor (`1.0` = unchanged duration).
    #[arg(long, default_value_t = 1.0)]
    pub time_factor: f32,

    /// `-b`: begin time in seconds.
    #[arg(long, default_value_t = 0.0)]
    pub begin: f32,

    /// `-e`: end time in seconds (`0` = end of file).
    #[arg(long, default_value_t = 0.0)]
    pub end: f32,

    /// `-t`: oscillator-bank resynthesis threshold, in dB.
    #[arg(long, default_value_t = -96.0, allow_hyphen_values = true)]
    pub oscbank_threshold: f32,

    /// `-A`: master (source + reverb) gain in dB - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub master_gain: ControlFn,

    /// `-S`: source gain in dB - a plain number, or `@path`.
    #[arg(long = "source-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_gain: ControlFn,

    /// `-f`: source frequency shift adder, in Hz - a plain number, or
    /// `@path`.
    #[arg(long = "source-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_freq_shift: ControlFn,

    /// `-p`: source pitch transposition, in semitones - a plain number,
    /// or `@path`.
    #[arg(long = "source-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_pitch: ControlFn,

    /// `-F`: reverb (feedback) gain in dB - a plain number, or `@path`.
    #[arg(long = "feedback-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_gain: ControlFn,

    /// `-H`: reverb frequency shift adder, in Hz - a plain number, or
    /// `@path`.
    #[arg(long = "feedback-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_freq_shift: ControlFn,

    /// `-P`: reverb pitch transposition, in semitones - a plain number,
    /// or `@path`.
    #[arg(long = "feedback-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_pitch: ControlFn,

    /// `-Z`: reverb (feedback delay line) decay time, in seconds - `0`
    /// disables the loop entirely. A plain number, or `@path`.
    #[arg(long = "feedback-decay", value_parser = parse_control_fn, default_value = "0")]
    pub feedback_decay: ControlFn,

    /// `-z`: reverb (input) envelope-follower gate threshold, in dB - a
    /// plain number, or `@path`.
    #[arg(long = "feedback-threshold", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub feedback_threshold: ControlFn,

    /// `-g`: reverb threshold pass mode.
    #[arg(long = "feedback-threshold-mode", value_parser = parse_threshold_mode, default_value = "above", num_args = 1)]
    pub feedback_threshold_mode: bool,

    /// `-l`: reverb (input) envelope attack time, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "attack", value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// `-L`: reverb (input) envelope release time, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "release", value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    /// `-O`: reverb (input) EQ low shelf gain, in dB - a plain number, or
    /// `@path`. Defaults to `200`, not `0` - see `pvc-core::tools::
    /// ringfilter`'s doc comment on the real swapped-default bug this
    /// reproduces.
    #[arg(long = "input-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub input_eq_low_gain: ControlFn,

    /// `-Y`: reverb (input) EQ high shelf gain, in dB - a plain number,
    /// or `@path`.
    #[arg(long = "input-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub input_eq_high_gain: ControlFn,

    /// `-d`: reverb (input) EQ low shelf frequency, in Hz - a plain
    /// number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ringfilter`'s doc comment.
    #[arg(long = "input-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub input_eq_low_freq: ControlFn,

    /// `-n`: reverb (input) EQ high shelf frequency, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "input-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub input_eq_high_freq: ControlFn,

    /// `-T`: reverb (in-loop feedback) EQ decay time, in seconds - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-decay", value_parser = parse_control_fn, default_value = "1")]
    pub loop_eq_decay: ControlFn,

    /// `-E`: reverb (feedback) signal balance gain limiter level, `0` to
    /// `96` dB (`0` disables balancing entirely).
    #[arg(long = "loop-balance-limit", default_value_t = 0.0)]
    pub loop_balance_limit: f32,

    /// `-X`: reverb (in-loop feedback) EQ low shelf gain, in dB - a plain
    /// number, or `@path`. Defaults to `200`, not `0` - see
    /// `pvc-core::tools::ringfilter`'s doc comment on the real
    /// swapped-default bug this reproduces (the same one as `-O`/`-d`,
    /// independently present here too).
    #[arg(long = "loop-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub loop_eq_low_gain: ControlFn,

    /// `-Q`: reverb (in-loop feedback) EQ high shelf gain, in dB - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub loop_eq_high_gain: ControlFn,

    /// `-U`: reverb (in-loop feedback) EQ low shelf frequency, in Hz - a
    /// plain number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ringfilter`'s doc comment.
    #[arg(long = "loop-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub loop_eq_low_freq: ControlFn,

    /// `-m`: reverb (in-loop feedback) EQ high shelf frequency, in Hz - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub loop_eq_high_freq: ControlFn,

    /// `-k`: reverb (output) EQ low shelf gain, in dB - a plain number,
    /// or `@path`. Defaults to `200`, not `0` - see `pvc-core::tools::
    /// ringfilter`'s doc comment on the real swapped-default bug this
    /// reproduces (the same one as `-O`/`-d`, independently present here
    /// too).
    #[arg(long = "output-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub output_eq_low_gain: ControlFn,

    /// `-c`: reverb (output) EQ high shelf gain, in dB - a plain number,
    /// or `@path`.
    #[arg(long = "output-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub output_eq_high_gain: ControlFn,

    /// `-s`: reverb (output) EQ low shelf frequency, in Hz - a plain
    /// number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ringfilter`'s doc comment.
    #[arg(long = "output-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub output_eq_low_freq: ControlFn,

    /// `-G`: reverb (output) EQ high shelf frequency, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "output-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub output_eq_high_freq: ControlFn,

    /// `-y`: path to a `.fr` frequency response file (the same format
    /// `pvc filter` reads) - required.
    #[arg(long = "filter-response")]
    pub filter_response: PathBuf,

    /// `-q`: reshapes the filter response curve. A plain number, or
    /// `@path`.
    #[arg(long = "filter-warpshape", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_warpshape: ControlFn,

    /// `-u`: filter response transposition, in semitones - a plain
    /// number, or `@path`.
    #[arg(long = "filter-transpose", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_transpose: ControlFn,

    /// `-V`: filter response shift, in Hz (applied before
    /// `-filter-transpose`) - a plain number, or `@path`.
    #[arg(long = "filter-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_shift: ControlFn,

    /// `-x`: blend between the filtered signal and the dry source, in dB.
    /// `-96` (the default) is fully filtered, `0` bypasses the filter
    /// entirely. A plain number, or `@path`.
    #[arg(long = "filter-source-gain", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub filter_source_gain: ControlFn,

    /// `-r`: filter decay time, in seconds - only used by
    /// `--filter-placement postfilter`. A plain number, or `@path`.
    #[arg(long = "filter-decay", value_parser = parse_control_fn, default_value = "1")]
    pub filter_decay: ControlFn,

    /// `-o`: where the filter is applied.
    #[arg(long = "filter-placement", value_parser = parse_filter_placement, default_value = "prefilter", num_args = 1)]
    pub filter_placement: bool,

    /// `-B`: whether the filter's own transpose/shift compensates for
    /// the reverb's own `-feedback-pitch`/`-feedback-freq-shift`.
    #[arg(long = "filter-pitch-mode", value_parser = parse_filter_pitch_mode, default_value = "source-only", num_args = 1)]
    pub filter_pitch_mode: bool,

    pub input: PathBuf,
    pub output: PathBuf,
}

#[derive(clap::Args, Debug)]
pub struct RingtvfilterArgs {
    /// FFT size (must be a power of two) - independent of the filter
    /// response file's own FFT size (see `pvc-core::tools::tvfilter`'s
    /// doc comment on `N_ratio`, reused here).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// Analysis/resynthesis window length. `0` means auto (`2 * fft`).
    #[arg(long, default_value_t = 0)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// Time expansion/contraction factor (`1.0` = unchanged duration).
    #[arg(long, default_value_t = 1.0)]
    pub time_factor: f32,

    /// `-b`: begin time in seconds.
    #[arg(long, default_value_t = 0.0)]
    pub begin: f32,

    /// `-e`: end time in seconds (`0` = end of file).
    #[arg(long, default_value_t = 0.0)]
    pub end: f32,

    /// `-t`: oscillator-bank resynthesis threshold, in dB.
    #[arg(long, default_value_t = -96.0, allow_hyphen_values = true)]
    pub oscbank_threshold: f32,

    /// `-S`: source gain in dB - a plain number, or `@path`.
    #[arg(long = "source-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_gain: ControlFn,

    /// `-f`: source frequency shift adder, in Hz - a plain number, or
    /// `@path`.
    #[arg(long = "source-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_freq_shift: ControlFn,

    /// `-p`: source pitch transposition, in semitones - a plain number,
    /// or `@path`.
    #[arg(long = "source-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_pitch: ControlFn,

    /// `-F`: reverb (feedback) gain in dB - a plain number, or `@path`.
    #[arg(long = "feedback-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_gain: ControlFn,

    /// `-H`: reverb frequency shift adder, in Hz - a plain number, or
    /// `@path`.
    #[arg(long = "feedback-freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_freq_shift: ControlFn,

    /// `-P`: reverb pitch transposition, in semitones - a plain number,
    /// or `@path`.
    #[arg(long = "feedback-pitch", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub feedback_pitch: ControlFn,

    /// `-Z`: reverb (feedback delay line) decay time, in seconds - `0`
    /// disables the loop entirely. A plain number, or `@path`.
    #[arg(long = "feedback-decay", value_parser = parse_control_fn, default_value = "0")]
    pub feedback_decay: ControlFn,

    /// `-z`: reverb (input) envelope-follower gate threshold, in dB - a
    /// plain number, or `@path`. Unlike `pvc ring`/`pvc ringfilter`,
    /// there is no threshold pass-mode flag here - see `pvc-core::tools::
    /// ringtvfilter`'s doc comment.
    #[arg(long = "feedback-threshold", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub feedback_threshold: ControlFn,

    /// `-l`: reverb (input) envelope attack time, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "attack", value_parser = parse_control_fn, default_value = "0")]
    pub attack: ControlFn,

    /// `-L`: reverb (input) envelope release time, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "release", value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    /// `-O`: reverb (input) EQ low shelf gain, in dB - a plain number, or
    /// `@path`. Defaults to `200`, not `0` - see `pvc-core::tools::
    /// ring`'s doc comment on the real swapped-default bug this
    /// reproduces (independently present here too).
    #[arg(long = "input-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub input_eq_low_gain: ControlFn,

    /// `-Y`: reverb (input) EQ high shelf gain, in dB - a plain number,
    /// or `@path`.
    #[arg(long = "input-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub input_eq_high_gain: ControlFn,

    /// `-d`: reverb (input) EQ low shelf frequency, in Hz - a plain
    /// number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ring`'s doc comment.
    #[arg(long = "input-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub input_eq_low_freq: ControlFn,

    /// `-n`: reverb (input) EQ high shelf frequency, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "input-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub input_eq_high_freq: ControlFn,

    /// `-T`: reverb (in-loop feedback) EQ decay time, in seconds - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-decay", value_parser = parse_control_fn, default_value = "1")]
    pub loop_eq_decay: ControlFn,

    /// `-E`: reverb (feedback) signal balance gain limiter level, `0` to
    /// `96` dB (`0` disables balancing entirely).
    #[arg(long = "loop-balance-limit", default_value_t = 0.0)]
    pub loop_balance_limit: f32,

    /// `-X`: reverb (in-loop feedback) EQ low shelf gain, in dB - a plain
    /// number, or `@path`. Defaults to `200`, not `0` - see
    /// `pvc-core::tools::ring`'s doc comment on the real swapped-default
    /// bug this reproduces (independently present here too).
    #[arg(long = "loop-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub loop_eq_low_gain: ControlFn,

    /// `-Q`: reverb (in-loop feedback) EQ high shelf gain, in dB - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub loop_eq_high_gain: ControlFn,

    /// `-U`: reverb (in-loop feedback) EQ low shelf frequency, in Hz - a
    /// plain number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ring`'s doc comment.
    #[arg(long = "loop-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub loop_eq_low_freq: ControlFn,

    /// `-m`: reverb (in-loop feedback) EQ high shelf frequency, in Hz - a
    /// plain number, or `@path`.
    #[arg(long = "loop-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub loop_eq_high_freq: ControlFn,

    /// `-k`: reverb (output) EQ low shelf gain, in dB - a plain number,
    /// or `@path`. Defaults to `200`, not `0` - see `pvc-core::tools::
    /// ring`'s doc comment on the real swapped-default bug this
    /// reproduces (independently present here too).
    #[arg(long = "output-eq-low-gain", value_parser = parse_control_fn, default_value = "200", allow_hyphen_values = true)]
    pub output_eq_low_gain: ControlFn,

    /// `-c`: reverb (output) EQ high shelf gain, in dB - a plain number,
    /// or `@path`.
    #[arg(long = "output-eq-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub output_eq_high_gain: ControlFn,

    /// `-s`: reverb (output) EQ low shelf frequency, in Hz - a plain
    /// number, or `@path`. Defaults to `0`, not `200` - see
    /// `pvc-core::tools::ring`'s doc comment.
    #[arg(long = "output-eq-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub output_eq_low_freq: ControlFn,

    /// `-G`: reverb (output) EQ high shelf frequency, in Hz - a plain
    /// number, or `@path`.
    #[arg(long = "output-eq-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub output_eq_high_freq: ControlFn,

    /// `-y`: path to the time-varying filter response file - a `.pva`
    /// analysis file, either the legacy layout or `pvc analyze`'s own
    /// `PVA1` format. Required.
    #[arg(long = "filter-response")]
    pub filter_response: PathBuf,

    /// `-~`: which filter-file channel to use (`0` = pair by channel
    /// index with the input sound file).
    #[arg(long = "analysis-channel", default_value_t = 0)]
    pub analysis_channel: usize,

    /// `-h`: filter time point origin, in seconds - a plain number, or
    /// `@path`.
    #[arg(long = "time-origin", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub time_origin: ControlFn,

    /// `-R`: filter rate multiplier - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "1", allow_hyphen_values = true)]
    pub rate: ControlFn,

    /// `-g`: filter time window low boundary, in seconds - a plain
    /// number, or `@path`.
    #[arg(long = "window-low", value_parser = parse_control_fn, default_value = "0")]
    pub window_low: ControlFn,

    /// `-J`: filter time window high boundary, in seconds (negative = end
    /// of the filter file) - a plain number, or `@path`.
    #[arg(long = "window-high", value_parser = parse_control_fn, default_value = "-1", allow_hyphen_values = true)]
    pub window_high: ControlFn,

    /// `/Q`: sampler-loop boundary behavior (only used outside autostop
    /// mode).
    #[arg(long = "loop-mode", value_parser = parse_loop_mode, default_value = "wrap")]
    pub loop_mode: pvc_core::timenav::LoopMode,

    /// `::`: trigger the time window only once it's first entered.
    #[arg(long = "onset-release")]
    pub onset_release: bool,

    /// `-@`: stop synthesis once the filter's time position exits its
    /// window, instead of looping.
    #[arg(long)]
    pub autostop: bool,

    /// `-A`: keep the filter's own amplitude roughly continuous across a
    /// sampler loop's seam.
    #[arg(long = "loop-normalization")]
    pub loop_normalization: bool,

    /// `-a`: peak loop-seam smoothing time, in seconds - a plain number,
    /// or `@path`.
    #[arg(long = "loop-smooth", value_parser = parse_control_fn, default_value = "0.2")]
    pub loop_smooth: ControlFn,

    /// `-q`: reshapes the filter response curve. A plain number, or
    /// `@path`.
    #[arg(long = "filter-warpshape", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_warpshape: ControlFn,

    /// `-W`: filter-spectrum compression threshold, in dB (`<= 0`). A
    /// plain constant, not a `(func)` - see `pvc-core::tools::
    /// ringtvfilter`'s doc comment.
    #[arg(
        long = "comp-threshold",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub comp_threshold: f32,

    /// `-v`: filter-spectrum decibels of compression (`<= 0`). A plain
    /// constant, not a `(func)`.
    #[arg(long = "comp-db", default_value_t = 0.0, allow_hyphen_values = true)]
    pub comp_db: f32,

    /// `-u`: filter response transposition, in semitones - a plain
    /// number, or `@path`.
    #[arg(long = "filter-transpose", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_transpose: ControlFn,

    /// `-V`: filter response shift, in Hz - a plain number, or `@path`.
    #[arg(long = "filter-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub filter_shift: ControlFn,

    /// `-x`: blend between the filtered signal and the dry source, in dB.
    /// `-96` (the default) is fully filtered, `0` bypasses the filter
    /// entirely. A plain number, or `@path`.
    #[arg(long = "filter-source-gain", value_parser = parse_control_fn, default_value = "-96", allow_hyphen_values = true)]
    pub filter_source_gain: ControlFn,

    /// `-r`: filter decay time, in seconds - only used by
    /// `--filter-placement postfilter`. A plain number, or `@path`.
    #[arg(long = "filter-decay", value_parser = parse_control_fn, default_value = "1")]
    pub filter_decay: ControlFn,

    /// `-o`: where the filter is applied.
    #[arg(long = "filter-placement", value_parser = parse_filter_placement, default_value = "prefilter", num_args = 1)]
    pub filter_placement: bool,

    /// `-B`: whether the filter's own transpose/shift compensates for
    /// the reverb's own `-feedback-pitch`/`-feedback-freq-shift`.
    #[arg(long = "filter-pitch-mode", value_parser = parse_filter_pitch_mode, default_value = "source-only", num_args = 1)]
    pub filter_pitch_mode: bool,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_filter_placement(s: &str) -> Result<bool, String> {
    match s {
        "prefilter" => Ok(false),
        "postfilter" => Ok(true),
        _ => Err(format!(
            "expected \"prefilter\" or \"postfilter\", got {s:?}"
        )),
    }
}

fn parse_threshold_mode(s: &str) -> Result<bool, String> {
    match s {
        "above" => Ok(true),
        "below" => Ok(false),
        _ => Err(format!("expected \"above\", or \"below\", got {s:?}")),
    }
}

fn parse_spectrum_type(s: &str) -> Result<pvc_core::tools::freqresponse::Method, String> {
    use pvc_core::tools::freqresponse::Method;
    match s {
        "average" => Ok(Method::Average),
        "peak" => Ok(Method::Peak),
        _ => Err(format!("expected \"average\" or \"peak\", got {s:?}")),
    }
}

fn parse_response_mode(s: &str) -> Result<bool, String> {
    match s {
        "bandpass" => Ok(false),
        "reject" => Ok(true),
        _ => Err(format!("expected \"bandpass\" or \"reject\", got {s:?}")),
    }
}

fn parse_accumulation(
    s: &str,
) -> Result<pvc_core::tools::chordresponsemaker::Accumulation, String> {
    use pvc_core::tools::chordresponsemaker::Accumulation;
    match s {
        "peak" => Ok(Accumulation::Peak),
        "sum" => Ok(Accumulation::Sum),
        _ => Err(format!("expected \"peak\" or \"sum\", got {s:?}")),
    }
}

fn parse_band_window(s: &str) -> Result<pvc_core::tools::chordresponsemaker::BandWindow, String> {
    use pvc_core::tools::chordresponsemaker::BandWindow;
    match s {
        "triangle" => Ok(BandWindow::Triangle),
        "rectangle" => Ok(BandWindow::Rectangle),
        _ => Err(format!("expected \"triangle\" or \"rectangle\", got {s:?}")),
    }
}

fn parse_overlap_method(
    s: &str,
) -> Result<pvc_core::tools::groupdelaymaker::OverlapMethod, String> {
    use pvc_core::tools::groupdelaymaker::OverlapMethod;
    match s {
        "shortest-delay" => Ok(OverlapMethod::ShortestDelay),
        "longest-delay" => Ok(OverlapMethod::LongestDelay),
        "average" => Ok(OverlapMethod::Average),
        "loudest" => Ok(OverlapMethod::Loudest),
        "softest" => Ok(OverlapMethod::Softest),
        "loudest-if-shortest" => Ok(OverlapMethod::LoudestIfShortest),
        "loudest-if-longest" => Ok(OverlapMethod::LoudestIfLongest),
        _ => Err(format!(
            "expected one of \"shortest-delay\", \"longest-delay\", \"average\", \"loudest\", \
             \"softest\", \"loudest-if-shortest\", \"loudest-if-longest\", got {s:?}"
        )),
    }
}

#[derive(clap::Args, Debug)]
pub struct ImpulseresponseArgs {
    /// `-b`: analysis window start, in seconds.
    #[arg(long = "begin", default_value_t = 0.0)]
    pub begin: f32,

    /// `-e`: analysis window end, in seconds (`0` = end of file).
    #[arg(long = "end", default_value_t = 0.0)]
    pub end: f32,

    /// `-N`: how the per-channel spectra are peak-normalized.
    #[arg(long, value_parser = parse_normalization, default_value = "together")]
    pub normalization: pvc_core::tools::impulseresponse::Normalization,

    /// `-d`: normalization target level, in dB.
    #[arg(
        long = "normalization-db",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub normalization_db: f32,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_normalization(s: &str) -> Result<pvc_core::tools::impulseresponse::Normalization, String> {
    use pvc_core::tools::impulseresponse::Normalization;
    match s {
        "off" => Ok(Normalization::Off),
        "independent" => Ok(Normalization::Independent),
        "together" => Ok(Normalization::Together),
        _ => Err(format!(
            "expected \"off\", \"independent\", or \"together\", got {s:?}"
        )),
    }
}

#[derive(clap::Args, Debug)]
pub struct IrconvolverArgs {
    /// `-E`: path to the `.ir` impulse-response file (from `pvc
    /// impulseresponse`).
    #[arg(long = "ir")]
    pub ir: PathBuf,

    /// `-b`: analysis window start, in seconds.
    #[arg(long = "begin", default_value_t = 0.0)]
    pub begin: f32,

    /// `-e`: analysis window end, in seconds (`0` = end of file).
    #[arg(long = "end", default_value_t = 0.0)]
    pub end: f32,

    /// `-d`: extend the processed window by one impulse-length of
    /// trailing silence, so the reverb tail isn't cut off.
    #[arg(long = "ring-tail")]
    pub ring_tail: bool,

    /// `-J`: which `.ir` channel to use (`0` = auto round-robin).
    #[arg(long = "impulse-channel", default_value_t = 0)]
    pub impulse_channel: usize,

    /// `-a` (the C's own usage text mislabels this flag - see
    /// `pvc-core::tools::irconvolver`'s doc comment).
    #[arg(long, value_parser = parse_irconvolver_mode, default_value = "convolve")]
    pub mode: pvc_core::tools::irconvolver::Mode,

    /// `-s`: impulse response bandpass low rolloff point, Hz.
    #[arg(long = "ir-low-freq", default_value_t = 0.0)]
    pub ir_low_freq: f32,

    /// `-t`: impulse response bandpass high rolloff point, Hz (`0` =
    /// Nyquist).
    #[arg(long = "ir-high-freq", default_value_t = 0.0)]
    pub ir_high_freq: f32,

    /// `-g`.
    #[arg(
        long = "ir-low-rolloff",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub ir_low_rolloff: f32,

    /// `-G`.
    #[arg(
        long = "ir-high-rolloff",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub ir_high_rolloff: f32,

    /// `-D`: input sound bandpass low rolloff point, Hz.
    #[arg(long = "source-low-freq", default_value_t = 0.0)]
    pub source_low_freq: f32,

    /// `-f`: input sound bandpass high rolloff point, Hz (`0` = Nyquist).
    #[arg(long = "source-high-freq", default_value_t = 0.0)]
    pub source_high_freq: f32,

    /// `-h`.
    #[arg(
        long = "source-low-rolloff",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub source_low_rolloff: f32,

    /// `-H`.
    #[arg(
        long = "source-high-rolloff",
        default_value_t = 0.0,
        allow_hyphen_values = true
    )]
    pub source_high_rolloff: f32,

    /// `-A`: dry-signal gain mixed back in after convolution, in dB - a
    /// plain number, or `@path`.
    #[arg(long = "source-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_gain: ControlFn,

    /// `-q`: gain applied before convolution, in dB - a plain number, or
    /// `@path`.
    #[arg(long = "input-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub input_gain: ControlFn,

    /// `-r`: gain applied to the convolution output, in dB - a plain
    /// number, or `@path`.
    #[arg(long = "output-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub output_gain: ControlFn,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_irconvolver_mode(s: &str) -> Result<pvc_core::tools::irconvolver::Mode, String> {
    use pvc_core::tools::irconvolver::Mode;
    match s {
        "convolve" => Ok(Mode::Convolution),
        "deconvolve" => Ok(Mode::Deconvolution),
        _ => Err(format!(
            "expected \"convolve\" or \"deconvolve\", got {s:?}"
        )),
    }
}

#[derive(clap::Args, Debug)]
pub struct IrconvolvesequencerArgs {
    /// `-I`: directory containing an `impulseFileNames` list file - its
    /// first whitespace-separated token is the impulse count, followed by
    /// that many impulse-response sound file paths, one sequence point
    /// each, in order.
    #[arg(long = "impulse-list-dir")]
    pub impulse_list_dir: PathBuf,

    /// `-b`: sequence window start, in seconds.
    #[arg(long = "begin", default_value_t = 0.0)]
    pub begin: f32,

    /// `-e`: sequence window end, in seconds (`0` = end of file).
    #[arg(long = "end", default_value_t = 0.0)]
    pub end: f32,

    /// `-d`: extend every segment's processed window by one impulse-length
    /// of trailing silence.
    #[arg(long = "ring-tail")]
    pub ring_tail: bool,

    /// `-J`: which impulse channel to use (`0` = auto round-robin).
    #[arg(long = "impulse-channel", default_value_t = 0)]
    pub impulse_channel: usize,

    /// `-v`: how the final mixed output is peak-normalized.
    #[arg(long, value_parser = parse_mix_normalization, default_value = "off")]
    pub normalization: pvc_core::tools::irconvolvesequencer::MixNormalization,

    /// `-s`: impulse response bandpass low rolloff point, Hz - a plain
    /// number, or `@path`, evaluated once per impulse at its sequence
    /// position.
    #[arg(long = "ir-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub ir_low_freq: ControlFn,

    /// `-t`: impulse response bandpass high rolloff point, Hz (`0` =
    /// Nyquist) - a plain number, or `@path`.
    #[arg(long = "ir-high-freq", value_parser = parse_control_fn, default_value = "0")]
    pub ir_high_freq: ControlFn,

    /// `-g` - a plain number, or `@path`.
    #[arg(long = "ir-low-rolloff", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub ir_low_rolloff: ControlFn,

    /// `-G` - a plain number, or `@path`.
    #[arg(long = "ir-high-rolloff", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub ir_high_rolloff: ControlFn,

    /// `-D`: input sound bandpass low rolloff point, Hz - a plain number,
    /// or `@path`.
    #[arg(long = "source-low-freq", value_parser = parse_control_fn, default_value = "0")]
    pub source_low_freq: ControlFn,

    /// `-f`: input sound bandpass high rolloff point, Hz (`0` = Nyquist) -
    /// a plain number, or `@path`.
    #[arg(long = "source-high-freq", value_parser = parse_control_fn, default_value = "0")]
    pub source_high_freq: ControlFn,

    /// `-h` - a plain number, or `@path`.
    #[arg(long = "source-low-rolloff", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_low_rolloff: ControlFn,

    /// `-H` - a plain number, or `@path`.
    #[arg(long = "source-high-rolloff", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_high_rolloff: ControlFn,

    /// `-A`: dry-signal gain mixed back in after convolution, in dB,
    /// evaluated once per impulse at its sequence position - a plain
    /// number, or `@path`.
    #[arg(long = "source-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub source_gain: ControlFn,

    /// `-r`: gain applied to each segment's convolution output, in dB -
    /// a plain number, or `@path`.
    #[arg(long = "output-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub output_gain: ControlFn,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_mix_normalization(
    s: &str,
) -> Result<pvc_core::tools::irconvolvesequencer::MixNormalization, String> {
    use pvc_core::tools::irconvolvesequencer::MixNormalization;
    match s {
        "off" => Ok(MixNormalization::Off),
        "independent" => Ok(MixNormalization::Independent),
        "together" => Ok(MixNormalization::Together),
        "if-clipping" => Ok(MixNormalization::IfClipping),
        _ => Err(format!(
            "expected \"off\", \"independent\", \"together\", or \"if-clipping\", got {s:?}"
        )),
    }
}

/// `pvc convert-units`'s `--from`/`--to` unit tag. Amplitude/dB
/// (`amptodB`/`dBtoamp`) and Hz/octave.pitchclass (`Hztopitch`/
/// `pitchtoHz`) are each other's only supported pair - `commands::
/// convert_units::run` rejects any other combination.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConvertUnit {
    Amp,
    Db,
    Hz,
    Oppc,
}

fn parse_convert_unit(s: &str) -> Result<ConvertUnit, String> {
    match s {
        "amp" => Ok(ConvertUnit::Amp),
        "db" => Ok(ConvertUnit::Db),
        "hz" => Ok(ConvertUnit::Hz),
        "oppc" => Ok(ConvertUnit::Oppc),
        _ => Err(format!(
            "expected \"amp\", \"db\", \"hz\", or \"oppc\", got {s:?}"
        )),
    }
}

/// `pvc spectralextractor`'s flag surface. Long names follow the same
/// convention as [`TvfilterArgs`]/[`crate::cli::IrconvolvesequencerArgs`].
#[derive(clap::Args, Debug)]
pub struct SpectralExtractorArgs {
    /// `-N`: FFT size (must be a power of two).
    #[arg(long, default_value_t = 1024)]
    pub fft: usize,

    /// `-M`: analysis/resynthesis window length. `0` means auto (`2 *
    /// fft`, or larger still if needed to fit the resynthesis hop).
    #[arg(long, default_value_t = 0)]
    pub window_size: usize,

    #[arg(long, value_parser = parse_window, default_value = "hamming")]
    pub window: Window,

    /// `-D`: analysis frames per second (sets the hop size).
    #[arg(long, default_value_t = 200.0)]
    pub frames_per_sec: f32,

    /// `-I`: time expansion/contraction factor (`1.0` = unchanged
    /// duration).
    #[arg(long, default_value_t = 1.0)]
    pub time_factor: f32,

    /// `-P`: pitch transposition in semitones - a plain number, or
    /// `@path`. Real, narrow effect only (see `pvc-core::tools::
    /// spectralextractor`'s doc comment): selects oscillator-bank
    /// resynthesis and shifts the shelf-EQ banding frequency, but never
    /// the output's actual pitch.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub pitch: ControlFn,

    /// `-a`: frequency shift adder in Hz - a plain number, or `@path`.
    /// Same narrow real effect as `--pitch` - see that flag's doc comment.
    #[arg(long = "freq-shift", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub freq_shift: ControlFn,

    /// `-A`: gain in dB - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub gain: ControlFn,

    /// `-b`: begin time in seconds.
    #[arg(long = "begin", default_value_t = 0.0)]
    pub begin: f32,

    /// `-e`: end time in seconds (`0` = end of file).
    #[arg(long = "end", default_value_t = 0.0)]
    pub end: f32,

    /// `-q`: which part of the spectrum to keep.
    #[arg(long = "spectral-type", value_parser = parse_spectral_type, default_value = "periodic")]
    pub spectral_type: pvc_core::tools::spectralextractor::SpectralType,

    /// `-Q`: max (periodic mode) / min (noise mode) frequency change
    /// allowed every 5 milliseconds, in Hz - a plain number, or `@path`.
    #[arg(long = "freq-change-threshold", value_parser = parse_control_fn, default_value = "0")]
    pub freq_change_threshold: ControlFn,

    /// `-g`: response time of the frequency-change threshold accumulator,
    /// in seconds - a plain number, or `@path`.
    #[arg(long = "freq-change-response", value_parser = parse_control_fn, default_value = "0")]
    pub freq_change_response: ControlFn,

    /// `-L`: amplitude-gate release time in seconds - a plain number, or
    /// `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0")]
    pub release: ControlFn,

    /// `-c`: complement amplitude spectrum proportion (0-1) - a plain
    /// number, or `@path`.
    #[arg(long = "complement", value_parser = parse_control_fn, default_value = "0")]
    pub complement: ControlFn,

    /// `-E`: frame normalization decibel limit (`0` disables
    /// normalization) - a plain number, or `@path`.
    #[arg(long = "frame-norm-limit", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub frame_norm_limit: ControlFn,

    /// `-W`: spectrum magnitude warp index - a plain number, or `@path`.
    #[arg(long, value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub warp: ControlFn,

    /// `-H`: low shelf EQ gain in dB - a plain number, or `@path`.
    #[arg(long = "shelf-low-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub shelf_low_gain: ControlFn,

    /// `-X`: high shelf EQ gain in dB - a plain number, or `@path`.
    #[arg(long = "shelf-high-gain", value_parser = parse_control_fn, default_value = "0", allow_hyphen_values = true)]
    pub shelf_high_gain: ControlFn,

    /// `-m`: low shelf EQ frequency in Hz - a plain number, or `@path`.
    #[arg(long = "shelf-low-freq", value_parser = parse_control_fn, default_value = "200")]
    pub shelf_low_freq: ControlFn,

    /// `-R`: high shelf EQ frequency in Hz - a plain number, or `@path`.
    #[arg(long = "shelf-high-freq", value_parser = parse_control_fn, default_value = "2000")]
    pub shelf_high_freq: ControlFn,

    /// `-t`: oscillator-bank resynthesis threshold in dB (only consumed
    /// when `--pitch`/`--freq-shift` select that resynthesis path).
    #[arg(long, default_value_t = -96.0, allow_hyphen_values = true)]
    pub threshold: f32,

    pub input: PathBuf,
    pub output: PathBuf,
}

fn parse_spectral_type(
    s: &str,
) -> Result<pvc_core::tools::spectralextractor::SpectralType, String> {
    use pvc_core::tools::spectralextractor::SpectralType;
    match s {
        "periodic" => Ok(SpectralType::Periodic),
        "noise" => Ok(SpectralType::Noise),
        _ => Err(format!("expected \"periodic\" or \"noise\", got {s:?}")),
    }
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
