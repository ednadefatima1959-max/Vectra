# ====================================================================
#  sources_rmr_core.cmake – Manifesto de fontes do módulo RMR
# ====================================================================
# Define grupos de fontes e a função rmr_manifest_apply_base.
# Os caminhos são relativos à raiz do projeto (CMAKE_SOURCE_DIR).

# Grupo 1: Núcleo obrigatório
set(RMR_SOURCE_GROUP_CORE
    engine/rmr/src/rmr_corelib.c
    engine/rmr/src/rmr_ll_ops.c
    engine/rmr/src/rmr_ll_tuning.c
    engine/rmr/src/rmr_math_fabric.c
    engine/rmr/src/rmr_execution_graph.c
    engine/rmr/src/rmr_unified_kernel.c
    engine/rmr/src/rmr_unified_jni_bridge.c
    engine/rmr/src/rmr_output_sync.c
    engine/rmr/src/bitraf.c
)

# Grupo 2: Apenas para builds host (não Android)
set(RMR_SOURCE_GROUP_HOST_ONLY
    engine/rmr/src/rmr_hw_detect_selftest.c
    engine/rmr/src/rmr_bench.c
    engine/rmr/src/rmr_bench_suite.c
)

# Grupo 3: Módulo de política (opcional)
set(RMR_SOURCE_GROUP_OPTIONAL_POLICY
    engine/rmr/src/rmr_policy_kernel.c
)

# Grupo 4: ASM para x86_64
set(RMR_SOURCE_GROUP_ASM_X86_64
    engine/rmr/interop/rmr_lowlevel_x86_64.S
    engine/rmr/interop/rmr_casm_x86_64.S
)

# Grupo 5: ASM para ARM64
set(RMR_SOURCE_GROUP_ASM_ARM64
    engine/rmr/interop/rmr_casm_arm64.S
)

# Grupo 6: ASM para RISC-V 64
set(RMR_SOURCE_GROUP_ASM_RISCV64
    engine/rmr/interop/rmr_casm_riscv64.S
)

# ====================================================================
#  Função que adiciona os arquivos dos grupos a uma variável de saída
# ====================================================================
function(rmr_manifest_apply_base out_var)
    set(_sources "")
    foreach(group ${ARGN})
        if(DEFINED ${group})
            list(APPEND _sources ${${group}})
        else()
            message(WARNING "Grupo de fontes ${group} não definido")
        endif()
    endforeach()
    set(${out_var} ${_sources} PARENT_SCOPE)
endfunction()
