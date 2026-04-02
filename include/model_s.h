#ifndef MODEL_S_H
#define MODEL_S_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t ax;
    uint32_t bx;
    int32_t cx;
} model_s_state_t;

model_s_state_t model_s_step(model_s_state_t current,
                             uint32_t dx,
                             uint32_t modulus_n,
                             int32_t cx_limit);

uint64_t model_s_run_deterministic(model_s_state_t initial,
                                   const uint32_t *dx_stream,
                                   uint32_t stream_len,
                                   uint32_t modulus_n,
                                   int32_t cx_limit,
                                   model_s_state_t *final_state);

#ifdef __cplusplus
}
#endif

#endif
