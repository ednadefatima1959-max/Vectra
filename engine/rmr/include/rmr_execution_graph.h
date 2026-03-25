#ifndef RMR_EXECUTION_GRAPH_H
#define RMR_EXECUTION_GRAPH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rmr_exec_node {
  uint32_t id;
  uint32_t next_on_success;
  uint32_t next_on_fail;
  uint64_t payload;
} rmr_exec_node_t;

typedef struct {
  rmr_exec_node_t nodes[16];
  uint32_t node_count;
  uint32_t entry;
} rmr_exec_graph_t;

void rmr_execution_graph_build(rmr_exec_graph_t *graph);
uint64_t rmr_execution_graph_run(rmr_exec_graph_t *graph, uint64_t seed);

#ifdef __cplusplus
}
#endif

#endif
