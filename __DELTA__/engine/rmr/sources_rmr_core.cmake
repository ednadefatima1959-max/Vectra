# HOTFIX: BUG1/BUG2/BUG4 — canonical RMR source groups for host/Android and ASM routing
# Autor: ∆RafaelVerboΩ | Data: 2026-04-01
# Arquivo: engine/rmr/sources_rmr_core.cmake
# Verificado contra: engine/rmr/sources.cmake

# Canonical manifest is shared with legacy consumers via sources.cmake.
include(${CMAKE_CURRENT_LIST_DIR}/sources.cmake)

# Legacy aliases used by root/app CMake wiring.
set(RMR_SOURCE_GROUP_CORE ${RMR_ENGINE_CORE_SOURCES})
list(REMOVE_ITEM RMR_SOURCE_GROUP_CORE engine/rmr/src/bitraf.c)

set(RMR_SOURCE_GROUP_OPTIONAL_POLICY ${RMR_ENGINE_POLICY_SOURCES})
set(RMR_SOURCE_GROUP_ASM_X86_64
    engine/rmr/interop/rmr_lowlevel_x86_64.S
    engine/rmr/interop/rmr_casm_x86_64.S)
set(RMR_SOURCE_GROUP_ASM_ARM64
    engine/rmr/interop/rmr_casm_arm64.S)
set(RMR_SOURCE_GROUP_ASM_RISCV64
    engine/rmr/interop/rmr_casm_riscv64.S)

# Android-only must include everything not explicitly host-only.
set(RMR_SOURCE_GROUP_ANDROID_ONLY ${RMR_SOURCE_GROUP_CORE})
list(REMOVE_ITEM RMR_SOURCE_GROUP_ANDROID_ONLY
    engine/rmr/src/rmr_bench.c
    engine/rmr/src/rmr_bench_suite.c)

set(RMR_SOURCE_GROUP_HOST_ONLY
    engine/rmr/src/rmr_bench.c
    engine/rmr/src/rmr_bench_suite.c)

# QA/minimal profiles for root CMake.
set(RMR_SOURCE_GROUP_QA_EXTRAS
    engine/rmr/src/rmr_bench.c
    engine/rmr/src/rmr_benchmark.c
    engine/rmr/src/rmr_bench_suite.c
    engine/rmr/src/rmr_execution_graph.c)

set(RMR_SOURCE_GROUP_RUNTIME_MINIMAL ${RMR_SOURCE_GROUP_CORE})
list(REMOVE_ITEM RMR_SOURCE_GROUP_RUNTIME_MINIMAL ${RMR_SOURCE_GROUP_QA_EXTRAS})

function(rmr_manifest_apply_base out_var)
    set(_sources "")
    foreach(group IN LISTS ARGN)
        if(DEFINED ${group})
            list(APPEND _sources ${${group}})
        else()
            message(WARNING "Grupo de fontes ${group} não definido")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES _sources)
    set(${out_var} ${_sources} PARENT_SCOPE)
endfunction()
