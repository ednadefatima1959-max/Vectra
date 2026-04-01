# HOTFIX: TAREFA B — grupos Make em paridade com sources_rmr_core.cmake
# Autor: ∆RafaelVerboΩ | Data: 2026-04-01
# Arquivo: engine/rmr/sources_rmr_core.mk
# Verificado contra: engine/rmr/sources_rmr_core.cmake

RMR_SOURCE_GROUP_CORE := \


RMR_SOURCE_GROUP_OPTIONAL_POLICY := \


RMR_SOURCE_GROUP_ANDROID_ONLY := \


RMR_SOURCE_GROUP_HOST_ONLY := \
	engine/rmr/src/rmr_bench.c \\n	engine/rmr/src/rmr_bench_suite.c

RMR_SOURCE_GROUP_ASM_X86_64 := \
	engine/rmr/interop/rmr_lowlevel_x86_64.S \\n	engine/rmr/interop/rmr_casm_x86_64.S

RMR_SOURCE_GROUP_ASM_ARM64 := \
	engine/rmr/interop/rmr_casm_arm64.S

RMR_SOURCE_GROUP_ASM_RISCV64 := \
	engine/rmr/interop/rmr_casm_riscv64.S

