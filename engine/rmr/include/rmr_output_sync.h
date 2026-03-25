#ifndef RMR_OUTPUT_SYNC_H
#define RMR_OUTPUT_SYNC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int rmr_output_validate_with_state(uint64_t pipeline_state, uint64_t output_hash);
int rmr_output_commit_if_coherent(uint64_t pipeline_state, uint64_t output_hash);

#ifdef __cplusplus
}
#endif

#endif
