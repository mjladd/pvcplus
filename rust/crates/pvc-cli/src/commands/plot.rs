//! `pvc fn plot`: a dependency-free terminal sparkline, replacing the
//! legacy `showme` family's `reshape -A1 $1 > /tmp/... ; gnuplot ...`
//! pipeline (see `cli.rs`'s `FnCommand::Plot` doc comment for how that
//! was traced back to just "convert to ASCII, then plot").

use std::path::Path;

use anyhow::{ensure, Context, Result};

/// Eight levels of Unicode block characters, low to high - the standard
/// "sparkline" character set.
const LEVELS: [char; 8] = [
    '\u{2581}', '\u{2582}', '\u{2583}', '\u{2584}', '\u{2585}', '\u{2586}', '\u{2587}', '\u{2588}',
];

const DEFAULT_WIDTH: usize = 120;

pub fn run(input: &Path, width: Option<usize>) -> Result<()> {
    let data =
        pvc_io::read_control_file(input).with_context(|| format!("reading {}", input.display()))?;
    ensure!(
        !data.values.is_empty(),
        "{}: nothing to plot (empty control file)",
        input.display()
    );

    let width = width.unwrap_or(DEFAULT_WIDTH).max(1);
    let plotted = downsample(&data.values, width);

    let min = plotted.iter().copied().fold(f32::INFINITY, f32::min);
    let max = plotted.iter().copied().fold(f32::NEG_INFINITY, f32::max);
    let range = if max > min { max - min } else { 1.0 };

    let sparkline: String = plotted
        .iter()
        .map(|&v| {
            let t = ((v - min) / range).clamp(0.0, 1.0);
            let idx = (t * (LEVELS.len() - 1) as f32).round() as usize;
            LEVELS[idx.min(LEVELS.len() - 1)]
        })
        .collect();

    println!("{}", input.display());
    println!("{sparkline}");
    println!("min={min:.6}  max={max:.6}  n={}", data.values.len());
    Ok(())
}

/// Downsamples `values` to at most `width` points by averaging
/// consecutive buckets - a no-op if there are already `<= width` values.
fn downsample(values: &[f32], width: usize) -> Vec<f32> {
    if values.len() <= width {
        return values.to_vec();
    }
    let bucket = values.len() as f32 / width as f32;
    (0..width)
        .map(|i| {
            let start = (i as f32 * bucket) as usize;
            let end = (((i + 1) as f32 * bucket) as usize)
                .max(start + 1)
                .min(values.len());
            let slice = &values[start..end];
            slice.iter().sum::<f32>() / slice.len() as f32
        })
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn downsample_is_noop_when_already_short_enough() {
        let values = vec![1.0, 2.0, 3.0];
        assert_eq!(downsample(&values, 10), values);
    }

    #[test]
    fn downsample_averages_into_requested_width() {
        let values: Vec<f32> = (0..100).map(|i| i as f32).collect();
        let out = downsample(&values, 10);
        assert_eq!(out.len(), 10);
        // Each bucket of 10 consecutive values 0..9, 10..19, ... averages
        // to its midpoint.
        assert!((out[0] - 4.5).abs() < 1e-4);
        assert!((out[9] - 94.5).abs() < 1e-4);
    }
}
