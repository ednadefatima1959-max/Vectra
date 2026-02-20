/* rmr_policy_kernel.h — RMR POLICY KERNEL (Φ_ethica layer)
 * ∆RAFAELIA_CORE·Ω
 * Φ_ethica = Min(Entropy) × Max(Coherence)
 * Governa: dispatch de paths, prioridade de células, coerência
 * ─────────────────────────────────────────────────────── */
#ifndef RMR_POLICY_KERNEL_H
#define RMR_POLICY_KERNEL_H

#include "rmr_unified_kernel.h"

/* ── Policy constants ── */
#define RMR_POLICY_MAX_PATHS     256u
#define RMR_POLICY_MAX_CELLS     1000u   /* 10³ cube */
#define RMR_POLICY_ETHICA_MAX    0xFFFFu
#define RMR_POLICY_ENTROPY_BITS  16u

/* ── Policy state ── */
typedef struct rmr_policy_state {
    rmr_u32 ethica;          /* Φ_ethica (0..0xFFFF) */
    rmr_u32 entropy;         /* current system entropy */
    rmr_u32 coherence;       /* coerência: anti-entropy */
    rmr_u32 cycle;           /* ψ→Ω cycle */
    rmr_u64 phi_state;       /* accumulated phi-state */
    rmr_u32 path_weights[RMR_POLICY_MAX_PATHS]; /* per-path coherence */
    rmr_u8  cell_priority[RMR_POLICY_MAX_CELLS]; /* cell dispatch priority */
} rmr_policy_state_t;

#ifdef __cplusplus
extern "C" {
#endif

int     rmr_policy_init(rmr_policy_state_t *ps, rmr_u32 seed);
int     rmr_policy_tick(rmr_policy_state_t *ps, rmr_u32 event);
rmr_u32 rmr_policy_get_ethica(const rmr_policy_state_t *ps);
rmr_u32 rmr_policy_select_path(const rmr_policy_state_t *ps, rmr_u32 path_count);
rmr_u32 rmr_policy_cell_priority(const rmr_policy_state_t *ps, rmr_u32 cell_idx);

/* math fabric bridge */
rmr_u64 rmr_policy_phi_spiral(rmr_u64 state, int step);

#ifdef __cplusplus
}
#endif

#endif /* RMR_POLICY_KERNEL_H */
