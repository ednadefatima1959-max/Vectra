#ifndef RMR_SIMD_GEOMETRY_H
#define RMR_SIMD_GEOMETRY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t rmr_simd_process_state_vector(const uint32_t *in, uint32_t *out, uint32_t count);
float rmr_simd_coherence_vector(const uint32_t *a, const uint32_t *b, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif
