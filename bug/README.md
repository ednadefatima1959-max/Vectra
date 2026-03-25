# bug/

Camada de gestão de falhas do Vectras VM Android com inventário reconciliado entre análise, operação, histórico e evidências.

## Estrutura principal
- `core/`: fontes low-level C/ASM/Java com classificação de status por arquivo.
- `issues/`: issues técnicas atômicas.
- `prioridade/`: triagem e ordenação de execução.
- `fazer hotfix/`: fila de correção imediata.
- `feito/`: histórico de ciclos encerrados.
- `experiments/`: trilha experimental normalizada (`failed/`, `unstable/`, `validated/`) com `metadata.json`.
- `archive/`: trilha histórica para anexos não canônicos.

## Regras aplicadas neste ciclo
- `.docx` não canônicos foram movidos para `archive/evidencias/` mantendo ponte no `FILES_MAP.md` do nível superior.
- Cada subdiretório em `bug/` mantém par `README.md` + `FILES_MAP.md` para navegação consistente.

## Navegação
- [FILES_MAP.md](FILES_MAP.md)
- [core/README.md](core/README.md)
- [issues/README.md](issues/README.md)
- [prioridade/README.md](prioridade/README.md)
- [fazer hotfix/README.md](fazer hotfix/README.md)
- [feito/README.md](feito/README.md)
- [experiments/README.md](experiments/README.md)
- [archive/README.md](archive/README.md)
- [archive/evidencias/README.md](archive/evidencias/README.md)
