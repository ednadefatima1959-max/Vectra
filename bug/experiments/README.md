# bug/experiments/

Subárvore canônica para organizar experimentos de bug com metadados normalizados e rastreabilidade para o dataset de erros.

## Subárvores
- `failed/`: experimentos que quebraram invariantes e devem ser ligados a `dataset/errors/`.
- `unstable/`: experimentos em análise, ainda sem decisão final.
- `validated/`: experimentos validados, sem quebra de invariante ativa.

## Template de metadados obrigatório
Cada experimento deve possuir `metadata.json` com os campos:

- `context`
- `input`
- `expected`
- `observed`
- `invariant_broken`
- `status`

Template base: [`TEMPLATE.metadata.json`](TEMPLATE.metadata.json).

## Contrato para `failed` ↔ `dataset/errors/`
Para experimentos em `failed/`, o campo `input.error_dataset_id` é obrigatório e deve existir em `dataset/errors/error_samples.jsonl`.

## Migração gradual
Use o script:

```bash
python3 tools/bug_experiment_inventory.py \
  --apply \
  --migrate-limit 10 \
  --status-target unstable
```

Esse fluxo:
1. inventaria artefatos legados em `bug/`;
2. cria stubs de experimento com `metadata.json` no novo padrão;
3. gera `bug/experiments/inventory.json`;
4. valida links `failed` com `dataset/errors/`.
