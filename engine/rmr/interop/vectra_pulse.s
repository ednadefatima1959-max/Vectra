/*
 * vectra_pulse.s
 *
 * Arquitetura alvo: x86_64 (AMD64)
 * ABI: System V AMD64 ABI
 * Contrato de símbolos exportados:
 *   - rmr_vectra_pulse_u32_x86_64(uint32_t seed) -> uint32_t
 *     Entrada em %edi (32-bit), retorno em %eax (32-bit).
 *   - rmr_vectra_pulse_abi_tag_x86_64(void) -> uint32_t
 *     Retorna tag de contrato "VP64" (0x56503634).
 */

.text

.global rmr_vectra_pulse_u32_x86_64
.type rmr_vectra_pulse_u32_x86_64, @function
rmr_vectra_pulse_u32_x86_64:
  mov %edi, %eax
  xor $0x9e3779b9, %eax

  mov %eax, %ecx
  shl $13, %ecx
  xor %ecx, %eax

  mov %eax, %ecx
  shr $17, %ecx
  xor %ecx, %eax

  mov %eax, %ecx
  shl $5, %ecx
  xor %ecx, %eax

  xor $0xa5a5a5a5, %eax
  ret
.size rmr_vectra_pulse_u32_x86_64, .-rmr_vectra_pulse_u32_x86_64

.global rmr_vectra_pulse_abi_tag_x86_64
.type rmr_vectra_pulse_abi_tag_x86_64, @function
rmr_vectra_pulse_abi_tag_x86_64:
  mov $0x56503634, %eax
  ret
.size rmr_vectra_pulse_abi_tag_x86_64, .-rmr_vectra_pulse_abi_tag_x86_64

.section .note.GNU-stack,"",@progbits
