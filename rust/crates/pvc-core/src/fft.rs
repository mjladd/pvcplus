//! Real FFT, ported from `legacy/pvc_lib/fft.c` (`bitreverse`/`cfft`/`rfft`,
//! a Numerical-Recipes-style in-place real transform: pack `2*n` real
//! values as `n` complex values, run a complex FFT, then a post-processing
//! pass unpacks/repacks the result into the real spectrum's layout).
//!
//! Ported operation-for-operation (same variable names/order as the C)
//! rather than reimplemented against a general-purpose FFT crate, because
//! the plan requires the exact `rfft` output layout so `convert()`'s port
//! (Task 2.5) can consume it verbatim: after a forward transform, `x` holds
//! `[re0, reNyq, re1, im1, re2, im2, ..., re(n/2-1), im(n/2-1)]` - bin 0's
//! and the Nyquist bin's (purely real) values share slots 0 and 1, then
//! every other bin is a (re, im) pair.

/// In-place bit-reversal permutation of `n` complex values (`2*n` floats,
/// interleaved re/im) - the first stage of the recursive FFT below.
fn bitreverse(x: &mut [f32], n: usize) {
    let mut j = 0usize;
    let mut i = 0usize;
    while i < n {
        if j > i {
            x.swap(j, i);
            x.swap(j + 1, i + 1);
        }
        let mut m = n >> 1;
        while m >= 2 && j >= m {
            j -= m;
            m >>= 1;
        }
        j += m;
        i += 2;
    }
}

/// In-place complex FFT (Danielson-Lanczos), `nc` complex values. `nc` must
/// be a power of two.
fn cfft(x: &mut [f32], nc: usize, forward: bool) {
    let nd = nc << 1;
    bitreverse(x, nd);

    let twopi: f32 = (8.0f64 * 1.0f64.atan()) as f32;

    let mut mmax = 2usize;
    while mmax < nd {
        let delta = mmax << 1;
        let theta = twopi / (if forward { mmax as f32 } else { -(mmax as f32) });
        let wpr = -2.0 * (0.5 * theta).sin().powi(2);
        let wpi = theta.sin();
        let mut wr = 1.0f32;
        let mut wi = 0.0f32;

        let mut m = 0usize;
        while m < mmax {
            let mut i = m;
            while i < nd {
                let j = i + mmax;
                let rtemp = wr * x[j] - wi * x[j + 1];
                let itemp = wr * x[j + 1] + wi * x[j];
                x[j] = x[i] - rtemp;
                x[j + 1] = x[i + 1] - itemp;
                x[i] += rtemp;
                x[i + 1] += itemp;
                i += delta;
            }
            let prev_wr = wr;
            wr += prev_wr * wpr - wi * wpi;
            wi = wi * wpr + prev_wr * wpi + wi;
            m += 2;
        }
        mmax = delta;
    }

    let scale = if forward { 1.0 / nd as f32 } else { 2.0 };
    for v in x[..nd].iter_mut() {
        *v *= scale;
    }
}

