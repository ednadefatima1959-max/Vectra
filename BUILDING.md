# BUILDING

## CLI prerequisites
- JDK 17 (baseline runtime)
- Android SDK Platform 35 + Build Tools 35.0.0
- NDK 27.2.12479018
- CMake 3.22.1

> Baseline único de CMake: host (raiz) e Android JNI usam 3.22.1 para manter paridade de toolchain.

## Setup environment (example)
```bash
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
export ANDROID_SDK_ROOT=/workspace/android-sdk
printf 'sdk.dir=%s\n' "$ANDROID_SDK_ROOT" > local.properties
```

If `local.properties` is missing, `./tools/gradle_with_jdk21.sh` now auto-writes `sdk.dir`
from `ANDROID_SDK_ROOT` (or `ANDROID_HOME`) when the directory exists.

## Build commands
```bash
./tools/gradle_with_jdk21.sh --version
./tools/gradle_with_jdk21.sh clean
./tools/gradle_with_jdk21.sh :app:assembleDebug --stacktrace
./tools/gradle_with_jdk21.sh :app:assembleRelease --stacktrace
./tools/gradle_with_jdk21.sh :app:lintDebug --stacktrace
```

> Use `./tools/gradle_with_jdk21.sh` como comando canônico: o wrapper aplica a política de JVM suportada (17/21) e faz autoajuste de `sdk.dir` quando possível.

## Binary Semantics Map
Mapeamento obrigatório entre alvo binário, contrato de entrada/saída e invariantes semânticos.

| Alvo/binário | Entrada esperada | Saída esperada | Invariantes verificados | Módulo responsável |
|---|---|---|---|---|
| `build/rmr_core_host` (host CMake) | Harness nativa com vetores determinísticos (`seed=0x5A17`) | Exit code `0` + logs de selftest no `bench/results/` | ABI pública estável (`rmr_unified_kernel.h`), CRC dos vetores, ausência de divergência entre lanes | `rmr_unified_kernel.h`, `rmr_policy_kernel.h`, targets CMake de selftest |
| `app/build/outputs/apk/debug/app-debug.apk` | Build Gradle com `APP_ABI_POLICY` explícita + SDK/NDK válidos | APK debug gerado sem erro + tasks de validação concluídas | Compatibilidade API mínima, matriz ABI coerente com política, parity de toolchain CMake | `app` (Gradle), `tools/gradle_with_jdk21.sh`, `gradle.properties` |
| `app/build/outputs/apk/release/app-release.apk` | Build release com assinatura e `buildStrict=true` | APK release assinado + gates de compliance aprovados | Mesmo comportamento semântico do debug; zero relaxamento de invariantes de segurança/compliance | pipeline Gradle release + `tools/ci/*` |
| `run_selftest` / `make run-selftest` | Execução local ou CI da suíte nativa por arquitetura | Relatório de selftest + falha imediata em regressão | Resultados equivalentes entre backends suportados (dentro da tolerância definida), sem regressão funcional | `Makefile`, `CMakeLists.txt`, targets `rmr_*_selftest` |

## SIMD/Backend Flags and Semantics
Os backends devem preservar semântica idêntica. SIMD acelera execução, mas **não** pode alterar resultado lógico.

| Backend | Ganho esperado | Risco/limitação | Invariantes que não podem mudar entre backends |
|---|---|---|---|
| `NEON` (ARM64) | Melhor throughput em operações vetoriais, menor latência em loops críticos | Dependente de hardware ARMv8-A com suporte NEON; diferenças de alinhamento podem expor bugs de implementação | Mesmo output bit-a-bit (ou mesma tolerância formal definida nos testes), mesma política de erro/retorno, mesma ordem semântica de transformação |
| `AVX2` (x86_64) | Aceleração relevante em hosts de validação e benchmarks locais | Não disponível em CPUs legadas; risco de variação se instruções não forem corretamente protegidas por feature-detect | Exatamente os mesmos invariantes funcionais do NEON e do fallback escalar |
| `scalar fallback` | Portabilidade máxima e baseline de referência | Menor desempenho absoluto; pode mascarar gargalos que aparecem apenas em SIMD | Referência semântica canônica: qualquer backend SIMD deve convergir para o mesmo resultado observado no fallback |

