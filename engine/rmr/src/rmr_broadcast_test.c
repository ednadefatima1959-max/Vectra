#include "rmr_bit_broadcast.h"

int rmr_broadcast_test_run(void) {
  return (int)rmr_bit_broadcast_dispatch(1u, 0xFFu).mapped_targets;
}
