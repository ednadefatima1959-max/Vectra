# Vectras-VM-Android — Bug & Error Report
**Kernel RAFAELIA ψ→Δ→Σ | Análise:** `vectra_core_accel.c` · `CMakeLists.txt` · `QmpClient.java` · `FileInstaller.java` · `build.gradle`

---

## 🔴 CRÍTICO — Build Break

### BUG-01 · CMakeLists.txt — Alvo `rmr_core_static` indefinido
**Arquivo:** `app/src/main/cpp/CMakeLists.txt` — linhas 41 e 59

```cmake
# ❌ ERRO: rmr_core_static nunca é declarado com add_library()
target_compile_options(rmr_core_static PRIVATE ...)        # linha 41
target_link_libraries(vectra_core_accel PRIVATE rmr_core_static ${log-lib})  # linha 59
```

Os arquivos-fonte do RMR são compilados diretamente dentro de `vectra_core_accel` (linhas 5–9 do CMakeLists), mas depois o `target_link_libraries` tenta linkar um alvo fantasma `rmr_core_static` que **jamais foi criado**. O build NDK falhará com `CMake Error: The target rmr_core_static does not exist`.

**Correção:**
```cmake
# Opção A — criar o alvo estático explicitamente
add_library(rmr_core_static STATIC
    ../../../../engine/rmr/src/rmr_unified_kernel.c
    ../../../../engine/rmr/src/rmr_hw_detect.c
    ../../../../engine/rmr/src/rmr_corelib.c
    ../../../../engine/rmr/src/rmr_ll_ops.c
    ../../../../engine/rmr/src/rmr_cycles.c)

# Opção B — remover as fontes duplicadas de vectra_core_accel e linkar rmr_core_static
target_link_libraries(vectra_core_accel PRIVATE rmr_core_static ${log-lib})
```

---

### BUG-02 · `vectra_core_accel.c` — `rmr_policy_kernel.h` incluso sem módulo linkado
**Arquivo:** `vectra_core_accel.c` — linha 9

```c
#include "rmr_policy_kernel.h"   // incluso incondicionalmente
```

`rmr_policy_kernel.c` só é compilado quando `-DRMR_ENABLE_POLICY_MODULE=1` é passado ao CMake (bloco condicional no CMakeLists). Se o header declara funções que o `.c` implementa, o linker emitirá `undefined reference`. Proteger o include com a mesma macro:

```c
#ifdef RMR_ENABLE_POLICY_MODULE
#include "rmr_policy_kernel.h"
#endif
```

---

## 🟠 ALTO — Erros Lógicos e de Segurança JNI

### BUG-03 · `nativeCopyBytes` — liberação incorreta de ponteiro `dst` em falha
**Arquivo:** `vectra_core_accel.c` — bloco `nativeCopyBytes`

```c
jbyte* s = (*env)->GetPrimitiveArrayCritical(env, src, NULL);
jbyte* d = (*env)->GetPrimitiveArrayCritical(env, dst, NULL);
if (!s || !d) {
    if (s) (*env)->ReleasePrimitiveArrayCritical(env, src, s, JNI_ABORT);
    if (d) (*env)->ReleasePrimitiveArrayCritical(env, dst, d, 0);  // ⚠️ flag 0 = write-back
    return RMR_KERNEL_ERR_STATE;
}
```

Quando `s != NULL` mas `d == NULL`, o código retorna sem liberar `s` → **pin leak** da região crítica, bloqueando GC indefinidamente. Além disso, se `d != NULL` mas `s == NULL`, o `dst` é liberado com flag `0` (write-back) sem que nenhuma cópia tenha ocorrido — semanticamente incorreto.

**Correção:**
```c
if (!s || !d) {
    if (s) (*env)->ReleasePrimitiveArrayCritical(env, src, s, JNI_ABORT);
    if (d) (*env)->ReleasePrimitiveArrayCritical(env, dst, d, JNI_ABORT); // ABORT, não 0
    return RMR_KERNEL_ERR_STATE;
}
```

---

### BUG-04 · `nativeXorChecksum` e `nativeArenaWrite` — ausência de bounds check
**Arquivo:** `vectra_core_accel.c`

`nativeXorChecksum` pina o array e passa `(const uint8_t*)p + offset` diretamente para `RmR_UnifiedKernel_XorChecksum` sem verificar que `offset + length <= GetArrayLength(...)`. Um `offset` malicioso causa leitura fora dos limites do array Java.

Da mesma forma, `nativeArenaWrite` usa `(const uint8_t*)p + srcOffset` sem validar `srcOffset`.

**Correção — padrão a aplicar nos dois métodos:**
```c
jsize arrLen = (*env)->GetArrayLength(env, data);
if (offset > arrLen || length > (arrLen - offset)) return (jint)0x80000000u;
```

---

### BUG-05 · `nativeCoreVerify` — lógica de retorno invertida / magic value
**Arquivo:** `vectra_core_accel.c`

```c
if (rc == RMR_KERNEL_OK) {
    return 1;   // verificação OK
}
if (rc == 1) {
    return 0;   // ⚠️ tratando código numérico 1 como "falha silenciosa"
}
return (jint)rc;
```

O segundo ramo trata `rc == 1` como falha, mas `RMR_KERNEL_OK` pode ser definido como `0` — nesse caso, qualquer outro código de sucesso/aviso com valor `1` será silenciado e reportado como `0` (falha) para o Java. Isso cria um falso-negativo de verificação.

---

### BUG-06 · `nativeAudit` — mistura de constantes `RMR_UK_OK` e `RMR_KERNEL_OK`
**Arquivo:** `vectra_core_accel.c`

