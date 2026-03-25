# API — Vectra Engine & Android

Este documento define o contrato público estável das APIs em duas superfícies:

- **Engine C APIs** (headers em `engine/rmr/include/*`)
- **Android/Java APIs** (classes públicas em `app/src/main/java/com/vectras/vm/*`)

---

## Engine C APIs

### Headers públicos
- `engine/rmr/include/bitraf.h`
- `engine/rmr/include/bitraf_version.h`
- `engine/rmr/include/rmr_coherence_engine.h`
- `engine/rmr/include/rmr_execution_graph.h`
- `engine/rmr/include/rmr_predictive_cache.h`

### Versão pública (SemVer)
- `BITRAF_VERSION_MAJOR`
- `BITRAF_VERSION_MINOR`
- `BITRAF_VERSION_PATCH`
- `BITRAF_VERSION_STRING`

### Convenções gerais de erro (C)
As APIs C deste módulo evitam alocação dinâmica e exceções. O modelo de erro é por retorno/sentinela.

| Código | Nome | Semântica | Mapeamento típico |
|---|---|---|---|
| `0` | `RMR_OK_ZERO` | sucesso para inicialização (`bitraf_init`) | `bitraf_init` |
| `1` | `RMR_OK_TRUE` | sucesso booleano | `bitraf_verify == 1` |
| `0` | `RMR_ERR_FALSE_OR_EMPTY` | falha booleana ou saída vazia/erro | `bitraf_verify == 0`, `*_compress/reconstruct == 0`, `rmr_predict_state == 0` |
| `>0` | `RMR_OK_SIZE_OR_VALUE` | sucesso com tamanho/valor calculado | `bitraf_compress`, `bitraf_reconstruct_ex`, `rmr_execution_graph_run` |

> Nota: para `bitraf_reconstruct_ex`, detalhamento de falha é exposto em `bitraf_diag.status`.

---

## `bitraf_*` API (C)

### Tabela de códigos de erro/diagnóstico (`bitraf_*`)

| Código | Origem | Nome | Quando ocorre |
|---|---|---|---|
| `0` | retorno de `bitraf_compress`/`bitraf_reconstruct`/`bitraf_reconstruct_ex` | `BITRAF_ERR_GENERIC` | ponteiro inválido, capacidade insuficiente, frame inválido, seed incompatível, hash/chunk inválido em modo strict |
| `0` | retorno de `bitraf_verify` | `BITRAF_VERIFY_FAIL` | hash esperado difere do calculado (ou entrada inválida em `bitraf_hash`) |
| `1` | `bitraf_diag.status` | `BITRAF_RECON_STATUS_FRAME` | header/magic/extensão/capacidade/frame inconsistente |
| `2` | `bitraf_diag.status` | `BITRAF_RECON_STATUS_CHUNK` | checksum de chunk divergente |
| `3` | `bitraf_diag.status` | `BITRAF_RECON_STATUS_HASH` | hash global divergente após reconstrução |

### `int bitraf_init(uint64_t seed)`
- **Signature:** `int bitraf_init(uint64_t seed)`
- **Input schema:** `seed: u64`.
- **Output schema:** `int` (`0` em sucesso).
- **Invariants:** não altera ABI; pode ser chamada repetidamente sem degradar estado funcional.
- **Error model:** atualmente não sinaliza erro operacional (sempre `0`).
- **Determinism/side effects:** determinística; sem I/O e sem alocação dinâmica.

### `uint64_t bitraf_hash(const uint8_t *data, size_t len, uint64_t seed)`
- **Signature:** `uint64_t bitraf_hash(const uint8_t *data, size_t len, uint64_t seed)`
- **Input schema:**
  - `data: pointer<uint8_t>|NULL`
  - `len: size_t` (se `len > 0`, `data` deve ser não nulo)
  - `seed: u64`
- **Output schema:** `u64` hash; `0` em entrada inválida (`data == NULL && len > 0`).
- **Invariants:** mesmo `data+len+seed+versão` ⇒ mesmo hash.
- **Error model:** falha retornada como `0` sentinela.
- **Determinism/side effects:** determinística pura; sem efeitos colaterais observáveis.

