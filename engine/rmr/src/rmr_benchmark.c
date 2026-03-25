#include "rmr_execution_graph.h"
#include "rmr_ll_ops.h"

typedef struct {
  uint64_t cycles;
  uint64_t reuse_hits;
  uint64_t skip_execution;
} rmr_benchmark_result_t;

rmr_benchmark_result_t rmr_benchmark_run(uint64_t seed, uint32_t iterations) {
  rmr_exec_graph_t graph;
  rmr_benchmark_result_t out = {0u, 0u, 0u};
  uint64_t start = RmR_LL_ReadCycles();
  uint32_t i;

  rmr_execution_graph_build(&graph);
  for (i = 0u; i < iterations; ++i) {
    uint64_t v = rmr_execution_graph_run(&graph, seed + i);
    out.reuse_hits += (v & 1u);
    out.skip_execution += ((v >> 1u) & 1u);
  }
  out.cycles = RmR_LL_ReadCycles() - start;
  return out;
}
