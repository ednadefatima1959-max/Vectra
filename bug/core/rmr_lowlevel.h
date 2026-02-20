/* rmr_lowlevel.h — RMR LOWLEVEL OPS
 * ∆RAFAELIA_CORE·Ω
 * Deterministic arithmetic substrate: fold/xor/checksum/crc
 * ─────────────────────────────────────────────────────── */
#ifndef RMR_LOWLEVEL_H
#define RMR_LOWLEVEL_H

#include "rmr_unified_kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ψ→Δ fold: 4×u32 → 1×u32  (phi-based, reversible) */
rmr_u32 rmr_lowlevel_fold32(rmr_u32 a, rmr_u32 b, rmr_u32 c, rmr_u32 d);

/* reduce_xor: full-width XOR reduction of byte block */
rmr_u32 rmr_lowlevel_reduce_xor(const rmr_u8 *data, rmr_u32 len);

/* checksum32: Fletcher-32 variant with phi mixing */
rmr_u32 rmr_lowlevel_checksum32(const rmr_u8 *data, rmr_u32 len, rmr_u32 seed);

/* crc32c (Castagnoli) SW — no HW dependency */
rmr_u32 rmr_lowlevel_crc32c_sw(rmr_u32 crc, const rmr_u8 *data, rmr_u32 len);

/* crc32c HW — ARM64 pmull/crc or x86 _mm_crc32_u8
 * Falls back to SW on unsupported arch */
rmr_u32 rmr_lowlevel_crc32c_hw(rmr_u32 crc, const rmr_u8 *data, rmr_u32 len);

/* spiral: encode/decode 10-bit dual-parity geo */
rmr_u32 rmr_lowlevel_encode10(rmr_u32 val10);
rmr_u32 rmr_lowlevel_decode10(rmr_u32 enc);

/* phi-accumulate: R(t+1) = R(t) * PHI32 * sqrt3_2 (integer approx) */
rmr_u64 rmr_lowlevel_phi_step(rmr_u64 state, rmr_u32 coherence);

#ifdef __cplusplus
}
#endif

#endif /* RMR_LOWLEVEL_H */