### `size_t bitraf_compress(...)`
- **Signature:**
  `size_t bitraf_compress(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap, uint64_t seed)`
- **Input schema:**
  - `in: pointer<uint8_t>|NULL` (nulo apenas se `in_len == 0`)
  - `in_len: size_t` (`<= UINT32_MAX`)
  - `out: pointer<uint8_t>` não nulo
  - `out_cap: size_t` (`>= frame_size`)
  - `seed: u64`
- **Output schema:** bytes escritos (`size_t > 0`) ou `0` em erro.
- **Invariants:** payload codificado é reversível por `bitraf_reconstruct(_ex)` com o mesmo `seed`.
- **Error model:** `0` em ponteiro inválido, overflow de tamanho, chunk_count fora do limite, capacidade insuficiente.
- **Determinism/side effects:** determinística para mesma entrada; escreve apenas em `out`.

### `size_t bitraf_reconstruct(...)`
- **Signature:**
  `size_t bitraf_reconstruct(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap, uint64_t seed)`
- **Input schema:** frame `bitraf` válido, `out` não nulo e capacidade adequada.
- **Output schema:** tamanho plaintext (`>0`) ou `0` em erro.
- **Invariants:** modo strict (interrompe em inconsistência de frame/chunk/hash).
- **Error model:** `0` em frame inválido, seed divergente, capacidade insuficiente, checksum/hash inválido.
- **Determinism/side effects:** determinística; escreve apenas no buffer de saída.

### `size_t bitraf_reconstruct_ex(...)`
- **Signature:**
  `size_t bitraf_reconstruct_ex(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_cap, uint64_t seed, int mode, bitraf_diag *diag)`
- **Input schema:**
  - `mode`: `BITRAF_RECON_MODE_STRICT(0)` ou `BITRAF_RECON_MODE_REPORT(1)`
  - `diag`: opcional (`NULL` permitido)
- **Output schema:**
  - strict: plaintext size (`>0`) ou `0`
  - report: pode retornar plaintext size mesmo com divergências, marcando `diag`
- **Invariants:** `diag` é sempre inicializado no início da chamada quando não nulo.
- **Error model:**
  - retorno `0` para falhas de frame e falhas strict
  - `diag.status` detalha `FRAME/CHUNK/HASH`.
- **Determinism/side effects:** determinística; side effect local em `out` e `diag`.

### `int bitraf_verify(const uint8_t *data, size_t len, uint64_t expected_hash, uint64_t seed)`
- **Signature:** `int bitraf_verify(const uint8_t *data, size_t len, uint64_t expected_hash, uint64_t seed)`
- **Input schema:** payload + hash esperado + seed.
- **Output schema:** `1` (válido) ou `0` (inválido).
- **Invariants:** comparação de hash em fluxo bitwise constante no resultado final.
- **Error model:** `0` para mismatch/entrada inválida.
- **Determinism/side effects:** determinística; sem efeitos colaterais externos.

### `rmr_bit_state_t bitraf_get_state_ref(uint64_t id)`
- **Signature:** `rmr_bit_state_t bitraf_get_state_ref(uint64_t id)`
- **Input schema:** `id: u64`.
- **Output schema:** struct preenchida (`id`, `state_hash`, `neighbors_mask`, `coherence_q16`).
- **Invariants:** `out.id == id` sempre.
- **Error model:** sem código de erro explícito (total function).
- **Determinism/side effects:** determinística; sem side effects globais.

### `rmr_bit_state_t bitraf_get_neighbors(uint64_t id)`
- **Signature:** `rmr_bit_state_t bitraf_get_neighbors(uint64_t id)`
- **Input schema:** `id: u64`.
- **Output schema:** struct com máscara de vizinhança expandida.
- **Invariants:** `out.id == id`; `state_hash/neighbors_mask` derivados deterministicamente.
- **Error model:** sem código de erro explícito.
- **Determinism/side effects:** determinística; sem side effects globais.

### Exemplos (`bitraf_*`)

