#ifndef RMR_ARCH_CONTRACT_H
#define RMR_ARCH_CONTRACT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RMR_ARCH_CONTRACT_VERSION 1u

#define RMR_ARCH_CALLCONV_UNKNOWN 0u
#define RMR_ARCH_CALLCONV_CDECL32 1u
#define RMR_ARCH_CALLCONV_SYSV_AMD64 2u
#define RMR_ARCH_CALLCONV_AAPCS32 3u
#define RMR_ARCH_CALLCONV_AAPCS64 4u
#define RMR_ARCH_CALLCONV_RISCV64 5u

#define RMR_ARCH_ASM_SUPPORT_NONE 0u
#define RMR_ARCH_ASM_SUPPORT_LOWLEVEL (1u << 0)
#define RMR_ARCH_ASM_SUPPORT_CASM (1u << 1)

typedef struct {
  uint32_t contract_version;
  uint32_t arch_signature;
  uint32_t register_width_bits;
  uint32_t pointer_width_bits;
  uint32_t alignment_bytes;
  uint32_t address_limit_bits;
  uint32_t calling_convention;
  uint32_t asm_support_mask;
} rmr_arch_contract_t;

#ifdef __cplusplus
}
#endif

#endif
