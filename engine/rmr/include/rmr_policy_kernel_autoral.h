#ifndef RMR_POLICY_KERNEL_AUTORAL_H
#define RMR_POLICY_KERNEL_AUTORAL_H

#include <stddef.h>
#include <stdint.h>

#include "rmr_corelib.h"

/*
 * Helpers autorais da policy kernel:
 * - nomes exclusivos (RMR_PK_*)
 * - inline para reduzir fricção de chamada
 * - sem dependência direta de libc para operações de memória
 */
static inline void RMR_PK_Zero(void *dst, size_t len) {
  rmr_mem_set(dst, 0, len);
}

static inline uint32_t RMR_PK_SelectIoBatch(size_t chunk_size, uint32_t tuned_batch_size) {
  if (chunk_size == 0u) return 0u;
  if (tuned_batch_size > 0u && (size_t)tuned_batch_size < chunk_size) {
    return tuned_batch_size;
  }
  return (uint32_t)chunk_size;
}

#endif