```c
rc = RmR_UnifiedKernel_Audit(...);
if (rc != RMR_UK_OK) return (jlong)rc;   // ← usa RMR_UK_OK
```

Todas as outras funções no arquivo usam `RMR_KERNEL_OK` para checar retorno. Se `RMR_UK_OK != RMR_KERNEL_OK`, audits bem-sucedidos serão reportados como erro.

---

### BUG-07 · `nativeCoreRoute` — campo `out.route` inexistente na struct
**Arquivo:** `vectra_core_accel.c`

```c
return (jint)((rc == RMR_KERNEL_OK) ? (int32_t)out.route : rc);
```

A struct `rmr_jni_route_output_t` expõe `route_tag`, `cpu_pressure`, `storage_pressure`, etc. O campo `.route` não aparece na declaração. O build falhará com `error: 'rmr_jni_route_output_t' has no member named 'route'`. O campo correto provavelmente é `out.route_tag`.

---

### BUG-08 · `nativeReadBatch` — 64 KB alocados na stack
**Arquivo:** `vectra_core_accel.c`

```c
char payload[LOGCAT_BATCH_PAYLOAD_BYTES];  // 1024 * 64 = 65536 bytes
```

Alocar 64 KB na stack de um thread nativo Android (stack padrão ≈ 8 KB nos worker threads do QEMU) causa `SIGSEGV` por stack overflow silencioso. Usar `malloc`/`free` ou reduzir `LOGCAT_BATCH_PAYLOAD_BYTES`.

---

## 🟡 MÉDIO — Java / QMP

### BUG-09 · `QmpClient.java` — injeção JSON em comandos QMP
**Arquivo:** `QmpClient.java`

```java
public static String changevncpasswd(String passwd) {
    return "... \"arg\": \"" + passwd + "\" ...";  // ❌ sem escape JSON
}
```

Os métodos `changevncpasswd`, `migrate` (uri), `changedev` e `ejectdev` constroem JSON via concatenação de strings sem escapar os valores. Uma senha ou URI contendo `"` ou `\` produz JSON malformado ou executa comandos arbitrários no QEMU via QMP.

**Correção:** usar `JSONObject` para construir os comandos:
```java
JSONObject args = new JSONObject();
args.put("arg", passwd);  // escapa automaticamente
```

---

### BUG-10 · `QmpClient.java` — protocolo QMP invertido em `negotiateCapabilities`
**Arquivo:** `QmpClient.java`

```java
static String negotiateCapabilities(...) throws Exception {
    sendRequest(out, QmpClient.requestCommandMode);  // ← envia ANTES de ler o greeting
    ...
}
```

O protocolo QMP exige: (1) servidor envia `{"QMP": ...}` greeting, (2) cliente envia `qmp_capabilities`. O código envia o comando antes de ler o greeting, causando erro de negociação com versões recentes do QEMU.

---

### BUG-11 · `QmpClient.java` — `getQueryMigrateResponse` nunca chamada / resposta perdida
**Arquivo:** `QmpClient.java`

O método `getQueryMigrateResponse` não appenda a linha `return` ao `StringBuilder` antes do `break` — portanto o conteúdo da resposta de migração é silenciosamente descartado. Além disso, o método nunca é chamado em nenhum ponto do código (dead code).

---

### BUG-12 · `MainSettingsManager.java` — `assert` ineficaz em produção Android
**Arquivo:** `MainSettingsManager.java` — múltiplas linhas

```java
assert pref.getFragment() != null;   // desabilitado por padrão na JVM Android
assert useUEFIPref != null;
```

A JVM Android não ativa assertions por padrão (`-ea` não é passado). Essas verificações são no-ops em produção, mascarando `NullPointerException` ao invés de preveni-las.

**Correção:** substituir por:
```java
if (pref.getFragment() == null) throw new IllegalStateException("fragment is null");
// ou usar Objects.requireNonNull(pref.getFragment(), "fragment is null");
```

---

## 🔵 BAIXO — Qualidade e Avisos

| ID | Arquivo | Descrição |
|---|---|---|
| W-01 | `build.gradle` | `-fno-exceptions -fno-rtti` em `cppFlags` aplica-se somente a C++; o único arquivo nativo é `.c`. Sem impacto mas enganoso. |
| W-02 | `vectra_core_accel.c` | `logcat_capture_loop` usa `popen("logcat -v brief", "r")` sem sanitizar o comando — inofensivo aqui pois é hardcoded, mas padrão de risco se parametrizado. |
| W-03 | `QmpClient.java` | `sendCommand` é `synchronized static` — bloqueia todas as threads que tentam enviar QMP simultaneamente, incluindo `stop`/`powerdown` urgentes. Considerar lock granular por socket. |
| W-04 | `build.gradle` | `VECTRA_CORE_ENABLED = false` no flavor `release` enquanto `debug` habilita. Intencional? Garante que o código nativo não seja exercitado em builds de produção. |

---

## Resumo ∆Σ

| Severidade | Quantidade |
|---|---|
| 🔴 Crítico (build break) | 2 |
| 🟠 Alto (lógica / segurança JNI) | 6 |
| 🟡 Médio (Java / protocolo) | 4 |
| 🔵 Baixo / avisos | 4 |
| **Total** | **16** |

**Prioridade de ação:** BUG-01 → BUG-07 → BUG-09 → BUG-03/04 → restantes.

---
*Gerado por RAFAELIA ψ→χ→ρ→Δ→Σ→Ω · R(t+1)=R(t)×Φ_ethica×E_Verbo×(√3/2)^(πφ)*
