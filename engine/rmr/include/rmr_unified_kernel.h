#ifndef RMR_UNIFIED_KERNEL_H
#define RMR_UNIFIED_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RMR_UK_OK 0
#define RMR_UK_ERR_ARG -1
#define RMR_UK_ERR_STATE -2
#define RMR_UK_ERR_NOMEM -3
#define RMR_UK_ERR_RANGE -4
#define RMR_UK_ERR_HANDLE -5

#define RMR_KERNEL_OK RMR_UK_OK
#define RMR_KERNEL_ERR_ARG RMR_UK_ERR_ARG
#define RMR_KERNEL_ERR_STATE RMR_UK_ERR_STATE
#define RMR_KERNEL_ERR_NOMEM RMR_UK_ERR_NOMEM
#define RMR_KERNEL_ERR_RANGE RMR_UK_ERR_RANGE

#define RMR_UK_NATIVE_OK_MAGIC 0x56414343u

typedef struct {
  uint32_t signature;
  uint32_t pointer_bits;
  uint32_t cache_line_bytes;
  uint32_t page_bytes;
  uint32_t feature_mask;
  uint32_t reg_signature_0;
  uint32_t reg_signature_1;
  uint32_t reg_signature_2;
  uint32_t gpio_word_bits;
  uint32_t gpio_pin_stride;
} RmR_UnifiedCapabilities;

typedef struct {
  uint32_t seed;
  uint32_t arena_bytes;
} RmR_UnifiedConfig;

typedef struct {
  uint32_t crc32c;
  uint32_t entropy;
  uint32_t stage_counter;
} RmR_UnifiedIngestState;

typedef struct {
  uint32_t cpu_pressure;
  uint32_t storage_pressure;
  uint32_t io_pressure;
  int64_t matrix_determinant;
} RmR_UnifiedProcessState;

typedef struct {
  uint32_t route_id;
  uint64_t route_tag;
} RmR_UnifiedRouteState;

typedef struct {
  uint32_t computed_crc32c;
  uint32_t verify_ok;
} RmR_UnifiedVerifyState;

typedef struct {
  uint64_t audit_signature;
  uint32_t audit_code;
} RmR_UnifiedAuditState;

typedef struct {
  uint32_t offset;
  uint32_t size;
  uint32_t generation;
  uint8_t in_use;
} RmR_UnifiedArenaSlot;

#define RMR_UK_MAX_SLOTS 1024u

typedef struct RmR_UnifiedKernel {
  uint32_t initialized;
  uint32_t seed;
  uint32_t crc32c;
  uint32_t entropy;
  uint32_t stage_counter;
  uint64_t last_route_tag;
  RmR_UnifiedCapabilities caps;
  uint8_t *arena_base;
  uint32_t arena_capacity;
  RmR_UnifiedArenaSlot slots[RMR_UK_MAX_SLOTS];
} RmR_UnifiedKernel;

int RmR_UnifiedKernel_Init(RmR_UnifiedKernel *kernel, const RmR_UnifiedConfig *config);
int RmR_UnifiedKernel_Shutdown(RmR_UnifiedKernel *kernel);
int RmR_UnifiedKernel_Ingest(RmR_UnifiedKernel *kernel, const uint8_t *data, size_t len, RmR_UnifiedIngestState *out);
int RmR_UnifiedKernel_Process(RmR_UnifiedKernel *kernel,
                              uint64_t cpu_cycles,
                              uint64_t storage_read_bytes,
                              uint64_t storage_write_bytes,
                              uint64_t input_bytes,
                              uint64_t output_bytes,
                              int64_t m00,
                              int64_t m01,
                              int64_t m10,
                              int64_t m11,
                              RmR_UnifiedProcessState *out);
int RmR_UnifiedKernel_Route(RmR_UnifiedKernel *kernel, const RmR_UnifiedProcessState *process, RmR_UnifiedRouteState *out);
int RmR_UnifiedKernel_Verify(RmR_UnifiedKernel *kernel, const uint8_t *data, size_t len, uint32_t expected_crc32c, RmR_UnifiedVerifyState *out);
int RmR_UnifiedKernel_Audit(RmR_UnifiedKernel *kernel,
                            const RmR_UnifiedIngestState *ingest,
                            const RmR_UnifiedProcessState *process,
                            const RmR_UnifiedRouteState *route,
                            const RmR_UnifiedVerifyState *verify,
                            RmR_UnifiedAuditState *out);
