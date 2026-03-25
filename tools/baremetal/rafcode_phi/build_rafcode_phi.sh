#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
CC_BIN="${CC:-cc}"
CFLAGS="${CFLAGS:--O2 -Wall -Wextra}"

mkdir -p "${BUILD_DIR}"

HOST_ARCH="${RAFPHI_FORCE_ARCH:-$(uname -m 2>/dev/null || echo unknown)}"
HWCAPS=""
if [[ -r /proc/cpuinfo ]]; then
  HWCAPS="$(grep -m1 -E 'Features|flags' /proc/cpuinfo | sed 's/^[^:]*:[[:space:]]*//' || true)"
fi

BACKEND_KIND="c"
BACKEND_ARCH="unknown"
BACKEND_SOURCES=(
  "${ROOT_DIR}/c/rafcode_phi_emit_word_c.c"
)

case "${HOST_ARCH}" in
  aarch64|arm64)
    BACKEND_KIND="asm"
    BACKEND_ARCH="aarch64"
    BACKEND_SOURCES=(
      "${ROOT_DIR}/asm/rafcode_phi_emit_word.S"
      "${ROOT_DIR}/asm/rafcode_phi_compare.S"
      "${ROOT_DIR}/asm/rafcode_phi_branch.S"
      "${ROOT_DIR}/asm/rafcode_phi_loop.S"
      "${ROOT_DIR}/asm/rafcode_phi_blkmix.S"
    )
    ;;
  x86_64|amd64)
    BACKEND_KIND="asm"
    BACKEND_ARCH="x86_64"
    BACKEND_SOURCES=(
      "${ROOT_DIR}/asm/rafcode_phi_emit_word.S"
      "${ROOT_DIR}/asm/rafcode_phi_compare.S"
      "${ROOT_DIR}/asm/rafcode_phi_branch.S"
      "${ROOT_DIR}/asm/rafcode_phi_loop.S"
      "${ROOT_DIR}/asm/rafcode_phi_blkmix.S"
    )
    ;;
  riscv64)
    BACKEND_KIND="c"
    BACKEND_ARCH="riscv64"
    ;;
  *)
    BACKEND_KIND="c"
    BACKEND_ARCH="unknown"
    ;;
esac

printf 'rafcode_phi.build.host_arch=%s\n' "${HOST_ARCH}"
printf 'rafcode_phi.build.backend_kind=%s\n' "${BACKEND_KIND}"
printf 'rafcode_phi.build.backend_arch=%s\n' "${BACKEND_ARCH}"
printf 'rafcode_phi.build.backend_src=%s\n' "${BACKEND_SOURCES[*]}"
printf 'rafcode_phi.build.hwcaps=%s\n' "${HWCAPS}"

"${CC_BIN}" ${CFLAGS} -I"${ROOT_DIR}/include" -c "${ROOT_DIR}/c/rafcode_phi_front_shell.c" -o "${BUILD_DIR}/rafcode_phi_front_shell.o"

backend_objects=()
for src in "${BACKEND_SOURCES[@]}"; do
  obj="${BUILD_DIR}/$(basename "${src}").o"
  "${CC_BIN}" ${CFLAGS} -I"${ROOT_DIR}/include" -c "${src}" -o "${obj}"
  backend_objects+=("${obj}")
done

"${CC_BIN}" ${CFLAGS} -I"${ROOT_DIR}/include" -c "${ROOT_DIR}/c/rafcode_phi_cli.c" -o "${BUILD_DIR}/rafcode_phi_cli.o"

"${CC_BIN}" ${CFLAGS} \
  "${BUILD_DIR}/rafcode_phi_front_shell.o" \
  "${backend_objects[@]}" \
  "${BUILD_DIR}/rafcode_phi_cli.o" \
  -o "${BUILD_DIR}/rafcode_phi_cli"

printf 'ok: %s\n' "${BUILD_DIR}/rafcode_phi_cli"
