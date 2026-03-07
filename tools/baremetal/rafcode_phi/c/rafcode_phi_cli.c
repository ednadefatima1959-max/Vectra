#include "rafcode_phi_abi.h"

#include <stdio.h>

static raf_u32 rafphi_len(const char *s) {
  raf_u32 n = 0u;
  if (!s) {
    return 0u;
  }
  while (s[n] != '\0') {
    n++;
  }
  return n;
}

static const char *rafphi_arch_name(void) {
#if defined(__aarch64__)
  return "aarch64";
#elif defined(__x86_64__)
  return "x86_64";
#else
  return "unknown";
#endif
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s TOKEN...\n", argv[0]);
    fprintf(stderr, "example: %s NOP RET BRK HLT\n", argv[0]);
    return 2;
  }

  const char *tokens[512];
  raf_u32 lens[512];
  raf_u32 out_words[512];
  raf_u32 n = 0u;
  int i;

  for (i = 1; i < argc && n < 512u; i++) {
    tokens[n] = argv[i];
    lens[n] = rafphi_len(argv[i]);
    n++;
  }

  rafphi_emit_stats_t stats = rafphi_emit_block_hex(tokens, lens, n, out_words, 512u);

  printf("rafcode_phi.arch=%s\n", rafphi_arch_name());
  printf("rafcode_phi.tokens=%u\n", n);
  printf("rafcode_phi.accepted=%u\n", stats.accepted);
  printf("rafcode_phi.rejected=%u\n", stats.rejected);
  printf("rafcode_phi.crc32c=0x%08X\n", stats.crc32c);

  raf_u32 w;
  for (w = 0u; w < stats.accepted; w++) {
    printf("word[%u]=0x%08X\n", w, out_words[w]);
  }

  return stats.rejected == 0u ? 0 : 1;
}
