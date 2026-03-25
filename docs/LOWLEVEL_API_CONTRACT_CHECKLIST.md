# Checklist de Revisão — APIs Low-Level

Objetivo: impedir a introdução de funções low-level sem contrato explícito.

## Escopo

- Aplica-se a novos símbolos públicos em headers low-level (ex.: `engine/rmr/include/*`).
- Aplica-se a mudanças de assinatura em funções públicas existentes.

## Gate obrigatório de revisão

Marque **todos** os itens antes de aprovar:

- [ ] A função pública possui bloco de comentário imediatamente acima da declaração.
- [ ] O bloco usa formato padronizado com as tags:
  - [ ] `@contract`
  - [ ] `@invariant`
  - [ ] `@complexity`
- [ ] Em `@contract`, constam obrigatoriamente:
  - [ ] Pré-condições
  - [ ] Pós-condições
  - [ ] Comportamento em erro
- [ ] `@invariant` descreve propriedades preservadas e ausência/presença de efeitos colaterais.
- [ ] `@complexity` informa custo de tempo e espaço assintóticos.
- [ ] Regras de ponteiros (`NULL`, faixa legível/escrevível, alinhamento quando aplicável) estão explícitas.
- [ ] Caso haja comportamento indefinido por violação de contrato, isso está declarado de forma textual.
- [ ] O header raiz de forwarding não duplica contrato; referencia o header canônico.
- [ ] Mudanças em API low-level passaram por revisão de desempenho (sem abstração adicional desnecessária).

## Critério de reprovação automática

Reprovar PR quando qualquer função low-level pública nova/alterada não atender 100% dos itens acima.
