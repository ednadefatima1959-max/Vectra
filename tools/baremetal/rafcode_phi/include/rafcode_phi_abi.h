#ifndef RAFCODE_PHI_ABI_H
#define RAFCODE_PHI_ABI_H

/*
 * RAFCODE❤️PHI ABI
 * C (casca) -> ASM (núcleo) com saída em palavras hex determinísticas.
 *
 * Contratos portados para o IR canônico de engine/rmr/include/rmr_ir_hex.h.
 */

/* Evita dependência de libc/stdint: tipos fixos locais. */
typedef unsigned char      raf_u8;
typedef unsigned short     raf_u16;
typedef unsigned int       raf_u32;
typedef unsigned long long raf_u64;

#define RAFPHI_IR_HEX_ABI_VERSION_U16 0x0100u

/* Mapeamento arquitetural compatível com RmR_IrArch. */
typedef enum {
  RAFPHI_ARCH_UNKNOWN = 0,
  RAFPHI_ARCH_AARCH64 = 1,
  RAFPHI_ARCH_X86_64  = 2
} rafphi_arch_t;

/* Campo canônico [31:24 opcode][23:16 flags][15:12 addr][11:8 ver][7:0 imm]. */
#define RAFPHI_IR_FIELD_OPCODE_SHIFT 24u
#define RAFPHI_IR_FIELD_FLAGS_SHIFT 16u
#define RAFPHI_IR_FIELD_ADDR_SHIFT 12u
#define RAFPHI_IR_FIELD_VER_SHIFT 8u
#define RAFPHI_IR_FIELD_IMM_SHIFT 0u

/* Opcodes hex mantidos para compatibilidade binária com rafphi_emit_word_asm. */
#define RAFPHI_OP_NOP      0x00000000u
#define RAFPHI_OP_RET_A64  0xD65F03C0u
#define RAFPHI_OP_BRK_A64  0xD4200000u
#define RAFPHI_OP_HLT_A64  0xD4400000u

/* Flags canônicas alinhadas ao IR do RMR. */
typedef enum {
  RAFPHI_FLAG_NONE = 0u,
  RAFPHI_FLAG_BAD_EVENT = (1u << 0),
  RAFPHI_FLAG_MISS = (1u << 1),
  RAFPHI_FLAG_TEMP_HINT = (1u << 2),
  RAFPHI_FLAG_ROUTE_CPU = (1u << 3),
  RAFPHI_FLAG_ROUTE_RAM = (1u << 4),
  RAFPHI_FLAG_ROUTE_DISK = (1u << 5),
  RAFPHI_FLAG_ROUTE_FALLBACK = (1u << 6),
  RAFPHI_FLAG_BRANCHLESS = (1u << 7)
} rafphi_ir_flags_t;

typedef struct {
  raf_u32 opcode_hex;
  raf_u32 flags;
} rafphi_emit_word_t;

typedef struct {
  raf_u32 accepted;
  raf_u32 rejected;
  raf_u32 crc32c;
} rafphi_emit_stats_t;

static inline raf_u32 rafphi_ir_pack_word(raf_u32 opcode_field,
                                          raf_u32 flags,
                                          raf_u32 addr_mode,
                                          raf_u32 abi_version_nibble,
                                          raf_u32 imm8) {
  return ((opcode_field & 0xFFu) << RAFPHI_IR_FIELD_OPCODE_SHIFT) |
         ((flags & 0xFFu) << RAFPHI_IR_FIELD_FLAGS_SHIFT) |
         ((addr_mode & 0x0Fu) << RAFPHI_IR_FIELD_ADDR_SHIFT) |
         ((abi_version_nibble & 0x0Fu) << RAFPHI_IR_FIELD_VER_SHIFT) |
         ((imm8 & 0xFFu) << RAFPHI_IR_FIELD_IMM_SHIFT);
}

/* Hook ASM: serializa uma palavra hex para buffer de saída. */
extern raf_u32 rafphi_emit_word_asm(raf_u32 opcode_hex, raf_u32 *out_words, raf_u32 cap_words, raf_u32 write_index);

/* Casca C: parser mínimo de mnemônicos autorais para hex. */
raf_u32 rafphi_encode_token_to_hex(const char *token, raf_u32 token_len, raf_u32 *out_hex);

/* Casca C: emite bloco de tokens em formato hex determinístico. */
rafphi_emit_stats_t rafphi_emit_block_hex(const char **tokens, const raf_u32 *token_lens, raf_u32 token_count,
                                          raf_u32 *out_words, raf_u32 cap_words);

#endif
