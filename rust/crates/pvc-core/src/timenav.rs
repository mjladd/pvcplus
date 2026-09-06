//! Time-position navigation through a `.pva` analysis file, ported from
//! `legacy/pvc_lib/findFilterTimeAndConstrainByWindow.c` and
//! `makeLoopSmoothTime.c` - the core of `twarp`'s "time warp": at each
//! output frame, advance a virtual time position into the analysis data
//! (at a possibly time-varying rate, from a possibly time-varying
//! origin), then constrain it to a window via wrap/fold/clip looping or
//! autostop.
//!
//! `ringTime == 0.0` is one of the conditions in the C's final autostop
//! check; `twarp.c` never touches the global `ringTime` (only tools with
//! delay/convolution tails - `filter`, `harmonizer`, `irconvolver`, ... -
//! set it), so it's always `0.0` for `twarp` and that condition is always
//! true here - omitted rather than modeled as a parameter nobody can ever
//! set to anything else.

use crate::warp::curve;

/// `wrap_0_fold_1_clip_2`: how the time position behaves at a window
/// boundary once autostop is off (i.e. "sampler loop" mode).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LoopMode {
    Wrap,
    Fold,
    Clip,
}

#[derive(Debug, Clone, Copy)]
pub struct TimeNavConfig {
    /// `Onset_and_Release_Segment_Mode__off_0__on_1`.
    pub onset_release: bool,
    /// `Mode__sampler_loop_0__autostop_1`: `true` = autostop (stop once
    /// time exits the window), `false` = sampler loop (wrap/fold/clip at
    /// the window edges forever).
    pub autostop: bool,
    pub loop_mode: LoopMode,
}

/// One frame's navigation result.
#[derive(Debug, Clone, Copy)]
pub struct TimeNavStep {
    /// The new analysis-file time position, in seconds.
    pub filttnow: f32,
    /// The pre-advance time position, *except* immediately after a
    /// fold/clip boundary event, where the C recomputes it to preserve
    /// the frame-to-frame delta across the teleport (`*oldfilttnow =
    /// *filttnow - NewMinusOldDiff`) - not reproduced after a *wrap*,
    /// which the C leaves as a genuine discontinuity. Feed this straight
    /// into the caller's own time-point smoothing, matching `twarp.c`'s
    /// `analysisDatatnow = tsmoothc*oldanalysisDatatnow +
    /// minustsmoothc*analysisDatatnow`.
    pub oldfilttnow: f32,
    pub autostop: bool,
}

/// Ports `findFilterTimeAndConstrainByWindow` + the `frame_count == 0`
/// reset block that precedes every call to it in `twarp.c`. One instance
/// per channel (`twarp.c` resets its equivalent `static` locals via that
/// `frame_count == 0` guard at the start of each channel's frame loop -
/// modeled here as fresh per-channel construction instead, since the
/// guard makes the two equivalent).
///
/// Deliberately not reproduced: the print-only `previousfilttnow`/
/// `filttdiff` bookkeeping (decides which of two stderr "WRAPPED AT"
/// messages to print - confirmed to have no effect on any numeric
/// output, so it's not state worth carrying here).
pub struct TimeNavigator {
    cfg: TimeNavConfig,
    loop_time_direction_inverter: f32,
    end_segment_flag: bool,
    use_time_window_flag: bool,
    save_filt_time_left_now_flag: bool,
    start_filt_time_left: f32,
    last_time_origin: f32,
    filttnow: f32,
}

impl TimeNavigator {
    /// `initial_time_origin`: `filttorigin`'s control function evaluated
    /// at `(dur, 0.0)` - the C's `frame_count == 0` reset seeds both
    /// `filttnow` and the "last known origin" (used to compute the
    /// origin's own rate of change on later calls) from this.
    pub fn new(cfg: TimeNavConfig, initial_time_origin: f32) -> Self {
        TimeNavigator {
            cfg,
            loop_time_direction_inverter: 1.0,
            end_segment_flag: false,
            use_time_window_flag: false,
            save_filt_time_left_now_flag: true,
            start_filt_time_left: 0.0,
            last_time_origin: initial_time_origin,
            filttnow: initial_time_origin,
        }
    }

