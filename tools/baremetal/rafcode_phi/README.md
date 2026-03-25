# tools/baremetal/rafcode_phi/

Micro-base RAFCODE❤️PHI para **C (casca)** e **ASM (núcleo)** com emissão em **hexadecimal**.

## Objetivo
- Tornar o caminho de compilação amigável para bare-metal:
  - parser/validação mínima em C;
  - emissão de palavras de instrução em ASM;
  - trilha de opcodes em hex (`raf_u32 opcode_hex`).

## Estrutura
- `include/rafcode_phi_abi.h`
  - ABI autoral C↔ASM;
  - tipos fixos sem dependência de `stdint`/libc;
  - versionamento de opcodes de controle de fluxo;
  - semântica de registradores/flags para a micro-ISA.
- `c/rafcode_phi_front_shell.c`
  - casca determinística em C;
  - parser de mnemônicos (`NOP`, `RET`, `BRK`, `HLT`, `CMP`, `BRZ`, `BRNZ`, `LOOP`, `BLKMIX`);
  - suporte a sufixo imediato (`MNEM:imm8`, decimal ou `0x`);
  - geração de trilha hex completa via `rafphi_emit_block_hex`.
- `asm/rafcode_phi_emit_word.S`
  - rotina ASM para gravação da palavra hex no buffer de saída.
- `asm/rafcode_phi_compare.S`
  - módulo ASM dedicado à operação mínima de comparação.
- `asm/rafcode_phi_branch.S`
  - módulo ASM dedicado a salto condicional.
- `asm/rafcode_phi_loop.S`
  - módulo ASM dedicado a loop contado.
- `asm/rafcode_phi_blkmix.S`
  - módulo ASM dedicado à cópia/mix de bloco.

## Tabela opcode → semântica → arquitetura

Formato canônico: `[31:24 opcode][23:16 flags][15:12 addr][11:8 ver][7:0 imm]`.

| Opcode | Mnemônico | Semântica | Flags/regs relevantes | Portabilidade arm64/x86_64 |
|---|---|---|---|---|
| `0x00` | `NOP` | no-op | nenhuma | estável nos dois targets |
| `0x40` | `CMP` / `CMP:imm8` | compara `R0` com `R1`/`imm8`, atualiza `FL` | `Z,N,C,V` em `FL`; `R0`,`R1` | codificação IR idêntica; backend ASM só empacota palavra |
| `0x41` | `BRZ` / `BRNZ` (`:rel8`) | salto relativo condicional pelo estado de `FL` | `BRANCH_IF_Z` / `BRANCH_IF_NZ`; usa `PC` | offset `imm8` em bytes lógicos IR, sem dependência de encoding nativo |
| `0x42` | `LOOP` (`:count8`) | decrementa `LC` e desvia enquanto `LC!=0` | `LOOP_COUNTED`; usa `LC`,`PC` | comportamento canônico em C/ASM, sem instrução nativa fixa |
| `0x43` | `BLKMIX` (`:span8`) | cópia/mix XOR de bloco lógico | `MIX_XOR`; usa `R0` (src), `R1` (dst), `R2` (seed) | semântica independente de endian quando consumidor lê palavra como `raf_u32` |

### Versionamento
- `ver=0x1`: núcleo base legado (`NOP/RET/BRK/HLT`).
- `ver=0x2`: família de controle de fluxo (`CMP/BRANCH/LOOP/BLKMIX`).

### Semântica de registradores/flags da micro-ISA
- `R0..R3`: registradores gerais de dados/endereços lógicos.
- `PC`: contador de programa lógico da trilha IR.
- `LC`: contador de loop para `LOOP`.
- `FL`: flags aritméticas (`Z,N,C,V`) alimentadas por `CMP` e consumidas por branch.

## Regras de portabilidade arm64/x86_64
1. **Não serializar encoding de máquina nativo** para fluxo (`B.cond`, `Jcc`, etc.); serializar somente palavra RAFPHI IR.
2. **Fixar largura em 32 bits** (`raf_u32`) para todo opcode emitido.
3. **Mascarar `flags` e `imm` em 8 bits** antes de empacotar em ASM/C fallback.
4. **Tratar ordem de bytes no consumidor**, não no emissor: o emissor grava palavras host-endian no buffer; persistência externa deve definir endianness explícito.
5. **Manter paridade de símbolos**: módulos ASM e fallback C devem exportar as mesmas funções (`rafphi_emit_*_word_asm`).

## Contrato de direção
- Núcleo técnico: **C → ASM**.
- `sh`/`rs`: somente suporte de pipeline/startup.

## Inspeção rápida
```bash
sed -n '1,260p' tools/baremetal/rafcode_phi/include/rafcode_phi_abi.h
sed -n '1,320p' tools/baremetal/rafcode_phi/c/rafcode_phi_front_shell.c
sed -n '1,220p' tools/baremetal/rafcode_phi/asm/rafcode_phi_compare.S
sed -n '1,220p' tools/baremetal/rafcode_phi/asm/rafcode_phi_branch.S
sed -n '1,220p' tools/baremetal/rafcode_phi/asm/rafcode_phi_loop.S
sed -n '1,220p' tools/baremetal/rafcode_phi/asm/rafcode_phi_blkmix.S
```
