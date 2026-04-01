#include "rmr_casm_bridge.h"

#include <stdint.h>
#include <stdio.h>

static int expect(int cond, const char *msg) {
  if (!cond) {
    fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
  }
  return 0;
}

int main(void) {
  int failed = 0;
  const uint32_t seed = 0x1234ABCDu;
  uint32_t asm_out = 0u;
  uint32_t c_out = 0u;
  uint32_t used_asm = 0u;

  uint32_t mismatch = RmR_CASM_VectraPulse_Interop(seed, &asm_out, &c_out);
  uint32_t final = RmR_CASM_VectraPulse(seed, &used_asm);
  uint32_t symbol_ready = RmR_CASM_VectraPulse_SymbolReady();

  failed += expect(mismatch == 0u, "vectra pulse asm/c equivalence");
  failed += expect(final == c_out, "vectra pulse final follows deterministic portable path");

#if defined(__x86_64__)
  if (used_asm != 0u) {
    failed += expect(symbol_ready == 1u, "x86_64 asm execution requires exported symbol");
  }
#endif

  if (failed != 0) {
    fprintf(stderr,
            "vectra_pulse_selftest FAILED (%d) seed=0x%08x asm=0x%08x c=0x%08x used_asm=%u symbol=%u\n",
            failed,
            seed,
            asm_out,
            c_out,
            used_asm,
            symbol_ready);
    return 1;
  }

  printf("vectra_pulse_selftest OK seed=0x%08x out=0x%08x used_asm=%u symbol_ready=%u\n",
         seed,
         final,
         used_asm,
         symbol_ready);
  return 0;
}
