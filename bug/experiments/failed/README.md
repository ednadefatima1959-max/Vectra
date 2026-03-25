# bug/experiments/failed

Experimentos que falharam e quebraram invariantes.

## Regras
- Cada pasta de experimento deve conter `metadata.json`.
- `status` deve ser `failed`.
- `input.error_dataset_id` deve apontar para um `id` existente em `dataset/errors/error_samples.jsonl`.
