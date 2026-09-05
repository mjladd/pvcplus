//! The phase vocoder analysis/synthesis engine: `Analyzer` (ports
//! `fold.c`+`fft.c`'s `rfft`+`convert.c`) and `Synthesizer` (ports
//! `unconvert.c`+`rfft`+ either `overlapadd.c` or `noscbank.c`).
//!
//! The legacy C makes each of `convert`/`unconvert`/`shiftin`/`shiftout`
//! re-entrant for up to 4 simultaneous channels by hand-duplicating each
//! function 3 times (`convert`/`convert1`/`convert2`/`convert3`, same
//! for `unconvert`) - each copy holds its own `static` phase-memory
//! array, so a 5th channel has no analogous function to call. Ported
//! here as genuine per-instance state instead (plan §2.3: "stateful,
//! re-entrant"), so one `Analyzer`/`Synthesizer` pair per channel
//! supports any channel count with no hardcoded limit.
//!
//! The legacy `shiftin`/`shiftout` pull samples from/push samples to
//! global file I/O (`bufferin`/`bufferout` in `fileio.c`) as part of their
//! ring-buffer bookkeeping. Ported here with that I/O call removed and
//! replaced with plain data in/out: `Analyzer::push` takes the new hop's
//! samples as an argument instead of pulling them from a file, and
//! `Synthesizer::overlap_add`/`OscBank::synthesize` return the completed
//! output hop instead of writing it to a file. Padding at end-of-input
//! (shiftin's "N-2*D zeros" flush behavior) becomes the caller's job:
//! push zero-filled hops after real input runs out.

use crate::fft::rfft;

/// One analysis frame: `n/2 + 1` (magnitude, frequency-in-Hz) pairs, one
/// per FFT bin from DC to Nyquist. Frequencies are already in Hz (`convert`
/// bakes in the sample-rate/decimation scaling), matching what's stored in
/// a `.pva` file (`pvc-io::pva`) - `Frame::to_pva_floats`/`from_pva_floats`
/// round-trip directly to/from that on-disk layout.
#[derive(Debug, Clone, PartialEq)]
pub struct Frame {
    pub bins: Vec<(f32, f32)>,
}

impl Frame {
    /// Flattens to the `[mag0, freq0, mag1, freq1, ...]` layout `.pva`
    /// frames use (and what `convert`/`unconvert` pass around as `C[]`).
    pub fn to_pva_floats(&self) -> Vec<f32> {
        let mut out = Vec::with_capacity(self.bins.len() * 2);
        for &(mag, freq) in &self.bins {
            out.push(mag);
            out.push(freq);
        }
        out
    }

    pub fn from_pva_floats(floats: &[f32]) -> Self {
        Frame {
            bins: floats.chunks_exact(2).map(|c| (c[0], c[1])).collect(),
        }
    }
}

/// Ports `fold.c`: multiplies `input` (length `nw`) by `window` (length
/// `nw`), then folds the result into `out` (length `n`) starting at
/// rotation point `n0` (modulo `n`). `out` must already be `n` long; it's
/// zeroed here first, matching the C.
fn fold(input: &[f32], window: &[f32], out: &mut [f32], n0: i64) {
    let n = out.len() as i64;
    out.fill(0.0);
    let mut pos = n0.rem_euclid(n);
    for i in 0..input.len() {
        out[pos as usize] += input[i] * window[i];
        pos += 1;
        if pos == n {
            pos = 0;
        }
    }
}

/// Ports `overlapadd.c`: unrotates/unfolds `folded_spectrum` (length `n`)
/// starting at rotation point `n0`, windowing by `synthesis_window`
/// (length `nw`), and accumulates into `out` (length `nw`, added to
/// in-place - the caller's ring buffer already holds prior overlap).
fn overlap_add(folded_spectrum: &[f32], synthesis_window: &[f32], out: &mut [f32], n0: i64) {
    let n = folded_spectrum.len() as i64;
    let mut pos = n0.rem_euclid(n);
    for i in 0..out.len() {
        out[i] += folded_spectrum[pos as usize] * synthesis_window[i];
        pos += 1;
        if pos == n {
            pos = 0;
        }
    }
}

/// Ports `convert.c`. `n2` is `N/2` (half the FFT size); `d` is the
/// analysis hop (decimation) size; `sample_rate` is `R`.
struct PhaseTracker {
    lastphase: Vec<f32>,
    fundamental: f32,
    factor: f32,
}

impl PhaseTracker {
    fn new_analysis(n2: usize, d: usize, sample_rate: u32) -> Self {
        let twopi: f32 = (8.0f64 * 1.0f64.atan()) as f32;
        PhaseTracker {
            lastphase: vec![0.0; n2 + 1],
            fundamental: sample_rate as f32 / (n2 as f32 * 2.0),
            factor: sample_rate as f32 / (d as f32 * twopi),
        }
    }

    fn new_synthesis(n2: usize, i_factor: usize, sample_rate: u32) -> Self {
        let twopi: f32 = (8.0f64 * 1.0f64.atan()) as f32;
        PhaseTracker {
            lastphase: vec![0.0; n2 + 1],
            fundamental: sample_rate as f32 / (n2 as f32 * 2.0),
            factor: twopi * i_factor as f32 / sample_rate as f32,
        }
    }

    /// `convert()`: rfft-format spectrum `s` (length `2*n2`) -> a `Frame`
    /// of `n2+1` (mag, freq-in-Hz) pairs.
    fn convert(&mut self, s: &[f32]) -> Frame {
        let pi: f32 = (4.0f64 * 1.0f64.atan()) as f32;
        let twopi: f32 = 2.0 * pi;
        let n2 = self.lastphase.len() - 1;
        let mut bins = Vec::with_capacity(n2 + 1);
        for i in 0..=n2 {
            let real = i << 1;
            let imag = real + 1;
            let a = if i == n2 { s[1] } else { s[real] };
            let b = if i == 0 || i == n2 { 0.0 } else { s[imag] };

            let mag = a.hypot(b);
            let freq_hz = if mag == 0.0 {
                i as f32 * self.fundamental
            } else {
                let phase = -b.atan2(a);
                let mut phasediff = phase - self.lastphase[i];
                self.lastphase[i] = phase;
                while phasediff > pi {
                    phasediff -= twopi;
                }
                while phasediff < -pi {
                    phasediff += twopi;
                }
                phasediff * self.factor + i as f32 * self.fundamental
            };
            bins.push((mag, freq_hz));
        }
        Frame { bins }
    }

    /// `unconvert()`: a `Frame` of `n2+1` (mag, freq-in-Hz) pairs -> an
    /// rfft-format spectrum (length `2*n2`, written into `s`).
    fn unconvert(&mut self, frame: &Frame, s: &mut [f32]) {
        let n2 = self.lastphase.len() - 1;
        for (i, &(mag, freq_hz)) in frame.bins.iter().enumerate() {
            let mut real = i << 1;
            let imag = real + 1;
            if i == n2 {
                real = 1;
            }
            self.lastphase[i] += freq_hz - i as f32 * self.fundamental;
            let phase = self.lastphase[i] * self.factor;
            s[real] = mag * phase.cos();
            if i != n2 {
                s[imag] = -mag * phase.sin();
            }
        }
    }
}

/// Phase vocoder analyzer: fold + rfft + convert, one instance per
/// channel. `push`'s precondition is `new_samples.len() == d` (the hop
/// size passed to `new`); anything else returns `None` rather than
/// guessing what was meant.
pub struct Analyzer {
    n: usize,
    d: usize,
    analysis_window: Vec<f32>,
    input_ring: Vec<f32>, // length nw, shiftin's ring buffer
    fft_buf: Vec<f32>,    // length n, fold's output / rfft's in-place buffer
    fold_pos: i64,
    phase: PhaseTracker,
}

impl Analyzer {
    /// `n`: FFT size. `nw`: analysis window length (`analysis_window.len()`
    /// must equal this). `d`: hop size (samples between analysis frames).
    /// `sample_rate`: `R`, used to scale `convert`'s frequency output.
    pub fn new(n: usize, analysis_window: Vec<f32>, d: usize, sample_rate: u32) -> Self {
        let nw = analysis_window.len();
        Analyzer {
            n,
            d,
            analysis_window,
            input_ring: vec![0.0; nw],
            fft_buf: vec![0.0; n],
            // plainpv.c: `in = -Nw`, then `in += D` *before* the first
            // fold() call each frame - so the first fold uses `D - Nw`.
            fold_pos: -(nw as i64),
            phase: PhaseTracker::new_analysis(n / 2, d, sample_rate),
        }
    }

    pub fn push(&mut self, new_samples: &[f32]) -> Option<Frame> {
        if new_samples.len() != self.d {
            return None;
        }
        let nw = self.input_ring.len();
        // shiftin: shift ring left by d, append the new hop at the end.
        self.input_ring.copy_within(self.d..nw, 0);
        self.input_ring[nw - self.d..].copy_from_slice(new_samples);

        // `in += D` happens before fold() is called in the C main loop.
        self.fold_pos += self.d as i64;
        fold(
            &self.input_ring,
            &self.analysis_window,
            &mut self.fft_buf,
            self.fold_pos,
        );

        rfft(&mut self.fft_buf, self.n / 2, true);
        Some(self.phase.convert(&self.fft_buf))
    }
}

/// Phase vocoder synthesizer using overlap-add resynthesis (the path taken
/// when only magnitudes changed - no pitch/frequency shift). One instance
/// per channel.
pub struct Synthesizer {
    n: usize,
    i_factor: usize,
    synthesis_window: Vec<f32>,
    output_ring: Vec<f32>, // length nw, shiftout's ring buffer
    fft_buf: Vec<f32>,
    fold_pos: i64,
    phase: PhaseTracker,
}

impl Synthesizer {
    /// `n`: FFT size. `synthesis_window` length is `Nw`. `i_factor`: hop
    /// size between resynthesis frames (legacy `I`; equals the analysis
    /// hop `D` for unmodified time, `stretch*D` otherwise). `d`: the
    /// paired `Analyzer`'s hop size - needed only to compute the
    /// synthesis rotation's starting offset (`on = (in*I)/D` in the C,
    /// truncating division, so pass the same `D` the `Analyzer` uses even
    /// though `I` may differ for a time-stretch).
    pub fn new(
        n: usize,
        synthesis_window: Vec<f32>,
        i_factor: usize,
        d: usize,
        sample_rate: u32,
    ) -> Self {
        let nw = synthesis_window.len();
        // plainpv.c: `in = -Nw`; `on = (in*I)/D` if D != 0 else `in`.
        let fold_pos = if d != 0 {
            (-(nw as i64) * i_factor as i64) / d as i64
        } else {
            -(nw as i64)
        };
        Synthesizer {
            n,
            i_factor,
            synthesis_window,
            output_ring: vec![0.0; nw],
            fft_buf: vec![0.0; n],
            fold_pos,
            phase: PhaseTracker::new_synthesis(n / 2, i_factor, sample_rate),
        }
    }

    /// Feeds one analysis (or modified-analysis) frame in; returns the
    /// next `i_factor` output samples (shiftout's "output first I samples,
    /// then shift left" - the ring's front `i_factor` samples, already
    /// final since nothing will add to them again).
    ///
    /// Returns an empty `Vec` during the initial warm-up period (while the
    /// synthesis rotation counter is still negative) - `shiftout` in the C
    /// only calls `bufferout` (the write) once its `n >= 0`, though it
    /// unconditionally shifts the ring either way, which is replicated
    /// here via `shift_out` always running. Skips roughly `Nw/I` hops at
    /// the very start of a stream, same as the reference tool.
    pub fn overlap_add(&mut self, frame: &Frame) -> Vec<f32> {
        self.phase.unconvert(frame, &mut self.fft_buf);
        rfft(&mut self.fft_buf, self.n / 2, false);
        // `on += I` happens before overlapadd() is called in the C main loop.
        self.fold_pos += self.i_factor as i64;
        overlap_add(
            &self.fft_buf,
            &self.synthesis_window,
            &mut self.output_ring,
            self.fold_pos,
        );
        let ready = self.fold_pos >= 0;
        let out = self.shift_out();
        if ready {
            out
        } else {
            Vec::new()
        }
    }

