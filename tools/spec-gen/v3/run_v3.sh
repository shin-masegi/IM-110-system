#!/usr/bin/env bash
# run_v3.sh — full v3 build (GPU). Run only after the fan fix + your Go.
# 1. re-slice Display.c + caller state-machine (so incremental hashing reflects current code)
# 2. gen_v3 body(43) + appendices(2): Devstral reads drawing + transitions + formulas + EEPROM map
# 3. regenerate probe(B) sections in Japanese-only (fixes ① English drift)
# 4. assemble the single markdown
# Flags: pass --changed-only to rebuild ONLY sections whose source changed.
set -uo pipefail
cd "$(dirname "$0")/../../.."   # repo root
LOG=tools/spec-gen/v3/out/batch_v3.log
mkdir -p tools/spec-gen/v3/out
export SLEEP_BETWEEN="${SLEEP_BETWEEN:-45}"
GEN_ARGS="${1:-}"   # e.g. --changed-only

echo "=== v3 batch start $(date '+%F %T') (cooldown ${SLEEP_BETWEEN}s, args='${GEN_ARGS}') ===" | tee -a "$LOG"

echo "--- re-slice code + caller (routing) ---" | tee -a "$LOG"
python3 tools/spec-gen/v2/slice_funcs.py 2>&1 | tee -a "$LOG"
python3 tools/spec-gen/v2/caller_context.py 2>&1 | tee -a "$LOG"

echo "--- gen_v3 (body + appendices) ---" | tee -a "$LOG"
python3 tools/spec-gen/v3/gen_v3.py $GEN_ARGS 2>&1 | tee -a "$LOG"

echo "--- regenerate probe(B) sections in Japanese (① fix) ---" | tee -a "$LOG"
python3 tools/spec-gen/gen.py --stage gen --force \
    --only B00,B01,B02,B03,B04,B05,B06,B07 2>&1 | tee -a "$LOG"

echo "--- assemble ---" | tee -a "$LOG"
python3 tools/spec-gen/v3/assemble_v3.py 2>&1 | tee -a "$LOG"
echo "=== v3 batch done $(date '+%F %T') ===" | tee -a "$LOG"