#### Positivo
```c
uint8_t out[512], restored[512];
uint64_t seed = 0x123456789ABCDEF0ULL;

bitraf_init(seed);
size_t n = bitraf_compress((const uint8_t*)"HELLO", 5, out, sizeof(out), seed);
size_t m = bitraf_reconstruct(out, n, restored, sizeof(restored), seed);
int ok = bitraf_verify(restored, m, bitraf_hash(restored, m, seed), seed);
/* esperado: n>0, m==5, ok==1 */
```

#### Negativo
```c
uint8_t out[16];
size_t n = bitraf_compress((const uint8_t*)"HELLO", 5, out, sizeof(out), 7);
/* esperado: n==0 (out_cap insuficiente para frame completo) */
```

---

## Novas APIs RMR (`rmr_coherence_*`, `rmr_execution_graph_*`, `rmr_predict_*`)

### Tabela de códigos de erro (RMR)

| Código/Retorno | Nome | API | Interpretação |
|---|---|---|---|
| `0.0f..1.0f` | `RMR_COHERENCE_SCORE` | `rmr_coherence_score` | valor válido de coerência (não é erro) |
| `0/1` | `RMR_DECISION_BOOL` | `rmr_coherence_should_execute` | decisão booleana (não é erro) |
| `0` | `RMR_EXEC_GRAPH_ERR_EMPTY` | `rmr_execution_graph_run` | `graph == NULL` ou `node_count == 0` |
| `0` | `RMR_PREDICT_MISS` | `rmr_predict_state` | cache miss (sem estado previsto) |
| `void` | `RMR_CACHE_LEARN_NOERR` | `rmr_cache_learn`, `rmr_execution_graph_build` | operação sem erro reportável |

### `float rmr_coherence_score(uint64_t a, uint64_t b)`
- **Signature:** `float rmr_coherence_score(uint64_t a, uint64_t b)`
- **Input schema:** dois estados/hashes `u64`.
- **Output schema:** `float` de coerência.
- **Invariants:** simétrica por mistura XOR de entrada; sem dependência de heap.
- **Error model:** sem erro explícito; sempre retorna float.
- **Determinism/side effects:** determinística pura.

### `int rmr_coherence_should_execute(uint64_t current, uint64_t target)`
- **Signature:** `int rmr_coherence_should_execute(uint64_t current, uint64_t target)`
- **Input schema:** estado atual e alvo.
- **Output schema:** `1` se deve executar (`score < 0.85f`), `0` caso contrário.
- **Invariants:** limiar fixo (`0.85f`) em versão atual.
- **Error model:** sem erro explícito.
- **Determinism/side effects:** determinística pura.

### `void rmr_execution_graph_build(rmr_exec_graph_t *graph)`
- **Signature:** `void rmr_execution_graph_build(rmr_exec_graph_t *graph)`
- **Input schema:** `graph` ponteiro mutável (pode ser `NULL`, vira no-op).
- **Output schema:** preenche `graph` com topologia padrão (3 nós) quando não nulo.
- **Invariants:** `node_count=3`, `entry=0` no perfil padrão.
- **Error model:** no-op silencioso para `NULL`.
- **Determinism/side effects:** determinística; escreve apenas em `graph`.

### `uint64_t rmr_execution_graph_run(rmr_exec_graph_t *graph, uint64_t seed)`
- **Signature:** `uint64_t rmr_execution_graph_run(rmr_exec_graph_t *graph, uint64_t seed)`
- **Input schema:** grafo inicializado + `seed`.
- **Output schema:** acumulador final (`u64`), ou `0` se entrada inválida.
- **Invariants:** laço limitado por `steps <= node_count` (evita loop infinito).
- **Error model:** `0` para `graph == NULL` ou `node_count == 0`.
- **Determinism/side effects:** determinística dado estado estático de cache; atualiza cache preditivo via `rmr_cache_learn`.

### `uint64_t rmr_predict_state(uint64_t pattern)`
- **Signature:** `uint64_t rmr_predict_state(uint64_t pattern)`
- **Input schema:** padrão `u64`.
- **Output schema:** resultado previsto (`u64`) ou `0` em cache miss.
- **Invariants:** tabela fixa de 32 slots (hash modulo 32).
- **Error model:** `0` significa "sem previsão".
- **Determinism/side effects:** leitura determinística do cache global.

### `void rmr_cache_learn(uint64_t pattern, uint64_t result)`
- **Signature:** `void rmr_cache_learn(uint64_t pattern, uint64_t result)`
- **Input schema:** par (`pattern`,`result`).
- **Output schema:** sem retorno.
- **Invariants:** sobrescreve slot determinado por hash do `pattern`.
- **Error model:** sem erro explícito.
- **Determinism/side effects:** efeito colateral global (atualiza cache estático).

### Exemplos (RMR)

#### Positivo
```c
rmr_exec_graph_t g;
rmr_execution_graph_build(&g);
uint64_t out = rmr_execution_graph_run(&g, 0x42u);
/* esperado: out != 0 na maior parte dos cenários */
```

#### Negativo
```c
uint64_t out = rmr_execution_graph_run(NULL, 0x42u);
/* esperado: out == 0 (RMR_EXEC_GRAPH_ERR_EMPTY) */
```

```c
uint64_t pred = rmr_predict_state(0xDEADu);
/* esperado: pred == 0 enquanto não houver aprendizado prévio */
```

---

## Android/Java APIs

> Escopo: APIs públicas de supervisão de VM e governança de endpoints.

### `com.vectras.vm.VMManager`
- **Signature:** `public class VMManager`
- **Input schema (principais entradas públicas):**
  - `registerVmProcess(Context context, String vmId, Process process)`
  - `stopVmProcess(Context context, String vmId, boolean tryQmp)`
  - `unregisterVmProcess(String vmId[, Process process])`
- **Output schema:**
  - operações de registro/unregister: `void`
  - parada: `boolean` (sucesso/falha)
- **Invariants:**
  - lifecycle por VM com estados coerentes (`STOPPED/STARTING/RUNNING/STOPPING`)
  - IDs nulos/vazios são normalizados
- **Error model:** rejeições por budget/estado resultam em no-op, fallback ou `false` (sem checked exceptions).
- **Determinism/side effects:** efeitos colaterais globais (mapas concorrentes, auditoria, sinais de processo OS).

### `com.vectras.vm.core.ProcessSupervisor`
- **Signature:** `public class ProcessSupervisor`
- **Input schema (métodos públicos):**
  - `bindProcess(Process process)`
  - `onDegraded(int droppedLogs, long bytes)`
  - `stopGracefully(boolean tryQmp)`
- **Output schema:**
  - `bindProcess/onDegraded`: `void`
  - `stopGracefully`: `boolean`
- **Invariants:** transições auditáveis `START -> VERIFY -> RUN -> ... -> STOP`.
- **Error model:**
  - `bindProcess(null)` lança `IllegalArgumentException`
  - `stopGracefully` retorna `false` em timeout/kill não confirmado
- **Determinism/side effects:** efeitos em processo real (QMP, TERM, KILL), relógio e trilha de auditoria.

### `com.vectras.vm.network.EndpointFeature`
- **Signature:** `public enum EndpointFeature`
- **Input schema:** host normalizado e path para validação.
- **Output schema:**
  - `isAllowedHost/isAllowedPath`: `boolean`
  - getters: `Set<String>` imutável e descrição regex.
- **Invariants:** allowlist imutável por feature.
- **Error model:** não lança em fluxo normal; rejeição por `false`.
- **Determinism/side effects:** determinística pura.

### `com.vectras.vm.network.NetworkEndpoints`
- **Signature:** `public final class NetworkEndpoints`
- **Input schema:** parâmetros textuais (`contentId`, `username`, `languageCode`).
- **Output schema:** `String` URL HTTPS canônica.
- **Invariants:** hosts fixos e schema `https://`; `languageCode` normalizado para minúsculas.
- **Error model:** sem validação estrita interna; validação posterior via `EndpointValidator/EndpointPolicy`.
- **Determinism/side effects:** determinística pura.