    fn shift_out(&mut self) -> Vec<f32> {
        let nw = self.output_ring.len();
        let i = self.i_factor;
        let out = self.output_ring[..i].to_vec();
        self.output_ring.copy_within(i..nw, 0);
        self.output_ring[nw - i..].fill(0.0);
        out
    }

    /// Flushes any remaining tail without adding a new frame - matches
    /// `shiftout`'s `flushflag` path (drops the "advance a hop" semantics,
    /// just returns what's already accumulated in the ring for a final
    /// partial output).
    pub fn flush(&mut self) -> Vec<f32> {
        std::mem::replace(
            &mut self.output_ring,
            vec![0.0; self.synthesis_window.len()],
        )
    }
}

/// Ports `getthresh()`: the [`OscBank`] threshold for one frame - the
/// peak amplitude among `bins[1..]` (bin 0, the DC bin, is excluded from
/// the search, matching the C's loop starting at array index 2, i.e.
/// skipping `amp[0]`) times a linear ratio `tgen` (typically
/// `10^(dB/20)`, computed exactly - not the `DbToAmp` lookup-table
/// approximation).
///
/// Takes a plain bins slice rather than a whole [`Frame`] because the
/// legacy call site matters here: `plainpv.c` calls `getthresh(channel,
/// N, threshfac)` - passing the FFT size `N`, not `N + 2` (despite the
/// C's own parameter being named `Nplus2`) - which *also* excludes the
/// Nyquist bin (`frame.bins`' last entry) from the peak search, not just
/// bin 0. Whether that's deliberate or an off-by-one in the original
/// tool, it's what the real tool does, so the caller passes exactly the
/// bins it wants considered (typically `&frame.bins[..n2]` to match
/// `plainpv`) rather than this function assuming a fixed exclusion.
///
/// Recomputed fresh every frame in the legacy tool (it's a `static`-free
/// plain function there, unlike this port's other per-channel state),
/// hence a plain function here too rather than a method needing an
/// instance.
pub fn getthresh(bins: &[(f32, f32)], tgen: f32) -> f32 {
    let peak = bins
        .iter()
        .skip(1)
        .map(|&(amp, _)| amp)
        .fold(0.0f32, f32::max);
    peak * tgen
}

/// Oscillator-bank resynthesizer (`noscbank.c`): used instead of
/// overlap-add when frequencies have been modified (pitch/frequency
/// shift), since overlap-add doesn't cleanly reproduce a changed spectrum.
/// One instance per channel; unlike `Synthesizer`, doesn't need a separate
/// `unconvert`/inverse-FFT pass - it synthesizes directly from (mag, freq)
/// pairs via a bank of table-lookup oscillators, one per retained bin.
pub struct OscBank {
    table: Vec<f32>, // length L, precomputed cosine table
    l: usize,
    i_factor: usize,
    sample_rate: u32,
    n: usize,
    pitch: f32,
    last_amp: Vec<f32>,
    last_freq: Vec<f32>,
    index: Vec<f32>,
}

impl OscBank {
    const TABLE_LEN: usize = 8192;

