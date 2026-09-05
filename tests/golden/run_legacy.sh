#!/usr/bin/env bash
# Thin wrapper: builds+installs the legacy toolkit if it isn't already at
# $PVC_LEGACY_BIN (default /opt/pvc-legacy/bin), regenerates the fixtures,
# then runs every case in tests/golden/cases/ against it, recording
# results under tests/golden/expected/. See run_legacy.py for the actual
# per-case logic and compare.py for what checks a candidate (Phase 3+)
# against what's recorded here.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PVC_LEGACY_BIN="${PVC_LEGACY_BIN:-/opt/pvc-legacy/bin}"

if [ ! -d "$PVC_LEGACY_BIN" ]; then
	echo "Building legacy toolkit (not found at $PVC_LEGACY_BIN)..."
	cmake -S "$REPO_ROOT/legacy" -B /tmp/golden_build -DCMAKE_BUILD_TYPE=Release
	cmake --build /tmp/golden_build -j"$(nproc)"
	cmake --install /tmp/golden_build --prefix /opt/pvc-legacy
	PVC_LEGACY_BIN=/opt/pvc-legacy/bin
fi

if ! ls "$REPO_ROOT/tests/golden/fixtures"/*.wav >/dev/null 2>&1; then
	echo "Generating fixtures..."
	bash "$REPO_ROOT/tests/golden/fixtures/gen.sh"
fi

python3 "$REPO_ROOT/tests/golden/run_legacy.py" --bin-dir "$PVC_LEGACY_BIN" --keep-going "$@"
