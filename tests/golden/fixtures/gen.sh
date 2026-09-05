#!/usr/bin/env bash
# Generates the deterministic test-input fixtures for the golden-file
# harness. Never commit the .wav output of this script (see .gitignore) -
# regenerate it instead, in CI or locally, so the harness never depends on
# binary blobs sitting in git history.
#
# Requires: sox (with libsox-fmt-all for WAV) and python3 (stdlib only,
# used only for the one fixture - vibrato/FM - that sox's synth generator
# can't express directly).
set -euo pipefail

OUT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$OUT_DIR"

command -v sox >/dev/null 2>&1 || { echo "error: sox not found (apt: sox libsox-fmt-all)" >&2; exit 1; }
command -v python3 >/dev/null 2>&1 || { echo "error: python3 not found" >&2; exit 1; }

for RATE in 44100 48000; do
	SUFFIX="44k"
	[ "$RATE" = "48000" ] && SUFFIX="48k"

	# --- sine440: plain 440 Hz tone, 2s mono. The simplest possible case -
	# exercises the overlap-add path with nothing else going on.
	sox -n -r "$RATE" -c 1 -b 16 "sine440_2s_${SUFFIX}.wav" \
		synth 2 sine 440 gain -6

	# --- sweep: a 3s log sweep 100Hz-8kHz mono. Exercises frequency
	# tracking / oscillator-bank tools across a continuously changing
	# spectrum rather than a single fixed partial.
	sox -n -r "$RATE" -c 1 -b 16 "sweep_3s_${SUFFIX}.wav" \
		synth 3 sine 100-8000 gain -6

	# --- noise_then_tone: 0.5s white noise at -20dB, then 2s of 440Hz
	# tone with that same noise floor continuing underneath (2.5s total).
	# Exercises noisefilter's noise-floor estimation window (which is
	# meant to sample the noise-only lead-in) and its behavior once the
	# tone is added on top of a floor it already measured.
	sox -n -r "$RATE" -c 1 -b 16 "/tmp/_noise_lead.wav" \
		synth 0.5 whitenoise gain -20
	sox -n -r "$RATE" -c 1 -b 16 "/tmp/_noise_bed.wav" \
		synth 2.5 whitenoise gain -20
	sox -n -r "$RATE" -c 1 -b 16 "/tmp/_tone_only.wav" \
		synth 0.5 sine 0 : synth 2 sine 440 gain -6
	sox -m "/tmp/_noise_bed.wav" "/tmp/_tone_only.wav" \
		"noise_then_tone_${SUFFIX}.wav"
	rm -f /tmp/_noise_lead.wav /tmp/_noise_bed.wav /tmp/_tone_only.wav

	# --- stereo_two_tones: 2s stereo, left=440Hz right=660Hz (a perfect
	# fifth). Exercises per-channel processing / channel-count handling.
	sox -n -r "$RATE" -c 2 -b 16 "stereo_two_tones_${SUFFIX}.wav" \
		synth 2 sine 440 sine 660 gain -6

	# --- partials_vibrato: a "voice-like" tone - 8 harmonic partials of a
	# 220Hz fundamental (amplitude falling off with partial number) with a
	# slow 5Hz vibrato (+/-1.5% frequency modulation) and a slight 4Hz
	# amplitude tremolo, 2s mono. Exercises pitch-tracking/harmonic-
	# tracking tools (pitchtracker, formantsmapper, harmonizer, ...) against
	# a continuously time-varying fundamental, which a single fixed sine
	# can't do. sox's synth generator can't express FM directly, so this
	# one fixture is synthesized directly with a small stdlib-only script.
	python3 "$OUT_DIR/gen_partials_vibrato.py" \
		"partials_vibrato_2s_${SUFFIX}.wav" "$RATE"
done

# --- control/data files (tiny, deterministic, ASCII - written directly
# rather than needing sox/python; still regenerated here rather than
# committed so every input the harness uses comes from one script). ---

# plainpv -P@ramp.txt: 0..12 semitone ramp for a func-able parameter.
printf '0\n3\n6\n9\n12\n' > ramp.txt

# harmonizer -F: one line per harmony voice, 8 fields: shift factor, window
# low/high boundary, window CENTER (undocumented in harmonizer's own usage
# text - it's not one of the 8 fields listed there, but it's a real column;
# see docs/dev/parameter-inventory.md), peak dB, stopband dB, Q-index, delay.
# center must differ from both boundaries - harmonizer.c divides by
# (centerChannel - lowChannel) and (highChannel - centerChannel) with no
# zero-width-window guard, so e.g. center==low segfaults (found via the
# golden harness; not fixed in the tool itself since it's degenerate input,
# not a realistic default). A fifth-above and an octave-above voice, both
# frequency-multiplier shifts, centered mid-band.
cat > harmonizer_table.txt <<'EOF'
1.5 0 22050 1000 0 -96 0 0
2.0 0 22050 2000 -3 -96 0 0
EOF

# chordresponsemaker -F: sextuples (pitch, numPartials, bandwidth-prop,
# dB, partial-spacing-prop, dB-rolloff/octave) - a simple 8-partial tone
# at 220Hz.
cat > chord_table.txt <<'EOF'
220.0 8 0.05 0.0 1.0 -3.0
EOF

# filtresponsemaker -F: unordered (freq-or-octave.pitchclass, dB) duples
# describing a gentle low-pass shape. Values <=12 mean octave.pitchclass,
# so the low end uses 20Hz rather than 0 to stay unambiguously in Hz.
cat > filter_breakpoints.txt <<'EOF'
20 0
1000 0
4000 -6
10000 -24
22050 -48
EOF

echo "Generated fixtures in $OUT_DIR:"
for f in "$OUT_DIR"/*.wav; do
	soxi -V0 "$f" >/dev/null || { echo "INVALID: $f" >&2; exit 1; }
	printf '  %-32s %s\n' "$(basename "$f")" "$(soxi -D "$f")s"
done
