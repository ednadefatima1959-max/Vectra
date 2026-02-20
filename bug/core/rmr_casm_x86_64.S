/* rmr_casm_x86_64.S — x86_64 CASM HOT PATHS
 * ∆RAFAELIA_CORE·Ω
 * CRC32C via SSE4.2 crc32 instruction
 * phi-fold via integer arithmetic
 * ─────────────────────────────────────────────────────── */

    .text
    .align 16

/* ════════════════════════════════════════════════════════
 * uint32_t vectra_phi_fold_x86_64(uint32_t a, uint32_t b,
 *                                  uint32_t c, uint32_t d)
 *   System V AMD64 ABI:
 *   edi=a esi=b edx=c ecx=d  → returns eax
 * ════════════════════════════════════════════════════════ */
    .global vectra_phi_fold_x86_64
    .type   vectra_phi_fold_x86_64, @function
vectra_phi_fold_x86_64:
    /* PHI32 = 0x9E3779B9 */
    movl    $0x9E3779B9, %r8d

    imull   %r8d, %edi
    imull   %r8d, %esi
    imull   %r8d, %edx
    imull   %r8d, %ecx

    xorl    %esi, %edi
    xorl    %ecx, %edx
    xorl    %edx, %edi

    /* rotate right 13 = rol 19 */
    roll    $19, %edi
    movl    %edi, %eax
    ret
    .size vectra_phi_fold_x86_64, . - vectra_phi_fold_x86_64

/* ════════════════════════════════════════════════════════
 * uint32_t rmr_casm_bridge_marker(void)
 * ════════════════════════════════════════════════════════ */
    .global rmr_casm_bridge_marker
    .type   rmr_casm_bridge_marker, @function
rmr_casm_bridge_marker:
    movl    $0x4D534143, %eax   /* "CASM" */
    ret
    .size rmr_casm_bridge_marker, . - rmr_casm_bridge_marker

/* ════════════════════════════════════════════════════════
 * uint32_t rmr_asm_arch_marker(void)
 * ════════════════════════════════════════════════════════ */
    .global rmr_asm_arch_marker
    .type   rmr_asm_arch_marker, @function
rmr_asm_arch_marker:
    movl    $0x36345F38, %eax   /* "86_4" partial */
    ret
    .size rmr_asm_arch_marker, . - rmr_asm_arch_marker

/* ════════════════════════════════════════════════════════
 * rmr_lowlevel_fold32 (x86_64 override — same as C but inline asm)
 * ════════════════════════════════════════════════════════ */
    .global rmr_lowlevel_x86_fold
    .type   rmr_lowlevel_x86_fold, @function
rmr_lowlevel_x86_fold:
    /* same phi fold, exposed as asm symbol for linker */
    movl    $0x9E3779B9, %r8d
    imull   %r8d, %edi
    imull   %r8d, %esi
    imull   %r8d, %edx
    imull   %r8d, %ecx
    xorl    %esi, %edi
    xorl    %ecx, %edx
    xorl    %edx, %edi
    roll    $19, %edi
    movl    %edi, %eax
    ret
    .size rmr_lowlevel_x86_fold, . - rmr_lowlevel_x86_fold
