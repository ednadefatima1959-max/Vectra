# KEYINDEX — Mapa por camadas

Este índice organiza os pontos de entrada técnicos do repositório por camadas funcionais, com **arquivos centrais** e **ordem de leitura recomendada** para reduzir risco de mudanças acopladas.

---

## 1) Camada Semântica

Foco: modelo formal, invariantes, contrato de entrada/saída e governança do estado canônico.

### Arquivos centrais
- `README.md` — modelo formal (`S = (N, E, V, T, D)`), fluxo canônico e invariantes.
- `VECTRA_CORE.md` — referência do runtime e do contrato operacional.
- `engine/vectra_policy_kernel/src/lib.rs` — API semântica principal do kernel de política.
- `engine/vectra_policy_kernel/src/ops/*.rs` — operações semânticas (anchor/focus/len/trim/replace).
- `engine/rmr/include/rmr_policy_kernel.h` e `engine/rmr/src/rmr_policy_kernel.c` — espelho C da política/invariantes.

### Ordem de leitura recomendada
1. `README.md`
2. `VECTRA_CORE.md`
3. `engine/vectra_policy_kernel/src/lib.rs`
4. `engine/vectra_policy_kernel/src/ops/mod.rs` e demais `ops/*.rs`
5. `engine/rmr/include/rmr_policy_kernel.h`
6. `engine/rmr/src/rmr_policy_kernel.c`

---

## 2) Camada Execução

Foco: pipeline de execução, ponte JNI, grafo de execução e sincronização de saída.

### Arquivos centrais
- `engine/rmr/include/rmr_execution_graph.h` e `engine/rmr/src/rmr_execution_graph.c` — orquestração do grafo.
- `engine/rmr/include/rmr_unified_kernel.h` e `engine/rmr/src/rmr_unified_kernel.c` — kernel unificado de execução.
- `engine/rmr/include/rmr_unified_jni_base.h` e `engine/rmr/src/rmr_unified_jni_bridge.c` — fronteira com Android/JNI.
- `engine/rmr/include/rmr_output_sync.h` e `engine/rmr/src/rmr_output_sync.c` — sincronização de saída.
- `terminal-emulator/src/main/java/com/termux/terminal/TerminalSession.java` — sessão terminal e integração de runtime.
- `terminal-emulator/src/main/java/com/vectras/vm/core/ExecutionExecutors.java` — política de executores.

### Ordem de leitura recomendada
1. `engine/rmr/include/rmr_execution_graph.h`
2. `engine/rmr/src/rmr_execution_graph.c`
3. `engine/rmr/include/rmr_unified_kernel.h`
4. `engine/rmr/src/rmr_unified_kernel.c`
5. `engine/rmr/src/rmr_unified_jni_bridge.c`
6. `engine/rmr/src/rmr_output_sync.c`
7. `terminal-emulator/.../TerminalSession.java`
8. `terminal-emulator/.../ExecutionExecutors.java`

---

## 3) Camada Hardware

Foco: detecção de hardware, modelagem virtual, low-level ops e caches orientados ao host.

### Arquivos centrais
- `engine/rmr/include/rmr_hw_detect.h` e `engine/rmr/src/rmr_hw_detect_selftest.c` — detecção/autoteste de HW.
- `engine/rmr/include/rmr_vhw_model.h` e `engine/rmr/src/rmr_vhw_model.c` — modelo de hardware virtual.
- `engine/rmr/include/rmr_lowlevel.h`, `engine/rmr/src/rmr_lowlevel_portable.c`, `engine/rmr/src/rmr_lowlevel_reduce.c` — base low-level.
- `engine/rmr/include/rmr_ll_ops.h` e `engine/rmr/src/rmr_ll_ops.c` — operações low-level.
- `engine/rmr/include/rmr_tcg_cache.h` e `engine/rmr/src/rmr_tcg_cache.c` — cache de tradução/execução.
- `engine/rmr/include/rmr_predictive_cache.h` e `engine/rmr/src/rmr_predictive_cache.c` — cache preditivo.
- `engine/rmr/include/rmr_coherence_engine.h` e `engine/rmr/src/rmr_coherence_engine.c` — coerência entre camadas.

