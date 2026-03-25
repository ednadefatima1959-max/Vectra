#include "rmr_predictive_cache.h"

#define RMR_PREDICTIVE_CACHE_SLOTS 32u

typedef struct {
  uint64_t pattern;
  uint64_t result;
  uint8_t valid;
} rmr_predictive_slot_t;

static rmr_predictive_slot_t g_rmr_predictive_cache[RMR_PREDICTIVE_CACHE_SLOTS];

static uint32_t rmr_predictive_index(uint64_t pattern) {
  return (uint32_t)((pattern ^ (pattern >> 32u)) % RMR_PREDICTIVE_CACHE_SLOTS);
}

uint64_t rmr_predict_state(uint64_t pattern) {
  uint32_t idx = rmr_predictive_index(pattern);
  if (g_rmr_predictive_cache[idx].valid && g_rmr_predictive_cache[idx].pattern == pattern) {
    return g_rmr_predictive_cache[idx].result;
  }
  return 0u;
}

void rmr_cache_learn(uint64_t pattern, uint64_t result) {
  uint32_t idx = rmr_predictive_index(pattern);
  g_rmr_predictive_cache[idx].pattern = pattern;
  g_rmr_predictive_cache[idx].result = result;
  g_rmr_predictive_cache[idx].valid = 1u;
}
