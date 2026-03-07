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
  - contratos de emissão (`rafphi_emit_word_asm`, `rafphi_emit_block_hex`).
- `c/rafcode_phi_front_shell.c`
  - casca determinística em C;
  - mapa mínimo de mnemônicos (`NOP`, `RET`, `BRK`, `HLT`) para hex;
  - CRC32C local para integridade da saída.
- `asm/rafcode_phi_emit_word.S`
  - rotina ASM para gravação da palavra hex no buffer de saída;
  - caminhos para `__aarch64__` e `__x86_64__`.

## Contrato de direção
- Núcleo técnico: **C → ASM**.
- `sh`/`rs`: somente suporte de pipeline/startup.

## Inspeção rápida
```bash
sed -n '1,220p' tools/baremetal/rafcode_phi/include/rafcode_phi_abi.h
sed -n '1,260p' tools/baremetal/rafcode_phi/c/rafcode_phi_front_shell.c
sed -n '1,220p' tools/baremetal/rafcode_phi/asm/rafcode_phi_emit_word.S
```


## Build/execução local (PC/cell/server)
```bash
bash tools/baremetal/rafcode_phi/build_rafcode_phi.sh
./tools/baremetal/rafcode_phi/build/rafcode_phi_cli NOP RET BRK HLT
```

## Demo rápida
```bash
bash tools/baremetal/rafcode_phi/demo_emit_hex.sh
```

## Portabilidade alvo de casca
- Linux, Android/Termux, macOS, BSD, Raspberry Pi (via `cc` compatível).
- Windows: via ambiente com toolchain C/ASM compatível (MSYS2/Clang/MinGW).
- Núcleo continua C→ASM com saída hex determinística.
