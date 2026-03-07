/* ═══════════════════════════════════════════════════════════════════
   rmr_types.h — Tipos canônicos centralizados do RAFAELIA RMR
   Solução para BUG-01: typedef duplicado em 7 headers
   
   REGRA: Este é o ÚNICO lugar onde u8/u32/u64 são definidos.
   Todos os outros headers do engine devem incluir este arquivo
   e REMOVER suas próprias definições de typedef.
   
   ∆RAFAELIA_CORE·Ω · zero-dep · baremetal-safe
   ═══════════════════════════════════════════════════════════════════ */
#ifndef RMR_TYPES_H
#define RMR_TYPES_H

/* ── Detecção de ambiente ──────────────────────────────────────────── */
#if defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 1)
#  include <stdint.h>
#  include <stddef.h>
   /* Em ambiente hosted, usar tipos canônicos C99 como base */
   typedef uint8_t  u8;
   typedef uint32_t u32;
   typedef uint64_t u64;
   typedef int32_t  i32;
   typedef int64_t  i64;
#else
   /* Baremetal freestanding: definir independentemente */
   typedef unsigned char      u8;
   typedef unsigned int       u32;
   typedef unsigned long long u64;
   typedef signed   int       i32;
   typedef signed   long long i64;

   /* size_t e uintptr_t baremetal */
#  ifndef _SIZE_T_DEFINED
#  define _SIZE_T_DEFINED
   typedef unsigned long long size_t;
#  endif
#  ifndef _UINTPTR_T_DEFINED
#  define _UINTPTR_T_DEFINED
   typedef unsigned long long uintptr_t;
#  endif
#  ifndef NULL
#  define NULL ((void*)0)
#  endif
#endif

/* ── Aliases de inteiros padrão para interop com stdint.h ─────────── */
/* Garantem que u32 e uint32_t sejam equivalentes em qualquer contexto */
#if !defined(RMR_TYPES_COMPAT_CHECK)
#define RMR_TYPES_COMPAT_CHECK 1
#if defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 1)
   /* Verificação estática de compatibilidade em C11 */
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
   _Static_assert(sizeof(u8)  == 1, "RMR: u8 deve ser 1 byte");
   _Static_assert(sizeof(u32) == 4, "RMR: u32 deve ser 4 bytes");
   _Static_assert(sizeof(u64) == 8, "RMR: u64 deve ser 8 bytes");
#  endif
#endif
#endif

/* ── Macros de largura de ponteiro ────────────────────────────────── */
#define RMR_PTR_BITS   ((u32)(sizeof(void*)  * 8u))
#define RMR_WORD_BITS  ((u32)(sizeof(size_t) * 8u))

#endif /* RMR_TYPES_H */
