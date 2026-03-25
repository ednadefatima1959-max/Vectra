#include "rmr_execution_graph.h"

#include "rmr_bit_broadcast.h"
#include "rmr_coherence_engine.h"
#include "rmr_predictive_cache.h"

void rmr_execution_graph_build(rmr_exec_graph_t *graph) {
  if (!graph) return;
  graph->node_count = 3u;
  graph->entry = 0u;

  graph->nodes[0].id = 0u;
  graph->nodes[0].next_on_success = 1u;
  graph->nodes[0].next_on_fail = 2u;
  graph->nodes[0].payload = 0xA1u;

  graph->nodes[1].id = 1u;
  graph->nodes[1].next_on_success = 2u;
  graph->nodes[1].next_on_fail = 2u;
  graph->nodes[1].payload = 0xB2u;

  graph->nodes[2].id = 2u;
  graph->nodes[2].next_on_success = 2u;
  graph->nodes[2].next_on_fail = 2u;
  graph->nodes[2].payload = 0xC3u;
}

uint64_t rmr_execution_graph_run(rmr_exec_graph_t *graph, uint64_t seed) {
  uint64_t acc = seed;
  uint32_t idx;
  uint32_t steps = 0u;

  if (!graph || graph->node_count == 0u) return 0u;

  idx = graph->entry;
  while (idx < graph->node_count && steps++ < graph->node_count) {
    rmr_exec_node_t *node = &graph->nodes[idx];
    uint64_t pattern = acc ^ node->payload;
    uint64_t predicted = rmr_predict_state(pattern);
    float coherence = rmr_coherence_score(predicted, pattern);

    if (predicted != 0u && coherence > 0.90f) {
      acc = predicted;
      idx = node->next_on_success;
      continue;
    }

    acc ^= node->payload;
    acc ^= rmr_bit_broadcast_dispatch(node->payload, acc).mapped_targets;
    rmr_cache_learn(pattern, acc);

    idx = (coherence > 0.50f) ? node->next_on_success : node->next_on_fail;
  }

  return acc;
}
