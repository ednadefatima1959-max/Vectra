# Relatório de Governança de Promoção (bug/core → engine/rmr/src)

- Pares analisados: **9**
- Promoção em bloco bloqueada em: **9** pares

## Resultado por par

| Par | API (assinatura) | ABI (exports) | Ordem | Includes críticos | Candidatas incrementais | Status |
|---|---:|---:|---:|---:|---:|---|
| `bitraf.c` | 0 | 0 | 0 | 5 | 0 | BLOQUEADO |
| `rmr_casm_bridge.c` | 0 | 0 | 0 | 3 | 0 | BLOQUEADO |
| `rmr_cycles.c` | 0 | 0 | 0 | 3 | 0 | BLOQUEADO |
| `rmr_hw_detect.c` | 0 | 0 | 0 | 5 | 0 | BLOQUEADO |
| `rmr_ll_ops.c` | 0 | 0 | 0 | 4 | 0 | BLOQUEADO |
| `rmr_ll_tuning.c` | 0 | 2 | 0 | 3 | 0 | BLOQUEADO |
| `rmr_math_fabric.c` | 0 | 0 | 0 | 4 | 0 | BLOQUEADO |
| `rmr_policy_kernel.c` | 0 | 0 | 0 | 8 | 0 | BLOQUEADO |
| `rmr_unified_kernel.c` | 0 | 0 | 0 | 4 | 0 | BLOQUEADO |

## Detalhes

### bitraf.c
- Status: **BLOQUEADO**
- Includes críticos divergentes: `bitraf.h, bitraf_version.h, rmr_lowlevel.h, rmr_unified_kernel.h, zero.h`

### rmr_casm_bridge.c
- Status: **BLOQUEADO**
- Includes críticos divergentes: `rmr_casm_bridge.h, rmr_lowlevel.h, rmr_unified_kernel.h`

### rmr_cycles.c
- Status: **BLOQUEADO**
- Includes críticos divergentes: `rmr_cycles.h, rmr_ll_ops.h, rmr_unified_kernel.h`

### rmr_hw_detect.c
- Status: **BLOQUEADO**
- Includes críticos divergentes: `rmr_cycles.h, rmr_hw_detect.h, rmr_ll_ops.h, rmr_unified_kernel.h, zero.h`

### rmr_ll_ops.c
- Status: **BLOQUEADO**
- Includes críticos divergentes: `rmr_bit_broadcast.h, rmr_ll_ops.h, rmr_lowlevel.h, rmr_unified_kernel.h`

### rmr_ll_tuning.c
- Status: **BLOQUEADO**
- ABI divergente (exports): `rmr_ll_tuning_hw_crc, rmr_ll_tuning_neon`
- Includes críticos divergentes: `rmr_ll_tuning.h, rmr_lowlevel.h, rmr_unified_kernel.h`

### rmr_math_fabric.c
- Status: **BLOQUEADO**
- Includes críticos divergentes: `rmr_ll_ops.h, rmr_lowlevel.h, rmr_math_fabric.h, rmr_unified_kernel.h`

### rmr_policy_kernel.c
- Status: **BLOQUEADO**
- Includes críticos divergentes: `rmr_corelib.h, rmr_hw_detect.h, rmr_ll_ops.h, rmr_ll_tuning.h, rmr_lowlevel.h, rmr_math_fabric.h, rmr_policy_kernel_autoral.h, zero.h`

### rmr_unified_kernel.c
- Status: **BLOQUEADO**
- Includes críticos divergentes: `rmr_execution_graph.h, rmr_invariant_extractor.h, rmr_output_sync.h, rmr_zipraf_core.h`
