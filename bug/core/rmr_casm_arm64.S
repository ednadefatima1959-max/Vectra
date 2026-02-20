/* rmr_casm_arm64.S — ARM64 CASM HOT PATHS
 * ∆RAFAELIA_CORE·Ω
 * CRC32C HW (armv8-a+crc)
 * phi-fold NEON dispatch
 * ─────────────────────────────────────────────────────── */

    .arch armv8-a+crc
    .text
    .align 4

/* ════════════════════════════════════════════════════════
 * zipraf_crc32c_arm64(uint32_t crc, const uint8_t *data, uint32_t len)
 *   x0 = crc (seed)
 *   x1 = data pointer
 *   x2 = len
 *   returns x0 = result
 * ════════════════════════════════════════════════════════ */
    .global zipraf_crc32c_arm64
    .type   zipraf_crc32c_arm64, %function
zipraf_crc32c_arm64:
    cbz     x2, .Lcrc_done

    /* Process 8 bytes at a time */
.Lcrc_loop8:
    cmp     x2, #8
    b.lt    .Lcrc_tail4
    ldr     x3, [x1], #8
    crc32cx w0, w0, x3
    sub     x2, x2, #8
    b       .Lcrc_loop8

    /* 4-byte chunk */
.Lcrc_tail4:
    cmp     x2, #4
    b.lt    .Lcrc_tail1
    ldr     w3, [x1], #4
    crc32cw w0, w0, w3
    sub     x2, x2, #4

    /* 1-byte tail */
.Lcrc_tail1:
    cbz     x2, .Lcrc_done
.Lcrc_loop1:
    ldrb    w3, [x1], #1
    crc32cb w0, w0, w3
    subs    x2, x2, #1
    b.ne    .Lcrc_loop1

.Lcrc_done:
    ret
    .size zipraf_crc32c_arm64, . - zipraf_crc32c_arm64

/* ════════════════════════════════════════════════════════
 * vectra_phi_fold_arm64(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
 *   w0=a w1=b w2=c w3=d
 *   PHI32 = 0x9E3779B9
 *   returns w0 = fold result
 * ════════════════════════════════════════════════════════ */
    .global vectra_phi_fold_arm64
    .type   vectra_phi_fold_arm64, %function
vectra_phi_fold_arm64:
    /* Load PHI32 */
    movz    w4, #0x79B9
    movk    w4, #0x9E37, lsl #16

    /* multiply each lane by PHI32 */
    mul     w0, w0, w4
    mul     w1, w1, w4
    mul     w2, w2, w4
    mul     w3, w3, w4

    /* fold: (a^b) ^ (c^d) */
    eor     w0, w0, w1
    eor     w2, w2, w3
    eor     w0, w0, w2

    /* rotate right 13 */
    ror     w0, w0, #13
    ret
    .size vectra_phi_fold_arm64, . - vectra_phi_fold_arm64

/* ════════════════════════════════════════════════════════
 * rmr_casm_bridge_marker() → uint32_t "CASM"
 * ════════════════════════════════════════════════════════ */
    .global rmr_casm_bridge_marker
    .type   rmr_casm_bridge_marker, %function
rmr_casm_bridge_marker:
    /* "CASM" = 0x4D534143 */
    movz    w0, #0x4143
    movk    w0, #0x4D53, lsl #16
    ret
    .size rmr_casm_bridge_marker, . - rmr_casm_bridge_marker

/* ════════════════════════════════════════════════════════
 * rmr_lowlevel_x86_64 stub for ARM64 compat
 * ════════════════════════════════════════════════════════ */
    .global rmr_asm_arch_marker
    .type   rmr_asm_arch_marker, %function
rmr_asm_arch_marker:
    /* "ARM8" = 0x384D5241 */
    movz    w0, #0x5241
    movk    w0, #0x384D, lsl #16
    ret
    .size rmr_asm_arch_marker, . - rmr_asm_arch_marker