    /// `n`: FFT size (number of bins is `n/2 + 1`, indexed 0..=n/2 as
    /// `chan` below, matching the legacy `N+1`-sized state arrays - the
    /// loop bound `NP` just controls how many of them are actually
    /// driven). `nw`: window length (only used to pick the table's
    /// amplitude scale, matching the legacy `Nw >= N ? N : 8*N`).
    /// `i_factor`: samples synthesized per frame (legacy `I`). `pitch`:
    /// frequency scaling factor (legacy `P`; `1.0` = no transposition).
    pub fn new(n: usize, nw: usize, sample_rate: u32, i_factor: usize, pitch: f32) -> Self {
        let twopi: f32 = (8.0f64 * 1.0f64.atan()) as f32;
        let l = Self::TABLE_LEN;
        let tabscale = if nw >= n { n as f32 } else { 8.0 * n as f32 };
        let twopi_over_l = twopi / l as f32;
        let table = (0..l)
            .map(|k| tabscale * (twopi_over_l * k as f32).cos())
            .collect();
        let num_bins = n / 2 + 1;
        OscBank {
            table,
            l,
            i_factor,
            sample_rate,
            n,
            pitch,
            last_amp: vec![0.0; num_bins],
            last_freq: vec![0.0; num_bins],
            index: vec![0.0; num_bins],
        }
    }

    /// Synthesizes `i_factor` samples from one (mag, freq-in-Hz) frame,
    /// linearly interpolating amplitude/frequency from the previous
    /// frame's values across the hop, per bin. `threshold`: bins with
    /// magnitude below this are skipped (legacy `synt`, a *global*
    /// re-derived fresh every frame from that frame's own peak amplitude
    /// via `getthresh()` - not a fixed value, so it's a per-call argument
    /// here rather than fixed at construction like the rest of this
    /// struct's parameters). Already linear amplitude, not dB - the
    /// caller converts.
    pub fn synthesize(&mut self, frame: &Frame, threshold: f32) -> Vec<f32> {
        let l = self.l as f32;
        let iinv = 1.0 / self.i_factor as f32;
        let pinc = self.pitch * l / self.sample_rate as f32;

        let np = if self.pitch > 1.0 {
            (self.n as f32 / self.pitch) as usize
        } else {
            self.n
        };

        let mut out = vec![0.0f32; self.i_factor];
        for (chan, &(amp0, freq0)) in frame.bins.iter().enumerate().take(np) {
            if amp0 < threshold {
                continue;
            }
            let freq_scaled = freq0 * pinc;
            let mut f = self.last_freq[chan];
            let finc = (freq_scaled - f) * iinv;
            let mut a = self.last_amp[chan];
            let ainc = (amp0 - a) * iinv;
            let mut address = self.index[chan];

            for sample in out.iter_mut() {
                *sample += a * self.table[address as usize];
                address += f;
                while address >= l {
                    address -= l;
                }
                while address < 0.0 {
                    address += l;
                }
                a += ainc;
                f += finc;
            }

            self.last_freq[chan] = freq_scaled;
            self.last_amp[chan] = amp0;
            self.index[chan] = address;
        }
        out
    }
}

