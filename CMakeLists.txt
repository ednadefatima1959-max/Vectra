cmake_minimum_required(VERSION 3.22.1)
project(rafaelia_rmr C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ========================
# DEBUG / ESTABILIDADE (NÃO QUEBRA NADA)
# ========================
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# ========================
# INCLUDE SOURCES MANIFEST (PROTEGIDO)
# ========================
set(RMR_MANIFEST "${CMAKE_SOURCE_DIR}/engine/rmr/sources_rmr_core.cmake")

if(NOT EXISTS ${RMR_MANIFEST})
    message(FATAL_ERROR "Manifesto não encontrado: ${RMR_MANIFEST}")
endif()

include(${RMR_MANIFEST})

if(NOT COMMAND rmr_manifest_apply_base)
    message(FATAL_ERROR "Função rmr_manifest_apply_base não definida pelo manifesto")
endif()

# ========================
# OPTIONS
# ========================
option(RMR_JNI_BUILD "Use hosted/JNI libc path for root builds" ON)
set(RMR_BUILD_HOST_TOOLING "AUTO" CACHE STRING "Host-tooling compatibility path")
set_property(CACHE RMR_BUILD_HOST_TOOLING PROPERTY STRINGS AUTO ON OFF)
set(RMR_PROFILE "qa" CACHE STRING "Build profile: minimal|qa")
set_property(CACHE RMR_PROFILE PROPERTY STRINGS minimal qa)

option(RMR_ENABLE_POLICY_MODULE "Habilita módulo de política RMR" ON)
option(RMR_ASM_CORE_EXPERIMENTAL "Habilita rotas ASM experimentais do core" ON)
option(RMR_ASM_CORE_X86_64_VALIDATED "Habilita ASM x86_64 validado" ON)
option(RMR_ASM_CORE_ARM64_VALIDATED "Habilita ASM ARM64 validado" ON)
option(RMR_ASM_CORE_RISCV64_VALIDATED "Habilita ASM RISC-V64 validado" ON)

# ========================
# RESOLVE FLAGS
# ========================
set(RMR_JNI_BUILD_VALUE 0)
if(RMR_JNI_BUILD)
  set(RMR_JNI_BUILD_VALUE 1)
endif()

if(RMR_BUILD_HOST_TOOLING STREQUAL "AUTO")
  set(RMR_BUILD_HOST_TOOLING_VALUE ${RMR_JNI_BUILD_VALUE})
elseif(RMR_BUILD_HOST_TOOLING)
  set(RMR_BUILD_HOST_TOOLING_VALUE 1)
else()
  set(RMR_BUILD_HOST_TOOLING_VALUE 0)
endif()

set(RMR_ENABLE_POLICY_MODULE_VALUE 0)
if(RMR_ENABLE_POLICY_MODULE)
  set(RMR_ENABLE_POLICY_MODULE_VALUE 1)
endif()

set(RMR_ASM_CORE_EXPERIMENTAL_VALUE 0)
if(RMR_ASM_CORE_EXPERIMENTAL)
  set(RMR_ASM_CORE_EXPERIMENTAL_VALUE 1)
endif()

set(RMR_ASM_CORE_X86_64_VALIDATED_VALUE 0)
if(RMR_ASM_CORE_X86_64_VALIDATED)
  set(RMR_ASM_CORE_X86_64_VALIDATED_VALUE 1)
endif()

set(RMR_ASM_CORE_ARM64_VALIDATED_VALUE 0)
if(RMR_ASM_CORE_ARM64_VALIDATED)
  set(RMR_ASM_CORE_ARM64_VALIDATED_VALUE 1)
endif()

set(RMR_ASM_CORE_RISCV64_VALIDATED_VALUE 0)
if(RMR_ASM_CORE_RISCV64_VALIDATED)
  set(RMR_ASM_CORE_RISCV64_VALIDATED_VALUE 1)
endif()

message(STATUS "RMR_JNI_BUILD=${RMR_JNI_BUILD_VALUE}")
message(STATUS "RMR_BUILD_HOST_TOOLING=${RMR_BUILD_HOST_TOOLING_VALUE}")
message(STATUS "RMR_ENABLE_POLICY_MODULE=${RMR_ENABLE_POLICY_MODULE_VALUE}")
message(STATUS "RMR_PROFILE=${RMR_PROFILE}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")

if(NOT RMR_PROFILE STREQUAL "minimal" AND NOT RMR_PROFILE STREQUAL "qa")
  message(FATAL_ERROR "RMR_PROFILE inválido: ${RMR_PROFILE}. Use minimal ou qa.")
endif()

# ========================
# ENV EXPORT
# ========================
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rmr_build_config.env"
  "RMR_JNI_BUILD=${RMR_JNI_BUILD_VALUE}\n"
  "RMR_BUILD_HOST_TOOLING=${RMR_BUILD_HOST_TOOLING_VALUE}\n"
  "RMR_ENABLE_POLICY_MODULE=${RMR_ENABLE_POLICY_MODULE_VALUE}\n"
  "RMR_PROFILE=${RMR_PROFILE}\n"
)