    /// Advances the time position by one frame. `filttinc`: seconds per
    /// analysis frame (`D / R`). `time_origin`/`rate`/`win_low`/`win_hi`:
    /// this frame's already-`fval()`-resolved control values (raw,
    /// un-clamped - clamping to `[0, analysis_dur]` and low/high
    /// swapping both happen inside, matching the C). `t`/`dur`: the
    /// caller's current output time and duration; `dur` is mutated
    /// in-place when onset/release mode extends it to reach a release
    /// boundary, exactly like the C's `*dur` out-parameter.
    #[allow(clippy::too_many_arguments)]
    pub fn advance(
        &mut self,
        filttinc: f32,
        time_origin: f32,
        rate: f32,
        win_low_raw: f32,
        win_hi_raw: f32,
        analysis_dur: f32,
        t: f32,
        dur: &mut f32,
    ) -> TimeNavStep {
        let filttnow_at_start = self.filttnow;

        let origin_change = time_origin - self.last_time_origin;
        self.last_time_origin = time_origin;

        let temp = origin_change + filttinc * rate;
        self.filttnow += temp * self.loop_time_direction_inverter;
        let time_direction_now: i32 = if temp >= 0.0 { 1 } else { -1 };

        let new_minus_old_diff = self.filttnow - filttnow_at_start;
        let mut oldfilttnow = filttnow_at_start;

        let mut win_low = win_low_raw.clamp(0.0, analysis_dur);
        let mut win_hi = win_hi_raw.clamp(0.0, analysis_dur);
        if win_hi < win_low {
            std::mem::swap(&mut win_hi, &mut win_low);
        }

        if !self.use_time_window_flag
            && !self.end_segment_flag
            && ((self.filttnow > win_low && self.filttnow < win_hi) || !self.cfg.onset_release)
        {
            self.use_time_window_flag = true;
        }

        if self.use_time_window_flag
            && self.cfg.onset_release
            && !self.cfg.autostop
            && self.loop_time_direction_inverter == 1.0
            && !self.end_segment_flag
        {
            let mut test_dur = if time_direction_now == -1 {
                win_hi
            } else {
                analysis_dur - win_low
            };
            if self.cfg.loop_mode == LoopMode::Fold {
                test_dur += win_hi - win_low;
            }
            let remaining_time = *dur - t;
            if remaining_time <= test_dur {
                self.use_time_window_flag = false;
                self.end_segment_flag = true;
            }
        }

        let mut autostop = false;

        if self.use_time_window_flag && !self.end_segment_flag {
            if self.cfg.autostop {
                if self.filttnow > win_hi || self.filttnow < win_low {
                    autostop = true;
                }
            } else {
                match self.cfg.loop_mode {
                    LoopMode::Wrap => {
                        let win_size = win_hi - win_low;
                        while self.filttnow > win_hi {
                            self.filttnow -= win_size;
                        }
                        while self.filttnow < win_low {
                            self.filttnow += win_size;
                        }
                    }
                    LoopMode::Fold => {
                        let win_size = win_hi - win_low;
                        if self.filttnow > win_hi {
                            let prop = (self.filttnow - win_hi) / (2.0 * win_size);
                            let prop_frac = prop - (prop as i32) as f32;
                            self.filttnow = win_low + win_size * (-1.0 + prop_frac * 2.0).abs();
                            if -1.0 + prop_frac * 2.0 < 0.0 {
                                self.loop_time_direction_inverter = -1.0;
                            }
                            oldfilttnow = self.filttnow - new_minus_old_diff;
                        }
                        if self.filttnow < win_low {
                            let prop = (win_low - self.filttnow) / (2.0 * win_size);
                            let prop_frac = prop - (prop as i32) as f32;
                            self.filttnow = win_hi - win_size * (1.0 - prop_frac * 2.0).abs();
                            if 1.0 - prop_frac * 2.0 > 0.0 {
                                self.loop_time_direction_inverter = 1.0;
                            }
                            oldfilttnow = self.filttnow - new_minus_old_diff;
                        }
                    }
                    LoopMode::Clip => {
                        if self.filttnow > win_hi {
                            self.filttnow = win_hi;
                            oldfilttnow = self.filttnow - new_minus_old_diff;
                        }
                        if self.filttnow < win_low {
                            self.filttnow = win_low;
                            oldfilttnow = self.filttnow - new_minus_old_diff;
                        }
                    }
                }
            }
        }

        if self.filttnow > analysis_dur {
            self.filttnow = analysis_dur;
            oldfilttnow = analysis_dur;
        }
        if self.filttnow < 0.0 {
            self.filttnow = 0.0;
            oldfilttnow = 0.0;
        }

        if self.end_segment_flag {
            let filt_time_left_now = if time_direction_now == 1 {
                analysis_dur - self.filttnow
            } else {
                self.filttnow
            };
            if self.save_filt_time_left_now_flag {
                self.start_filt_time_left = filt_time_left_now;
                self.save_filt_time_left_now_flag = false;
            }
            let filt_time_left_prop = 1.0 - (filt_time_left_now / self.start_filt_time_left);
            let filt_time_left_prop = curve(0.0, 1.0, filt_time_left_prop, 7.0);
            *dur += if time_direction_now == 1 {
                filt_time_left_prop * ((analysis_dur - self.filttnow) - (*dur - t))
            } else {
                filt_time_left_prop * (self.filttnow - (*dur - t))
            };
        }

        if self.end_segment_flag && (self.filttnow == 0.0 || self.filttnow == analysis_dur) {
            autostop = true;
        }

        TimeNavStep {
            filttnow: self.filttnow,
            oldfilttnow,
            autostop,
        }
    }
}