### Ordem de leitura recomendada
1. `engine/rmr/include/rmr_hw_detect.h`
2. `engine/rmr/src/rmr_hw_detect_selftest.c`
3. `engine/rmr/include/rmr_vhw_model.h`
4. `engine/rmr/src/rmr_vhw_model.c`
5. `engine/rmr/include/rmr_lowlevel.h` + `engine/rmr/include/rmr_ll_ops.h`
6. `engine/rmr/src/rmr_lowlevel_portable.c` + `engine/rmr/src/rmr_ll_ops.c`
7. `engine/rmr/src/rmr_tcg_cache.c` + `engine/rmr/src/rmr_predictive_cache.c`
8. `engine/rmr/src/rmr_coherence_engine.c`

---

## 4) Camada Dataset

Foco: fontes de dados para execução, catálogo web e artefatos de benchmark.

### Arquivos centrais
- `dataset/README.md` — visão geral de dataset local.
- `web/data/*.json` — catálogos de ROMs/setup/software usados por superfícies web.
- `web/store_list.json` — índice de loja/catálogos.
- `bench/results/.gitkeep` e `bench/results/.gitignore` — convenções de persistência de resultados.
- `reports/metrics/*.json` — snapshots de métricas.

### Ordem de leitura recomendada
1. `dataset/README.md`
2. `web/README.md`
3. `web/store_list.json`
4. `web/data/roms-store.json` e `web/data/software-store.json`
5. `bench/results/.gitignore`
6. `reports/metrics/README.md`

---

## 5) Camada Bench / Validação

Foco: repetibilidade de benchmark, validações de desempenho e verificação de invariantes.

### Arquivos centrais
- `bench/README.md` — fluxo de benchmark.
- `bench/scripts/run_bench.sh` — runner principal.
- `bench/src/rmr_benchmark_main.c` — ponto de entrada C para bench.
- `engine/rmr/include/rmr_bench.h`, `engine/rmr/src/rmr_bench.c` — API/execução de bench.
- `engine/rmr/include/rmr_bench_suite.h`, `engine/rmr/src/rmr_bench_suite.c` — suíte de validação.
- `engine/vectra_policy_kernel/tests/policy_kernel_tests.rs` — testes da camada semântica em Rust.
- `reports/POST_FIX_VALIDATION.md` — validação pós-correções.

### Ordem de leitura recomendada
1. `bench/README.md`
2. `bench/scripts/run_bench.sh`
3. `bench/src/rmr_benchmark_main.c`
4. `engine/rmr/include/rmr_bench.h` + `engine/rmr/src/rmr_bench.c`
5. `engine/rmr/include/rmr_bench_suite.h` + `engine/rmr/src/rmr_bench_suite.c`
6. `engine/vectra_policy_kernel/tests/policy_kernel_tests.rs`
7. `reports/POST_FIX_VALIDATION.md`

---

## How to change safely

Regras práticas para evitar regressões por impacto cruzado:

1. **Coherence ↔ Cache ↔ Graph ↔ Output Sync**
   - Mudança em `rmr_coherence_engine.*` exige revisão de `rmr_tcg_cache.*` + `rmr_predictive_cache.*` + `rmr_execution_graph.*` + `rmr_output_sync.*`.
   - Sempre validar: ordem de aplicação, invalidation de cache e determinismo de saída.

2. **Semântica ↔ Execução (policy kernel)**
   - Mudança em `vectra_policy_kernel/src/ops/*.rs` deve ser espelhada (quando aplicável) em `rmr_policy_kernel.*`.
   - Reexecutar testes Rust de policy e, no mínimo, smoke de execução C/JNI.

3. **Hardware ↔ Low-level ops ↔ Bench**
   - Ajuste em detecção/modelo de hardware (`rmr_hw_detect*`, `rmr_vhw_model*`) impacta caminhos low-level e métricas.
   - Comparar bench antes/depois com mesmo cenário e registrar desvio.

4. **Dataset ↔ Bench/Validação**
   - Alterações em `web/data/*.json` ou catálogos precisam de validação de compatibilidade de schema e consistência de chaves.
   - Nunca interpretar melhoria/piora de benchmark sem congelar o dataset usado.

5. **JNI/Terminal ↔ Output Sync**
   - Mudança de contrato em JNI/Terminal (`rmr_unified_jni_bridge.c`, `TerminalSession.java`) exige revisão de flush/sincronização de saída.
   - Verificar ordem de eventos (write, resize, teardown) e ausência de race em sessão.

6. **Regras mínimas de segurança de mudança**
   - Começar pelo header (`include/*.h`) antes do `src/*.c` para confirmar contrato.
   - Atualizar documentação afetada (`README`, `FILES_MAP`, `reports`) no mesmo commit da mudança funcional.
   - Se tocar caminho crítico de execução, incluir evidência de validação (comando + resultado) no PR.
