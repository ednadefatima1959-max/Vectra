# ====================================================================
#  sources_rmr_core.cmake – Manifesto de fontes do módulo RMR
#  HOTFIX v2 — ∆RafaelVerboΩ / Vectra
# ====================================================================
# BUGS CORRIGIDOS:
#   BUG1 — Paths de ASM apontavam para engine/rmr/src/asm/... inexistente.
#           Arquivos reais estão em engine/rmr/interop/*.S (maiúsculo).
#   BUG2 — RMR_SOURCE_GROUP_ANDROID_ONLY nunca definido mas referenciado
#           em app/src/main/cpp/CMakeLists.txt → produzia lista vazia silenciosa.
# ====================================================================

# ----------------------------------------------------------------
# Grupo 1: Núcleo obrigatório (host + Android)
# ----------------------------------------------------------------
set(RMR_SOURCE_GROUP_CORE
    engine/rmr/src/rmr_corelib.c
    engine/rmr/src/rmr_ll_ops.c
    engine/rmr/src/rmr_ll_tuning.c
    engine/rmr/src/rmr_math_fabric.c
    engine/rmr/src/rmr_execution_graph.c
    engine/rmr/src/rmr_unified_kernel.c
    engine/rmr/src/rmr_unified_jni_bridge.c
    engine/rmr/src/rmr_output_sync.c
    # NOTA: bitraf.c é compilado separadamente como bitraf_static.
    # NÃO inclua aqui para evitar símbolo duplicado (BUG4 no root CMakeLists.txt).
)

# ----------------------------------------------------------------
# Grupo 2: Somente builds host (não Android)
# ----------------------------------------------------------------
set(RMR_SOURCE_GROUP_HOST_ONLY
    engine/rmr/src/rmr_hw_detect_selftest.c
    engine/rmr/src/rmr_bench.c
    engine/rmr/src/rmr_bench_suite.c
    engine/rmr/src/rmr_baremetal_compat.c
)

# ----------------------------------------------------------------
# Grupo 3: Somente builds Android (delta entre Android e host)
# FIX BUG2: grupo antes ausente → app CMakeLists produzia lista vazia
# ----------------------------------------------------------------
set(RMR_SOURCE_GROUP_ANDROID_ONLY
    engine/rmr/src/bitomega.c
    engine/rmr/src/rmr_cycles.c
    engine/rmr/src/rmr_hw_detect.c
    engine/rmr/src/rmr_casm_bridge.c
    engine/rmr/src/rafaelia_formulas_core.c
    engine/rmr/src/rmr_attractor.c
    engine/rmr/src/rmr_neon_simd.c
    engine/rmr/src/rmr_simd_geometry.c
    engine/rmr/src/rmr_lowlevel_mix.c
    engine/rmr/src/rmr_lowlevel_portable.c
    engine/rmr/src/rmr_lowlevel_reduce.c
    engine/rmr/src/rmr_zipraf_core.c
    engine/rmr/src/rmr_invariant_extractor.c
    engine/rmr/src/rmr_ir_bridge.c
    engine/rmr/src/rmr_isorf.c
    engine/rmr/src/rmr_apk_module.c
    engine/rmr/src/rmr_coherence_engine.c
    engine/rmr/src/rmr_bit_broadcast.c
    engine/rmr/src/rmr_qemu_bridge.c
    engine/rmr/src/rmr_tcg_cache.c
    engine/rmr/src/rmr_virtio_blk.c
    engine/rmr/src/rmr_vhw_model.c
    engine/rmr/src/rmr_ethica_loss.c
    engine/rmr/src/rmr_predictive_cache.c
    engine/rmr/src/rmr_host_compat.c
    engine/rmr/src/rmr_benchmark.c
)

# ----------------------------------------------------------------
# Grupo 4: Módulo de política (opcional)
# ----------------------------------------------------------------
set(RMR_SOURCE_GROUP_OPTIONAL_POLICY
    engine/rmr/src/rmr_policy_kernel.c
)

# ----------------------------------------------------------------
# Grupo 5: ASM para x86_64
# FIX BUG1: path era engine/rmr/src/asm/x86_64/*.s (inexistente)
#            real:     engine/rmr/interop/*.S  (maiúsculo)
# ----------------------------------------------------------------
set(RMR_SOURCE_GROUP_ASM_X86_64
    engine/rmr/interop/rmr_lowlevel_x86_64.S
    engine/rmr/interop/rmr_casm_x86_64.S
)

# ----------------------------------------------------------------
# Grupo 6: ASM para ARM64
# FIX BUG1: path era engine/rmr/src/asm/arm64/rmr_casm_arm64.s
#            real:     engine/rmr/interop/rmr_casm_arm64.S
# ----------------------------------------------------------------
set(RMR_SOURCE_GROUP_ASM_ARM64
    engine/rmr/interop/rmr_casm_arm64.S
)

# ----------------------------------------------------------------
# Grupo 7: ASM para RISC-V 64
# FIX BUG1: path era engine/rmr/src/asm/riscv64/rmr_casm_riscv64.s
#            real:     engine/rmr/interop/rmr_casm_riscv64.S
# ----------------------------------------------------------------
set(RMR_SOURCE_GROUP_RISCV64
    engine/rmr/interop/rmr_casm_riscv64.S
)
# Alias retrocompatível referenciado no root CMakeLists.txt
set(RMR_SOURCE_GROUP_ASM_RISCV64 ${RMR_SOURCE_GROUP_RISCV64})

# ====================================================================
#  Função: adiciona arquivos dos grupos à variável de saída
# ====================================================================
function(rmr_manifest_apply_base out_var)
    set(_sources "")
    foreach(group ${ARGN})
        if(DEFINED ${group})
            foreach(_item ${${group}})
                list(APPEND _sources ${_item})
            endforeach()
        else()
            message(WARNING "[RMR] Grupo de fontes '${group}' não definido — verifique sources_rmr_core.cmake")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES _sources)
    set(${out_var} ${_sources} PARENT_SCOPE)
endfunction()