# ========================
# COMMON FLAGS
# ========================
function(rmr_apply_common_flags target_name)
  target_compile_options(${target_name} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/O2 /W4>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-O3 -Wall -Wextra -pedantic>
  )

  target_compile_definitions(${target_name} PRIVATE
    RMR_JNI_BUILD=${RMR_JNI_BUILD_VALUE}
    RMR_BUILD_HOST_TOOLING=${RMR_BUILD_HOST_TOOLING_VALUE}
    RMR_ENABLE_POLICY_MODULE=${RMR_ENABLE_POLICY_MODULE_VALUE}
    RMR_ASM_CORE_EXPERIMENTAL=${RMR_ASM_CORE_EXPERIMENTAL_VALUE}
    RMR_ASM_CORE_X86_64_VALIDATED=${RMR_ASM_CORE_X86_64_VALIDATED_VALUE}
    RMR_ASM_CORE_ARM64_VALIDATED=${RMR_ASM_CORE_ARM64_VALIDATED_VALUE}
    RMR_ASM_CORE_RISCV64_VALIDATED=${RMR_ASM_CORE_RISCV64_VALIDATED_VALUE}
  )
endfunction()

# ========================
# SOURCES COLLECTION (PROTEGIDO)
# ========================
rmr_manifest_apply_base(RMR_SOURCES RMR_SOURCE_GROUP_RUNTIME_MINIMAL)

if(RMR_PROFILE STREQUAL "qa")
  list(APPEND RMR_SOURCES ${RMR_SOURCE_GROUP_QA_EXTRAS})
endif()

if(RMR_ENABLE_POLICY_MODULE)
  rmr_manifest_apply_base(RMR_POLICY_SOURCES RMR_SOURCE_GROUP_OPTIONAL_POLICY)
  list(APPEND RMR_SOURCES ${RMR_POLICY_SOURCES})
endif()

list(REMOVE_DUPLICATES RMR_SOURCES)

if(NOT RMR_SOURCES)
  message(FATAL_ERROR "RMR_SOURCES está vazio")
endif()

# ========================
# ARCH DETECT (CORRIGIDO)
# ========================
set(RMR_HAS_CASM FALSE)

if(NOT CMAKE_SYSTEM_PROCESSOR)
    set(PROC_LOWER "unknown")
else()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" PROC_LOWER)
endif()

