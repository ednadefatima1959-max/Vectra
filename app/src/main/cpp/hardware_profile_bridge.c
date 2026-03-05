#include <jni.h>
#include <stdint.h>

#include "rmr_hw_detect.h"

static const char* vectra_effective_abi(void) {
#if defined(__aarch64__)
    return "arm64-v8a";
#elif defined(__arm__)
    return "armeabi-v7a";
#elif defined(__x86_64__)
    return "x86_64";
#elif defined(__i386__)
    return "x86";
#elif defined(__riscv)
    return "riscv64";
#else
    return "unknown";
#endif
}

static uint32_t vectra_simd_mask(void) {
    uint32_t mask = 0u;
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    mask |= 1u;
#endif
#if defined(__SSE2__)
    mask |= (1u << 1);
#endif
#if defined(__SSE4_2__)
    mask |= (1u << 2);
#endif
#if defined(__AVX__)
    mask |= (1u << 3);
#endif
#if defined(__riscv_vector)
    mask |= (1u << 4);
#endif
    return mask;
}

JNIEXPORT jintArray JNICALL
Java_com_vectras_vm_core_HardwareProfileBridge_nativeCollectSnapshot(JNIEnv* env, jclass clazz) {
    (void)clazz;
    RmR_HW_Info info;
    RmR_HW_Detect(&info);

    jint values[9];
    values[0] = (jint)info.arch;
    values[1] = (jint)info.arch_hex;
    values[2] = (jint)info.ptr_bits;
    values[3] = (jint)info.is_little_endian;
    values[4] = (jint)info.has_cycle_counter;
    values[5] = (jint)info.has_asm_probe;
    values[6] = (jint)info.feature_bits_0;
    values[7] = (jint)info.feature_bits_1;
    values[8] = (jint)vectra_simd_mask();

    jintArray out = (*env)->NewIntArray(env, 9);
    if (!out) return NULL;
    (*env)->SetIntArrayRegion(env, out, 0, 9, values);
    return out;
}

JNIEXPORT jstring JNICALL
Java_com_vectras_vm_core_HardwareProfileBridge_nativeEffectiveAbi(JNIEnv* env, jclass clazz) {
    (void)clazz;
    return (*env)->NewStringUTF(env, vectra_effective_abi());
}
