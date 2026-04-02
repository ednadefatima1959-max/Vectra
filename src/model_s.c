#include "model_s.h"

#include <stddef.h>

#if defined(__x86_64__)
extern uint32_t model_s_ax_transform_x86_64(uint32_t ax, uint32_t bx_norm);
#elif defined(__aarch64__)
extern uint32_t model_s_ax_transform_arm64(uint32_t ax, uint32_t bx_norm);
#endif

static inline uint32_t normalize_bx(uint32_t bx, uint32_t n) {
    if (n == 0U) {
        return bx;
    }
    return bx % n;
}

static inline int32_t clamp_cx(int64_t value, int32_t limit) {
    if (limit < 0) {
        limit = -limit;
    }
    if (value > limit) {
        return limit;
    }
    if (value < -((int64_t)limit)) {
        return -limit;
    }
    return (int32_t)value;
}

static inline uint32_t ax_transform(uint32_t ax, uint32_t bx_norm) {
#if defined(__x86_64__)
    return model_s_ax_transform_x86_64(ax, bx_norm);
#elif defined(__aarch64__)
    return model_s_ax_transform_arm64(ax, bx_norm);
#else
    return ax ^ bx_norm;
#endif
}

model_s_state_t model_s_step(model_s_state_t current,
                             uint32_t dx,
                             uint32_t modulus_n,
                             int32_t cx_limit) {
    model_s_state_t next = current;

    const uint32_t phase_acc = current.bx + dx;
    const uint32_t bx_norm = normalize_bx(phase_acc, modulus_n);
    const uint32_t ax_next = ax_transform(current.ax, bx_norm);
    const int64_t cx_delta = (int64_t)current.cx + ((int64_t)ax_next - (int64_t)bx_norm);

    next.ax = ax_next;
    next.bx = bx_norm;
    next.cx = clamp_cx(cx_delta, cx_limit);
    return next;
}

uint64_t model_s_run_deterministic(model_s_state_t initial,
                                   const uint32_t *dx_stream,
                                   uint32_t stream_len,
                                   uint32_t modulus_n,
                                   int32_t cx_limit,
                                   model_s_state_t *final_state) {
    uint64_t signature = 1469598103934665603ULL;
    model_s_state_t state = initial;

    if (dx_stream == NULL && stream_len != 0U) {
        if (final_state != NULL) {
            *final_state = state;
        }
        return 0ULL;
    }

    for (uint32_t i = 0; i < stream_len; ++i) {
        state = model_s_step(state, dx_stream[i], modulus_n, cx_limit);

        signature ^= state.ax;
        signature *= 1099511628211ULL;
        signature ^= state.bx;
        signature *= 1099511628211ULL;
        signature ^= (uint32_t)state.cx;
        signature *= 1099511628211ULL;
    }

    if (final_state != NULL) {
        *final_state = state;
    }
    return signature;
}
