# Compatibility shim: canonical source manifest lives in engine/rmr/sources.cmake.
# This file intentionally exposes the legacy groups consumed by host/Android CMake.

include(${CMAKE_CURRENT_LIST_DIR}/sources.cmake)

# Legacy group aliases used by existing build logic.
set(RMR_SOURCE_GROUP_CORE ${RMR_ENGINE_CORE_SOURCES})
set(RMR_SOURCE_GROUP_OPTIONAL_POLICY ${RMR_ENGINE_POLICY_SOURCES})
set(RMR_SOURCE_GROUP_ASM_X86_64
    ${RMR_ENGINE_ASM_X86_64_LOWLEVEL_SOURCES}
    ${RMR_ENGINE_ASM_X86_64_CASM_SOURCES})
set(RMR_SOURCE_GROUP_ASM_ARM64 ${RMR_ENGINE_ASM_ARM64_SOURCES})
set(RMR_SOURCE_GROUP_ASM_RISCV64 ${RMR_ENGINE_ASM_RISCV64_SOURCES})

# Historical partition kept for Android/host compatibility.
set(RMR_SOURCE_GROUP_ANDROID_ONLY
    engine/rmr/src/rmr_tcg_cache.c
    engine/rmr/src/rmr_virtio_blk.c
    engine/rmr/src/rmr_attractor.c
    engine/rmr/src/rmr_vhw_model.c
    engine/rmr/src/rmr_ethica_loss.c)

set(RMR_SOURCE_GROUP_HOST_ONLY
    engine/rmr/src/rmr_bench.c
    engine/rmr/src/rmr_bench_suite.c)

function(rmr_manifest_apply_base out_var)
    set(_sources "")
    foreach(group ${ARGN})
        if(DEFINED ${group})
            list(APPEND _sources ${${group}})
        else()
            message(WARNING "Grupo de fontes ${group} não definido")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES _sources)
    set(${out_var} ${_sources} PARENT_SCOPE)
endfunction()
