.PHONY: build demo

CARGO := cargo
PVC := rust/target/release/pvc
FIXTURE := examples/output/input.wav
OUT_DIR := examples/output
PRESETS := stretch pitch ring-reverb denoise spectwarp-compress

build:
	cd rust && $(CARGO) build --release -p pvc-cli

# Generates a test tone, then runs every preset under examples/presets/
# against it - see docs/presets.md and examples/README.md. Requires sox
# (same dependency tests/golden/fixtures/gen.sh already uses).
demo: build
	@command -v sox >/dev/null 2>&1 || { echo "error: sox not found (apt: sox libsox-fmt-all)" >&2; exit 1; }
	@mkdir -p $(OUT_DIR)
	sox -n -r 44100 -c 1 -b 16 $(FIXTURE) synth 2 sine 440 gain -6
	@echo ""
	@echo "Input: $(FIXTURE)"
	@for p in $(PRESETS); do \
		echo ""; \
		echo "== $$p =="; \
		$(PVC) run examples/presets/$$p.toml \
			--set input=$(FIXTURE) --set output=$(OUT_DIR)/$$p.wav --quiet; \
		$(PVC) info $(OUT_DIR)/$$p.wav; \
	done
	@echo ""
	@echo "Demo output written to $(OUT_DIR)/. Play each file and compare it against $(FIXTURE)."
