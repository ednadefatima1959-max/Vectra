#ifndef RMR_IR_HEX_H
#define RMR_IR_HEX_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RMR_IR_HEX_ABI_VERSION_U16 0x0100u

/* Formato de instrução canônica (32-bit):
 * [31:24] opcode field
 * [23:16] flags field
 * [15:12] addressing mode
 * [11:8]  version nibble
 * [7:0]   immediate / compact payload
 */
#define RMR_IR_FIELD_OPCODE_SHIFT 24u
#define RMR_IR_FIELD_FLAGS_SHIFT 16u
#define RMR_IR_FIELD_ADDR_SHIFT 12u
#define RMR_IR_FIELD_VER_SHIFT 8u
#define RMR_IR_FIELD_IMM_SHIFT 0u

#define RMR_IR_FIELD_OPCODE_MASK 0xFF000000u
#define RMR_IR_FIELD_FLAGS_MASK  0x00FF0000u
#define RMR_IR_FIELD_ADDR_MASK   0x0000F000u
#define RMR_IR_FIELD_VER_MASK    0x00000F00u
#define RMR_IR_FIELD_IMM_MASK    0x000000FFu

typedef enum {
  RMR_IR_ARCH_UNKNOWN = 0,
  RMR_IR_ARCH_AARCH64 = 1,
  RMR_IR_ARCH_X86_64 = 2
} RmR_IrArch;

typedef enum {
  RMR_IR_OPCODE_NOP = 0x00u,
  RMR_IR_OPCODE_RET = 0x01u,
  RMR_IR_OPCODE_BRK = 0x02u,
  RMR_IR_OPCODE_HLT = 0x03u
} RmR_IrOpcodeField;

typedef enum {
  RMR_IR_ADDR_NONE = 0u,
  RMR_IR_ADDR_IMM8 = 1u,
  RMR_IR_ADDR_CHUNK_OFFSET = 2u,
  RMR_IR_ADDR_HASH64 = 3u,
  RMR_IR_ADDR_STAGE_SIG = 4u
} RmR_IrAddrMode;

typedef enum {
  RMR_IR_FLAG_NONE = 0u,
  RMR_IR_FLAG_BAD_EVENT = (1u << 0),
  RMR_IR_FLAG_MISS = (1u << 1),
  RMR_IR_FLAG_TEMP_HINT = (1u << 2),
  RMR_IR_FLAG_ROUTE_CPU = (1u << 3),
  RMR_IR_FLAG_ROUTE_RAM = (1u << 4),
  RMR_IR_FLAG_ROUTE_DISK = (1u << 5),
  RMR_IR_FLAG_ROUTE_FALLBACK = (1u << 6),
  RMR_IR_FLAG_BRANCHLESS = (1u << 7)
} RmR_IrFlags;

#define RMR_IR_OPCODE_HEX_NOP      0x00000000u
#define RMR_IR_OPCODE_HEX_RET_A64  0xD65F03C0u
#define RMR_IR_OPCODE_HEX_BRK_A64  0xD4200000u
#define RMR_IR_OPCODE_HEX_HLT_A64  0xD4400000u

typedef struct {
  uint32_t opcode_hex;
  uint32_t flags;
} RmR_IrEmitWordAbi;

typedef struct {
  uint32_t accepted;
  uint32_t rejected;
  uint32_t crc32c;
} RmR_IrEmitStatsAbi;

typedef struct {
  uint32_t ir_word;
  uint32_t opcode_hex;
} RmR_IrHexInstruction;

static inline uint32_t RmR_IrPackWord(uint32_t opcode_field,
                                       uint32_t flags,
                                       uint32_t addr_mode,
                                       uint32_t abi_version_nibble,
                                       uint32_t imm8) {
  return ((opcode_field & 0xFFu) << RMR_IR_FIELD_OPCODE_SHIFT) |
         ((flags & 0xFFu) << RMR_IR_FIELD_FLAGS_SHIFT) |
         ((addr_mode & 0x0Fu) << RMR_IR_FIELD_ADDR_SHIFT) |
         ((abi_version_nibble & 0x0Fu) << RMR_IR_FIELD_VER_SHIFT) |
         ((imm8 & 0xFFu) << RMR_IR_FIELD_IMM_SHIFT);
}

static inline uint32_t RmR_IrNativeLittleEndian(void) {
  const uint32_t x = 0x01020304u;
  const uint8_t *p = (const uint8_t *)&x;
  return (uint32_t)(p[0] == 0x04u);
}

size_t RmR_IR_ToOpcodeHex(const RmR_IrHexInstruction *ir_seq,
                          size_t ir_count,
                          uint32_t *out_words,
                          size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif
