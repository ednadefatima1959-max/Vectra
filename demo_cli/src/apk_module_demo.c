#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "rmr_apk_module.h"

static int parse_u32(const char *src, u32 *out) {
  char *end = NULL;
  unsigned long value;

  if (src == NULL || src[0] == '\0') {
    return -1;
  }

  errno = 0;
  value = strtoul(src, &end, 10);
  if (errno != 0 || end == src || (end != NULL && *end != '\0') || value > 0xFFFFFFFFul) {
    return -1;
  }

  *out = (u32)value;
  return 0;
}

int main(int argc, char **argv) {
  RmR_ApkProfile profile;
  RmR_ApkStableIdentity stable;
  char plan[4096];
  u64 fp_stable;
  u32 compile_sdk;
  u32 ndk_major;
  u32 build_tools_major;
  u32 build_tools_minor;
  u32 build_tools_patch;

  if (argc != 13) {
    printf("uso: %s <keystore> <store_pass> <alias> <key_pass> <termux_prefix> <home> <shell> <compile_sdk> <ndk_major> <build_tools_major> <build_tools_minor> <build_tools_patch>\n", argv[0]);
    return 1;
  }

  if (parse_u32(argv[8], &compile_sdk) != 0 ||
      parse_u32(argv[9], &ndk_major) != 0 ||
      parse_u32(argv[10], &build_tools_major) != 0 ||
      parse_u32(argv[11], &build_tools_minor) != 0 ||
      parse_u32(argv[12], &build_tools_patch) != 0) {
    fprintf(stderr, "falha ao converter versão numérica do toolchain.\n");
    return 2;
  }

  RmR_ApkModule_InitProfile(&profile);
  profile.abi_mask = RMR_APK_ABI_UNIVERSAL;
  profile.termux_mode = RmR_ApkModule_DetectTermuxLike(argv[5], argv[6], argv[7]);
  RmR_ApkModule_AutotuneProfile(&profile);

  RmR_ApkModule_FillStableIdentity(&profile,
                                   compile_sdk,
                                   ndk_major,
                                   build_tools_major,
                                   build_tools_minor,
                                   build_tools_patch,
                                   &stable);

  if (RmR_ApkModule_BuildPlan(&profile, argv[1], argv[2], argv[3], argv[4], plan,
                              (u32)sizeof(plan)) == 0u) {
    printf("falha ao gerar plano determinístico de compilação/assinatura.\n");
    return 2;
  }

  fp_stable = RmR_ApkModule_StableFingerprint(&stable, 0xCAFEBABEULL);

  printf("termux_mode=%u\n", profile.termux_mode);
  printf("host_abi_mask=0x%08X\n", profile.host_abi_mask);
  printf("hw_cacheline=0x%08X\n", profile.hw_cacheline_bytes);
  printf("hw_page=0x%08X\n", profile.hw_page_bytes);
  printf("build_plan=%s\n", plan);
  printf("stable_fp=0x%016llX\n", (unsigned long long)fp_stable);

  return 0;
}
