#include "rmr_simd_geometry.h"

#include "bitomega.h"
#include "rmr_neon_simd.h"

uint32_t rmr_simd_process_state_vector(const uint32_t *in, uint32_t *out, uint32_t count) {
  uint32_t checksum;
  if (!in || !out || count == 0u) return 0u;
  rmr_neon_memcpy((uint8_t *)out, (const uint8_t *)in, count * sizeof(uint32_t));
  rmr_neon_phi_step_bulk(out, count);
  checksum = rmr_neon_xor_fold32((const uint8_t *)out, count * sizeof(uint32_t));
  return checksum;
}

float rmr_simd_coherence_vector(const uint32_t *a, const uint32_t *b, uint32_t count) {
  uint64_t total = 0u;
  uint32_t i;
  if (!a || !b || count == 0u) return 0.0f;
  for (i = 0u; i < count; ++i) {
    total += (uint64_t)(bitomega_coherence_score(a[i], b[i]) * 1000.0f);
  }
  return (float)total / (1000.0f * (float)count);
}
