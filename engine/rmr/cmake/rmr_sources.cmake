# Compatibility shim for consumers expecting absolute-path source lists.
# Canonical source list is engine/rmr/sources.cmake.

get_filename_component(RMR_SOURCE_MANIFEST_DIR "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
get_filename_component(RMR_REPO_ROOT "${RMR_SOURCE_MANIFEST_DIR}/../../.." ABSOLUTE)

include(${RMR_REPO_ROOT}/engine/rmr/sources.cmake)

function(rmr_abs_list out_var)
  set(_abs "")
  foreach(_rel IN LISTS ARGN)
    list(APPEND _abs ${RMR_REPO_ROOT}/${_rel})
  endforeach()
  set(${out_var} ${_abs} PARENT_SCOPE)
endfunction()

rmr_abs_list(RMR_CORE_COMMON_SOURCES ${RMR_ENGINE_CORE_SOURCES})
rmr_abs_list(RMR_EXTENDED_MODULE_SOURCES ${RMR_SOURCE_GROUP_ANDROID_ONLY})
rmr_abs_list(RMR_POLICY_MODULE_SOURCES ${RMR_ENGINE_POLICY_SOURCES})
rmr_abs_list(RMR_CASM_X86_64_SOURCES ${RMR_SOURCE_GROUP_ASM_X86_64})
rmr_abs_list(RMR_CASM_ARM64_SOURCES ${RMR_ENGINE_ASM_ARM64_SOURCES})
rmr_abs_list(RMR_CASM_RISCV64_SOURCES ${RMR_ENGINE_ASM_RISCV64_SOURCES})
