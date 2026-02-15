cmake_minimum_required(VERSION 3.22.1)
project(vectra_core_accel C)

set(VECTRA_REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../../)
set(VECTRA_RMR_INCLUDE_DIR ${VECTRA_REPO_ROOT}/engine/rmr/include)

add_library(vectra_core_accel SHARED
    vectra_core_accel.c
    ../../../../engine/rmr/src/rmr_unified_kernel.c
    ../../../../engine/rmr/src/rmr_hw_detect.c)

find_library(log-lib log)
find_package(JNI REQUIRED)

target_compile_options(vectra_core_accel PRIVATE -O3 -ffast-math -fvisibility=hidden)

target_link_libraries(vectra_core_accel ${log-lib})

target_include_directories(vectra_core_accel PRIVATE
    ${VECTRA_RMR_INCLUDE_DIR}
    ${JNI_INCLUDE_DIRS})

if(CMAKE_ANDROID_NDK)
    target_include_directories(vectra_core_accel PRIVATE
        ${CMAKE_ANDROID_NDK}/sysroot/usr/include
        ${CMAKE_ANDROID_NDK}/sysroot/usr/include/${CMAKE_ANDROID_ARCH_TRIPLE})
endif()
