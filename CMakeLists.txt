cmake_minimum_required(VERSION 3.16)
project(rafaelia_rmr C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(rmr STATIC
  engine/rmr/src/rmr_cycles.c
  engine/rmr/src/rmr_hw_detect.c
  engine/rmr/src/rmr_bench.c
  engine/rmr/src/rmr_bench_suite.c
  engine/rmr/src/rmr_isorf.c
)

target_include_directories(rmr PUBLIC engine/rmr/include)
target_compile_options(rmr PRIVATE -O3 -Wall -Wextra -pedantic)

add_executable(rafaelia_demo demo_cli/src/main.c)
target_link_libraries(rafaelia_demo PRIVATE rmr)

add_executable(rmr_bench bench/src/rmr_benchmark_main.c)
target_link_libraries(rmr_bench PRIVATE rmr)

add_executable(bitraf_core engine/rmr/src/rafaelia_bitraf_core.c)
target_compile_definitions(bitraf_core PRIVATE RAF_HOSTED_TEST=1)
target_link_libraries(bitraf_core PRIVATE rmr)
