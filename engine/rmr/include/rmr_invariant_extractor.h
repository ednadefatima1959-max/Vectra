#ifndef RMR_INVARIANT_EXTRACTOR_H
#define RMR_INVARIANT_EXTRACTOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  RMR_INVARIANT_VALID = 1,
  RMR_INVARIANT_AMBIGUOUS = 0,
  RMR_INVARIANT_BROKEN = -1
};

typedef struct {
  uint64_t fingerprint;
  uint32_t length;
  uint16_t class_mask;
  uint8_t transition_count;
  uint8_t status;
} rmr_invariant_fingerprint_t;

int rmr_invariant_extract(const uint8_t *input, size_t len, rmr_invariant_fingerprint_t *out);
size_t rmr_invariant_fingerprint_serialize(const rmr_invariant_fingerprint_t *fp, char *out, size_t out_cap);

#ifdef __cplusplus
}
#endif

#endif