Flags recomendadas:
- ARM64/NEON: manter flags de arquitetura no toolchain sem alterar contrato de saída.
- x86_64/AVX2: habilitar apenas quando feature detect/runtime gate confirmar suporte.
- Fallback escalar: sempre disponível como rota determinística de validação.

## Reproducibility Contract
Contrato mínimo para repetibilidade de build/test e validação semântica.

- Seed fixa de validação: `0x5A17` (deve ser registrada nos logs de benchmark/selftest).
- Versão de dataset vetorial de teste: `vectra-selftest-dataset v1`.
- Ambiente mínimo: JDK 17, NDK 27.2.12479018, CMake 3.22.1, SDK Platform 35.
- Critério de sucesso:
  - Build termina com exit code `0`.
  - Selftests obrigatórios por arquitetura passam.
  - Invariantes do mapa semântico permanecem verdadeiros entre execuções e entre backends.
- Critério de falha:
  - Divergência semântica entre backends (NEON/AVX2/scalar) fora da tolerância definida.
  - Ausência de artefato obrigatório (APK/binário/log de selftest).
  - Quebra de contrato API/ABI ou gate de compliance em `buildStrict=true`.

## Semantic outcomes per build/test command
Cada comando abaixo possui resultado semântico observável (não apenas compilação).

| Comando | Resultado semântico esperado |
|---|---|
| `./tools/gradle_with_jdk21.sh --version` | Prova que o wrapper de toolchain está operacional e seleciona JVM suportada sem desvio de política. |
| `./tools/gradle_with_jdk21.sh clean` | Remove artefatos antigos para evitar falso-positivo de reprodutibilidade por cache residual. |
| `./tools/gradle_with_jdk21.sh :app:assembleDebug --stacktrace` | Gera APK debug semanticamente equivalente ao contrato do runtime, com validações de API/ABI aplicadas. |
| `./tools/gradle_with_jdk21.sh :app:assembleRelease --stacktrace` | Gera binário de release apto para distribuição oficial, mantendo invariantes de debug + gates estritos de release. |
| `./tools/gradle_with_jdk21.sh :app:lintDebug --stacktrace` | Verifica integridade estrutural/código Android sem alterar semântica funcional declarada. |
| `make run-selftest` | Executa validação funcional canônica por arquitetura; falha indica quebra semântica real do core nativo. |
| `cmake --build <build-dir> --target run_selftest` | Mesmo contrato de `make run-selftest`, garantindo consistência no fluxo CMake puro. |

## ABI policy
Configured by `APP_ABI_POLICY` and `SUPPORTED_ABIS` in `gradle.properties`.
Accepted policies in code and docs are exactly:
- `APP_ABI_POLICY=arm64-only` → `SUPPORTED_ABIS=arm64-v8a` (official minimum distribution)
- `APP_ABI_POLICY=with-32bit` → `SUPPORTED_ABIS=arm64-v8a,armeabi-v7a` (official distribution with 32-bit ARM)
- `APP_ABI_POLICY=all` → `SUPPORTED_ABIS=arm64-v8a,armeabi-v7a,x86,x86_64` (**internal validation only; not for official distribution**)

Default is arm64-only.

To include 32-bit ARM:
```bash
./tools/gradle_with_jdk21.sh -PAPP_ABI_POLICY=with-32bit -PSUPPORTED_ABIS=arm64-v8a,armeabi-v7a :app:assembleDebug
```

To run full internal ABI validation coverage:
```bash
./tools/gradle_with_jdk21.sh -PAPP_ABI_POLICY=all -PSUPPORTED_ABIS=arm64-v8a,armeabi-v7a,x86,x86_64 :app:assembleDebug
```


