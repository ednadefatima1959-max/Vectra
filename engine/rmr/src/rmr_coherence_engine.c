#include "rmr_coherence_engine.h"

#include "bitomega.h"

float rmr_coherence_score(uint64_t a, uint64_t b) {
  uint64_t mixed = (a ^ b) + 0x9E3779B97F4A7C15ULL;
  uint32_t ah = (uint32_t)(mixed & 0xFFFFFFFFu);
  uint32_t bh = (uint32_t)((mixed >> 32u) & 0xFFFFFFFFu);
  return bitomega_coherence_score(ah, bh);
}

int rmr_coherence_should_execute(uint64_t current, uint64_t target) {
  return rmr_coherence_score(current, target) < 0.85f;
}
