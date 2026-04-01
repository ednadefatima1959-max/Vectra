#!/usr/bin/env bash
# HOTFIX: TAREFA A — valida alinhamento entre manifestos canônico/legacy
# Autor: ∆RafaelVerboΩ | Data: 2026-04-01
# Arquivo: tools/verify_rmr_source_alignment.sh
# Verificado contra: engine/rmr/sources.cmake, engine/rmr/sources_rmr_core.cmake
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

python3 tools/sync_engine_sources.py --check
python3 tools/verify_engine_source_parity.py

python3 - <<'PY'
import pathlib, re, sys
root = pathlib.Path('.')
legacy = (root/'engine/rmr/sources.cmake').read_text(encoding='utf-8')
core = (root/'engine/rmr/sources_rmr_core.cmake').read_text(encoding='utf-8')

m_core = re.search(r"set\(RMR_SOURCE_GROUP_CORE\s*(.*?)\)", core, re.S)
if not m_core:
    print('[rmr-manifest] missing RMR_SOURCE_GROUP_CORE', file=sys.stderr)
    sys.exit(1)
if 'engine/rmr/src/bitraf.c' in m_core.group(1):
    print('[rmr-manifest] bitraf.c found in RMR_SOURCE_GROUP_CORE (forbidden)', file=sys.stderr)
    sys.exit(1)

if 'engine/rmr/interop/rmr_lowlevel_x86_64.S' not in core or 'engine/rmr/interop/rmr_casm_x86_64.S' not in core:
    print('[rmr-manifest] x86_64 ASM group incomplete in sources_rmr_core.cmake', file=sys.stderr)
    sys.exit(1)

m_legacy = re.search(r"set\(RMR_ENGINE_CORE_SOURCES\s*(.*?)\)", legacy, re.S)
if not m_legacy:
    print('[rmr-manifest] missing RMR_ENGINE_CORE_SOURCES', file=sys.stderr)
    sys.exit(1)
if 'engine/rmr/src/bitraf.c' in m_legacy.group(1):
    print('[rmr-manifest] bitraf.c still present in sources.cmake core', file=sys.stderr)
    sys.exit(1)

print('[rmr-manifest] core manifest parity OK')
PY

echo "[rmr-manifest] source alignment verified"
