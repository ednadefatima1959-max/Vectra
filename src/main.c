#include "model_s.h"

#include <stdint.h>
#include <stdio.h>

int main(void) {
    const uint32_t dx_stream[] = {1U, 0U, 1U, 1U, 0U, 1U, 0U, 0U, 1U, 1U};
    const uint32_t modulus_n = 17U;
    const int32_t cx_limit = 31;

    const model_s_state_t initial = {
        .ax = 0xA5A5U,
        .bx = 0U,
        .cx = 3,
    };

    model_s_state_t final_state;
    const uint64_t signature = model_s_run_deterministic(initial,
                                                         dx_stream,
                                                         (uint32_t)(sizeof(dx_stream) / sizeof(dx_stream[0])),
                                                         modulus_n,
                                                         cx_limit,
                                                         &final_state);

    printf("MODEL_S_FINAL ax=%u bx=%u cx=%d\n", final_state.ax, final_state.bx, final_state.cx);
    printf("MODEL_S_SIGNATURE %llu\n", (unsigned long long)signature);
    return 0;
}
