/*
 * rmr_core.h — RMR PolyLib Core
 * Linguagem-raiz: C bare-metal
 * Todas as outras linguagens fazem FFI/binding para cá.
 *
 * Princípio: miss-cache, shadow, tail, overhead viram FEATURES, não bugs.
 *   miss-cache  → prefetch controlado (pipeline feed)
 *   shadow reg  → aliases de ponteiro sem cópia
 *   tail call   → recursão sem stack growth
 *   overhead    → medido, não escondido (via TSC)
 *   memory leak → arena com reset, sem free() individual
 *
 * I/O harmônico: throughput = bandwidth × (1 - miss_rate) × parallelism
 * Produto de fatores = composição matemática de camadas.
 */

#ifndef RMR_CORE_H
#define RMR_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* ── Detecção de arquitetura em compile-time ── */
#if defined(__x86_64__) || defined(_M_X64)
  #define RMR_ARCH_X86_64 1
  #define RMR_CACHE_LINE  64
  #define RMR_PREFETCH(p) __builtin_prefetch((p), 0, 3)
  #define RMR_PREFETCH_W(p) __builtin_prefetch((p), 1, 3)
  static inline uint64_t rmr_tsc(void){
      uint32_t lo,hi;
      __asm__ volatile("rdtsc":"=a"(lo),"=d"(hi));
      return ((uint64_t)hi<<32)|lo;
  }
#elif defined(__aarch64__)
  #define RMR_ARCH_AARCH64 1
  #define RMR_CACHE_LINE  64
  #define RMR_PREFETCH(p) __builtin_prefetch((p), 0, 3)
  #define RMR_PREFETCH_W(p) __builtin_prefetch((p), 1, 3)
  static inline uint64_t rmr_tsc(void){
      uint64_t v;
      __asm__ volatile("mrs %0, cntvct_el0":"=r"(v));
      return v;
  }
#else
  #define RMR_CACHE_LINE  64
  #define RMR_PREFETCH(p)   (void)(p)
  #define RMR_PREFETCH_W(p) (void)(p)
  static inline uint64_t rmr_tsc(void){ return 0; }
#endif

/* ── Atributos compiler-hint ── */
#define RMR_HOT    __attribute__((hot))
#define RMR_COLD   __attribute__((cold))
#define RMR_INLINE static __attribute__((always_inline)) inline
#define RMR_NOINLINE __attribute__((noinline))
#define RMR_ALIGNED(n) __attribute__((aligned(n)))
#define RMR_PACKED  __attribute__((packed))
#define RMR_LIKELY(x)   __builtin_expect(!!(x), 1)
#define RMR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define RMR_RESTRICT __restrict__

/* ── Arena: memória sem free() individual (sem leak por design) ──
 * Alinhada em cache line. Reset = ponteiro volta a zero.
 * Substitui malloc/free no hot path → sem fragmentation, sem overhead.
 */
#define RMR_ARENA_MAX (1 << 20)  /* 1 MiB default */

typedef struct RMR_ALIGNED(RMR_CACHE_LINE) {
    uint8_t  buf[RMR_ARENA_MAX];
    uint32_t pos;
    uint32_t cap;
} rmr_arena_t;

RMR_INLINE void rmr_arena_init(rmr_arena_t *a){
    a->pos = 0; a->cap = RMR_ARENA_MAX;
}
RMR_INLINE void* rmr_arena_alloc(rmr_arena_t *a, uint32_t sz){
    /* alinha sz à cache line */
    sz = (sz + (RMR_CACHE_LINE-1)) & ~(RMR_CACHE_LINE-1);
    if(RMR_UNLIKELY(a->pos + sz > a->cap)) return 0;
    void *p = a->buf + a->pos;
    a->pos += sz;
    return p;
}
RMR_INLINE void rmr_arena_reset(rmr_arena_t *a){ a->pos = 0; }

/* ── Shadow pointer: alias sem cópia — zero overhead
 * "shadow" = ponteiro que aponta para subestrutura interna
 * sem memcpy, sem alloc, sem free — o compilador usa o mesmo endereço
 */
#define RMR_SHADOW(T, name, src_ptr, offset) \
    T * const name = (T*)((uint8_t*)(src_ptr) + (offset))

/* ── Prefetch pipeline: alimenta o cache antes de precisar ──
 * miss-cache → oportunidade: se sabemos o próximo bloco, prefetch agora
 * Uso: rmr_prefetch_stream(ptr, n_bytes, stride)
 */
RMR_INLINE void rmr_prefetch_stream(
    const void *base, size_t bytes, size_t stride)
{
    const uint8_t *p = (const uint8_t*)base;
    const uint8_t *end = p + bytes;
    for(; p < end; p += stride)
        RMR_PREFETCH(p);
}

/* ── I/O Harmônico: produto de fatores de throughput ──
 *
 *   T = B × (1 - M) × P
 *   T = throughput efetivo
 *   B = bandwidth pico
 *   M = miss_rate  [0,1]
 *   P = parallelism (pipeline depth / SIMD width)
 *
 * Uso: rmr_io_throughput(bandwidth_MBs, miss_rate, parallelism)
 */
RMR_INLINE double rmr_io_throughput(
    double bandwidth_mbs, double miss_rate, double parallelism)
{
    return bandwidth_mbs * (1.0 - miss_rate) * parallelism;
}

/* Composição: produto de N fatores harmônicos
 * Cada fator é uma dimensão independente do pipeline.
 * produto_total = Π fator_i  — multiplicação pura, sem soma
 */
RMR_INLINE double rmr_harmonic_product(const double *factors, int n){
    double p = 1.0;
    for(int i = 0; i < n; i++) p *= factors[i];
    return p;
}

/* ── Tail-call iterativo: substitui recursão profunda ──
 * Evita stack overflow. Compila para loop simples.
 * Uso: defina fn_t e chame rmr_tail_loop.
 */
typedef int (*rmr_fn_t)(void *state);

RMR_INLINE int rmr_tail_loop(rmr_fn_t fn, void *state, int max_iter){
    int r = 0, i = 0;
    while(i++ < max_iter && (r = fn(state)) > 0);
    return r;
}

/* ── Medidor de ciclos (wraps TSC) ── */
typedef struct { uint64_t t0; } rmr_timer_t;
RMR_INLINE void      rmr_timer_start(rmr_timer_t *t){ t->t0 = rmr_tsc(); }
RMR_INLINE uint64_t  rmr_timer_elapsed(rmr_timer_t *t){ return rmr_tsc() - t->t0; }

/* ── API pública exportável (para FFI em todas as linguagens) ── */
#ifdef __cplusplus
extern "C" {
#endif

/* Versão */
const char* rmr_version(void);

/* Arena */
rmr_arena_t* rmr_arena_new(void);
void         rmr_arena_free(rmr_arena_t *a);
void*        rmr_arena_push(rmr_arena_t *a, uint32_t sz);
void         rmr_arena_clear(rmr_arena_t *a);

/* Hashing (wraps BLAKE3 portable) */
void rmr_hash32(const void *in, size_t len, uint8_t out[32]);

/* I/O harmônico */
double rmr_throughput(double bw, double miss, double par);
double rmr_product   (const double *v, int n);

/* Prefetch */
void rmr_prefetch(const void *p, size_t bytes);

/* Timer */
uint64_t rmr_cycles(void);

/* Medição de miss-cache simulada: retorna razão estimada */
double rmr_miss_rate_estimate(const void *buf, size_t sz, size_t stride);

#ifdef __cplusplus
}
#endif

#endif /* RMR_CORE_H */
