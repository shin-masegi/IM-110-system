#!/usr/bin/env bash
# run_v2.sh — full v2 batch: Devstral reads Display.c per screen and writes the spec.
# 45s cooldown between sections (GPU thermal relief; fan control is unreliable).
# Resumable and failure-tolerant; logs to out/batch_v2.log + out/gen_v2_log.jsonl.
set -uo pipefail
cd "$(dirname "$0")/../../.."   # repo root
LOG=tools/spec-gen/v2/out/batch_v2.log
mkdir -p tools/spec-gen/v2/out
export SLEEP_BETWEEN="${SLEEP_BETWEEN:-45}"

echo "=== v2 batch start $(date '+%F %T') (cooldown ${SLEEP_BETWEEN}s) ===" | tee -a "$LOG"
python3 tools/spec-gen/v2/gen_v2.py 2>&1 | tee -a "$LOG"
echo "--- assemble ---" | tee -a "$LOG"
python3 tools/spec-gen/v2/assemble_v2.py 2>&1 | tee -a "$LOG"
echo "=== v2 batch done $(date '+%F %T') ===" | tee -a "$LOG"
