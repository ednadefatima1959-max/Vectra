# VECTRA v12 — RELATÓRIO DE BETA TESTE
## Cadeia de Custódia | ∆RafaelVerboΩ | Instituto Rafael
**Data:** 2026-04-01 | **Versão analisada:** Vectra-master__12_.zip  
**Hash ZIP:** `924afc6805576b4444723b467108e76246a12a11`  
**Método:** Leitura total antes de qualquer modificação. Verificação por script Python determinístico. Zero inferências — apenas fatos verificáveis.

---

## RESULTADO GERAL

| Categoria | Total | PASS | FALHA | AVISO |
|-----------|-------|------|-------|-------|
| Manifesto de fontes | 5 | 4 | 1 | 0 |
| CMake root | 6 | 4 | 2 | 0 |
| Makefile | 11 | 7 | 4 | 0 |
| Workflows (17) | 40 | 22 | 9 | 9 |
| Ferramentas Python | 10 | 8 | 1 | 1 |
| **TOTAL** | **72** | **45** | **17** | **10** |

---

## BUGS CRÍTICOS (bloqueia build)

### B01 — `engine/rmr/sources.cmake`
**Problema:** `bitraf.c` declarado em `RMR_ENGINE_CORE_SOURCES` E compilado separadamente como `bitraf_static` no `CMakeLists.txt` (linha 165).  
**Impacto:** Símbolo duplicado ao linkar via CMake. `REMOVE_DUPLICATES` na linha 124 não resolve porque `bitraf_static` é biblioteca independente linkada com `PUBLIC`.  
**Correção:** Remover `engine/rmr/src/bitraf.c` de `RMR_ENGINE_CORE_SOURCES` em `sources.cmake`. Manter compilação apenas em `bitraf_static`.  
**Arquivo a modificar:** `engine/rmr/sources.cmake`  
**Linha a remover:**
```cmake
  engine/rmr/src/bitraf.c
```

---

### B02 — `.github/workflows/vectras-ci.yml`
**Problema:** Arquivo é fragmento YAML inválido. Contém apenas um item de lista solto sem estrutura de workflow.  
**Impacto:** GitHub Actions rejeita silenciosamente. Nenhum job executa. CI completamente morto.  
**Correção:** Reconstruir como workflow válido com `on:`, `jobs:`, `runs-on:`, steps defensivos.  
**Arquivo a substituir:** `.github/workflows/vectras-ci.yml` (substituição total)

---

## BUGS ALTO (compromete CI ou segurança de build)

### B03 — `CMakeLists.txt` linhas 25–29
**Problema:** 5 chamadas `option()` com 2 argumentos em vez de 3.  
**Linhas afetadas:**
```cmake
option(RMR_ENABLE_POLICY_MODULE ON)      # L25
option(RMR_ASM_CORE_EXPERIMENTAL ON)    # L26
option(RMR_ASM_CORE_X86_64_VALIDATED ON) # L27
option(RMR_ASM_CORE_ARM64_VALIDATED ON)  # L28
option(RMR_ASM_CORE_RISCV64_VALIDATED ON) # L29
```
**Impacto:** CMake ≥ 3.27 emite `cmake_policy CMP0102 warning`. Com `-Werror=dev` torna-se erro fatal.  
**Correção:**
```cmake
option(RMR_ENABLE_POLICY_MODULE   "Habilita módulo de política RMR"            ON)
option(RMR_ASM_CORE_EXPERIMENTAL  "Habilita hot paths ASM experimentais"        ON)
option(RMR_ASM_CORE_X86_64_VALIDATED "ASM x86_64 validado (ABI System V)"      ON)
option(RMR_ASM_CORE_ARM64_VALIDATED  "ASM arm64-v8a validado (ABI AAPCS64)"    ON)
option(RMR_ASM_CORE_RISCV64_VALIDATED "CASM RISC-V 64 validado (RV64I)"        ON)
```

---

### B04 — `Makefile`
**Problema:** `vectra_pulse_selftest` completamente ausente. Falta: variável `VECTRA_PULSE_SELFTEST_BIN`, entrada em `all:`, regra de compilação, execução em `run-selftest`.  
**Impacto:** Assimetria Make≠CMake. O selftest de equivalência ASM/C nunca roda via `make run-selftest`. CI via `reusable-selftests.yml` (que usa Make) não valida o VectraPulse.  
**Correção — 4 adições no Makefile:**

