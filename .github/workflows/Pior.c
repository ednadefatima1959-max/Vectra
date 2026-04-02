// ============================================================
// VECTRA 56-CYCLE GEOMETRIC DYNAMICS SIMULATION
// ============================================================
// Sistema determinístico com:
// - 56 ciclos de fase
// - Derivada, Antiderivada, Recursão indireta, Inversão
// - 5 pesos adaptativos
// ============================================================

#include <stdio.h>
#include <math.h>

#define DIM 3
#define CYCLES 56
#define STEPS 200

typedef struct {
    double x[DIM];   // posição
    double prev[DIM];
    double prev2[DIM];
    double phase;
    double delta;
    double H; // entropia
    double C; // coerência
} State;

// ================= DERIVADA =================
void derivative(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++) {
        out[i] = s->x[i] - s->prev[i];
    }
}

// ================= ANTIDERIVADA =================
void antiderivative(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++) {
        out[i] = s->x[i] + s->prev[i] + s->prev2[i];
    }
}

// ================= RECURSÃO INDIRETA =================
void recursive_term(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++) {
        out[i] = sin(s->prev[i] + s->prev2[i] + s->phase);
    }
}

// ================= INVERSÃO =================
void inverse_term(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++) {
        out[i] = (fabs(s->x[i]) > 1e-6) ? 1.0 / s->x[i] : 0.0;
    }
}

// ================= DELTA =================
double compute_delta(State* s) {
    double d = 0.0;
    for(int i=0;i<DIM;i++) {
        double diff = s->x[i] - s->prev[i];
        d += diff * diff;
    }
    return sqrt(d);
}

// ================= PESOS ADAPTATIVOS =================
void update_weights(double W[5], double delta) {
    double eta = 0.01;

    if(delta > 1.0) {
        W[1] += eta; // memória
        W[3] += eta; // inversão
    } else {
        W[0] += eta; // derivada
    }

    // normalização simples
    double sum = 0;
    for(int i=0;i<5;i++) sum += W[i];
    for(int i=0;i<5;i++) W[i] /= sum;
}

// ================= PASSO PRINCIPAL =================
void step(State* s, double W[5]) {

    double D[DIM], A[DIM], R[DIM], I[DIM];
    derivative(s, D);
    antiderivative(s, A);
    recursive_term(s, R);
    inverse_term(s, I);

    double new_x[DIM];

    for(int i=0;i<DIM;i++) {
        new_x[i] =
            W[0]*D[i] +
            W[1]*A[i] +
            W[2]*R[i] +
            W[3]*I[i] +
            W[4]*s->delta;
    }

    // shift histórico
    for(int i=0;i<DIM;i++) {
        s->prev2[i] = s->prev[i];
        s->prev[i]  = s->x[i];
        s->x[i]     = new_x[i];
    }

    // atualizar fase
    s->phase = fmod(s->phase + 1.0, CYCLES);

    // delta
    s->delta = compute_delta(s);

    // coerência / entropia simples
    s->C = 1.0 / (1.0 + s->delta);
    s->H = log(1.0 + s->delta);
}

// ================= MAIN =================
int main() {

    State s = {0};

    // estado inicial
    for(int i=0;i<DIM;i++) {
        s.x[i] = 0.1 * (i+1);
        s.prev[i] = 0;
        s.prev2[i] = 0;
    }

    s.phase = 0;
    s.delta = 0.0;
    s.C = 0.5;
    s.H = 0.5;

    // pesos iniciais
    double W[5] = {0.2, 0.2, 0.2, 0.2, 0.2};

    printf("t, x0, x1, x2, delta, C, H, phase\n");

    for(int t=0;t<STEPS;t++) {

        step(&s, W);
        update_weights(W, s.delta);

        printf("%d, %.5f, %.5f, %.5f, %.5f, %.5f, %.5f, %.0f\n",
            t,
            s.x[0], s.x[1], s.x[2],
            s.delta,
            s.C,
            s.H,
            s.phase
        );
    }

    return 0;
}