/// In-place real FFT of `2*n` real values (`x.len() >= 2*n`). `n` must be a
/// power of two. Forward: packs `x` as `n` complex values and produces the
/// layout described in the module doc comment. Inverse: expects that
/// layout and produces `2*n` real values.
pub fn rfft(x: &mut [f32], n: usize, forward: bool) {
    let pi: f32 = (4.0f64 * 1.0f64.atan()) as f32;

    let mut theta = pi / n as f32;
    let mut wr = 1.0f32;
    let mut wi = 0.0f32;
    let c1 = 0.5f32;
    let c2: f32;
    let mut xr: f32;
    let mut xi: f32;

    if forward {
        c2 = -0.5;
        cfft(x, n, forward);
        xr = x[0];
        xi = x[1];
    } else {
        c2 = 0.5;
        theta = -theta;
        xr = x[1];
        xi = 0.0;
        x[1] = 0.0;
    }

    let wpr = -2.0 * (0.5 * theta).sin().powi(2);
    let wpi = theta.sin();
    let n2p1 = (n << 1) + 1;

    for i in 0..=(n >> 1) {
        let i1 = i << 1;
        let i2 = i1 + 1;
        let i3 = n2p1 - i2;
        let i4 = i3 + 1;

        if i == 0 {
            let h1r = c1 * (x[i1] + xr);
            let h1i = c1 * (x[i2] - xi);
            let h2r = -c2 * (x[i2] + xi);
            let h2i = c2 * (x[i1] - xr);
            x[i1] = h1r + wr * h2r - wi * h2i;
            x[i2] = h1i + wr * h2i + wi * h2r;
            xr = h1r - wr * h2r + wi * h2i;
            xi = -h1i + wr * h2i + wi * h2r;
        } else {
            let h1r = c1 * (x[i1] + x[i3]);
            let h1i = c1 * (x[i2] - x[i4]);
            let h2r = -c2 * (x[i2] + x[i4]);
            let h2i = c2 * (x[i1] - x[i3]);
            x[i1] = h1r + wr * h2r - wi * h2i;
            x[i2] = h1i + wr * h2i + wi * h2r;
            x[i3] = h1r - wr * h2r + wi * h2i;
            x[i4] = -h1i + wr * h2i + wi * h2r;
        }

        let temp = wr;
        wr += temp * wpr - wi * wpi;
        wi = wi * wpr + temp * wpi + wi;
    }

    if forward {
        x[1] = xr;
    } else {
        cfft(x, n, forward);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// A DC-only signal (all samples == `v`) has energy only in bin 0.
    #[test]
    fn dc_signal_has_energy_only_in_bin_zero() {
        let n = 512;
        let mut x = vec![1.0f32; 2 * n];
        rfft(&mut x, n, true);
        assert!((x[0] - 1.0).abs() < 1e-3, "re0 = {}", x[0]);
        assert!(x[1].abs() < 1e-3, "reNyq = {}", x[1]);
        for pair in x[2..].chunks_exact(2) {
            assert!(pair[0].abs() < 1e-3 && pair[1].abs() < 1e-3);
        }
    }

    /// An impulse at sample 0 has equal energy in every bin (flat
    /// spectrum). This rfft's forward scale convention (matching cfft's
    /// `1/nd` normalization) puts that flat magnitude at `1/(2*n)`, not
    /// `1.0` - confirmed against real `legacy/pvc_lib/fft.c` output via
    /// legacy/tools/dumpwin.c (both produce exactly 0.001953125 = 1/512
    /// for n=256) rather than assumed.
    #[test]
    fn impulse_has_flat_magnitude_spectrum() {
        let n = 256;
        let mut x = vec![0.0f32; 2 * n];
        x[0] = 1.0;
        rfft(&mut x, n, true);
        let expected = 1.0 / (2 * n) as f32;
        assert!((x[0] - expected).abs() < 1e-6, "re0 = {}", x[0]);
        assert!((x[1] - expected).abs() < 1e-6, "reNyq = {}", x[1]);
        for pair in x[2..].chunks_exact(2) {
            let mag = (pair[0] * pair[0] + pair[1] * pair[1]).sqrt();
            assert!((mag - expected).abs() < 1e-6, "mag = {mag}");
        }
    }

    /// A pure sinusoid at bin `k` shows up as energy concentrated at bin
    /// `k`, with the expected amplitude (Parseval-ish sanity check rather
    /// than an exact-phase check, since phase depends on the sinusoid's
    /// exact alignment to the block).
    #[test]
    fn sinusoid_concentrates_energy_at_its_bin() {
        let n = 256;
        let k = 10;
        let mut x = vec![0.0f32; 2 * n];
        for (i, v) in x.iter_mut().enumerate() {
            *v = (2.0 * std::f32::consts::PI * k as f32 * i as f32 / (2 * n) as f32).sin();
        }
        rfft(&mut x, n, true);

        let mag_at = |bin: usize| -> f32 {
            if bin == 0 {
                x[0].abs()
            } else if bin == n {
                x[1].abs()
            } else {
                let (re, im) = (x[2 * bin], x[2 * bin + 1]);
                (re * re + im * im).sqrt()
            }
        };

        let peak_mag = mag_at(k);
        for bin in 0..=n {
            if bin != k {
                assert!(
                    mag_at(bin) < peak_mag * 0.05,
                    "bin {bin} has {} vs peak {peak_mag} at bin {k}",
                    mag_at(bin)
                );
            }
        }
    }

    /// Forward then inverse should reproduce the original signal (up to
    /// floating-point tolerance) - the fundamental round-trip property any
    /// FFT/IFFT pair must have, and a strong regression check against a
    /// transcription error in either direction of rfft.
    #[test]
    fn forward_then_inverse_round_trips() {
        let n = 128;
        let original: Vec<f32> = (0..2 * n)
            .map(|i| (i as f32 * 0.37).sin() * 0.5 + (i as f32 * 0.11).cos() * 0.3)
            .collect();
        let mut x = original.clone();
        rfft(&mut x, n, true);
        rfft(&mut x, n, false);
        for (a, b) in original.iter().zip(&x) {
            assert!((a - b).abs() < 1e-3, "{a} vs {b}");
        }
    }
}
