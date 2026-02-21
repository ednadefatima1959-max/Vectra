# Vectra VM Rafaelia — Surgical Patchset

Objetivo: estabilidade operacional do Terminal/Termux UI + proteção contra reentrância de processos.

## Arquivos
- app/src/main/java/com/vectras/vm/VmProcessGuard.java (novo)
- app/src/main/java/com/vectras/vterm/Terminal.java (editar 1 chamada)
- app/src/main/java/com/termux/app/TermuxActivity.java (editar onDestroy)
- app/src/main/res/layout/fragment_home_system_monitor.xml (editar 1 atributo)

## Efeitos esperados
- evita crash "process already bound"
- reduz leak/memória do Terminal UI (detach session)
- previne InflateException por layout_width ausente/edge
