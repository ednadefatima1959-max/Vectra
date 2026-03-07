#!/usr/bin/env bash
# RAFAELIA ZERO — Fix Phase 1 (CI unblock)
# Execute from repository root
set -euo pipefail
echo "ψ FASE 1 — Desbloqueando CI RAFAELIA ZERO"

# BUG-03
[ -f ".github/workflows/neon_simd_selftest.c" ] && git rm ".github/workflows/neon_simd_selftest.c" && echo "✓ BUG-03 fixed"

# BUG-04
for f in "android (1)" "android (2)" "android-verified (1)"; do
  [ -f ".github/workflows/${f}.yml" ] && git rm ".github/workflows/${f}.yml" && echo "✓ BUG-04: removed ${f}.yml"
done

# BUG-01: create rmr_types.h
cat > engine/rmr/include/rmr_types.h << 'RMRTYPES'
#ifndef RMR_TYPES_H
#define RMR_TYPES_H
#if defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 1)
#  include <stdint.h>
#  include <stddef.h>
   typedef uint8_t  u8;
   typedef uint32_t u32;
   typedef uint64_t u64;
   typedef int32_t  i32;
   typedef int64_t  i64;
#else
   typedef unsigned char      u8;
   typedef unsigned int       u32;
   typedef unsigned long long u64;
   typedef signed   int       i32;
   typedef signed   long long i64;
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
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(u8)  == 1, "RMR: u8 deve ser 1 byte");
_Static_assert(sizeof(u32) == 4, "RMR: u32 deve ser 4 bytes");
_Static_assert(sizeof(u64) == 8, "RMR: u64 deve ser 8 bytes");
#endif
#define RMR_PTR_BITS   ((u32)(sizeof(void*)  * 8u))
#define RMR_WORD_BITS  ((u32)(sizeof(size_t) * 8u))
#endif
RMRTYPES
echo "✓ BUG-01: engine/rmr/include/rmr_types.h criado"

echo ""
echo "Σ FASE 1 concluída. Próximo: editar manualmente 7 headers (ver VECTRAS_SOLUTIONS.md)"
echo "F_ok: CI files limpos, rmr_types.h criado"
echo "F_next: Fase 2 — GpioPinStride + app CMakeLists"
