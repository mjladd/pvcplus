//! The core `pvanalysis` phase-vocoder analysis pipeline
//! (`legacy/pvc_src/pvanalysis.c`) - analysis only, no resynthesis. Writes
//! frames rather than audio; pairs with `pvc_io::pva::write_pva` (the
//! plan's §8.1.3 new-format-only decision - this never produces a legacy
//! `.pva` file).
//!
//! Two things that make this simpler than [`crate::tools::pv`]'s
//! `process_channel`:
//!
//! - None of `pvanalysis.c`'s per-frame parameters (`-H`/`-X`/`-m`/`-R`
//!   shelf EQ, `-W` warpshape, `-A` gain) are control-function strings -
//!   the flag parser reads every one with a plain `atof`, never
//!   `crackfloat`/`fval()`. So unlike `plainpv`, there is nothing here
//!   that varies per frame; [`AnalyzeParams`]' fields are plain `f32`s,
//!   not `ControlFn`s, and `timenow(dur)`'s per-frame `t` (which *would*
//!   drive such a control function) is dead code in this tool - worth
//!   noting since it's easy to assume otherwise by analogy with
//!   `plainpv`.
//! - `pvanalysis.c`'s own per-channel *file*-writing loop (re-seeking
//!   into the shared output file, blank-filling other channels' slots on
//!   the first channel's pass) exists only to build the legacy
//!   interleaved-by-frame file layout (see `pvc_io::pva`'s module doc
//!   comment) - a concern of the legacy *file format*, not the DSP
//!   pipeline. Since `pvc analyze` only ever writes the new format,
//!   [`process_channel`] just returns one channel's frames; the caller
//!   assembles a multi-channel [`crate::pvoc::Frame`] list per channel
//!   into a `PvaData` however that format wants it.
//!
//! `Nw`'s legacy hardcoded default is a literal `4096`, *not* `2 *
//! fft_size` - the same "auto-scale only fires on an explicit `-M0`
//! override" gotcha as `plainpv`'s window size (see
//! `tools::pv::PvParams::window_size`'s doc comment). Callers wanting the
//! real tool's default behavior should pass `4096` directly, not `0`.
//!
//! No `phaselock` call - `pvanalysis.c`'s loop goes straight from
//! `convert` to `eq`/`spectmagwarp`/gain, confirmed by reading the loop
//! body; phase-locking is a resynthesis-quality concern `plainpv` needs
//! and `pvanalysis` (which never resynthesizes) doesn't.

use crate::eq::eq;
use crate::pvoc::{Analyzer, Frame};
use crate::units::DbToAmp;
use crate::warp::spectmagwarp;
use crate::window::{make_windows, Window};

/// `pvanalysis`'s fixed (non-control-function) parameters - see this
/// module's doc comment for why these are plain `f32`s rather than
/// [`crate::ControlFn`]s.
#[derive(Debug, Clone)]
pub struct AnalyzeParams {
    pub fft_size: usize,
    /// `0` means "auto": `2 * fft_size`. The C's own hardcoded default is
    /// a literal `4096` - pass that directly for real-tool-default
    /// behavior (see this module's doc comment).
    pub window_size: usize,
    pub window: Window,
    pub frames_per_sec: f32,
    pub gain_db: f32,
    pub shelf_low_db: f32,
    pub shelf_high_db: f32,
    pub shelf_low_freq: f32,
    pub shelf_high_freq: f32,
    pub warpshape: f32,
}

/// Processes one channel of audio into a list of analysis frames,
/// matching `pvanalysis.c`'s per-channel frame loop exactly (fold/rfft/
/// convert via [`Analyzer`], shelf [`eq`], `spectmagwarp`, then gain -
/// see this module's doc comment for what's deliberately *not* here:
/// no per-frame control functions, no `phaselock`, no resynthesis).
pub fn process_channel(input: &[f32], sample_rate: u32, params: &AnalyzeParams) -> Vec<Frame> {
    let r = sample_rate as f32;
    let n = params.fft_size;
    let d = (r / params.frames_per_sec) as usize;

    let mut nw = params.window_size;
    if nw == 0 {
        nw = 2 * n;
    }

    let fundamental = r / n as f32;
    let db_to_amp = DbToAmp::new();
    let ampgain = db_to_amp.convert(params.gain_db);

    // `I` (synthesis interpolation stride) only ever affects the
    // synthesis window `make_windows` also computes - `pvanalysis.c`
    // hardcodes an unused `I = 1024` and never reads the synthesis
    // window it produces (only `Wanal` feeds `fold()`). Passing `0` here
    // is the documented "no separate interpolation stride" case and
    // changes nothing about the analysis window we actually use.
    let window_pair = make_windows(params.window, nw, n, 0);
    let mut analyzer = Analyzer::new(n, window_pair.analysis, d, sample_rate);

    // Same `shiftin` end-of-input state machine as `tools::pv` - see that
    // module's doc comment for why this exact replication (not a rough
    // hop-count estimate) is what makes frame counts match the real tool.
    let mut valid: i64 = nw as i64;
    let mut pos = 0usize;

    let mut frames = Vec::new();

    loop {
        let mut hop = vec![0.0f32; d];
        if valid == nw as i64 {
            let available = d.min(input.len().saturating_sub(pos));
            hop[..available].copy_from_slice(&input[pos..pos + available]);
            pos += available;
            if available < d {
                valid = nw as i64 - d as i64 + available as i64;
            }
        }
        if valid < nw as i64 {
            valid -= d as i64;
        }
        let eof_after_this_hop = valid <= 0;

        let frame = analyzer.push(&hop).expect("hop is exactly d samples");
        let mut flat = frame.to_pva_floats();

        eq(
            &mut flat,
            params.shelf_low_db,
            params.shelf_high_db,
            params.shelf_low_freq,
            params.shelf_high_freq,
            fundamental,
            1.0,
            0.0,
            false,
            &db_to_amp,
        );
        spectmagwarp(&mut flat, params.warpshape, false);

        // `for (i = 1; i < N; i += 2) channel[i-1] *= ampgain` - same
        // "excludes the Nyquist bin" `i < N` bound as `plainpv`'s gain
        // stage (`flat` is `N+2` long; index `N` is the Nyquist bin's
        // amplitude slot, one past this loop's last touched index `N-2`).
        for k in (1..n).step_by(2) {
            flat[k - 1] *= ampgain;
        }

        frames.push(Frame::from_pva_floats(&flat));

        if eof_after_this_hop {
            break;
        }
    }

    frames
}

#[cfg(test)]
mod tests {
    use super::*;

    fn default_params(fft_size: usize) -> AnalyzeParams {
        AnalyzeParams {
            fft_size,
            window_size: 0,
            window: Window::Hamming,
            frames_per_sec: 200.0,
            gain_db: 0.0,
            shelf_low_db: 0.0,
            shelf_high_db: 0.0,
            shelf_low_freq: 200.0,
            shelf_high_freq: 2000.0,
            warpshape: 0.0,
        }
    }

    #[test]
    fn silence_in_zero_amplitude_frames_out() {
        let params = default_params(1024);
        let input = vec![0.0f32; 44100 / 4];
        let frames = process_channel(&input, 44100, &params);
        assert!(!frames.is_empty());
        for frame in &frames {
            for &(mag, _freq) in &frame.bins {
                assert!(mag.abs() < 1e-6);
            }
        }
    }

    #[test]
    fn sine_input_produces_a_dominant_bin() {
        let params = default_params(1024);
        let sample_rate = 44100u32;
        let input: Vec<f32> = (0..sample_rate)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();
        let frames = process_channel(&input, sample_rate, &params);

        assert!(!frames.is_empty());
        let mid_frame = &frames[frames.len() / 2];
        let (peak_bin, &(peak_mag, peak_freq)) = mid_frame
            .bins
            .iter()
            .enumerate()
            .max_by(|a, b| a.1 .0.partial_cmp(&b.1 .0).unwrap())
            .unwrap();
        assert!(peak_mag > 1e-4, "dominant bin {peak_bin} too quiet");
        assert!(
            (peak_freq - 440.0).abs() < 20.0,
            "dominant bin frequency {peak_freq} not near 440Hz"
        );
    }
}
