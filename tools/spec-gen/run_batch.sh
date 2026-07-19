#!/usr/bin/env bash
# run_batch.sh — full IM-110 spec generation batch against the 77 server.
# Two-stage (Devstral drafts -> mistralprism reviews) to keep each model warm,
# then assemble into the single markdown. Idempotent/resumable: re-running
# skips sections already drafted/reviewed. Logs to out/batch.log.
set -uo pipefail
cd "$(dirname "$0")/../.."   # repo root: IM-110-system/
LOG=tools/spec-gen/out/batch.log
mkdir -p tools/spec-gen/out

# Thermal relief for the 77 server GPU (6900XT throttles under sustained load).
# Idle this many seconds between sections; model stays warm in VRAM. Override via env.
export SLEEP_BETWEEN="${SLEEP_BETWEEN:-25}"
echo "cooldown between sections: ${SLEEP_BETWEEN}s" | tee -a "$LOG"

echo "=== batch start $(date '+%F %T') ===" | tee -a "$LOG"

echo "--- stage 1: Devstral drafts ---" | tee -a "$LOG"
python3 tools/spec-gen/gen.py --stage gen 2>&1 | tee -a "$LOG"

echo "--- stage 2: mistralprism reviews ---" | tee -a "$LOG"
python3 tools/spec-gen/gen.py --stage review 2>&1 | tee -a "$LOG"

echo "--- stage 3: assemble ---" | tee -a "$LOG"
python3 tools/spec-gen/assemble.py 2>&1 | tee -a "$LOG"
# also keep a raw (pre-review) assembly for A/B comparison
python3 tools/spec-gen/assemble.py --raw 2>&1 | tee -a "$LOG"
cp "docs/specs/IM-110プログラム仕様書.md" "tools/spec-gen/out/assembled_raw.md" 2>/dev/null || true
python3 tools/spec-gen/assemble.py 2>&1 | tee -a "$LOG"   # restore reviewed version as the canonical file

echo "=== batch done $(date '+%F %T') ===" | tee -a "$LOG"
