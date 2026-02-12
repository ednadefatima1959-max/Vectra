# app/

## Camada 1 — Propósito do diretório
Módulo Android principal com UI, runtime e testes unitários.

## Camada 2 — Estrutura (até 3 níveis)
- Nível 1: `app/`
- Nível 2: `src/`
- Nível 3: detalhamento por arquivo em [`FILES_MAP.md`](FILES_MAP.md).

## Camada 3 — Arquivos e vínculos
- Catálogo completo: [`FILES_MAP.md`](FILES_MAP.md)
- Contexto global de camadas: [`docs/THREE_LAYER_ANALYSIS.md`](../docs/THREE_LAYER_ANALYSIS.md)

## Cadeia de comando (lógica de inspeção)
```bash
find app -maxdepth 3 -type d | sort
sed -n '1,120p' app/FILES_MAP.md
```
