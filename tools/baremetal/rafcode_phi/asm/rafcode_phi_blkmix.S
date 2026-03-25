/* Módulo ASM dedicado: cópia/mix de bloco. */
#if defined(__aarch64__)
    .text
    .align 2
    .global rafphi_emit_blkmix_word_asm
    .type rafphi_emit_blkmix_word_asm, %function
rafphi_emit_blkmix_word_asm:
    /* w0=mix_flags, w1=span8 */
    and     w0, w0, #0xFF
    and     w1, w1, #0xFF
    mov     w2, #0x43
    lsl     w2, w2, #24
    orr     w2, w2, w0, lsl #16
    mov     w3, #0x2
    lsl     w3, w3, #8
    orr     w0, w2, w3
    orr     w0, w0, w1
    ret
#endif

#if defined(__x86_64__)
    .text
    .global rafphi_emit_blkmix_word_asm
    .type rafphi_emit_blkmix_word_asm, @function
rafphi_emit_blkmix_word_asm:
    /* edi=mix_flags, esi=span8 */
    andl    $0xFF, %edi
    andl    $0xFF, %esi
    movl    $0x43, %eax
    shll    $24, %eax
    movl    %edi, %edx
    shll    $16, %edx
    orl     %edx, %eax
    movl    $0x2, %edx
    shll    $8, %edx
    orl     %edx, %eax
    orl     %esi, %eax
    ret
#endif

#if defined(__ELF__)
    .section .note.GNU-stack,"",@progbits
#endif
