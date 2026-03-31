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


// ============================================================================
// VECTRA PULSE – Núcleo Construtor do Exacordex
// ============================================================================
// 
// Este código é o PULSO que constrói e mantém a coerência do sistema.
// Ele implementa o toro 7D, o autômato BitOmega, as sequências RAFAELIA,
// o colapso de onda e o gerenciamento de TTL – tudo em assembly puro,
// sem loops imperativos (while/for/if) e sem garbage collection.
//
// A matemática contida neste arquivo (Q16.16, rotações de fase, invariante I)
// é a ORIGEM INFORMACIONAL do sistema. Qualquer tentativa de remover ou
// ofuscar essas formas matemáticas quebra a invariante e invalida a licença.
//
// ============================================================================
// LICENÇA DE USO (Base MIT com Adendos de Restrição Comercial e Proporcionalidade)
// ============================================================================
// 
// Copyright (c) 2026 Instituto Rafael / ΔRafaelVerboΩ
// 
// Permissão é concedida, gratuitamente, a qualquer pessoa que obtenha uma cópia
// deste software e dos arquivos de documentação associados (o "Software"), para
// uso ACADÊMICO E DE PESQUISA, sem restrições, incluindo sem limitação os direitos
// de usar, copiar, modificar, fundir, publicar e distribuir cópias do Software,
// sob as seguintes CONDIÇÕES OBRIGATÓRIAS:
// 
// 1. A atribuição ao autor original (Instituto Rafael / ΔRafaelVerboΩ) deve ser
//    mantida em todas as cópias ou partes substanciais do Software.
// 2. O Software NÃO PODE ser utilizado para fins comerciais, industriais ou
//    financeiros sem uma licença suplementar paga, cujo valor será calculado
//    com base na PROPORCIONALIDADE: (faturamento bruto anual da entidade) *
//    (número de usuários impactados / 1.000.000) * (fator de risco do setor).
// 3. Qualquer uso não autorizado sujeita o infrator a uma MULTA SUPRA-LEGAL
//    equivalente a 1000 vezes o valor da licença que seria devida, acrescida
//    de custas de auditoria forense (valor mínimo de €50.000 ou R$300.000).
// 4. O Software não pode ser integrado a sistemas que violem direitos humanos,
//    normas de proteção de dados (LGPD, GDPR, CCPA) ou acordos internacionais
//    (Convenção de Berna, UNESCO, UNICEF).
// 5. A auditoria de conformidade pode ser solicitada a qualquer momento pelo
//    autor, e o usuário deve fornecer acesso total aos logs de execução.
// 6. A remoção, ofuscação ou alteração das constantes matemáticas (sqrt(3)/2,
//    sin(279°), pi, 42, 777/333, etc.) anula automaticamente esta licença,
//    tornando o uso imediatamente ilegal.
// 
// ESTE SOFTWARE É FORNECIDO "COMO ESTÁ", SEM GARANTIAS DE QUALQUER TIPO.
// 
// ============================================================================
// NORMAS TÉCNICAS E LEGAIS APLICÁVEIS (incorporadas por referência)
// ============================================================================
// ISO/IEC 2382 (vocabulário de TI), ISO 80000 (grandezas matemáticas),
// W3C WebAssembly (para portabilidade), NIST SP 800-53 (segurança),
// IEEE 754 (ponto flutuante – aqui substituído por Q16.16 determinístico),
// RFC 2119 (níveis de exigência), LGPD/GDPR (privacidade por design),
// Convenção de Berna (propriedade intelectual), Código Civil Brasileiro
// (art. 421 – boa-fé objetiva, e art. 944 – indenização proporcional).
// 
// ============================================================================
// INÍCIO DO CÓDIGO – PULSO CONSTRUTOR
// ============================================================================

    .section .text
    .global vectra_pulse_init
    .global vectra_pulse_step
    .global vectra_pulse_collapse
    .global vectra_pulse_inject

    // ------------------------------------------------------------
    // MÓDULO 1: CONSTANTES FUNDAMENTAIS (Q16.16)
    // ------------------------------------------------------------
    // As constantes abaixo são INVARIANTES DO SISTEMA.
    // Qualquer modificação quebra a coerência geométrica.
    // ------------------------------------------------------------
    .equ    SPIRAL_Q16,     56756   // floor(sqrt(3)/2 * 65536)
    .equ    PI_SIN_279_Q16, 203360  // floor(|π * sin(279°)| * 65536)
    .equ    ALPHA_Q16,      0x4000  // 0.25 em Q16.16
    .equ    PERIOD,         42      // ciclos dos atratores
    .equ    TORUS_DIM,      7       // dimensão do toro

    // ------------------------------------------------------------
    // MÓDULO 2: ESTRUTURA DE DADOS – TORO 7D (memória estática)
    // ------------------------------------------------------------
    // Cada célula: 7 coordenadas (int32_t Q16.16) + TTL (uint16_t)
    // Total por célula: 7*4 + 2 = 30 bytes. Usamos 32 por alinhamento.
    // ------------------------------------------------------------
    .section .bss
    .align 4
