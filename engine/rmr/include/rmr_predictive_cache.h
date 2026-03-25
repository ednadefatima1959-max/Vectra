#ifndef RMR_PREDICTIVE_CACHE_H
#define RMR_PREDICTIVE_CACHE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t rmr_predict_state(uint64_t pattern);
void rmr_cache_learn(uint64_t pattern, uint64_t result);

#ifdef __cplusplus
}
#endif

#endif
