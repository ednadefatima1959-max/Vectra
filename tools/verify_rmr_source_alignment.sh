#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

python3 tools/sync_engine_sources.py --check
python3 tools/verify_bug_core_promotion.py --report reports/promotion_governance_report.md
python3 tools/verify_engine_source_parity.py

if ! rg -Fq 'include(${CMAKE_SOURCE_DIR}/engine/rmr/sources.cmake)' CMakeLists.txt; then
  echo "[rmr-manifest] root CMakeLists.txt does not include canonical manifest" >&2
  exit 1
fi

if ! rg -Fq 'include engine/rmr/sources.mk' Makefile; then
  echo "[rmr-manifest] Makefile does not include generated canonical mk manifest" >&2
  exit 1
fi

if ! rg -Fq 'include(${VECTRA_REPO_ROOT}/engine/rmr/sources.cmake)' app/src/main/cpp/CMakeLists.txt; then
  echo "[rmr-manifest] Android CMakeLists.txt does not include canonical manifest" >&2
  exit 1
fi

echo "[rmr-manifest] source alignment verified"
