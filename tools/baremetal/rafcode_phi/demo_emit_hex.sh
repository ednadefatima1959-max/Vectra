#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
"${ROOT_DIR}/build_rafcode_phi.sh"

"${ROOT_DIR}/build/rafcode_phi_cli" NOP RET BRK HLT
