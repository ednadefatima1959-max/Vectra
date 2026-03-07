/* rmr_asm_x86_64.S - Primitivas ASM bare-metal x86_64
 * AT&T/GAS syntax. Comments: slash-star or hash only.
 * Build: gcc -c rmr_asm_x86_64.S -o rmr_asm_x86_64.o
 */
.section .note.GNU-stack,"",@progbits
.text

/* rmr_rdtsc: read TSC 64-bit. uint64_t rmr_rdtsc(void) */
.global rmr_rdtsc
.type   rmr_rdtsc, @function
.p2align 6
rmr_rdtsc:
    rdtsc
    shl  $32, %rdx
    or   %rdx, %rax
    ret
.size rmr_rdtsc, .-rmr_rdtsc

/* rmr_cpuid_family: EBX<<32|EAX from cpuid(1). uint64_t rmr_cpuid_family(void) */
.global rmr_cpuid_family
.type   rmr_cpuid_family, @function
.p2align 6
rmr_cpuid_family:
    push %rbx
    mov  $1, %eax
    xor  %ecx, %ecx
    cpuid
    shl  $32, %rbx
    or   %rbx, %rax
    pop  %rbx
    ret
.size rmr_cpuid_family, .-rmr_cpuid_family

/* rmr_prefetch_line: L1 prefetch. void rmr_prefetch_line(const void *ptr) */
.global rmr_prefetch_line
.type   rmr_prefetch_line, @function
.p2align 6
rmr_prefetch_line:
    prefetcht0 (%rdi)
    ret
.size rmr_prefetch_line, .-rmr_prefetch_line

/* rmr_clflush: flush cache line. void rmr_clflush(void *ptr) */
.global rmr_clflush
.type   rmr_clflush, @function
.p2align 6
rmr_clflush:
    clflush (%rdi)
    ret
.size rmr_clflush, .-rmr_clflush

/* rmr_mfence: full memory barrier. void rmr_mfence(void) */
.global rmr_mfence
.type   rmr_mfence, @function
.p2align 6
rmr_mfence:
    mfence
    ret
.size rmr_mfence, .-rmr_mfence

/* rmr_xor_block64: XOR 64 bytes via GPR.
 * void rmr_xor_block64(uint8_t *dst, const uint8_t *src)
 * 8 GPR loads + 8 xor-mem = exactly 1 cache line */
.global rmr_xor_block64
.type   rmr_xor_block64, @function
.p2align 6
rmr_xor_block64:
    mov  0x00(%rsi), %r8
    mov  0x08(%rsi), %r9
    mov  0x10(%rsi), %r10
    mov  0x18(%rsi), %r11
    mov  0x20(%rsi), %r12
    mov  0x28(%rsi), %r13
    mov  0x30(%rsi), %r14
    mov  0x38(%rsi), %r15
    xor  %r8,  0x00(%rdi)
    xor  %r9,  0x08(%rdi)
    xor  %r10, 0x10(%rdi)
    xor  %r11, 0x18(%rdi)
    xor  %r12, 0x20(%rdi)
    xor  %r13, 0x28(%rdi)
    xor  %r14, 0x30(%rdi)
    xor  %r15, 0x38(%rdi)
    ret
.size rmr_xor_block64, .-rmr_xor_block64

/* rmr_rotr32: ROR 32-bit. 1 cycle latency.
 * uint32_t rmr_rotr32(uint32_t x, int n) RDI=x RSI=n -> EAX */
.global rmr_rotr32
.type   rmr_rotr32, @function
.p2align 6
rmr_rotr32:
    mov  %edi, %eax
    mov  %esi, %ecx
    ror  %cl, %eax
    ret
.size rmr_rotr32, .-rmr_rotr32

/* rmr_popcnt64: hardware popcount. uint64_t rmr_popcnt64(uint64_t x) */
.global rmr_popcnt64
.type   rmr_popcnt64, @function
.p2align 6
rmr_popcnt64:
    popcnt %rdi, %rax
    ret
.size rmr_popcnt64, .-rmr_popcnt64
