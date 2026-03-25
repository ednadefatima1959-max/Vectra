#include "rmr_output_sync.h"

#include "rmr_coherence_engine.h"

int rmr_output_validate_with_state(uint64_t pipeline_state, uint64_t output_hash) {
  return rmr_coherence_score(pipeline_state, output_hash) >= 0.70f;
}

int rmr_output_commit_if_coherent(uint64_t pipeline_state, uint64_t output_hash) {
  return rmr_output_validate_with_state(pipeline_state, output_hash) ? 1 : 0;
}
