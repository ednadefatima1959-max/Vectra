/* rmr_coherence_engine.h - COHERENCE CORE BASE
 * XOR operations, no libc, direct register manipulation
 * 0xDEADBEEF patterns, pure C + critical ASM sections */

#ifndef RMR_COHERENCE_ENGINE_H
#define RMR_COHERENCE_ENGINE_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct {
  u32 s0; u32 s1; u32 s2; u32 s3;
} rmr_state_t;

typedef struct {
  u32 c0; u32 c1; u32 c2; u32 c3;
} rmr_coherence_t;

static inline u32 rmr_xor_popcount(u32 x) {
  x ^= x >> 16; x ^= x >> 8; x ^= x >> 4; x ^= x >> 2; x ^= x >> 1;
  return x & 1u;
}

static inline u32 rmr_coherence_score(u32 a, u32 b) {
  u32 d = a ^ b;
  u32 c = 0u;
  c |= (rmr_xor_popcount(d & 0xFFu)) << 0;
  c |= (rmr_xor_popcount((d >> 8) & 0xFFu)) << 8;
  c |= (rmr_xor_popcount((d >> 16) & 0xFFu)) << 16;
  c |= (rmr_xor_popcount((d >> 24) & 0xFFu)) << 24;
  return (~c) & 0xFFFFFFFFu;
}

static inline u8 rmr_coherence_should_execute(rmr_state_t cur, rmr_state_t tgt) {
  u32 diff = (cur.s0 ^ tgt.s0) | (cur.s1 ^ tgt.s1) | (cur.s2 ^ tgt.s2) | (cur.s3 ^ tgt.s3);
  return (diff == 0u) ? 1u : 0u;
}

static inline rmr_coherence_t rmr_coherence_vector(rmr_state_t s) {
  rmr_coherence_t c;
  c.c0 = rmr_coherence_score(s.s0, 0xDEADBEEFu);
  c.c1 = rmr_coherence_score(s.s1, 0xCAFEBABEu);
  c.c2 = rmr_coherence_score(s.s2, 0xFEEDFACEu);
  c.c3 = rmr_coherence_score(s.s3, 0xDEADC0DEu);
  return c;
}

#endif