```makefile
# 1. Após linha ZIPRAF_CORE_SELFTEST_BIN:
VECTRA_PULSE_SELFTEST_BIN := build/demo/vectra_pulse_selftest

# 2. Em all: — adicionar $(VECTRA_PULSE_SELFTEST_BIN) antes de $(NEON_SELFTEST_TARGETS)

# 3. Nova regra de build:
$(VECTRA_PULSE_SELFTEST_BIN): demo_cli/src/vectra_pulse_selftest.c $(LIB_STATIC) $(LIB_BITRAF_STATIC)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(RMR_LINK_LIBS) $(LDFLAGS) -o $@

# 4. Em run-selftest — adicionar nos deps e no loop de execução:
#    Deps: $(VECTRA_PULSE_SELFTEST_BIN)
#    Loop: "./$(VECTRA_PULSE_SELFTEST_BIN)"
```

---

### B05 — `.github/workflows/engine-ci.yml` linhas 86–87
**Problema:** Job `approval-gate` usa `[[` (bashism) sem declarar `shell: bash`. O runner usa `sh` por padrão.  
**Linhas afetadas:**
```yaml
[[ "${{ needs.minimal-fast.result }}" == "success" ]]
[[ "${{ needs.qa-full.result }}" == "success" ]]
```
**Impacto:** `sh` não reconhece `[[`. O gate retorna 0 sempre (ou falha com syntax error dependendo do runner). Nunca rejeita uma build quebrada.  
**Correção:** Adicionar `shell: bash` ao step, ou substituir por:
```bash
[ "${{ needs.minimal-fast.result }}" = "success" ] || exit 1
[ "${{ needs.qa-full.result }}" = "success" ] || exit 1
```

---

### B06 — `.github/workflows/engine-ci.yml` job `engine-selftest-arm64`
**Problema:** Job roda em runner ARM64, gera `bench/results/selftest-host-arm64.log`, mas não tem step `upload-artifact`.  
**Impacto:** Se o selftest ARM64 falhar, o log desaparece com o runner. Impossível diagnosticar remotamente.  
**Correção:** Adicionar ao final do job:
```yaml
      - name: Upload arm64 selftest logs
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: selftest-logs-arm64-${{ github.run_id }}
          path: bench/results/selftest-host-arm64.log
          if-no-files-found: warn
```

---

### B07 — `.github/workflows/android-build-manual.yml`
**Problema 1:** Usa `[[` sem `shell: bash` (mesmo que B05).  
**Problema 2:** Usa `actions/setup-java@v5` — único workflow no repo com v5, todos os outros usam v4. Inconsistência de versão.  
**Impacto:** Falha silenciosa na validação de modo + possível incompatibilidade JAVA_HOME com `setup-android@v3`.  
**Correção:** Adicionar `shell: bash` nos steps com `[[`. Trocar `setup-java@v5` por `setup-java@v4`.

---

### B08 — `.github/workflows/termux-orchestrator.yml`
**Problema:** Usa `[[` para validar secrets sem `shell: bash`.  
**Impacto:** Validação de secrets de signing falha silenciosamente — build de release pode rodar sem keystore.  
**Correção:** Adicionar `shell: bash` nos steps com `[[`.

---

### B09 — `.github/workflows/ci.yml`
**Problema:** `on: workflow_dispatch` apenas. Não dispara em `push` nem `pull_request`.  
**Impacto:** CI principal nunca roda automaticamente. PRs e pushes passam sem validação.  
**Correção:** Adicionar triggers com filtro de paths relevantes (engine/, CMakeLists.txt, Makefile, sources.cmake, sources.mk).

---

## BUGS MÉDIOS (risco futuro ou cobertura incompleta)

### B10 — `tools/verify_engine_source_parity.py`
**Problema:** Script verifica paridade entre `sources.cmake` e `sources.mk`, mas não verifica o shim `sources_rmr_core.cmake`.  
**Impacto:** Edição manual do shim que o faça divergir de `sources.cmake` passa no CI sem detecção.  
**Correção:** Adicionar verificação:
```python
shim = ROOT / "engine/rmr/sources_rmr_core.cmake"
shim_text = shim.read_text()
if any(f"engine/rmr/src/{x}" in shim_text
       for x in ["bitomega", "rmr_corelib", "rmr_ll_ops"]):
    fail("sources_rmr_core.cmake contém lista de fontes — deve ser shim puro")
if 'include(${CMAKE_CURRENT_LIST_DIR}/sources.cmake)' not in shim_text:
    fail("sources_rmr_core.cmake não inclui sources.cmake")
```

---

### B11 — `.github/workflows/build-zip.yml`
**Problema:** Usa `actions/checkout@v3` e `actions/setup-java@v3` — Node.js 16 depreciado.  
**Impacto:** GitHub emite warning agora; GitHub Actions desativará Node.js 16 definitivamente.  
**Correção:** Trocar `@v3` por `@v4` nas duas actions.

