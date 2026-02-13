#include "rmr_policy_kernel.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static int parse_u32_dec(const char *src, unsigned int fallback, unsigned int *out) {
  char *end = NULL;
  unsigned long value;

  if (src == NULL || src[0] == '\0') {
    *out = fallback;
    return 0;
  }

  errno = 0;
  value = strtoul(src, &end, 10);
  if (errno != 0 || end == src || (end != NULL && *end != '\0') || value > 0xFFFFFFFFul) {
    return -1;
  }

  *out = (unsigned int)value;
  return 0;
}

static int parse_u8_hex(const char *src, unsigned char fallback, unsigned char *out) {
  char *end = NULL;
  unsigned long value;

  if (src == NULL || src[0] == '\0') {
    *out = fallback;
    return 0;
  }

  errno = 0;
  value = strtoul(src, &end, 16);
  if (errno != 0 || end == src || (end != NULL && *end != '\0') || value > 0xFFul) {
    return -1;
  }

  *out = (unsigned char)value;
  return 0;
}

static int parse_bool01(const char *src, unsigned char fallback, unsigned char *out) {
  unsigned int value;
  if (parse_u32_dec(src, fallback, &value) != 0 || value > 1u) {
    return -1;
  }

  *out = (unsigned char)value;
  return 0;
}

static void usage(const char *bin) {
  printf("uso: %s <input> <output> <audit.log> [chunk_size] [xor_hex] [stride] [cpu_ok] [ram_ok] [disk_ok]\n", bin);
}

int main(int argc, char **argv) {
  unsigned int chunk_size = 4096u;
  unsigned int mutation_stride = 31u;
  unsigned char mutation_xor = 0xA5u;
  unsigned char cpu_ok = 1u;
  unsigned char ram_ok = 1u;
  unsigned char disk_ok = 1u;

  if (argc < 4) {
    usage(argv[0]);
    return 2;
  }

  if (parse_u32_dec((argc > 4) ? argv[4] : NULL, 4096u, &chunk_size) != 0 || chunk_size == 0u) {
    fprintf(stderr, "invalid chunk_size: %s\n", (argc > 4) ? argv[4] : "");
    return 2;
  }

  if (parse_u8_hex((argc > 5) ? argv[5] : NULL, 0xA5u, &mutation_xor) != 0) {
    fprintf(stderr, "invalid xor_hex: %s\n", (argc > 5) ? argv[5] : "");
    return 2;
  }

  if (parse_u32_dec((argc > 6) ? argv[6] : NULL, 31u, &mutation_stride) != 0 || mutation_stride == 0u) {
    fprintf(stderr, "invalid stride: %s\n", (argc > 6) ? argv[6] : "");
    return 2;
  }

  if (parse_bool01((argc > 7) ? argv[7] : NULL, 1u, &cpu_ok) != 0 ||
      parse_bool01((argc > 8) ? argv[8] : NULL, 1u, &ram_ok) != 0 ||
      parse_bool01((argc > 9) ? argv[9] : NULL, 1u, &disk_ok) != 0) {
    fprintf(stderr, "invalid triad flags (expected 0 or 1)\n");
    return 2;
  }

  RmR_PipelineConfig cfg;
  cfg.chunk_size = (size_t)chunk_size;
  cfg.mutation_xor = mutation_xor;
  cfg.mutation_stride = mutation_stride;
  cfg.triad.cpu_ok = cpu_ok;
  cfg.triad.ram_ok = ram_ok;
  cfg.triad.disk_ok = disk_ok;

  RmR_AuditSummary summary;
  int rc = RmR_RunPolicyPipeline(argv[1], argv[2], argv[3], &cfg, &summary);

  printf("pipeline_rc=%d planned=%u applied=%u diff=%u verified=%u verify_failures=%u\n",
         rc,
         summary.chunks_planned,
         summary.chunks_applied,
         summary.chunks_diff,
         summary.chunks_verified,
         summary.verify_failures);
  return (rc < 0) ? 1 : 0;
}
