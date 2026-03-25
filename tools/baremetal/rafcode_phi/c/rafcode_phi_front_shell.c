#include "rafcode_phi_abi.h"

static raf_u32 rafphi_ascii_eq(const char *a, raf_u32 a_len, const char *b) {
  raf_u32 i = 0u;
  while (i < a_len && b[i] != '\0') {
    if (a[i] != b[i]) {
      return 0u;
    }
    i++;
  }
  return (i == a_len && b[i] == '\0') ? 1u : 0u;
}

static raf_u32 rafphi_parse_u8_auto(const char *s, raf_u32 len, raf_u32 *out_v, raf_u32 *parsed_len) {
  raf_u32 i = 0u;
  raf_u32 base = 10u;
  raf_u32 v = 0u;

  if (!s || !out_v || !parsed_len || len == 0u) {
    return 0u;
  }

  if (len >= 2u && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    base = 16u;
    i = 2u;
  }

  for (; i < len; i++) {
    raf_u32 d;
    char c = s[i];
    if (c >= '0' && c <= '9') {
      d = (raf_u32)(c - '0');
    } else if (base == 16u && c >= 'a' && c <= 'f') {
      d = 10u + (raf_u32)(c - 'a');
    } else if (base == 16u && c >= 'A' && c <= 'F') {
      d = 10u + (raf_u32)(c - 'A');
    } else {
      break;
    }

    if (d >= base) {
      return 0u;
    }

    v = (v * base) + d;
    if (v > 255u) {
      return 0u;
    }
  }

  if (i == 0u || (base == 16u && i == 2u)) {
    return 0u;
  }

  *out_v = v;
  *parsed_len = i;
  return 1u;
}

static raf_u32 rafphi_parse_suffix_u8(const char *token, raf_u32 token_len, const char *prefix, raf_u32 *out_u8) {
  raf_u32 i = 0u;
  raf_u32 parsed_len = 0u;
  if (!token || !prefix || !out_u8) {
    return 0u;
  }

  while (prefix[i] != '\0') {
    if (i >= token_len || token[i] != prefix[i]) {
      return 0u;
    }
    i++;
  }

  if (i >= token_len || token[i] != ':') {
    return 0u;
  }
  i++;

  if (!rafphi_parse_u8_auto(token + i, token_len - i, out_u8, &parsed_len)) {
    return 0u;
  }

  return ((i + parsed_len) == token_len) ? 1u : 0u;
}

raf_u32 rafphi_encode_token_to_hex(const char *token, raf_u32 token_len, raf_u32 *out_hex) {
  raf_u32 imm8 = 0u;

  if (!token || token_len == 0u || !out_hex) {
    return 0u;
  }

  if (rafphi_ascii_eq(token, token_len, "NOP")) {
    *out_hex = RAFPHI_OP_NOP;
    return 1u;
  }
  if (rafphi_ascii_eq(token, token_len, "RET")) {
    *out_hex = RAFPHI_OP_RET_A64;
    return 1u;
  }
  if (rafphi_ascii_eq(token, token_len, "BRK")) {
    *out_hex = RAFPHI_OP_BRK_A64;
    return 1u;
  }
  if (rafphi_ascii_eq(token, token_len, "HLT")) {
    *out_hex = RAFPHI_OP_HLT_A64;
    return 1u;
  }

  if (rafphi_ascii_eq(token, token_len, "CMP")) {
    *out_hex = rafphi_emit_cmp_word_asm(RAFPHI_FLOW_FLAG_Z | RAFPHI_FLOW_FLAG_N, 0u);
    return 1u;
  }
  if (rafphi_parse_suffix_u8(token, token_len, "CMP", &imm8)) {
    *out_hex = rafphi_emit_cmp_word_asm(RAFPHI_FLOW_FLAG_Z | RAFPHI_FLOW_FLAG_N, imm8);
    return 1u;
  }

  if (rafphi_ascii_eq(token, token_len, "BRZ")) {
    *out_hex = rafphi_emit_branch_word_asm(RAFPHI_FLOW_FLAG_BRANCH_IF_Z, 0u);
    return 1u;
  }
  if (rafphi_parse_suffix_u8(token, token_len, "BRZ", &imm8)) {
    *out_hex = rafphi_emit_branch_word_asm(RAFPHI_FLOW_FLAG_BRANCH_IF_Z, imm8);
    return 1u;
  }

  if (rafphi_ascii_eq(token, token_len, "BRNZ")) {
    *out_hex = rafphi_emit_branch_word_asm(RAFPHI_FLOW_FLAG_BRANCH_IF_NZ, 0u);
    return 1u;
  }
  if (rafphi_parse_suffix_u8(token, token_len, "BRNZ", &imm8)) {
    *out_hex = rafphi_emit_branch_word_asm(RAFPHI_FLOW_FLAG_BRANCH_IF_NZ, imm8);
    return 1u;
  }

  if (rafphi_ascii_eq(token, token_len, "LOOP")) {
    *out_hex = rafphi_emit_loop_word_asm(RAFPHI_FLOW_FLAG_LOOP_COUNTED, 1u);
    return 1u;
  }
  if (rafphi_parse_suffix_u8(token, token_len, "LOOP", &imm8)) {
    *out_hex = rafphi_emit_loop_word_asm(RAFPHI_FLOW_FLAG_LOOP_COUNTED, imm8);
    return 1u;
  }

  if (rafphi_ascii_eq(token, token_len, "BLKMIX")) {
    *out_hex = rafphi_emit_blkmix_word_asm(RAFPHI_FLOW_FLAG_MIX_XOR, 16u);
    return 1u;
  }
  if (rafphi_parse_suffix_u8(token, token_len, "BLKMIX", &imm8)) {
    *out_hex = rafphi_emit_blkmix_word_asm(RAFPHI_FLOW_FLAG_MIX_XOR, imm8);
    return 1u;
  }

  return 0u;
}

/* CRC32C simplificado autoral (polinômio castagnoli refletido). */
static raf_u32 rafphi_crc32c_u32(raf_u32 crc, raf_u32 v) {
  raf_u32 x = crc ^ v;
  raf_u32 i;
  for (i = 0u; i < 32u; i++) {
    raf_u32 lsb = x & 1u;
    x >>= 1u;
    if (lsb) {
      x ^= 0x82F63B78u;
    }
  }
  return x;
}

rafphi_emit_stats_t rafphi_emit_block_hex(const char **tokens, const raf_u32 *token_lens, raf_u32 token_count,
                                          raf_u32 *out_words, raf_u32 cap_words) {
  rafphi_emit_stats_t stats;
  stats.accepted = 0u;
  stats.rejected = 0u;
  stats.crc32c = 0u;

  if (!tokens || !token_lens || !out_words || cap_words == 0u) {
    return stats;
  }

  raf_u32 write_index = 0u;
  raf_u32 i;
  for (i = 0u; i < token_count; i++) {
    raf_u32 hex = 0u;
    if (!rafphi_encode_token_to_hex(tokens[i], token_lens[i], &hex)) {
      stats.rejected++;
      continue;
    }

    raf_u32 written = rafphi_emit_word_asm(hex, out_words, cap_words, write_index);
    if (written == 0u) {
      stats.rejected++;
      continue;
    }

    write_index += written;
    stats.accepted++;
    stats.crc32c = rafphi_crc32c_u32(stats.crc32c, hex);
  }

  return stats;
}
