# Análise low-level: atrator, fase acumulada e delta de verificação

## Objetivo
Este documento traduz a lógica de laço observacional (estado/fase/Δ/observação) para os componentes reais do código-fonte C do núcleo `bug/core`, com foco em operação bit a bit, sem abstrações de alto nível.

## Escopo analisado (fonte primária)
- `bug/core/bitraf.c`
- `bug/core/rmr_unified_kernel.c`

## Mapeamento formal dos registradores da máquina conceitual para o código real

### AX = `estado atual (s_t)`
No código, o “estado” é distribuído por slot em `slot_phi[64]` e evolui a cada evento em `bitraf_push`:
- `slot_phi[slot] = rmr_lowlevel_phi_step(slot_phi[slot], coherence)`

Interpretação: o estado não é um único registrador global, mas um vetor de atratores locais por rota (slot), cada um atualizado por transição determinística.

### BX = `fase acumulada (phi)`
A fase acumulada aparece como:
- `slot_phi[slot]` (fase por slot)
- `coherence` (campo global que perturba/parametriza o passo de fase)

Interpretação: `coherence` atua como injeção estrutural sobre cada transição de fase. A fase é persistente no tempo e não é zerada entre eventos válidos.

### CX = `delta (Δ)` entre modelo e sistema
No código atual, Δ não existe como variável única explícita; ele é decomposto em:
- `slot_crc[slot]` (integridade incremental por slot via CRC32C)
- `rolling_crc32c` e `rolling_bitraf_hash` no kernel legado
- `verify->crc_ok` e `verify->hash_ok` como decisão booleana de conformidade

Interpretação: Δ é operacionalizado como erro de consistência acumulada e depois convertido em decisão de verificação (`ok`/`fail`), equivalente ao “ultrapassar limiar”.

### DX = `observação / bit injetado`
No caminho de ingestão, a observação é representada por `data + data_len`:
- `rmr_legacy_kernel_ingest(...)` injeta os bytes observados na cadeia de CRC/hash.
- `bitraf_push(slot, value, flags)` injeta evento discreto no anel (`ring`), com timestamp monotônico (`tick`).

Interpretação: cada observação altera simultaneamente o estado de fase e a assinatura de verificação.

## Equivalência operacional com o pseudocódigo ASM

1. **Injeção de fase (DX em BX)**  
   Equivalente funcional: entrada do evento em `bitraf_push`, seguida de atualização de `slot_phi`.

2. **Interferência no estado (AX xor BX)**  
   Equivalente funcional: mistura determinística entre estado anterior (`slot_phi`) e parâmetro de coerência (`coherence`) via `rmr_lowlevel_phi_step`.

3. **Atualização de Δ (CX xor AX)**  
   Equivalente funcional: atualização incremental de `slot_crc` por valor observado e acumulação de assinaturas no kernel (`rolling_crc32c`, `rolling_bitraf_hash`).

4. **Comparação com limiar e salto de atrator**  
   Equivalente funcional: não há `cmp/jg` literal, mas existe limiar lógico no estágio de verificação:  
   - se `crc_ok == 0` ou `hash_ok == 0`, retorno `RMR_STATUS_ERR_VERIFY`;  
   - caso contrário, trajetória continua válida.

## Invariantes low-level verificáveis

- `bitraf_init` define `magic` e ativa a malha completa (`route_mask = 0xFFFFFFFFFFFFFFFF`), estabelecendo estado inicial válido.
- `bitraf_push` protege fronteiras (`slot < 64`, ring não cheio), garantindo consistência estrutural antes de mutação.
- `bitraf_pop` preserva semântica FIFO do anel.
- `bitraf_route` usa CRC do bloco para seleção determinística de rota com fallback para próximo slot ativo.
- `rmr_legacy_kernel_*` impõe ciclo de vida (`NEW -> READY -> SHUTDOWN`) e bloqueia operações fora de estado válido.

## Riscos técnicos identificados

1. **Ausência de variável Δ explícita**
   - A dispersão de Δ em múltiplas estruturas dificulta auditoria formal de “erro total do sistema” por passo.

2. **Sem limiar escalar contínuo**
   - A decisão é binária no final (`crc_ok/hash_ok`), sem gradação de severidade (ex.: “quase falha”).

3. **Acoplamento de estado em singleton estático (`s_bitraf`)**
   - Facilita uso simples, mas reduz isolamento para cenários multi-instância e testes concorrentes.

## Recomendação de documentação/engenharia

- Definir explicitamente no contrato técnico a função de erro composta:
  - `Δ_t = f(slot_crc_t, rolling_crc32c_t, rolling_bitraf_hash_t)`
- Documentar um limiar quantitativo intermediário (pré-falha) antes do erro final de verificação.
- Incluir tabela de rastreio “evento -> mutação de estado -> sinal de verificação” para auditorias de bit-level.

## Checklist de verificação metódica (execução manual)

1. Inicializar kernel e BITRAF com seed fixo.
2. Injetar sequência determinística de eventos (slots e valores).
3. Registrar, por passo:
   - `slot_phi[slot]`
   - `slot_crc[slot]`
   - `rolling_crc32c`
   - `rolling_bitraf_hash`
4. Executar `verify` com assinaturas esperadas.
5. Confirmar transição de estado de retorno (`OK` ou `ERR_VERIFY`) sem ambiguidade.

---

Documento criado para suportar análise ultra-técnica low-level, com rastreabilidade direta entre formalismo de registradores e implementação C do repositório.
