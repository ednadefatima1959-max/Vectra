#ifndef RMR_IR_BRIDGE_H
#define RMR_IR_BRIDGE_H

#include <stddef.h>
#include <stdint.h>

#include "rmr_ir_hex.h"
#include "rmr_policy_kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t RmR_IR_FromPolicyChunkMeta(RmR_Stage stage,
                                  const RmR_ChunkMeta *meta,
                                  RmR_IrHexInstruction *out_seq,
                                  size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif
