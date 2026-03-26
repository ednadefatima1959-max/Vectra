#!/usr/bin/env bash
set -e

echo "=== VECTRA PROBE START ==="

echo "[N] Normalizer (structure)"
find . -type d | sort | head -n 20

echo "[E] Invariants (file count)"
COUNT=$(find . -type f | wc -l)
echo "files=$COUNT"

echo "[V] BitOmega (hash snapshot)"
git ls-files | sort | sha256sum || true

echo "[T] Matrix (top folders)"
du -h --max-depth=1 | sort -h | tail -n 10

echo "[D] Decision"
echo "status=ok"
