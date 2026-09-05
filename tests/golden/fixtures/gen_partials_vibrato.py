#!/usr/bin/env python3
"""Synthesizes a "voice-like" harmonic tone: 8 partials of a 220Hz
fundamental with falling amplitude, a slow vibrato (frequency modulation)
and a slight tremolo (amplitude modulation). Stdlib only (wave + math) so
tests/golden/fixtures/gen.sh doesn't need numpy/scipy just for this one
fixture - everything else there is plain sox.

Usage: gen_partials_vibrato.py <output.wav> <sample_rate>
"""
import math
import struct
import sys
import wave

FUNDAMENTAL = 220.0
NUM_PARTIALS = 8
DURATION_S = 2.0
VIBRATO_HZ = 5.0
VIBRATO_DEPTH = 0.015  # +/- 1.5% frequency deviation
TREMOLO_HZ = 4.0
TREMOLO_DEPTH = 0.15  # +/- 15% amplitude deviation
PEAK_AMP = 0.4  # of full scale, leaves headroom for the partial sum


def main():
    out_path, rate_str = sys.argv[1], sys.argv[2]
    rate = int(rate_str)
    n_samples = int(DURATION_S * rate)

    # Falling amplitude per partial (1/n), normalized so the summed peak
    # doesn't clip.
    partial_amps = [1.0 / (i + 1) for i in range(NUM_PARTIALS)]
    amp_sum = sum(partial_amps)
    partial_amps = [a / amp_sum for a in partial_amps]

    phases = [0.0] * NUM_PARTIALS
    samples = []
    for n in range(n_samples):
        t = n / rate
        vibrato = 1.0 + VIBRATO_DEPTH * math.sin(2 * math.pi * VIBRATO_HZ * t)
        tremolo = 1.0 + TREMOLO_DEPTH * math.sin(2 * math.pi * TREMOLO_HZ * t)
        value = 0.0
        for i in range(NUM_PARTIALS):
            freq = FUNDAMENTAL * (i + 1) * vibrato
            phases[i] += 2 * math.pi * freq / rate
            value += partial_amps[i] * math.sin(phases[i])
        value *= PEAK_AMP * tremolo
        samples.append(int(max(-1.0, min(1.0, value)) * 32767))

    with wave.open(out_path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(struct.pack("<%dh" % len(samples), *samples))


if __name__ == "__main__":
    main()
