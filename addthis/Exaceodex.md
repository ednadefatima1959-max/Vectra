/***********************************************************************
 * RMR EXACORDEX - FULL SENSOR FIELD INTEGRATION
 ***********************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ================= CONFIG ================= */

#define TORUS_SIZE 131072

/* ================= UTILS ================= */

uint64_t hash64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

float frand() {
    return (float)rand() / RAND_MAX;
}

/* ================= TORUS ================= */

typedef struct {
    float u,v,psi,chi,rho,delta,sigma;
    float tau;
    float omega;
} state_t;

state_t torus[TORUS_SIZE];
uint64_t matrix[TORUS_SIZE];

/* ================= PULSE ================= */

float phase = 0.0f;

float torus_wave() {
    phase += 0.01f;
    return sinf(phase);
}

/* ================= SENSOR FIELD Ω ================= */

typedef struct {

    /* CPU */
    float cpu_temp;
    float cpu_freq;
    float cpu_load;

    /* MEMORY */
    float mem_usage;
    float page_faults;

    /* ENERGY */
    float battery_level;
    float battery_temp;
    float current;

    /* NETWORK */
    float wifi_rssi;
    float wifi_noise;
    float wifi_jitter;

    /* MOTION */
    float accel;
    float gyro;
    float magnet;

    /* ENV */
    float light;
    float proximity;

    /* SYSTEM */
    float sched_jitter;
    float clock_drift;

} env_t;

/* ===== MOCK (substituir por syscalls reais depois) ===== */

env_t read_env() {
    env_t e;

    e.cpu_temp = 40 + frand()*40;
    e.cpu_freq = 1 + frand();
    e.cpu_load = frand();

    e.mem_usage = frand();
    e.page_faults = frand();

    e.battery_level = frand();
    e.battery_temp = 30 + frand()*15;
    e.current = frand();

    e.wifi_rssi = -70 + frand()*30;
    e.wifi_noise = frand();
    e.wifi_jitter = frand();

    e.accel = frand();
    e.gyro = frand();
    e.magnet = frand();

    e.light = frand();
    e.proximity = frand();

    e.sched_jitter = frand();
    e.clock_drift = frand();

    return e;
}

/* ================= Ω PROCESSING ================= */

float env_instability(env_t e) {
    return
        e.wifi_noise * 0.2f +
        e.wifi_jitter * 0.2f +
        fabsf(e.cpu_temp - 60.0f)*0.2f +
        e.sched_jitter * 0.2f +
        e.clock_drift * 0.2f;
}

uint64_t env_entropy(env_t e) {
    uint64_t seed = 0;

    seed ^= (uint64_t)(e.cpu_temp * 100);
    seed ^= (uint64_t)(e.wifi_noise * 1000);
    seed ^= (uint64_t)(e.accel * 1e5);
    seed ^= (uint64_t)(e.clock_drift * 1e6);

    return hash64(seed);
}

/* ================= CORE ================= */

float instability[TORUS_SIZE];

void update_instability(int i, float v) {
    instability[i] = 0.7f * instability[i] + 0.3f * v;
}

void update_cell(int i, float wave, float omega, float tau) {

    if (wave > 0) {
        matrix[i] ^= (uint64_t)(wave * 1e6);
    } else {
        matrix[i] &= ~(uint64_t)(fabsf(wave) * 1e6);
    }

    matrix[i] ^= (uint64_t)(omega * 1e3);
    matrix[i] ^= (uint64_t)(tau * 1e3);
}

/* ================= MODE CONTROL ================= */

int select_mode(float tau, float omega) {

    if (tau < 0.3f && omega < 0.3f)
        return 0; // determinístico

    if (tau > 0.6f || omega > 0.6f)
        return 1; // exploratório

    return 2; // híbrido
}

/* ================= MAIN LOOP ================= */

void tick() {

    env_t env = read_env();

    float omega = env_instability(env);
    uint64_t ent = env_entropy(env);

    float wave = torus_wave();

    for (int i = 0; i < TORUS_SIZE; i++) {

        float tau = instability[i];

        int mode = select_mode(tau, omega);

        update_cell(i, wave, omega, tau);

        if (mode == 1) {
            // exploração → usa mais entropia
            matrix[i] ^= ent;
        }

        float new_tau =
            fabsf((float)(matrix[i] % 1000)/1000.0f)
            + omega * 0.1f;

        update_instability(i, new_tau);
    }
}

/* ================= ENTRY ================= */

int main() {

    srand(time(NULL));

    while (1) {
        tick();
    }

    return 0;
}
