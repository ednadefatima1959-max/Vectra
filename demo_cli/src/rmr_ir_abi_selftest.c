#include "rmr_ir_bridge.h"
#include "rafcode_phi_abi.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

extern raf_u32 rafphi_emit_word_asm(raf_u32 opcode_hex, raf_u32 *out_words, raf_u32 cap_words, raf_u32 write_index);

#if defined(__cplusplus)
#define RMR_IR_STATIC_ASSERT(COND, MSG) static_assert((COND), MSG)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define RMR_IR_STATIC_ASSERT(COND, MSG) _Static_assert((COND), MSG)
#else
#define RMR_IR_STATIC_ASSERT(COND, MSG) typedef char rmr_ir_static_assertion_##__LINE__[(COND) ? 1 : -1]
#endif

RMR_IR_STATIC_ASSERT(sizeof(RmR_IrEmitWordAbi) == sizeof(rafphi_emit_word_t), "emit_word ABI size mismatch");
RMR_IR_STATIC_ASSERT(sizeof(RmR_IrEmitStatsAbi) == sizeof(rafphi_emit_stats_t), "emit_stats ABI size mismatch");
RMR_IR_STATIC_ASSERT(offsetof(RmR_IrEmitWordAbi, opcode_hex) == offsetof(rafphi_emit_word_t, opcode_hex), "opcode field offset mismatch");
RMR_IR_STATIC_ASSERT(offsetof(RmR_IrEmitWordAbi, flags) == offsetof(rafphi_emit_word_t, flags), "flags field offset mismatch");
RMR_IR_STATIC_ASSERT((uint32_t)RMR_IR_OPCODE_HEX_RET_A64 == (uint32_t)RAFPHI_OP_RET_A64, "RET opcode mismatch");
RMR_IR_STATIC_ASSERT((uint32_t)RMR_IR_OPCODE_HEX_BRK_A64 == (uint32_t)RAFPHI_OP_BRK_A64, "BRK opcode mismatch");
RMR_IR_STATIC_ASSERT((uint32_t)RMR_IR_OPCODE_HEX_HLT_A64 == (uint32_t)RAFPHI_OP_HLT_A64, "HLT opcode mismatch");

static int expect_u32(const char *name, uint32_t lhs, uint32_t rhs) {
  if (lhs != rhs) {
    fprintf(stderr, "FAIL: %s lhs=0x%08x rhs=0x%08x\n", name, lhs, rhs);
    return 1;
  }
  return 0;
}

static int expect_bytes_le32(const char *name, uint32_t value, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
  const uint8_t *p = (const uint8_t *)&value;
  if (p[0] != b0 || p[1] != b1 || p[2] != b2 || p[3] != b3) {
    fprintf(stderr,
            "FAIL: %s got=[%02x %02x %02x %02x] exp=[%02x %02x %02x %02x]\n",
            name,
            (unsigned)p[0],
            (unsigned)p[1],
            (unsigned)p[2],
            (unsigned)p[3],
            (unsigned)b0,
            (unsigned)b1,
            (unsigned)b2,
            (unsigned)b3);
    return 1;
  }
  return 0;
}

int main(void) {
  int failed = 0;
  RmR_ChunkMeta meta;
  RmR_IrHexInstruction ir_seq[8];
  uint32_t out_words[8];
  size_t ir_count;
  size_t hex_count;

  meta.offset = 0x1122334455667788ull;
  meta.size = 0x00000080u;
  meta.crc32c = 0xA1B2C3D4u;
  meta.hash64 = 0xCAFEBABEDEADBEEFull;
  meta.stage_signature = 0x1020304050607080ull;
  meta.entropy_milli = 725u;
  meta.math_signature = 0x55AA13F0u;
  meta.route_id = (uint8_t)RMR_ROUTE_RAM;
  meta.domain_hint = 3u;
  meta.decision_mode = (uint8_t)RMR_DECISION_MODE_BRANCHLESS;
  meta.route_target = "RAM";
  meta.flags.bad_event = 0u;
  meta.flags.miss = 0u;
  meta.flags.temp_hint = 1u;

  ir_count = RmR_IR_FromPolicyChunkMeta(RMR_STAGE_APPLY, &meta, ir_seq, 8u);
  failed += expect_u32("ir_count", (uint32_t)ir_count, 4u);

  hex_count = RmR_IR_ToOpcodeHex(ir_seq, ir_count, out_words, 8u);
  failed += expect_u32("hex_count", (uint32_t)hex_count, 4u);

  failed += expect_u32("opcode0", out_words[0], (uint32_t)RAFPHI_OP_RET_A64);

  if (RmR_IrNativeLittleEndian()) {
    failed += expect_bytes_le32("endianness_ret_a64", out_words[0], 0xC0u, 0x03u, 0x5Fu, 0xD6u);
  }

  failed += expect_u32("emit_word_asm slot0", rafphi_emit_word_asm(out_words[0], out_words, 8u, 0u), 1u);
  failed += expect_u32("emit_word_asm slot7", rafphi_emit_word_asm(out_words[1], out_words, 8u, 7u), 1u);
  failed += expect_u32("emit_word_asm overflow", rafphi_emit_word_asm(out_words[2], out_words, 8u, 8u), 0u);

  if (failed != 0) {
    fprintf(stderr, "rmr_ir_abi_selftest FAILED (%d)\n", failed);
    return 1;
  }

  printf("rmr_ir_abi_selftest OK ir=%u opcode0=0x%08x little_endian=%u\n",
         (unsigned)ir_count,
         out_words[0],
         (unsigned)RmR_IrNativeLittleEndian());
  return 0;
}