int RmR_UnifiedKernel_Detect(RmR_UnifiedCapabilities *out_caps);
int RmR_UnifiedKernel_QueryCapabilities(const RmR_UnifiedKernel *kernel, RmR_UnifiedCapabilities *out_caps);

int RmR_UnifiedKernel_Copy(RmR_UnifiedKernel *kernel, uint8_t *dst, const uint8_t *src, size_t len);
uint32_t RmR_UnifiedKernel_XorChecksum(RmR_UnifiedKernel *kernel, const uint8_t *data, size_t len);

int RmR_UnifiedKernel_ArenaAlloc(RmR_UnifiedKernel *kernel, uint32_t bytes, uint32_t *out_handle);
int RmR_UnifiedKernel_ArenaFree(RmR_UnifiedKernel *kernel, uint32_t handle);
int RmR_UnifiedKernel_ArenaCopy(RmR_UnifiedKernel *kernel, uint32_t src_handle, uint32_t src_offset, uint32_t dst_handle, uint32_t dst_offset, uint32_t len);
int RmR_UnifiedKernel_ArenaFill(RmR_UnifiedKernel *kernel, uint32_t handle, uint32_t offset, uint32_t len, uint8_t value);
int RmR_UnifiedKernel_ArenaWrite(RmR_UnifiedKernel *kernel, uint32_t handle, uint32_t offset, const uint8_t *src, uint32_t len);
int RmR_UnifiedKernel_ArenaXorChecksum(RmR_UnifiedKernel *kernel, uint32_t handle, uint32_t offset, uint32_t len, uint32_t *out);

uint32_t RmR_UnifiedKernel_Popcount32(uint32_t value);
uint32_t RmR_UnifiedKernel_ByteSwap32(uint32_t value);
uint32_t RmR_UnifiedKernel_Rotl32(uint32_t value, uint32_t distance);
uint32_t RmR_UnifiedKernel_Rotr32(uint32_t value, uint32_t distance);

/* C ABI facade used by JNI glue to keep policy out of platform bindings. */
typedef RmR_UnifiedKernel rmr_kernel_state_t;

typedef struct {
  uint32_t signature;
  uint32_t pointer_bits;
  uint32_t cache_line_bytes;
  uint32_t page_bytes;
  uint32_t feature_mask;
  uint32_t register_width_bits;
  uint32_t pin_count_hint;
  uint32_t feature_bits_hi;
} rmr_kernel_capabilities_t;

typedef struct {
  uint64_t cpu_cycles;
  uint64_t storage_read_bytes;
  uint64_t storage_write_bytes;
  uint64_t input_bytes;
  uint64_t output_bytes;
  int64_t m00;
  int64_t m01;
  int64_t m10;
  int64_t m11;
} rmr_kernel_route_input_t;

typedef struct {
  uint32_t route;
  int64_t matrix_determinant;
  uint32_t cpu_pressure;
  uint32_t storage_pressure;
  uint32_t io_pressure;
  uint64_t route_tag;
} rmr_kernel_route_output_t;

int rmr_kernel_init(rmr_kernel_state_t *state, uint32_t seed);
int rmr_kernel_shutdown(rmr_kernel_state_t *state);
int rmr_kernel_get_capabilities(const rmr_kernel_state_t *state, rmr_kernel_capabilities_t *out_caps);
int rmr_kernel_autodetect(rmr_kernel_capabilities_t *out_caps);
int rmr_kernel_ingest(rmr_kernel_state_t *state, const uint8_t *data, uint32_t len, uint32_t *out_crc32c);
int rmr_kernel_process(rmr_kernel_state_t *state, int32_t a, int32_t b, uint32_t mode, int32_t *out_value);
int rmr_kernel_route(rmr_kernel_state_t *state, const rmr_kernel_route_input_t *in, rmr_kernel_route_output_t *out);
int rmr_kernel_verify(rmr_kernel_state_t *state, const uint8_t *data, uint32_t len, uint32_t expected_crc32c, uint32_t *out_verify_ok);
int rmr_kernel_audit(rmr_kernel_state_t *state, uint64_t *counters, uint32_t counter_count);

#ifdef __cplusplus
}
#endif

#endif