## Supported version matrix
All values below are defaults from `gradle.properties` and can be overridden with `-P`.

| Area | Property | Min | Default | Max/Policy |
|---|---|---:|---:|---:|
| Compile SDK | `compile.api` → fallback `COMPILE_API` | 35 | 35 | follows Android baseline updates |
| Target SDK | `target.api` → fallback `TARGET_API` | 35 (`release.min.target.api`) | 35 | follows Android baseline updates |
| Build Tools | `tools.version` → fallback `TOOLS_VERSION` | 35.0.0 | 35.0.0 | keep aligned with compile SDK |
| NDK | `ndk.version` → fallback `NDK_VERSION` | 23.x | 27.2.12479018 | latest validated in CI |
| CMake | `cmake.version` → fallback `CMAKE_VERSION` | 3.22.1 | 3.22.1 | keep host+JNI parity |
| Java language level | `java.language.version` → fallback `JAVA_LANGUAGE_VERSION` | 17 | 17 | 21 (when toolchain validated) |
| Gradle runtime JVM | `gradle.java.runtime.version` → fallback `GRADLE_JAVA_RUNTIME_VERSION` | 17 | 17 | `gradle.max.runtime.java.version` (default 21) |


Property precedence rule (to avoid config drift):
- Canonical property names use dotted lowercase keys (for example: `compile.api`, `tools.version`).
- Legacy aliases in uppercase snake case (for example: `COMPILE_API`, `TOOLS_VERSION`) are fallback-only for backward compatibility.
- When a legacy alias is used, the Gradle bootstrap emits a deprecation warning and continues.

Strictness control by pipeline context:
- A validação de bootstrap (`verifyBootstrapAssets`) e a validação final (`verifyGradleRuntimeJvm` + gates de API/ABI) compartilham a mesma política de `buildStrict` (warning em modo local, bloqueante em CI/release).
- `-PbuildStrict=false` (default): local/debug mode; validations emit warnings where allowed.
- `-PbuildStrict=true`: official CI/release mode; validations are blocking.

## CI blocking checks by pipeline type
- Official release CI (`buildStrict=true`):
  - `verifyGradleRuntimeJvm` (blocking)
  - API/ABI validations (`verifyMinApiAbiCompatibility`, release target checks) (blocking)
  - `verifyBootstrapAssets` + `verifyRepoFileDependencies` (blocking; requires Python)
- Local/dev or debug CI (`buildStrict=false`):
  - Same checks run, but max-JVM/API-ABI non-release gates can warn.
  - Python-dependent checks are skipped with warning if Python is unavailable.


## Selftest matrix expectations
Canonical gate names:
- Make: `make run-selftest`
- CMake: `cmake --build <build-dir> --target run_selftest`

Expected per architecture:

| Environment | Canonical gate | Required architecture-specific selftests | Artifact logs |
|---|---|---|---|
| Host x86_64 Linux | `make run-selftest` and/or `run_selftest` | `rmr_casm_bridge_selftest` (supported ABI), `rmr_neon_simd_selftest` not required | `bench/results/selftest-host-x86_64.log`, `bench/results/rmr_casm_bridge_selftest-x86_64.log` |
| Host arm64 Linux (aarch64/arm64) | `make run-selftest` and/or `run_selftest` | `rmr_neon_simd_selftest` (required), `rmr_casm_bridge_selftest` optional/unsupported | `bench/results/selftest-host-arm64.log`, `bench/results/rmr_neon_simd_selftest-arm64.log`, `bench/results/rmr_casm_bridge_selftest-arm64.log` |
| Android (NDK / app ABI lanes) | Native selftests must be wired through the same gate contract before release | ABI-specific execution required for `arm64-v8a`; other ABIs per policy (`APP_ABI_POLICY`) | Keep per-ABI logs in CI artifacts and fail lane when required selftests are missing or failing |
