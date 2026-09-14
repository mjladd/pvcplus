# Concepts

This page explains the phase-vocoder ideas and terms that many `pvc` subcommands share. Read it once. Then use each tool's own page under `docs/tools/` for its specific flags.

## Overlap/add versus oscillator bank resynthesis

A phase vocoder rebuilds a signal from FFT frames in one of two ways. When only magnitudes change, the fast overlap/add method works. The frame's own frequency content stays intact in that case. When frequencies change too, overlap/add breaks down. The tool then switches to an oscillator bank instead. It resynthesizes each bin as its own sine wave, changing in frequency and amplitude. The oscillator bank method is slower. Many tools expose a resynthesis threshold that turns off any bin below a set amplitude. This trades a little quality for speed. A threshold of -60 dB is a reasonable starting point.

## FFT size, window size, and window type

The FFT size sets the frequency resolution of each analysis frame. It must be a power of two. A larger FFT resolves frequencies more finely. It tracks fast transients more poorly, though. 1024 or 2048 works well for most sounds.

The window size shapes each analysis frame before the FFT runs. It must also be a power of two. A window twice the FFT size is a common choice. Some tools default the window size to a fixed value instead of doubling the FFT size automatically. Check the individual tool's page for its own default.

The window's shape also affects analysis and resynthesis quality. Common choices include these: Hamming, Rectangular, Blackman, Bartlett (triangular), and several Kaiser variants. A higher Kaiser alpha value gives lower sidelobes and better frequency isolation. It costs some time resolution in return. Blackman or a mid-range Kaiser window is a reasonable default for most material. When a tool loses transient detail, Rectangular can help. Watch for added pops in that case.

## Frames per second

This setting controls how often the phase vocoder analyzes the signal. It trades time resolution against processing speed. Around 200 frames per second is a good reference point. If you stretch time, raise this value to match. That keeps roughly the same analysis density.

## Time expansion and contraction

A stretch factor scales the output's own duration directly. A factor above 1 lengthens the sound. A factor below 1 shortens it. A factor of 1 leaves duration unchanged.

## Pitch transposition and frequency shift

Pitch transposition multiplies every bin's frequency by a constant factor. The factor is expressed in semitones, where 12 semitones make one octave. This is classic, harmonic-preserving transposition.

Frequency shift instead adds a fixed value to every bin's frequency. The shift is additive, not multiplicative. So it distorts the harmonic relationships between partials. This produces effects related to ring modulation. Use frequency shift for deliberate, nonlinear pitch-domain distortion, not for clean transposition.

## Decibels

This toolkit handles amplitude in decibel units. A value of 1.0, full scale, equals 0 dB. A change of about 6 dB doubles or halves amplitude. A tool's own page documents its specific dB ranges and defaults. Exact clamping behavior varies by tool, so check there.

## Warp index

Several tools reshape a 0-1 range of values through an exponential warp function. This applies to a frequency response's own amplitudes, or to successive FFT frame amplitudes. A warp index `w` controls the curve:

```
y = (1 - e^(x * w)) / (1 - e^w)
```

A warp index of 0 leaves the input unchanged. Positive values pull values toward the low end. This accentuates peaks and suppresses weaker content. Negative values pull values toward the high end instead. This compresses dynamic range and raises the level of weaker, noise-like content.

## Envelope response time (attack/decay smoothing)

Many tools smooth how quickly amplitude can change. They use a one-pole lowpass filter:

```
y(n) = (1 - A) * x(n) + A * y(n-1)
```

A response time in seconds sets the coefficient `A`. The response time is the time a signal takes, moving from one level to another, to settle within -60 dB of the new level. Separate attack and decay response times let rising and falling amplitude smooth at different rates. Short response times avoid pops from abrupt dynamic processing. Longer response times smooth or blur onsets and offsets instead. A long decay time on amplitude can raise the level of residual noise, though.

## Analysis data access modes: rate versus explicit

Some tools read a `.pva` analysis file over time instead of processing it straight through. Each one uses a rate mode or an explicit mode.

- **Rate mode**: a rate value sets how fast the read position moves through the file. Positive is forward, negative is backward. The rate can also vary over time, driven by a function. A time-point value sets the starting position.
- **Explicit mode**: the time-point value directly names the analysis time to read from. This mode ignores the rate value. When the time-point itself varies, typically driven by a control function, this mode makes sense.

Both modes stay within a lower and upper data-window boundary. Moving past either boundary wraps back in from the other end. This makes the window circular, not clamped.

## Response accumulation: peak versus average

Tools that build a frequency response from an analyzed sound can accumulate it by peak or by average. Peak accumulation records the highest amplitude ever seen per bin. This suits a sound with an intermittent loud moment worth capturing. Average accumulation instead reflects the sound's typical, ongoing characteristic.

## Compression and expansion

Compression reduces amplitudes above a threshold. Expansion reduces amplitudes below a threshold, widening the dynamic range below it. Both are expressed as a decibel amount. A compression of -6 dB halves the dynamic range above the threshold. An expansion of -6 dB doubles the dynamic range below it. "Companding" is the general term for both. Some tools apply it as one global gain change. Others apply it independently per frequency bin, against a reference response file.
