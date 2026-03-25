#include "rmr_bit_broadcast.h"

#include "rmr_coherence_engine.h"

rmr_bit_state_t rmr_bit_broadcast_map(uint64_t bit) {
  return bitraf_get_neighbors(bit);
}

rmr_bit_broadcast_result_t rmr_bit_broadcast_dispatch(uint64_t bit, uint64_t task_mask) {
  rmr_bit_broadcast_result_t out;
  rmr_bit_state_t mapped = rmr_bit_broadcast_map(bit);
  float coherence = rmr_coherence_score(mapped.state_hash, task_mask);

  out.source_bit = bit;
  out.task_mask = task_mask;
  out.mapped_targets = mapped.neighbors_mask;
  if (coherence < 0.40f) {
    out.mapped_targets = 0u;
  }
  return out;
}
