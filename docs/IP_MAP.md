# IP_MAP — RAFAELIA / Vectras

## 1) Núcleo autoral (prioridade de proteção)
- `engine/rmr/src/rafaelia_bitraf_core.c`
- `engine/rmr/src/rmr_*` (cycles/hw_detect/bench/isorf)
- `bench/src/rmr_benchmark_main.c`
- `bench/scripts/run_bench.sh`

Potencial patenteável (avaliar com counsel):
- Método de scoring determinístico orientado a ciclos em suite compacta.
- Estratégia de detecção+adaptação de benchmark por arquitetura.
- Estrutura de integração engine-runtime com trilha de evidência CSV/JSON.

## 2) Fork/terceiros
- Repositório base é fork de `xoureldeen/Vectras-VM-Android` (GPL-2.0).
- Módulos Android/QEMU/3dfx seguem licenças já declaradas no projeto.
- Verificar e manter `THIRD_PARTY_NOTICES.md` atualizado para redistribuição.

## 3) Dependências e licenças
- Engine C: toolchain padrão (`cc`, `make`, `cmake`) sem libs externas novas.
- Runtime Android: ecossistema Gradle/Android existente no fork.

## 4) Riscos de IP
- Mistura entre código autoral novo e herança de fork sem fronteira explícita.
- Documentação dispersa com baixa rastreabilidade de ownership.

## 5) Mitigações aplicadas
- Core isolado em `engine/rmr`.
- Assets/experimental isolados em `archive/experimental`.
- Documentos de produto e benchmark centralizados em `docs/`.
