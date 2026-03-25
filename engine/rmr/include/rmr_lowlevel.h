#ifndef RMR_LOWLEVEL_H
#define RMR_LOWLEVEL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @contract
 * Pré-condições:
 * - `n` é normalizado internamente para o intervalo [0, 31].
 * Pós-condições:
 * - Retorna `v` rotacionado à esquerda por `n mod 32` bits.
 * - Não altera memória externa nem estado global.
 * Comportamento em erro:
 * - Sem estado de erro; todos os pares (`v`, `n`) são válidos.
 * @invariant
 * - Operação puramente funcional e determinística.
 * - Mesma entrada implica mesma saída em qualquer plataforma suportada.
 * @complexity
 * - Tempo: O(1)
 * - Espaço adicional: O(1)
 */
uint32_t rmr_lowlevel_rotl32(uint32_t v, uint32_t n);

/**
 * @contract
 * Pré-condições:
 * - Nenhuma; `a`, `b`, `c`, `d` são consumidos como palavras de 32 bits.
 * Pós-condições:
 * - Retorna um valor de 32 bits derivado da combinação determinística de
 *   `a`, `b`, `c`, `d` (fold sem alocação).
 * - Não altera memória externa nem estado global.
 * Comportamento em erro:
 * - Sem estado de erro; todas as combinações de entrada são válidas.
 * @invariant
 * - Função total, determinística e sem efeitos colaterais.
 * - Não depende de ordem de avaliação externa ao contrato C.
 * @complexity
 * - Tempo: O(1)
 * - Espaço adicional: O(1)
 */
uint32_t rmr_lowlevel_fold32(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

/**
 * @contract
 * Pré-condições:
 * - Se `len > 0`, `data` deve apontar para ao menos `len` bytes legíveis.
 * - Se `len == 0`, `data` pode ser `NULL`.
 * Pós-condições:
 * - Retorna a redução XOR dos `len` bytes de entrada, expandida em 32 bits.
 * - Não altera o buffer de entrada nem estado global.
 * Comportamento em erro:
 * - Contrato violado (ex.: `data == NULL` com `len > 0`) resulta em
 *   comportamento indefinido conforme C.
 * @invariant
 * - Leitura somente do intervalo `[data, data + len)`.
 * - Resultado é determinístico para o mesmo conteúdo de entrada.
 * @complexity
 * - Tempo: O(len)
 * - Espaço adicional: O(1)
 */
uint32_t rmr_lowlevel_reduce_xor(const uint8_t* data, size_t len);

/**
 * @contract
 * Pré-condições:
 * - Se `len > 0`, `data` deve apontar para ao menos `len` bytes legíveis.
 * - `seed` pode assumir qualquer valor de 32 bits.
 * - Se `len == 0`, `data` pode ser `NULL`.
 * Pós-condições:
 * - Retorna checksum de 32 bits calculado sobre `data[0..len-1]` com `seed`.
 * - Não altera o buffer de entrada nem estado global.
 * Comportamento em erro:
 * - Contrato violado (ex.: `data == NULL` com `len > 0`) resulta em
 *   comportamento indefinido conforme C.
 * @invariant
 * - Função determinística para o mesmo (`data`, `len`, `seed`).
 * - Não aloca memória dinâmica.
 * @complexity
 * - Tempo: O(len)
 * - Espaço adicional: O(1)
 */
uint32_t rmr_lowlevel_checksum32(const uint8_t* data, size_t len, uint32_t seed);

#ifdef __cplusplus
}
#endif

#endif
