# shell-loader/

## Camada 1 — Propósito do diretório
Loader Android e submódulo de stubs.

## Camada 2 — Estrutura (até 3 níveis)
- Nível 1: `shell-loader/`
- Nível 2: `.idea/`, `release/`, `src/`, `stub/`
- Nível 3: detalhamento por arquivo em [`FILES_MAP.md`](FILES_MAP.md).

## Camada 3 — Arquivos e vínculos
- Catálogo completo: [`FILES_MAP.md`](FILES_MAP.md)
- Contexto global de camadas: [`docs/THREE_LAYER_ANALYSIS.md`](../docs/THREE_LAYER_ANALYSIS.md)

## Cadeia de comando (lógica de inspeção)
```bash
find shell-loader -maxdepth 3 -type d | sort
sed -n '1,120p' shell-loader/FILES_MAP.md
```
