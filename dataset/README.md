# Dataset Contract (`dataset/`)

Este diretório contém dados de referência com rastreabilidade forte para avaliação estrutural, geométrica, de recorrência e de erro.

## `dataset_version`

- Versão atual: `1.0.0`.
- Formato: `MAJOR.MINOR.PATCH`.
- Regra de incremento:
  - `MAJOR`: mudança incompatível de schema, remoção/renomeação de campos obrigatórios, mudança de semântica de ground truth.
  - `MINOR`: novos campos opcionais, novas classes, novas amostras compatíveis.
  - `PATCH`: correções de metadados, hashes, documentação, ajustes não semânticos.
- A versão do dataset deve ser refletida em:
  - `dataset/MANIFEST.json` (`dataset_version`).
  - `version` de cada schema em `dataset/schemas/`.

## Contrato por pasta

- `core/`
  - Dados estruturais (classe `structural`).
  - Cada arquivo de amostra deve validar em `schemas/structural.schema.json`.
  - Deve conter campo `ground_truth` explícito por item.

- `errors/`
  - Casos de erro (classe `error`) com expectativa de validação/falha.
  - Deve registrar erro esperado em `ground_truth.expected_error`.
  - Inclui casos adversariais (entrada malformada, tipos inválidos, ranges inválidos).

- `geometric/`
  - Casos geométricos (classe `geometric`) com parâmetros e invariantes esperados.
  - Deve explicitar em `ground_truth` os resultados numéricos esperados e tolerância.

- `recursive/`
  - Casos de recorrência (classe `recurrence`) com condição inicial e recorrência.
  - `ground_truth.sequence_prefix` deve explicitar prefixo esperado.

- `schemas/`
  - Schemas JSON oficiais por tipo.
  - Mudanças incompatíveis exigem `dataset_version` `MAJOR`.

- `processed/`
  - Artefatos derivados (normalizados, tokenizados, vetorizados).
  - Não é fonte de verdade; sempre regenerável a partir de `core/`, `errors/`, `geometric/`, `recursive/`.

## Rastreabilidade

- `dataset/MANIFEST.json` lista hash SHA-256 por arquivo versionado no `dataset/`.
- O manifesto é o ponto único de auditoria de integridade.
