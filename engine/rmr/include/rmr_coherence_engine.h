#ifndef RMR_COHERENCE_ENGINE_H
#define RMR_COHERENCE_ENGINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

float rmr_coherence_score(uint64_t a, uint64_t b);
int rmr_coherence_should_execute(uint64_t current, uint64_t target);

#ifdef __cplusplus
}
#endif

#endif
