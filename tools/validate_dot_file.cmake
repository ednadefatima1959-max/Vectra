if(NOT DEFINED INPUT_DOT OR INPUT_DOT STREQUAL "")
    message(FATAL_ERROR "validação DOT: variável INPUT_DOT não foi informada.")
endif()

if(NOT EXISTS "${INPUT_DOT}")
    message(FATAL_ERROR "validação DOT: arquivo não encontrado: ${INPUT_DOT}")
endif()

file(SIZE "${INPUT_DOT}" INPUT_DOT_SIZE)
if(INPUT_DOT_SIZE LESS 24)
    message(FATAL_ERROR "validação DOT: arquivo DOT vazio ou pequeno demais (${INPUT_DOT_SIZE} bytes): ${INPUT_DOT}")
endif()

file(READ "${INPUT_DOT}" INPUT_DOT_CONTENT LIMIT 512)
string(FIND "${INPUT_DOT_CONTENT}" "digraph" DOT_HEADER_INDEX)
if(DOT_HEADER_INDEX EQUAL -1)
    message(FATAL_ERROR "validação DOT: conteúdo inválido, cabeçalho 'digraph' ausente em ${INPUT_DOT}")
endif()

message(STATUS "validação DOT: OK (${INPUT_DOT}, ${INPUT_DOT_SIZE} bytes)")
