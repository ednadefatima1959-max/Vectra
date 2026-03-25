# BENCHMARKS (RMR Engine)

## Build rápido
```bash
make clean && make all
```

## Selftest da API Bitraf
```bash
make run-selftest
```

## Rodar 1 execução
```bash
./build/bench/rmr_bench bench/results/latest.csv bench/results/latest.json
```

## Rodar N execuções + mediana/p95
```bash
bench/scripts/run_bench.sh 9 bench/results  # script shell + awk/sort (sem python)
```

Saídas:
- CSV por execução: `bench/results/run_*.csv`
- JSON por execução: `bench/results/run_*.json`
- Sumário estatístico comparável: `bench/results/summary.json` (`schema_version=bench_summary_v2`, thresholds e status pass/fail)

## Campos
- `score`: throughput relativo por teste
- `variance`: dispersão interna por teste
- `error_margin`: margem estimada
- `total_score`, `total_error`: agregados da suite (50 métricas)
- `coherence_mean`, `reuse_ratio`, `skip_execution_ratio`, `reconstruction_success_rate`, `invariant_violation_count`: métricas normalizadas (PPM) e invariantes para comparação entre runs


## Gate baremetal (autodetect + integridade de árvore)
```bash
make run-baremetal-gate
```

Artefatos:
- `reports/baremetal/hw_caps.env`
- `reports/baremetal/dir_integrity_matrix.json`
