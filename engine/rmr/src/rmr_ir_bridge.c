#include "rmr_ir_bridge.h"

static uint32_t rmr_ir_opcode_to_hex(uint32_t opcode_field) {
  if (opcode_field == (uint32_t)RMR_IR_OPCODE_RET) return RMR_IR_OPCODE_HEX_RET_A64;
  if (opcode_field == (uint32_t)RMR_IR_OPCODE_BRK) return RMR_IR_OPCODE_HEX_BRK_A64;
  if (opcode_field == (uint32_t)RMR_IR_OPCODE_HLT) return RMR_IR_OPCODE_HEX_HLT_A64;
  return RMR_IR_OPCODE_HEX_NOP;
}

static uint32_t rmr_ir_route_flag(const RmR_ChunkMeta *meta) {
  if (meta->route_id == (uint8_t)RMR_ROUTE_CPU) return (uint32_t)RMR_IR_FLAG_ROUTE_CPU;
  if (meta->route_id == (uint8_t)RMR_ROUTE_RAM) return (uint32_t)RMR_IR_FLAG_ROUTE_RAM;
  if (meta->route_id == (uint8_t)RMR_ROUTE_DISK) return (uint32_t)RMR_IR_FLAG_ROUTE_DISK;
  return (uint32_t)RMR_IR_FLAG_ROUTE_FALLBACK;
}

static uint32_t rmr_ir_opcode_for_meta(RmR_Stage stage, const RmR_ChunkMeta *meta) {
  if (meta->flags.bad_event || meta->flags.miss) return (uint32_t)RMR_IR_OPCODE_BRK;
  if (stage == RMR_STAGE_AUDIT || stage == RMR_STAGE_VERIFY) return (uint32_t)RMR_IR_OPCODE_HLT;
  if (meta->decision_mode == (uint8_t)RMR_DECISION_MODE_BRANCHLESS) return (uint32_t)RMR_IR_OPCODE_RET;
  return (uint32_t)RMR_IR_OPCODE_NOP;
}

size_t RmR_IR_FromPolicyChunkMeta(RmR_Stage stage,
                                  const RmR_ChunkMeta *meta,
                                  RmR_IrHexInstruction *out_seq,
                                  size_t out_cap) {
  uint32_t flags;
  uint32_t opcode;
  uint32_t abi_nibble;
  uint32_t word0;
  uint32_t word1;
  uint32_t word2;
  uint32_t word3;

  if (!meta || !out_seq || out_cap < 4u) return 0u;

  flags = rmr_ir_route_flag(meta);
  if (meta->flags.bad_event) flags |= (uint32_t)RMR_IR_FLAG_BAD_EVENT;
  if (meta->flags.miss) flags |= (uint32_t)RMR_IR_FLAG_MISS;
  if (meta->flags.temp_hint) flags |= (uint32_t)RMR_IR_FLAG_TEMP_HINT;
  if (meta->decision_mode == (uint8_t)RMR_DECISION_MODE_BRANCHLESS) flags |= (uint32_t)RMR_IR_FLAG_BRANCHLESS;

  opcode = rmr_ir_opcode_for_meta(stage, meta);
  abi_nibble = (uint32_t)((RMR_IR_HEX_ABI_VERSION_U16 >> 8u) & 0x0Fu);

  word0 = RmR_IrPackWord(opcode, flags, (uint32_t)RMR_IR_ADDR_CHUNK_OFFSET, abi_nibble,
                         (uint32_t)(meta->offset & 0xFFu));
  word1 = RmR_IrPackWord(opcode, flags, (uint32_t)RMR_IR_ADDR_IMM8, abi_nibble,
                         (uint32_t)(meta->size & 0xFFu));
  word2 = RmR_IrPackWord(opcode, flags, (uint32_t)RMR_IR_ADDR_HASH64, abi_nibble,
                         (uint32_t)(meta->hash64 & 0xFFu));
  word3 = RmR_IrPackWord(opcode, flags, (uint32_t)RMR_IR_ADDR_STAGE_SIG, abi_nibble,
                         (uint32_t)(((uint32_t)stage) & 0xFFu));

  out_seq[0].ir_word = word0;
  out_seq[0].opcode_hex = rmr_ir_opcode_to_hex(opcode);
  out_seq[1].ir_word = word1;
  out_seq[1].opcode_hex = rmr_ir_opcode_to_hex(opcode);
  out_seq[2].ir_word = word2;
  out_seq[2].opcode_hex = rmr_ir_opcode_to_hex(opcode);
  out_seq[3].ir_word = word3;
  out_seq[3].opcode_hex = rmr_ir_opcode_to_hex(opcode);

  return 4u;
}

size_t RmR_IR_ToOpcodeHex(const RmR_IrHexInstruction *ir_seq,
                          size_t ir_count,
                          uint32_t *out_words,
                          size_t out_cap) {
  size_t i;
  size_t n;

  if (!ir_seq || !out_words || out_cap == 0u) return 0u;

  n = (ir_count < out_cap) ? ir_count : out_cap;
  for (i = 0u; i < n; ++i) {
    out_words[i] = ir_seq[i].opcode_hex;
  }
  return n;
}