/// Ports `makeLoopSmoothTime()`: extra attack/release smoothing time
/// applied near a sampler loop's boundaries (peaking at
/// `peak_loop_smooth_time` right at either edge, tapering to `0` at the
/// window's midpoint), so a loop's seam doesn't click. Always `0.0` in
/// autostop mode, in clip mode, or outside the boundary zone.
pub fn make_loop_smooth_time(
    filttnow: f32,
    loop_mode: LoopMode,
    autostop: bool,
    win_low: f32,
    win_hi: f32,
    peak_loop_smooth_time: f32,
) -> f32 {
    if autostop {
        return 0.0;
    }
    if loop_mode == LoopMode::Clip {
        return 0.0;
    }
    if !(filttnow >= win_low - peak_loop_smooth_time && filttnow <= win_hi + peak_loop_smooth_time)
    {
        return 0.0;
    }
    let half_loop_time = 0.5 * (win_hi - win_low).abs();
    let loop_smooth_time = peak_loop_smooth_time
        - (half_loop_time - (filttnow - win_low - half_loop_time).abs()).abs();
    loop_smooth_time.max(0.0)
}

/// Ports `makeInterpolatedFilterFrame()`. Despite its name (and its own
/// source comments, "PROPORTIONAL POSITION BETWEEN FRAMES"), **this does
/// not actually interpolate**: the C declares `filtfprop` as `int`, not
/// `float`, sharing one declaration line with the frame indices (`int i,
/// k, filtflow, filtfhigh, filtfprop;`) - so `filtfprop = filtf - (float)
/// filtflow`, always a fractional value strictly in `[0, 1)`, truncates
/// to exactly `0` on *every* call. `channel[i] = F_lower[i] + filtfprop *
/// (F_higher[i] - F_lower[i])` therefore always evaluates to `F_lower[i]`
/// exactly - `F_higher` is read from the analysis file but its value is
/// never actually used. Confirmed empirically (`legacy/tools/
/// dumptwarp.c`): querying a fractional timepoint returns the floor
/// frame's values verbatim, not a value partway toward the next frame.
///
/// Reproduced faithfully here rather than "fixed": `pvc twarp` lands on
/// whichever analysis frame `timepoint` floors to, matching the real
/// tool's actual (if misleadingly-named) behavior. `frames` is one
/// channel's frames from a loaded `.pva` file
/// (`pvc_io::PvaData::channels[ch]`); the floor index is clamped to
/// `0..frames.len()` for timepoints at or beyond the last frame (the C's
/// unchecked `fseek`/`fread` would instead silently read nothing and
/// leave `F_lower` holding a stale previous call's contents there - not
/// practical or meaningful to reproduce, since it depends on prior call
/// history rather than any real analysis data).
pub fn interpolate_frame(frames: &[Vec<f32>], iframes_per_sec: f32, timepoint: f32) -> Vec<f32> {
    let filtf = iframes_per_sec * timepoint;
    let filtflow = filtf as i64;
    let last = frames.len() as i64 - 1;
    frames[filtflow.clamp(0, last) as usize].clone()
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Runs `n_frames` of `TimeNavigator::advance` with a constant zero
    /// origin and constant rate/window, returning `(filttnow,
    /// oldfilttnow, autostop)` per frame - mirrors
    /// `legacy/tools/dumptwarp.c`'s `run_scenario` exactly (same
    /// `analysis_dur = 1.0`, `dur = 100.0`, `t = frame_count *
    /// filttinc`), whose output these tests' expected values were
    /// recorded from.
    fn run_scenario(
        cfg: TimeNavConfig,
        win_low: f32,
        win_hi: f32,
        rate: f32,
        filttinc: f32,
        n_frames: usize,
    ) -> Vec<(f32, f32, bool)> {
        let mut nav = TimeNavigator::new(cfg, 0.0);
        let mut dur = 100.0f32;
        let mut out = Vec::new();
        for frame in 0..n_frames {
            let t = frame as f32 * filttinc;
            let step = nav.advance(filttinc, 0.0, rate, win_low, win_hi, 1.0, t, &mut dur);
            out.push((step.filttnow, step.oldfilttnow, step.autostop));
            if step.autostop {
                break;
            }
        }
        out
    }

    #[test]
    fn wrap_matches_c_oracle() {
        let cfg = TimeNavConfig {
            onset_release: false,
            autostop: false,
            loop_mode: LoopMode::Wrap,
        };
        let got = run_scenario(cfg, 0.0, 0.5, 1.0, 0.3, 8);
        let expected = [
            (0.3, 0.0),
            (0.1, 0.3),
            (0.4, 0.1),
            (0.2, 0.4),
            (0.0, 0.2),
            (0.3, 0.0),
            (0.1, 0.3),
            (0.4, 0.1),
        ];
        for (i, (f, o)) in expected.iter().enumerate() {
            assert!(
                (got[i].0 - f).abs() < 1e-5 && (got[i].1 - o).abs() < 1e-5,
                "frame {i}: got {:?}, expected ({f}, {o})",
                got[i]
            );
        }
    }

    #[test]
    fn fold_matches_c_oracle() {
        let cfg = TimeNavConfig {
            onset_release: false,
            autostop: false,
            loop_mode: LoopMode::Fold,
        };
        let got = run_scenario(cfg, 0.0, 0.5, 1.0, 0.3, 8);
        let expected = [
            (0.3, 0.0),
            (0.4, 0.1),
            (0.1, 0.4),
            (0.2, 0.5),
            (0.5, 0.2),
            (0.2, 0.5),
            (0.1, 0.4),
            (0.4, 0.1),
        ];
        for (i, (f, o)) in expected.iter().enumerate() {
            assert!(
                (got[i].0 - f).abs() < 1e-5 && (got[i].1 - o).abs() < 1e-5,
                "frame {i}: got {:?}, expected ({f}, {o})",
                got[i]
            );
        }
    }

    #[test]
    fn clip_matches_c_oracle() {
        let cfg = TimeNavConfig {
            onset_release: false,
            autostop: false,
            loop_mode: LoopMode::Clip,
        };
        let got = run_scenario(cfg, 0.0, 0.5, 1.0, 0.3, 8);
        assert!((got[0].0 - 0.3).abs() < 1e-5);
        for (f, o, _) in &got[1..] {
            assert!((f - 0.5).abs() < 1e-5);
            assert!((o - 0.2).abs() < 1e-5);
        }
    }

    #[test]
    fn autostop_matches_c_oracle() {
        let cfg = TimeNavConfig {
            onset_release: false,
            autostop: true,
            loop_mode: LoopMode::Wrap,
        };
        let got = run_scenario(cfg, 0.0, 0.5, 1.0, 0.3, 8);
        assert_eq!(got.len(), 2);
        assert!((got[0].0 - 0.3).abs() < 1e-5 && !got[0].2);
        assert!((got[1].0 - 0.6).abs() < 1e-5 && got[1].2);
    }

    #[test]
    fn reverse_rate_matches_c_oracle() {
        let cfg = TimeNavConfig {
            onset_release: false,
            autostop: false,
            loop_mode: LoopMode::Wrap,
        };
        let got = run_scenario(cfg, 0.0, 1.0, -1.0, 0.3, 6);
        let expected = [
            (0.7, 0.0),
            (0.4, 0.7),
            (0.1, 0.4),
            (0.8, 0.1),
            (0.5, 0.8),
            (0.2, 0.5),
        ];
        for (i, (f, o)) in expected.iter().enumerate() {
            assert!(
                (got[i].0 - f).abs() < 1e-5 && (got[i].1 - o).abs() < 1e-5,
                "frame {i}: got {:?}, expected ({f}, {o})",
                got[i]
            );
        }
    }

    #[test]
    fn loop_smooth_time_matches_c_oracle() {
        let cases = [
            (-0.05, 0.15),
            (0.0, 0.2),
            (0.05, 0.15),
            (0.5, 0.0),
            (0.95, 0.15),
            (1.0, 0.2),
            (1.05, 0.15),
            (1.3, 0.0),
        ];
        for (pos, expected) in cases {
            let got = make_loop_smooth_time(pos, LoopMode::Wrap, false, 0.0, 1.0, 0.2);
            assert!(
                (got - expected).abs() < 1e-5,
                "pos {pos}: got {got}, expected {expected}"
            );
        }
    }

    #[test]
    fn interpolate_frame_floors_never_lerps_matching_c_oracle_bug() {
        // legacy/tools/dumptwarp.c's synthetic file: frame k = [k*10,
        // k*10+1, k*10+2, k*10+3], iframes_per_sec = 10.
        let frames: Vec<Vec<f32>> = (0..4)
            .map(|k| {
                vec![
                    k as f32 * 10.0,
                    k as f32 * 10.0 + 1.0,
                    k as f32 * 10.0 + 2.0,
                    k as f32 * 10.0 + 3.0,
                ]
            })
            .collect();
        let cases = [
            (0.0, 0),
            (0.05, 0),
            (0.1, 1),
            (0.25, 2),
            (0.3, 3),
            (0.15, 1),
        ];
        for (t, expected_frame) in cases {
            let got = interpolate_frame(&frames, 10.0, t);
            assert_eq!(got, frames[expected_frame], "t={t}");
        }
    }
}
