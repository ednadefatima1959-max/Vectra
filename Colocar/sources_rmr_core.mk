# HOTFIX BUG-V12-D: sources_rmr_core.mk divergia de sources.mk.
# O Makefile inclui sources.mk (correto), mas sources_rmr_core.mk ficou como
# arquivo orfão com grupos diferentes e sem vectra_pulse.s no CASM x86_64.
# Corrigido: este arquivo agora é ESPELHO EXATO de sources.mk,
# incluindo vectra_pulse.s no grupo ASM x86_64.
#
# FONTE DE VERDADE: engine/rmr/sources.cmake
# Regenerar com: python3 tools/sync_engine_sources.py
#
# Copyright (c) 2026 Instituto Rafael / ∆RafaelVerboΩ

# ----------------------------------------------------------------
# CORE — compilado em host e Android (todos os módulos)
# ----------------------------------------------------------------
RMR_ENGINE_CORE_SOURCES :=
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/bitomega.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/bitraf.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_bit_broadcast.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_apk_module.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_attractor.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_baremetal_compat.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_bench.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_benchmark.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_bench_suite.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_casm_bridge.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_corelib.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_coherence_engine.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_cycles.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_ethica_loss.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_execution_graph.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_invariant_extractor.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_host_compat.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_hw_detect.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_isorf.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_ll_ops.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_ll_tuning.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_ir_bridge.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_lowlevel_mix.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_lowlevel_portable.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_lowlevel_reduce.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_math_fabric.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_output_sync.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_neon_simd.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_simd_geometry.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_qemu_bridge.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_predictive_cache.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_tcg_cache.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_unified_jni_bridge.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_unified_kernel.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_vhw_model.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_virtio_blk.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rmr_zipraf_core.c
RMR_ENGINE_CORE_SOURCES += engine/rmr/src/rafaelia_formulas_core.c

# ----------------------------------------------------------------
# POLICY — módulo opcional de política RMR
# ----------------------------------------------------------------
RMR_ENGINE_POLICY_SOURCES :=
RMR_ENGINE_POLICY_SOURCES += engine/rmr/src/rmr_policy_kernel.c

# ----------------------------------------------------------------
# ASM x86_64 — lowlevel + casm + vectra_pulse
# HOTFIX: vectra_pulse.s estava ausente neste arquivo (presente em sources.mk)
# ----------------------------------------------------------------
RMR_ENGINE_ASM_X86_64_LOWLEVEL_SOURCES :=
RMR_ENGINE_ASM_X86_64_LOWLEVEL_SOURCES += engine/rmr/interop/rmr_lowlevel_x86_64.S

RMR_ENGINE_ASM_X86_64_CASM_SOURCES :=
RMR_ENGINE_ASM_X86_64_CASM_SOURCES += engine/rmr/interop/rmr_casm_x86_64.S
RMR_ENGINE_ASM_X86_64_CASM_SOURCES += engine/rmr/interop/vectra_pulse.s

# ----------------------------------------------------------------
# ASM ARM64
# ----------------------------------------------------------------
RMR_ENGINE_ASM_ARM64_SOURCES :=
RMR_ENGINE_ASM_ARM64_SOURCES += engine/rmr/interop/rmr_casm_arm64.S

# ----------------------------------------------------------------
# ASM RISC-V 64
# ----------------------------------------------------------------
RMR_ENGINE_ASM_RISCV64_SOURCES :=
RMR_ENGINE_ASM_RISCV64_SOURCES += engine/rmr/interop/rmr_casm_riscv64.S

# ----------------------------------------------------------------
# Aliases para compatibilidade com Make targets legados
# ----------------------------------------------------------------
ENGINE_CORE_SRCS              := $(RMR_ENGINE_CORE_SOURCES)
ENGINE_POLICY_SRCS            := $(RMR_ENGINE_POLICY_SOURCES)
ENGINE_ASM_X86_64_LOWLEVEL_SRCS := $(RMR_ENGINE_ASM_X86_64_LOWLEVEL_SOURCES)
ENGINE_ASM_X86_64_CASM_SRCS   := $(RMR_ENGINE_ASM_X86_64_CASM_SOURCES)
ENGINE_ASM_ARM64_SRCS         := $(RMR_ENGINE_ASM_ARM64_SOURCES)
ENGINE_ASM_RISCV64_SRCS       := $(RMR_ENGINE_ASM_RISCV64_SOURCES)

# Alias legado: grupos nomeados esperados por sources_rmr_core.cmake shim
RMR_SOURCE_GROUP_CORE            := $(RMR_ENGINE_CORE_SOURCES)
RMR_SOURCE_GROUP_OPTIONAL_POLICY := $(RMR_ENGINE_POLICY_SOURCES)
RMR_SOURCE_GROUP_ASM_X86_64      := $(RMR_ENGINE_ASM_X86_64_LOWLEVEL_SOURCES) \
                                     $(RMR_ENGINE_ASM_X86_64_CASM_SOURCES)
RMR_SOURCE_GROUP_ASM_ARM64       := $(RMR_ENGINE_ASM_ARM64_SOURCES)
RMR_SOURCE_GROUP_ASM_RISCV64     := $(RMR_ENGINE_ASM_RISCV64_SOURCES)
RMR_SOURCE_GROUP_ANDROID_ONLY    := engine/rmr/src/rmr_tcg_cache.c \
                                     engine/rmr/src/rmr_virtio_blk.c \
                                     engine/rmr/src/rmr_attractor.c \
                                     engine/rmr/src/rmr_vhw_model.c \
                                     engine/rmr/src/rmr_ethica_loss.c
RMR_SOURCE_GROUP_HOST_ONLY       := engine/rmr/src/rmr_bench.c \
                                     engine/rmr/src/rmr_bench_suite.c
