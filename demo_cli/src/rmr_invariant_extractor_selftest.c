#include "rmr_invariant_extractor.h"

#include <stdint.h>
#include <stdio.h>

static int test_valid_case(void) {
  static const uint8_t in[] = "alpha-123-beta";
  rmr_invariant_fingerprint_t fp;
  char serialized[17];

  if (rmr_invariant_extract(in, sizeof(in) - 1u, &fp) != RMR_INVARIANT_VALID) return 11;
  if (rmr_invariant_fingerprint_serialize(&fp, serialized, sizeof(serialized)) != 16u) return 12;
  if (serialized[0] == '\0') return 13;
  return 0;
}

static int test_ambiguous_case(void) {
  static const uint8_t in[] = "11111111";
  rmr_invariant_fingerprint_t fp;

  if (rmr_invariant_extract(in, sizeof(in) - 1u, &fp) != RMR_INVARIANT_AMBIGUOUS) return 21;
  if (fp.transition_count != 0u) return 22;
  return 0;
}

static int test_invariance_break_case(void) {
  static const uint8_t stable[] = "A1-B2-C3";
  static const uint8_t broken[] = {'A', 0u, 'B', 0u};
  rmr_invariant_fingerprint_t fp_a;
  rmr_invariant_fingerprint_t fp_b;

  if (rmr_invariant_extract(stable, sizeof(stable) - 1u, &fp_a) != RMR_INVARIANT_VALID) return 31;
  if (rmr_invariant_extract(broken, sizeof(broken), &fp_b) != RMR_INVARIANT_BROKEN) return 32;
  if (fp_a.fingerprint == fp_b.fingerprint) return 33;
  return 0;
}

int main(void) {
  int rc;

  rc = test_valid_case();
  if (rc != 0) {
    fprintf(stderr, "rmr_invariant_extractor_selftest valid_case failed (%d)\n", rc);
    return rc;
  }

  rc = test_ambiguous_case();
  if (rc != 0) {
    fprintf(stderr, "rmr_invariant_extractor_selftest ambiguous_case failed (%d)\n", rc);
    return rc;
  }

  rc = test_invariance_break_case();
  if (rc != 0) {
    fprintf(stderr, "rmr_invariant_extractor_selftest break_case failed (%d)\n", rc);
    return rc;
  }

  printf("rmr_invariant_extractor_selftest OK\n");
  return 0;
}