### `com.vectras.vm.network.EndpointValidator`
- **Signature:** `public final class EndpointValidator`
- **Input schema:** URL (`String`) e opcionalmente allowlist de host.
- **Output schema:**
  - `isAllowed/isValidHttpUrl`: `boolean`
  - `requireValidHttpUrl`: `void` ou `IllegalArgumentException`
- **Invariants:** aceita apenas HTTPS + host em allowlist + porta padrão/443 + sem userinfo.
- **Error model:**
  - retorna `false` para vazio/malformado/não permitido
  - lança `IllegalArgumentException` apenas nos métodos `require*`
- **Determinism/side effects:** determinística; sem efeitos colaterais.

### `com.vectras.vm.network.EndpointPolicy`
- **Signature:** `public final class EndpointPolicy`
- **Input schema:** `Feature` + endpoint string.
- **Output schema:**
  - `isAllowedApi/isAllowedActionView`: `boolean`
  - `requireAllowedApi/requireAllowedActionView`: retorna endpoint ou lança exceção
- **Invariants:** feature sem prefixo registrado é bloqueada por padrão.
- **Error model:** lança `IllegalArgumentException` em `require*` quando bloqueado.
- **Determinism/side effects:** determinística; sem I/O.

### `com.vectras.vm.localization.NetworkEndpoints` (Kotlin)
- **Signature:** `object NetworkEndpoints`
- **Input schema:** `languageCode: String`.
- **Output schema:** URL do módulo de idioma (`String`).
- **Invariants:** base URL fixa de `raw.githubusercontent.com`.
- **Error model:** sem falha explícita; string sempre gerada.
- **Determinism/side effects:** determinística pura.

### `com.vectras.vm.localization.EndpointValidator` (Kotlin)
- **Signature:** `object EndpointValidator`
- **Input schema:** `url: String`.
- **Output schema:** URL normalizada (`String`) ou `null`.
- **Invariants:** somente HTTPS + host não vazio + path `.json`.
- **Error model:** parse inválido retorna `null` (sem exceção propagada).
- **Determinism/side effects:** determinística pura.

---

## Compatibility & SemVer

### O que **quebra contrato** (major)
Mudanças abaixo exigem incremento **MAJOR**:

1. Alterar assinatura pública de função/classe/método (nome, parâmetros, tipo de retorno).
2. Alterar semântica de sucesso/erro (ex.: função antes retornava `0` em erro e passa a retornar outro sentinela sem compatibilidade).
3. Alterar layout de structs públicas (`rmr_bit_state_t`, `bitraf_diag`, `rmr_exec_graph_t`, `rmr_exec_node_t`) de forma não backward-compatible.
4. Alterar formato de frame Bitraf de forma que versões antigas não consigam reconstruir payload válido com mesmo seed.
5. Alterar regras de validação de endpoint de modo mais restritivo sem feature flag/migração.

### O que é **extensão compatível** (minor/patch)
Mudanças abaixo são compatíveis:

1. Adição de novas funções/APIs sem remover ou alterar as existentes.
2. Adição de novos códigos de diagnóstico, mantendo códigos já documentados.
3. Otimizações internas (incluindo caminhos low-level) que preservem resultado funcional e contrato.
4. Novas features/prefixos de allowlist em `EndpointPolicy` sem bloquear fluxos já permitidos.
5. Correções de bug que mantenham mesma assinatura e modelo de erro público.

### Diretriz operacional para evolução
- Preserve **determinismo observável** das rotas críticas (`bitraf_*`, execução de grafo com mesmo estado).
- Evite overhead de abstração em caminho crítico; otimizações internas são permitidas desde que não alterem contrato externo.
- Qualquer mudança em contrato deve atualizar este arquivo e versão SemVer correspondente.

---

## Linkagem

### Make
```bash
make all
cc -O3 -Iengine/rmr/include app.c build/engine/libbitraf.a -o app
```

### CMake
```bash
cmake -S . -B build-cmake
cmake --build build-cmake -j
# alvo: bitraf_static (libbitraf.a) e bitraf_shared (libbitraf.so)
```
