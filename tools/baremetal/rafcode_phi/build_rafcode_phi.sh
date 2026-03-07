#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
CC_BIN="${CC:-cc}"
CFLAGS="${CFLAGS:--O2 -Wall -Wextra}"

mkdir -p "${BUILD_DIR}"

"${CC_BIN}" ${CFLAGS} -I"${ROOT_DIR}/include" -c "${ROOT_DIR}/c/rafcode_phi_front_shell.c" -o "${BUILD_DIR}/rafcode_phi_front_shell.o"
"${CC_BIN}" ${CFLAGS} -I"${ROOT_DIR}/include" -c "${ROOT_DIR}/asm/rafcode_phi_emit_word.S" -o "${BUILD_DIR}/rafcode_phi_emit_word.o"
"${CC_BIN}" ${CFLAGS} -I"${ROOT_DIR}/include" -c "${ROOT_DIR}/c/rafcode_phi_cli.c" -o "${BUILD_DIR}/rafcode_phi_cli.o"

"${CC_BIN}" ${CFLAGS} \
  "${BUILD_DIR}/rafcode_phi_front_shell.o" \
  "${BUILD_DIR}/rafcode_phi_emit_word.o" \
  "${BUILD_DIR}/rafcode_phi_cli.o" \
  -o "${BUILD_DIR}/rafcode_phi_cli"

printf 'ok: %s\n' "${BUILD_DIR}/rafcode_phi_cli"
