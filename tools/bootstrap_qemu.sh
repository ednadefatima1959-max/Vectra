#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════
# bootstrap_qemu.sh — RAFAELIA QEMU Bootstrap
# Δ ψ→χ→ρ→Δ→Σ→Ω  |  Auto-detect KVM/TCG | ARM64/x86_64
# Usage: ./tools/bootstrap_qemu.sh [ARCH] [MEM_MB] [IMAGE_PATH]
# ═══════════════════════════════════════════════════════════════
set -euo pipefail

ARCH="${1:-x86_64}"
MEM_MB="${2:-2048}"
IMAGE="${3:-}"
CPUS="${CPUS:-2}"
QEMU_BIN="qemu-system-${ARCH}"
FIRMWARE_DIR="$(cd "$(dirname "$0")/../app/src/main/assets/roms" && pwd)"

# ── detect KVM availability ────────────────────────────────────
USE_KVM=0
if [[ -c /dev/kvm ]] && [[ -r /dev/kvm ]] && [[ -w /dev/kvm ]]; then
    USE_KVM=1
    echo "[bootstrap] KVM detected → hardware acceleration enabled"
else
    echo "[bootstrap] KVM unavailable → TCG software emulation"
fi

# ── arch-specific firmware selection ──────────────────────────
BIOS_ARGS=()
case "${ARCH}" in
  x86_64)
    if [[ -f "${FIRMWARE_DIR}/RELEASEX64_OVMF.fd" ]]; then
        BIOS_ARGS+=(-drive "if=pflash,format=raw,readonly=on,file=${FIRMWARE_DIR}/RELEASEX64_OVMF.fd")
        if [[ -f "${FIRMWARE_DIR}/RELEASEX64_OVMF_VARS.fd" ]]; then
            BIOS_ARGS+=(-drive "if=pflash,format=raw,file=${FIRMWARE_DIR}/RELEASEX64_OVMF_VARS.fd")
        fi
    elif [[ -f "${FIRMWARE_DIR}/bios-vectras.bin" ]]; then
        BIOS_ARGS+=(-bios "${FIRMWARE_DIR}/bios-vectras.bin")
    fi
    CPU_MODEL="qemu64"
    [[ ${USE_KVM} -eq 1 ]] && CPU_MODEL="host"
    MACHINE_ARGS=(-machine "pc,accel=$([ ${USE_KVM} -eq 1 ] && echo kvm || echo tcg)")
    ;;
  aarch64)
    if [[ -f "${FIRMWARE_DIR}/QEMU_EFI.img" ]]; then
        BIOS_ARGS+=(-drive "if=pflash,format=raw,readonly=on,file=${FIRMWARE_DIR}/QEMU_EFI.img")
        if [[ -f "${FIRMWARE_DIR}/QEMU_VARS.img" ]]; then
            BIOS_ARGS+=(-drive "if=pflash,format=raw,file=${FIRMWARE_DIR}/QEMU_VARS.img")
        fi
    fi
    CPU_MODEL="cortex-a72"
    [[ ${USE_KVM} -eq 1 ]] && CPU_MODEL="host"
    MACHINE_ARGS=(-machine "virt,accel=$([ ${USE_KVM} -eq 1 ] && echo kvm || echo tcg),gic-version=3")
    ;;
  *)
    echo "[bootstrap] UNSUPPORTED arch: ${ARCH}" >&2; exit 1;;
esac

# ── virtio disk if image provided ─────────────────────────────
DISK_ARGS=()
if [[ -n "${IMAGE}" && -f "${IMAGE}" ]]; then
    DISK_ARGS+=(-drive "file=${IMAGE},if=virtio,cache=writeback,aio=threads")
fi

# ── compose final command ─────────────────────────────────────
QEMU_CMD=(
    "${QEMU_BIN}"
    "${MACHINE_ARGS[@]}"
    -cpu "${CPU_MODEL}"
    -smp "${CPUS}"
    -m "${MEM_MB}"
    -nographic
    -serial mon:stdio
    -nodefaults
    -device "virtio-net-pci,netdev=net0"
    -netdev "user,id=net0"
    "${BIOS_ARGS[@]}"
    "${DISK_ARGS[@]}"
)

echo "[bootstrap] Launching: ${QEMU_CMD[*]}"
exec "${QEMU_CMD[@]}"
