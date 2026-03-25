#include "rmr_predictive_cache.h"

int rmr_cache_test_run(void) {
  rmr_cache_learn(7u, 11u);
  return (int)rmr_predict_state(7u);
}