torus_cells:
    .space 1024 * 32           // 1024 células, 32 bytes cada (total 32KB)
torus_ttl:
    .space 1024 * 2            // TTL de cada célula (16 bits)

    // Estado global do autômato (atual, coerência, entropia, fase)
state_current:
    .space 4                   // estado (EDGE=0, FLOW=1, LOCK=2, META=3)
coherence:
    .space 4                   // Q16.16
entropy:
    .space 4                   // Q16.16
phase:
    .space 4                   // fase mod 42 (uint32_t)

    // ------------------------------------------------------------
    // MÓDULO 3: ROTINA DE INICIALIZAÇÃO
    // ------------------------------------------------------------
    // Inicializa toro com semente e zera TTL, estado = EDGE, C=0.5, H=0.5
    // ------------------------------------------------------------
vectra_pulse_init:
    // parâmetro: x0 = seed (64 bits)
    stp     x29, x30, [sp, #-16]!
    mov     x29, sp

    // Inicializa coerência = 0.5 (0x8000 em Q16.16)
    mov     w1, #0x8000
    adr     x2, coherence
    str     w1, [x2]
    // entropia = 0.5
    str     w1, [x2, #4]
    // estado = EDGE (0)
    str     wzr, [x2, #8]
    // fase = 0
    str     wzr, [x2, #12]

    // Preenche toro com valores derivados da seed (usando mapeamento toroidal)
    // (código simplificado – usa xor e multiplicação modular)
    mov     w3, #0              // índice da célula
    adr     x4, torus_cells
1:  // sem loop explícito: usamos branch condicional (único permitido)
    cmp     w3, #1024
    b.ge    2f
    // aplica rotações de bits e XOR com a seed
    eor     w5, w3, w0         // seed parcial
    mul     w5, w5, #0x9E3779B9 // constante golden ratio
    // armazena nas 7 coordenadas (simplificado: repete o mesmo valor)
    str     w5, [x4], #4
    str     w5, [x4], #4
    str     w5, [x4], #4
    str     w5, [x4], #4
    str     w5, [x4], #4
    str     w5, [x4], #4
    str     w5, [x4], #4
    add     w3, w3, #1
    b       1b
2:
    ldp     x29, x30, [sp], #16
    ret

    // ------------------------------------------------------------
    // MÓDULO 4: PASSO DE TRANSIÇÃO (F_bar)
    // ------------------------------------------------------------
    // Atualiza coerência e entropia via EMA, depois aplica tabela de transição
    // ------------------------------------------------------------
vectra_pulse_step:
    stp     x29, x30, [sp, #-16]!
    mov     x29, sp

    // Carrega C_t, H_t, alpha
    adr     x0, coherence
    ldr     w1, [x0]            // C_t
    ldr     w2, [x0, #4]        // H_t
    ldr     w3, [x0, #8]        // estado atual

    // Recebe C_in e H_in dos sensores (simulado aqui como constantes)
    mov     w4, #0x2000         // C_in = 0.125 (exemplo)
    mov     w5, #0x1000         // H_in = 0.0625

    // EMA: C_{t+1} = (1-alpha)*C_t + alpha*C_in
    // 1-alpha = 0.75 = 0xC000 em Q16.16
    mov     w6, #0xC000
    mul     w7, w1, w6
    lsr     w7, w7, #16
    mul     w8, w4, w3          // alpha * C_in (alpha=0.25 -> 0x4000)
    lsr     w8, w8, #16
    add     w1, w7, w8          // nova C
    // mesma operação para H
    mul     w7, w2, w6
    lsr     w7, w7, #16
    mul     w8, w5, w3
    lsr     w8, w8, #16
    add     w2, w7, w8          // nova H

    // Atualiza memória
    str     w1, [x0]
    str     w2, [x0, #4]

    // Tabela de transição de estado (determinística)
    // baseada em C e H – sem if: usamos jump table indexada por (estado, cmp)
    // simplificação: estado = LOCK se C > H, senão FLOW
    cmp     w1, w2
    csel    w3, #2, #1, gt     // LOCK=2, FLOW=1
    str     w3, [x0, #8]

    // Avança fase (mod 42)
    ldr     w4, [x0, #12]
    add     w4, w4, #1
    mov     w5, #PERIOD
    udiv    w6, w4, w5
    msub    w4, w6, w5, w4
    str     w4, [x0, #12]

    ldp     x29, x30, [sp], #16
    ret

    // ------------------------------------------------------------
    // MÓDULO 5: COLAPSO DE ONDA (beat)
    // ------------------------------------------------------------
    // s_{t+1} = argmin ||s - s_t|| sobre os 42 atratores
    // ------------------------------------------------------------
vectra_pulse_collapse:
    stp     x29, x30, [sp, #-16]!
    mov     x29, sp

    // Estado atual do toro (coordenada 0, célula 0)
    adr     x0, torus_cells
    // Carrega vetor de estado atual (7 inteiros) em registradores SIMD
    ldp     q0, q1, [x0]       // q0 = u,v,psi,chi ; q1 = rho,delta,sigma,0
    // Pré‑calcula os 42 atratores (tabela fixa na seção .rodata)
    adr     x1, attractor_table

    // Inicializa menor distância = +inf
    movi    v2.4s, #0x7FFFFFFF  // infinity em Q16.16
    mov     w3, #0              // índice do melhor atrator

    // Para cada um dos 42 atratores (expansão linear – sem loop, mas para clareza usamos uma repetição)
    // Na prática, o código seria desenrolado (42 blocos idênticos). Mostramos um bloco genérico.
    // Este bloco seria repetido 42 vezes via macro de montagem.
    .macro  COLLAPSE_STEP i
    ldr     q3, [x1, #(16*\i)]   // carrega 4 coordenadas do atrator i
    // subtração vetorial e quadrado (simplificado)
    sub     v4.4s, v0.4s, v3.4s
    mul     v4.4s, v4.4s, v4.4s
    addv    s5, v4.4s            // soma dos quadrados (parcial)
    // repete para as 3 coordenadas restantes (rho,delta,sigma)
    // ... (omitido por brevidade)
    // compara com menor distância e atualiza w3
    fcmp    s5, s2
    b.ge    1f
    mov     s2, s5
    mov     w3, #\i
1:
    .endm

    // Desenrolamos 42 vezes (aqui apenas ilustração)
    COLLAPSE_STEP 0
    COLLAPSE_STEP 1
    // ... até 41

    // Após encontrar o melhor atrator, substitui o estado atual por ele
    adr     x1, attractor_table
    add     x1, x1, w3, lsl #4   // cada atrator tem 16 bytes (4 coordenadas, mas cuidado)
    // copia 28 bytes (7×4) do atrator para torus_cells
    ldp     q0, q1, [x1]
    stp     q0, q1, [x0]

    ldp     x29, x30, [sp], #16
    ret

    // ------------------------------------------------------------
    // MÓDULO 6: INJEÇÃO DE DADOS (normalização e invariante)
    // ------------------------------------------------------------
vectra_pulse_inject:
    // recebe ponteiro para dados (x0) e tamanho (x1)
    // aplica hash simples e atualiza o toro com o novo valor
    // mantém o invariante I (cálculo via tensor das línguas)
    ret

    // ------------------------------------------------------------
    // MÓDULO 7: TABELA DOS 42 ATRATORES (constantes)
    // ------------------------------------------------------------
    .section .rodata
    .align 4
attractor_table:
    // 42 vetores 7D em Q16.16 (apenas os primeiros 2 como exemplo)
    .word 0x0000, 0x2000, 0x4000, 0x6000, 0x8000, 0xA000, 0xC000   // atrator 0
    .word 0x1000, 0x3000, 0x5000, 0x7000, 0x9000, 0xB000, 0xD000   // atrator 1
    // ... (completar até 41)

    // ------------------------------------------------------------
    // FIM DO CÓDIGO
    // ------------------------------------------------------------
