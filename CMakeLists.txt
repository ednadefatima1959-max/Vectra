cmake_minimum_required(VERSION 3.22.1)
project(rafaelia_rmr C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ========================
# INCLUDE SOURCES MANIFEST
# ========================
set(RMR_SOURCES_CMAKE "${CMAKE_SOURCE_DIR}/engine/rmr/sources_rmr_core.cmake")
if(NOT EXISTS ${RMR_SOURCES_CMAKE})
    message(FATAL_ERROR "Arquivo de manifesto de fontes não encontrado: ${RMR_SOURCES_CMAKE}")
endif()
include(${RMR_SOURCES_CMAKE})

# ========================
# OPTIONS (valores padrão que podem ser sobrescritos via -D)
# ========================
option(RMR_JNI_BUILD "Use hosted/JNI libc path for root builds" ON)
set(RMR_BUILD_HOST_TOOLING "AUTO" CACHE STRING "Host-tooling compatibility path")
set_property(CACHE RMR_BUILD_HOST_TOOLING PROPERTY STRINGS AUTO ON OFF)

option(RMR_ENABLE_POLICY_MODULE ON)
option(RMR_ASM_CORE_EXPERIMENTAL ON)
option(RMR_ASM_CORE_X86_64_VALIDATED ON)
option(RMR_ASM_CORE_ARM64_VALIDATED ON)
option(RMR_ASM_CORE_RISCV64_VALIDATED ON)

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

# ========================
# ENV EXPORT (para scripts externos)
# ========================
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rmr_build_config.env"
  "RMR_JNI_BUILD=${RMR_JNI_BUILD_VALUE}\n"
  "RMR_BUILD_HOST_TOOLING=${RMR_BUILD_HOST_TOOLING_VALUE}\n"
  "RMR_ENABLE_POLICY_MODULE=${RMR_ENABLE_POLICY_MODULE_VALUE}\n"
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
# SOURCES COLLECTION
# ========================
rmr_manifest_apply_base(RMR_SOURCES ${RMR_SOURCE_GROUP_CORE} ${RMR_SOURCE_GROUP_HOST_ONLY})

if(RMR_ENABLE_POLICY_MODULE)
  rmr_manifest_apply_base(RMR_POLICY_SOURCES ${RMR_SOURCE_GROUP_OPTIONAL_POLICY})
  list(APPEND RMR_SOURCES ${RMR_POLICY_SOURCES})
endif()

# ========================
# ARCH DETECT (C-ASM opcional)
# ========================
set(RMR_HAS_CASM FALSE)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" PROC_LOWER)

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
# BITRAF (já incluído no grupo core, mas mantemos por compatibilidade)
# ========================
add_library(bitraf_static STATIC engine/rmr/src/bitraf.c)
set_target_properties(bitraf_static PROPERTIES OUTPUT_NAME bitraf)
rmr_apply_common_flags(bitraf_static)
target_link_libraries(rmr PUBLIC bitraf_static)

# ========================
# SYMBOL CHECK (apenas host)
# ========================
if(NOT ANDROID)
    find_program(BASH_EXECUTABLE bash)
    if(BASH_EXECUTABLE)
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
        message(WARNING "bash não encontrado, pulando verificação determinística de símbolos.")
    endif()
    add_custom_target(verify_contracts DEPENDS rmr)
endif()

# ========================
# EXECUTÁVEIS (apenas host)
# ========================
if(NOT ANDROID)
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
endif()