/// Ports `phaselock.c`: for each interior bin that's a local amplitude
/// peak (strictly greater than both neighbors), the peak's immediate
/// neighbors have their *frequency* value pulled to match whichever
/// adjacent peak is closer (a "phase locking" technique that reduces
/// phasiness by keeping a peak's whole local neighborhood moving at one
/// coherent frequency). Operates in place on a `Frame`.
pub fn phaselock(frame: &mut Frame) {
    let n2 = frame.bins.len().saturating_sub(1);
    if n2 < 2 {
        return;
    }
    let mut peak_markers = vec![0u8; n2];
    for bin in 1..n2 - 1 {
        let amp = frame.bins[bin].0;
        if frame.bins[bin - 1].0 < amp && frame.bins[bin + 1].0 < amp {
            peak_markers[bin] = 2;
            peak_markers[bin - 1] = 1;
            peak_markers[bin + 1] = 1;
        } else {
            peak_markers[bin] = 0;
        }
    }
    for bin in 0..n2 {
        if peak_markers[bin] != 1 {
            continue;
        }
        let peak_below = bin > 0 && peak_markers[bin - 1] == 2;
        let peak_above = bin + 1 < n2 && peak_markers[bin + 1] == 2;
        let new_freq = if peak_below && peak_above {
            let up_dist = frame.bins[bin + 1].0 - frame.bins[bin].0;
            let down_dist = frame.bins[bin - 1].0 - frame.bins[bin].0;
            if up_dist > down_dist {
                frame.bins[bin + 1].1
            } else {
                frame.bins[bin - 1].1
            }
        } else if peak_below {
            frame.bins[bin - 1].1
        } else {
            frame.bins[bin + 1].1
        };
        frame.bins[bin].1 = new_freq;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::window::{make_windows, Window};

    #[test]
    fn getthresh_excludes_bin_zero_matches_c_oracle() {
        // Bin 0's amplitude (9.0, the largest) must not win the peak
        // search - confirmed against the real compiled getthresh().
        let bins = [(9.0, 0.0), (2.0, 0.0), (7.0, 0.0), (4.0, 0.0), (1.0, 0.0)];
        assert_eq!(getthresh(&bins, 0.5), 3.5);
    }

    /// The plan's explicit Task 2.5 acceptance test: analysis -> synthesis
    /// with I == D (unmodified time/pitch) should reproduce the input
    /// within -60dB, using overlap-add resynthesis.
    ///
    /// Two fixed offsets separate `output[i]` from the `input` sample it
    /// reconstructs, confirmed with an isolated impulse test before this
    /// was trusted (an impulse fed in at input sample `k` comes back out
    /// at output sample `k + (Nw - D)`, at ~0.997 amplitude, essentially
    /// exact): the `Nw - D` samples of algorithmic latency inherent to any
    /// overlap-add block processor (the window has to fully slide across
    /// a sample before that sample's contribution is fully summed), and
    /// the hop-granularity warm-up latency `Synthesizer::overlap_add`
    /// documents (it suppresses the first few hops' output entirely,
    /// mirroring `shiftout`'s write gate).
    #[test]
    fn analysis_synthesis_round_trip_within_60db() {
        let n = 1024;
        // Nw/4: a standard 4x-overlap hop that gives Hamming's squared
        // window near-exact constant-overlap-add (COLA), needed to hit
        // -60dB. An arbitrary hop like 220 only gets COLA to within
        // ~0.3% (worst-phase), capping reconstruction around -49dB - a
        // real property of that window/hop combination, not a bug
        // (confirmed against the window values directly: the per-phase
        // sum of `analysis[i]*synthesis[i]` at stride 220 ranges
        // 0.9966-1.0001 depending on phase, vs. dead-on 1.0 at every
        // phase for stride 256).
        let d = n / 4;
        let sample_rate = 44100u32;
        let pair = make_windows(Window::Hamming, n, n, d);

        let mut analyzer = Analyzer::new(n, pair.analysis.clone(), d, sample_rate);
        let mut synth = Synthesizer::new(n, pair.synthesis.clone(), d, d, sample_rate);

        // A few seconds of a simple tone, long enough to get well past
        // the filter's startup transient.
        let num_hops = 200;
        let input: Vec<f32> = (0..num_hops * d)
            .map(|i| {
                0.5 * (2.0 * std::f32::consts::PI * 440.0 * i as f32 / sample_rate as f32).sin()
            })
            .collect();

        // With I == D, `Synthesizer::overlap_add` suppresses output during
        // its startup latency (mirrors `shiftout`'s `n >= 0` write gate -
        // see its doc comment), so the first several hops produce no
        // samples at all. Once emission starts it never stops, so track
        // which input hop the first emitted output hop corresponds to and
        // align from there, rather than assuming output[i] <-> input[i].
        let mut output = Vec::with_capacity(input.len());
        let mut first_emitted_hop = None;
        for (hop_idx, hop) in input.chunks_exact(d).enumerate() {
            let frame = analyzer.push(hop).expect("hop is exactly d samples");
            let hop_out = synth.overlap_add(&frame);
            if !hop_out.is_empty() {
                first_emitted_hop.get_or_insert(hop_idx);
                output.extend(hop_out);
            }
        }
        let hop_warmup_offset = first_emitted_hop.expect("stream long enough to clear warm-up") * d;
        // The additional fixed algorithmic latency (see doc comment above).
        let input_offset = hop_warmup_offset - (n - d);

        // Skip another second or so (Nw/D hops) past that - the window
        // hasn't fully filled with real signal yet even once emission
        // starts, so the earliest emitted output is still a startup
        // transient, not steady-state reconstruction.
        let skip = (n / d + 2) * d;
        let mut max_err = 0.0f32;
        let mut peak = 0.0f32;
        for i in skip..output.len().min(input.len() - input_offset) {
            max_err = max_err.max((output[i] - input[input_offset + i]).abs());
            peak = peak.max(input[input_offset + i].abs());
        }
        let err_db = 20.0 * (max_err / peak).log10();
        assert!(
            err_db < -60.0,
            "reconstruction error {err_db} dB (max_err={max_err}, peak={peak})"
        );
    }

    /// Direct confirmation of the round-trip test's `Nw - D` latency claim:
    /// an impulse fed at input sample `k` should reappear, essentially
    /// unchanged in amplitude, at output sample `k + (Nw - D)`.
    #[test]
    fn impulse_reappears_after_nw_minus_d_samples_at_unity_gain() {
        let n = 1024;
        let d = 220;
        let sample_rate = 44100u32;
        let pair = make_windows(Window::Hamming, n, n, d);
        let mut analyzer = Analyzer::new(n, pair.analysis, d, sample_rate);
        let mut synth = Synthesizer::new(n, pair.synthesis, d, d, sample_rate);

        let num_hops = 30;
        let impulse_at = 5 * d + 13;
        let mut input = vec![0.0f32; num_hops * d];
        input[impulse_at] = 1.0;

        let mut output = Vec::new();
        let mut first_emitted_hop = None;
        for (hop_idx, hop) in input.chunks_exact(d).enumerate() {
            let frame = analyzer.push(hop).unwrap();
            let out = synth.overlap_add(&frame);
            if !out.is_empty() {
                first_emitted_hop.get_or_insert(hop_idx);
                output.extend(out);
            }
        }
        let input_offset = first_emitted_hop.unwrap() * d;

        let expected_at = impulse_at + (n - d);
        let (peak_i, &peak_v) = output
            .iter()
            .enumerate()
            .max_by(|a, b| a.1.abs().total_cmp(&b.1.abs()))
            .unwrap();
        assert_eq!(input_offset + peak_i, expected_at);
        assert!((peak_v - 1.0).abs() < 0.01, "peak amplitude {peak_v}");
    }

    #[test]
    fn frame_pva_float_round_trip() {
        let frame = Frame {
            bins: vec![(1.0, 0.0), (0.5, 440.0), (0.25, 880.0)],
        };
        let floats = frame.to_pva_floats();
        assert_eq!(floats, vec![1.0, 0.0, 0.5, 440.0, 0.25, 880.0]);
        assert_eq!(Frame::from_pva_floats(&floats), frame);
    }

    #[test]
    fn phaselock_pulls_neighbor_frequency_to_nearer_peak() {
        // bin 1 is a peak (amp 10 > neighbors' 1 and 2); bin 0 (only
        // neighbor below) should have its frequency pulled to bin 1's.
        let mut frame = Frame {
            bins: vec![(1.0, 100.0), (10.0, 200.0), (2.0, 300.0), (1.0, 400.0)],
        };
        phaselock(&mut frame);
        assert_eq!(frame.bins[0].1, 200.0);
    }

    #[test]
    fn silence_produces_silence() {
        let n = 512;
        let d = 128;
        let pair = make_windows(Window::Hamming, n, n, d);
        let mut analyzer = Analyzer::new(n, pair.analysis, d, 44100);
        let mut synth = Synthesizer::new(n, pair.synthesis, d, d, 44100);
        let zeros = vec![0.0f32; d];
        for _ in 0..10 {
            let frame = analyzer.push(&zeros).unwrap();
            let out = synth.overlap_add(&frame);
            assert!(out.iter().all(|&s| s.abs() < 1e-6));
        }
    }
}
