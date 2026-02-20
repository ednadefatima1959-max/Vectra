/* rmr_casm_riscv64.S — RISC-V 64 CASM STUBS
 * ∆RAFAELIA_CORE·Ω
 * ─────────────────────────────────────────────────────── */

    .text
    .align 2

/* rmr_casm_bridge_marker */
    .global rmr_casm_bridge_marker
    .type   rmr_casm_bridge_marker, @function
rmr_casm_bridge_marker:
    /* "CASM" = 0x4D534143 */
    lui     a0, %hi(0x4D534143)
    addi    a0, a0, %lo(0x4D534143)
    ret
    .size rmr_casm_bridge_marker, . - rmr_casm_bridge_marker

/* rmr_asm_arch_marker */
    .global rmr_asm_arch_marker
    .type   rmr_asm_arch_marker, @function
rmr_asm_arch_marker:
    /* "RV64" = 0x34365652 */
    lui     a0, %hi(0x34365652)
    addi    a0, a0, %lo(0x34365652)
    ret
    .size rmr_asm_arch_marker, . - rmr_asm_arch_marker
