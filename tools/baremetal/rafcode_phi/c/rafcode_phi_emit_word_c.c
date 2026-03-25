#include "rafcode_phi_abi.h"

/* Fallback portátil quando backend ASM não está disponível no host. */
raf_u32 rafphi_emit_word_asm(raf_u32 opcode_hex, raf_u32 *out_words, raf_u32 cap_words, raf_u32 write_index) {
  if (!out_words) {
    return 0u;
  }
  if (write_index >= cap_words) {
    return 0u;
  }
  out_words[write_index] = opcode_hex;
  return 1u;
}

raf_u32 rafphi_emit_cmp_word_asm(raf_u32 cmp_flags, raf_u32 imm8) {
  return rafphi_ir_pack_word(RAFPHI_FLOWOP_COMPARE, cmp_flags, 0u, RAFPHI_OPVER_FLOW_V1, imm8);
}

raf_u32 rafphi_emit_branch_word_asm(raf_u32 branch_flags, raf_u32 rel8) {
  return rafphi_ir_pack_word(RAFPHI_FLOWOP_BRANCH, branch_flags, 0u, RAFPHI_OPVER_FLOW_V1, rel8);
}

raf_u32 rafphi_emit_loop_word_asm(raf_u32 loop_flags, raf_u32 count8) {
  return rafphi_ir_pack_word(RAFPHI_FLOWOP_LOOP, loop_flags, 0u, RAFPHI_OPVER_FLOW_V1, count8);
}

raf_u32 rafphi_emit_blkmix_word_asm(raf_u32 mix_flags, raf_u32 span8) {
  return rafphi_ir_pack_word(RAFPHI_FLOWOP_BLKMIX, mix_flags, 0u, RAFPHI_OPVER_FLOW_V1, span8);
}