if(RMR_ASM_CORE_EXPERIMENTAL AND UNIX)
  if(PROC_LOWER MATCHES "x86_64|amd64" AND RMR_ASM_CORE_X86_64_VALIDATED)
    list(APPEND RMR_SOURCES ${RMR_SOURCE_GROUP_ASM_X86_64})
    set(RMR_HAS_CASM TRUE)
  elseif(PROC_LOWER MATCHES "arm64|aarch64" AND RMR_ASM_CORE_ARM64_VALIDATED)
    list(APPEND RMR_SOURCES ${RMR_SOURCE_GROUP_ASM_ARM64})
    set(RMR_HAS_CASM TRUE)
  elseif(PROC_LOWER MATCHES "riscv64" AND RMR_ASM_CORE_RISCV64_VALIDATED)
    list(APPEND RMR_SOURCES ${RMR_SOURCE_GROUP_ASM_RISCV64})
    set(RMR_HAS_CASM TRUE)
  endif()
endif()

# ========================
# LIB RMR
# ========================
add_library(rmr STATIC ${RMR_SOURCES})

target_include_directories(rmr PUBLIC
  ${CMAKE_SOURCE_DIR}/engine/rmr/include
)

if(RMR_HAS_CASM)
  target_compile_definitions(rmr PUBLIC VECTRA_HAS_CASM_MARKER=1)
else()
  target_compile_definitions(rmr PUBLIC VECTRA_HAS_CASM_MARKER=0)
endif()

rmr_apply_common_flags(rmr)

# ========================
# BITRAF (PROTEGIDO)
# ========================
if(NOT TARGET bitraf_static)
    add_library(bitraf_static STATIC engine/rmr/src/bitraf.c)
    set_target_properties(bitraf_static PROPERTIES OUTPUT_NAME bitraf)

    target_include_directories(bitraf_static PUBLIC
      ${CMAKE_SOURCE_DIR}/engine/rmr/include
    )

    rmr_apply_common_flags(bitraf_static)
endif()

target_link_libraries(rmr PUBLIC bitraf_static)

# ========================
# SYMBOL CHECK (ROBUSTO)
# ========================
if(NOT ANDROID)
    find_program(BASH_EXECUTABLE bash)

    if(BASH_EXECUTABLE AND EXISTS ${CMAKE_SOURCE_DIR}/tools/verify_librmr_symbols.sh)
        add_custom_command(
            TARGET rmr
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Verificando malha de símbolos do rmr..."
            COMMAND ${BASH_EXECUTABLE}
                    ${CMAKE_SOURCE_DIR}/tools/verify_librmr_symbols.sh
                    $<TARGET_FILE:rmr>
                    RmR_MathFabric_AutodetectPlan
                    RmR_MathFabric_VectorMix
            VERBATIM
        )
    else()
        message(WARNING "bash/script não encontrado, pulando verificação.")
    endif()

    add_custom_target(verify_contracts DEPENDS rmr)
endif()

# ========================
# EXECUTÁVEIS
# ========================
if(NOT ANDROID AND RMR_PROFILE STREQUAL "qa")

    add_executable(rafaelia_demo demo_cli/src/main.c)
    target_link_libraries(rafaelia_demo PRIVATE rmr)
    rmr_apply_common_flags(rafaelia_demo)

    add_executable(rmr_bench bench/src/rmr_benchmark_main.c)
    target_link_libraries(rmr_bench PRIVATE rmr)
    rmr_apply_common_flags(rmr_bench)
    target_compile_definitions(rmr_bench PRIVATE RMR_JNI_BUILD=1)

    add_executable(bitomega_smoketest demo_cli/src/bitomega_smoketest.c)
    target_link_libraries(bitomega_smoketest PRIVATE rmr)
    rmr_apply_common_flags(bitomega_smoketest)

    add_executable(vectra_pulse_selftest demo_cli/src/vectra_pulse_selftest.c)
    target_link_libraries(vectra_pulse_selftest PRIVATE rmr)
    rmr_apply_common_flags(vectra_pulse_selftest)

    enable_testing()

    add_test(NAME bitomega_smoke
      COMMAND bitomega_smoketest
              --csv bench/results/bitomega_transitions.csv
              --dot bench/results/bitomega_transitions.dot
    )

    add_test(NAME vectra_pulse_selftest COMMAND vectra_pulse_selftest)

endif()
