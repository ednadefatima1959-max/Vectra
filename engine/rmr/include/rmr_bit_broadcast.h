#ifndef RMR_BIT_BROADCAST_H
#define RMR_BIT_BROADCAST_H

#include <stdint.h>

#include "bitraf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint64_t source_bit;
  uint64_t task_mask;
  uint64_t mapped_targets;
} rmr_bit_broadcast_result_t;

rmr_bit_state_t rmr_bit_broadcast_map(uint64_t bit);
rmr_bit_broadcast_result_t rmr_bit_broadcast_dispatch(uint64_t bit, uint64_t task_mask);

#ifdef __cplusplus
}
#endif

#endif