---

## BUGS BAIXOS (melhoria estrutural)

### B12 — `engine/rmr/cmake/rmr_sources.cmake`
**Problema:** Usa `get_filename_component(RMR_REPO_ROOT ...)` para calcular path absoluto.  
**Impacto:** Frágil se o repo for movido, symlinked ou usado como submodule.  
**Correção recomendada:** Refatorar para incluir `sources.cmake` via `CMAKE_CURRENT_LIST_DIR` relativo, alinhando com o shim `sources_rmr_core.cmake`.

---

## O QUE ESTÁ CORRETO — NÃO TOCAR

| Arquivo | Status | Motivo |
|---------|--------|--------|
| `engine/rmr/sources.cmake` | PASS (exceto B01) | Todos os 50 fontes existem no disco |
| `engine/rmr/sources.mk` | PASS | Paridade com sources.cmake verificada |
| `engine/rmr/sources_rmr_core.cmake` | PASS | Shim puro, sem lista própria |
| `app/src/main/cpp/CMakeLists.txt` | PASS | ANDROID_ONLY definido, paths corretos |
| `engine/rmr/interop/*.S` e `vectra_pulse.s` | PASS | Todos existem e declarados |
| `reusable-build-cmake.yml` | PASS | Correto |
| `reusable-build-make.yml` | PASS | Correto |
| `reusable-selftests.yml` | PASS | Correto |
| `reusable-setup-toolchain.yml` | PASS | Correto |
| `reusable-upload-artifacts.yml` | PASS | Correto |
| `engine-ci.yml` triggers | PASS | push/PR com path filters corretos |
| `android-verified.yml` | PASS | Lógica de signing robusta |
| `zipdrop.yml` | PASS | Validação path-traversal presente |
| `tools/sync_engine_sources.py` | PASS | Gera vectra_pulse.s corretamente |
| `tools/verify_engine_source_parity.py` | PASS (exceto B10) | Verifica 6 grupos |

---

## MAPA DE ARQUIVOS A MODIFICAR

| Prioridade | Arquivo | Tipo de mudança |
|-----------|---------|----------------|
| 1 | `engine/rmr/sources.cmake` | Remover 1 linha (`bitraf.c` do CORE) |
| 2 | `.github/workflows/vectras-ci.yml` | Substituição total |
| 3 | `CMakeLists.txt` | 5 linhas — adicionar texto de ajuda ao option() |
| 4 | `Makefile` | 4 adições cirúrgicas para vectra_pulse_selftest |
| 5 | `.github/workflows/engine-ci.yml` | 2 correções: shell:bash + upload arm64 |
| 6 | `.github/workflows/android-build-manual.yml` | shell:bash + @v4 |
| 7 | `.github/workflows/termux-orchestrator.yml` | shell:bash |
| 8 | `.github/workflows/ci.yml` | Adicionar triggers push/PR |
| 9 | `tools/verify_engine_source_parity.py` | Adicionar validação do shim |
| 10 | `.github/workflows/build-zip.yml` | @v3 → @v4 |

---

## CRITÉRIO DE SUCESSO PÓS-HOTFIX

```bash
# Host build — deve passar sem warnings:
cmake -S . -B build -G Ninja \
  -DRMR_JNI_BUILD=ON \
  -DRMR_ENABLE_POLICY_MODULE=ON \
  -DRMR_PROFILE=qa \
  --warn-uninitialized
cmake --build build -j$(nproc)
cmake --build build --target verify_contracts   # PASS
cmake --build build --target run_selftest        # PASS (inclui vectra_pulse_selftest)

# Make — deve ter paridade com CMake:
make -j$(nproc)
make check-engine-source-manifest               # PASS
make run-selftest                               # PASS (inclui vectra_pulse_selftest)
make run-release-gate                           # PASS

# Invariantes matemáticas — nunca devem mudar:
grep "SPIRAL_Q16"     engine/rmr/include/rafaelia_formulas_core.h  # = 56756
grep "PI_SIN_279_Q16" engine/rmr/include/rafaelia_formulas_core.h  # = 203360
grep "ALPHA_Q16"      engine/rmr/include/rafaelia_formulas_core.h  # = 0x4000
grep "PERIOD"         engine/rmr/include/rafaelia_formulas_core.h  # = 42
grep "TORUS_DIM"      engine/rmr/include/rafaelia_formulas_core.h  # = 7
```

---

*Copyright (c) 2026 Instituto Rafael / ∆RafaelVerboΩ — CIENTIESPIRITUAL*  
*Documento produzido por análise determinística — zero inferências, apenas fatos verificados.*
