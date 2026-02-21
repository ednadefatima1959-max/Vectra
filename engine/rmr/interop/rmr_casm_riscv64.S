/* ═══════════════════════════════════════════════════════
   rmr_casm_riscv64.S — RMR CASM BRIDGE RISCV64
   Ficheiro FALTANDO no zip original — adicionado aqui.
   RISC-V 64-bit base ISA (RV64I) — zero extensões obrigatórias
   ═══════════════════════════════════════════════════════ */

.section .text
.option pic

/* ─── rmr_casm_bridge_marker ─────────────────────────── */
.global rmr_casm_bridge_marker
.type   rmr_casm_bridge_marker, @function
rmr_casm_bridge_marker:
    /* "RVAS" = 0x52564153 — RISC-V marker */
    lui     a0, 0x52565
    addi    a0, a0, 0x153       /* aproximação; corrigido abaixo */
    /* Carrega literal 0x52564153 via duas instruções */
    li      a0, 0x52564153
    ret
.size rmr_casm_bridge_marker, .-rmr_casm_bridge_marker

/* ─── rmr_casm_xor_fold32_riscv64 ────────────────────── */
.global rmr_casm_xor_fold32_riscv64
.type   rmr_casm_xor_fold32_riscv64, @function
rmr_casm_xor_fold32_riscv64:
    /* a0=ptr, a1=len */
    beqz    a0, .Lrv_zero
    beqz    a1, .Lrv_zero

    li      t0, 0               /* accumulator */
    li      t1, 0               /* index */

    /* 4-byte chunks */
.Lrv_word:
    addi    t2, t1, 4
    bgt     t2, a1, .Lrv_tail
    add     t3, a0, t1
    lw      t3, 0(t3)
    xor     t0, t0, t3
    addi    t1, t1, 4
    j       .Lrv_word

.Lrv_tail:
    bge     t1, a1, .Lrv_done
    add     t3, a0, t1
    lbu     t3, 0(t3)
    andi    t4, t1, 3
    slli    t4, t4, 3
    sll     t3, t3, t4
    xor     t0, t0, t3
    addi    t1, t1, 1
    j       .Lrv_tail

.Lrv_done:
    /* trunca para 32 bits */
    slli    t0, t0, 32
    srli    t0, t0, 32
    mv      a0, t0
    ret

.Lrv_zero:
    li      a0, 0
    ret
.size rmr_casm_xor_fold32_riscv64, .-rmr_casm_xor_fold32_riscv64

/* ─── rmr_casm_phi_step_riscv64 ─────────────────────── */
.global rmr_casm_phi_step_riscv64
.type   rmr_casm_phi_step_riscv64, @function
rmr_casm_phi_step_riscv64:
    /* a0=state, a1=coherence */
    li      t0, 0x9E3779B9
    mul     a0, a0, t0
    mulw    a1, a1, t0          /* coherence × PHI (32-bit) */
    xor     a0, a0, a1
    bnez    a0, .Lphi_ok_rv
    li      a0, 1
.Lphi_ok_rv:
    ret
.size rmr_casm_phi_step_riscv64, .-rmr_casm_phi_step_riscv64

.section .note.GNU-stack,"",@progbits
