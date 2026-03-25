#include "rmr_invariant_extractor.h"

#include <stdint.h>

static uint8_t rmr_is_alpha(uint8_t c) {
  return (uint8_t)(((c >= (uint8_t)'a' && c <= (uint8_t)'z') || (c >= (uint8_t)'A' && c <= (uint8_t)'Z')) ? 1u : 0u);
}

static uint8_t rmr_is_digit(uint8_t c) {
  return (uint8_t)((c >= (uint8_t)'0' && c <= (uint8_t)'9') ? 1u : 0u);
}

static uint8_t rmr_is_space(uint8_t c) {
  return (uint8_t)((c == (uint8_t)' ' || c == (uint8_t)'\t' || c == (uint8_t)'\n' || c == (uint8_t)'\r') ? 1u : 0u);
}

static uint8_t rmr_classify_byte(uint8_t c) {
  if (rmr_is_alpha(c)) return 0u;
  if (rmr_is_digit(c)) return 1u;
  if (rmr_is_space(c)) return 2u;
  if (c >= 0x20u && c <= 0x7Eu) return 3u;
  return 4u;
}

static uint64_t rmr_mix64(uint64_t x) {
  x ^= x >> 33u;
  x *= 0xff51afd7ed558ccdu;
  x ^= x >> 33u;
  x *= 0xc4ceb9fe1a85ec53u;
  x ^= x >> 33u;
  return x;
}

static void rmr_store_hex64(char *out, uint64_t v) {
  static const char hex[] = "0123456789abcdef";
  uint32_t i;
  for (i = 0u; i < 16u; ++i) {
    uint32_t shift = (15u - i) * 4u;
    out[i] = hex[(v >> shift) & 0x0Fu];
  }
  out[16] = '\0';
}

int rmr_invariant_extract(const uint8_t *input, size_t len, rmr_invariant_fingerprint_t *out) {
  uint32_t i;
  uint8_t prev_class = 0xFFu;
  uint8_t transition_count = 0u;
  uint16_t class_mask = 0u;
  uint64_t rolling = 0x9e3779b97f4a7c15u;
  uint8_t has_zero = 0u;

  if (!out) return RMR_INVARIANT_BROKEN;

  out->fingerprint = 0u;
  out->length = (uint32_t)len;
  out->class_mask = 0u;
  out->transition_count = 0u;
  out->status = (uint8_t)RMR_INVARIANT_BROKEN;

  if (!input && len != 0u) return RMR_INVARIANT_BROKEN;
  if (len == 0u) {
    out->status = (uint8_t)RMR_INVARIANT_AMBIGUOUS;
    return RMR_INVARIANT_AMBIGUOUS;
  }

  for (i = 0u; i < (uint32_t)len; ++i) {
    uint8_t cls;
    uint64_t point;
    uint8_t v = input[i];

    if (v == 0u) has_zero = 1u;

    cls = rmr_classify_byte(v);
    class_mask = (uint16_t)(class_mask | (uint16_t)(1u << cls));

    if (prev_class != 0xFFu && prev_class != cls) {
      transition_count = (uint8_t)(transition_count + 1u);
    }
    prev_class = cls;

    point = ((uint64_t)cls << 56u) ^ ((uint64_t)(i & 0xFFu) << 48u) ^ (uint64_t)v;
    rolling ^= rmr_mix64(point + (uint64_t)(i + 1u));
    rolling = (rolling << 7u) | (rolling >> (64u - 7u));
  }

  rolling ^= rmr_mix64((uint64_t)len);
  rolling ^= rmr_mix64(((uint64_t)class_mask << 16u) | (uint64_t)transition_count);

  out->fingerprint = rolling;
  out->class_mask = class_mask;
  out->transition_count = transition_count;

  if (has_zero && len > 1u) {
    out->status = (uint8_t)RMR_INVARIANT_BROKEN;
    return RMR_INVARIANT_BROKEN;
  }

  if (transition_count <= 1u || (class_mask & (class_mask - 1u)) == 0u) {
    out->status = (uint8_t)RMR_INVARIANT_AMBIGUOUS;
    return RMR_INVARIANT_AMBIGUOUS;
  }

  out->status = (uint8_t)RMR_INVARIANT_VALID;
  return RMR_INVARIANT_VALID;
}

size_t rmr_invariant_fingerprint_serialize(const rmr_invariant_fingerprint_t *fp, char *out, size_t out_cap) {
  if (!fp || !out || out_cap < 17u) return 0u;
  rmr_store_hex64(out, fp->fingerprint);
  return 16u;
}
