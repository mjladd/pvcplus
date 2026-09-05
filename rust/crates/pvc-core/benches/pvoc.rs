//! Benchmarks the phase vocoder engine's steady-state per-hop cost:
//! `Analyzer::push` (fold + forward rfft + convert) and
//! `Synthesizer::overlap_add` (unconvert + inverse rfft + overlap-add),
//! at a typical size (1024-point FFT, 256-sample hop, 44.1kHz).

use criterion::{black_box, criterion_group, criterion_main, Criterion};
use pvc_core::{make_windows, Analyzer, Synthesizer, Window};

fn bench_analyzer_push(c: &mut Criterion) {
    let n = 1024;
    let d = n / 4;
    let pair = make_windows(Window::Hamming, n, n, d);
    let mut analyzer = Analyzer::new(n, pair.analysis, d, 44100);
    let hop: Vec<f32> = (0..d).map(|i| (i as f32 * 0.1).sin()).collect();

    c.bench_function("Analyzer::push (n=1024, d=256)", |b| {
        b.iter(|| black_box(analyzer.push(black_box(&hop))))
    });
}

fn bench_synthesizer_overlap_add(c: &mut Criterion) {
    let n = 1024;
    let d = n / 4;
    let analysis_pair = make_windows(Window::Hamming, n, n, d);
    let synth_pair = make_windows(Window::Hamming, n, n, d);
    let mut analyzer = Analyzer::new(n, analysis_pair.analysis, d, 44100);
    let mut synth = Synthesizer::new(n, synth_pair.synthesis, d, d, 44100);
    let hop: Vec<f32> = (0..d).map(|i| (i as f32 * 0.1).sin()).collect();
    let frame = analyzer.push(&hop).unwrap();

    c.bench_function("Synthesizer::overlap_add (n=1024, d=256)", |b| {
        b.iter(|| black_box(synth.overlap_add(black_box(&frame))))
    });
}

criterion_group!(benches, bench_analyzer_push, bench_synthesizer_overlap_add);
criterion_main!(benches);
