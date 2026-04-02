// ============================================================================
// VECTRA – 56 CYCLE GEOMETRIC EXPLORER
// ============================================================================

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define DIM 6
#define STEPS 300
#define CYCLE 56

typedef struct {
    double x[DIM];
    double prev[DIM];
    double prev2[DIM];

    double alpha;
    double beta;

    double delta;
    double C;
    double H;
    double I;
} State;

// ================= DERIVADA =================
void derivative(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++)
        out[i] = s->x[i] - s->prev[i];
}

// ================= ANTIDERIVADA =================
void antiderivative(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++)
        out[i] = (s->x[i] + s->prev[i] + s->prev2[i]) / 3.0;
}

// ================= INVERSÃO =================
void inverse(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++)
        out[i] = fabs(s->x[i]) > 1e-6 ? 1.0 / s->x[i] : 0.0;
}

// ================= RECURSÃO INDIRETA =================
double rec_term(double x, double prev, int depth) {
    if(depth == 0) return sin(x + prev);
    return sin(x) + 0.3 * rec_term(prev, x, depth-1);
}

void recursive(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++)
        out[i] = rec_term(s->x[i], s->prev[i], 3);
}

// ================= HEURÍSTICA (NOVO) =================
void heuristic(State* s, double out[DIM]) {
    for(int i=0;i<DIM;i++) {
        double trend = s->x[i] - s->prev2[i];
        out[i] = tanh(trend);
    }
}

// ================= DELTA =================
double compute_delta(State* s) {
    double sum=0;
    for(int i=0;i<DIM;i++) {
        double d = s->x[i] - s->prev[i];
        sum += d*d;
    }
    return sqrt(sum);
}

// ================= INVARIANTE =================
double compute_I(State* s) {
    double a=0,b=0;
    for(int i=0;i<DIM;i++) {
        a += s->x[i]*s->x[i];
        b += s->prev[i]*s->prev[i];
    }
    return a-b;
}

// ================= PESOS DINÂMICOS =================
void update_weights(double W[5], State* s) {

    double eta = 0.02;

    if(s->delta > 1.0) {
        W[1] += eta; // memória
        W[3] += eta; // inversão
    }

    if(s->C < 0.3) {
        W[2] += eta; // recursão
    }

    if(s->H > 1.5) {
        W[4] += eta; // heurística
    }

    if(s->delta < 0.2 && s->C > 0.8) {
        W[0] += eta; // derivada
    }

    // normalizar
    double sum=0;
    for(int i=0;i<5;i++) sum+=W[i];
    for(int i=0;i<5;i++) W[i]/=sum;
}

// ================= REVERSÃO PARCIAL =================
void partial_reverse(State* s) {
    for(int i=0;i<DIM;i++) {
        double tmp = s->x[i];
        s->x[i] = s->prev[i];
        s->prev[i] = tmp;
    }
}

// ================= STEP =================
void step(State* s, double W[5], int t) {

    double D[DIM], A[DIM], R[DIM], I[DIM], Hx[DIM];

    derivative(s,D);
    antiderivative(s,A);
    recursive(s,R);
    inverse(s,I);
    heuristic(s,Hx);

    double parity = ((int)(s->alpha + s->beta) % 2 == 0) ? 1 : -1;
    double q = cos(s->alpha - s->beta);

    double new_x[DIM];

    for(int i=0;i<DIM;i++) {
        new_x[i] =
            parity * (
                W[0]*D[i] +
                W[1]*A[i] +
                W[2]*R[i] +
                W[3]*I[i] +
                W[4]*Hx[i]
            ) + 0.1*q;
    }

    // SHIFT
    for(int i=0;i<DIM;i++) {
        s->prev2[i] = s->prev[i];
        s->prev[i] = s->x[i];
        s->x[i] = new_x[i];
    }

    // FASES (DESACOPLADAS → EXPLORAÇÃO REAL)
    s->alpha = fmod(s->alpha + 1.0, CYCLE);
    s->beta  = fmod(s->beta  + 3.0, CYCLE);

    // MÉTRICAS
    s->delta = compute_delta(s);
    s->C = 1.0 / (1.0 + s->delta);
    s->H = log(1.0 + s->delta);
    s->I = compute_I(s);

    // REVERSÃO EM PONTO CRÍTICO
    if(fabs(s->I) > 2.0) {
        partial_reverse(s);
    }
}

// ================= MAIN =================
int main() {

    State s = {0};

    for(int i=0;i<DIM;i++) {
        s.x[i] = 0.1*(i+1);
    }

    double W[5] = {0.2,0.2,0.2,0.2,0.2};

    printf("t,x0,x1,x2,delta,C,H,I,alpha,beta\n");

    for(int t=0;t<STEPS;t++) {

        step(&s,W,t);
        update_weights(W,&s);

        printf("%d,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.0f,%.0f\n",
            t,
            s.x[0], s.x[1], s.x[2],
            s.delta, s.C, s.H, s.I,
            s.alpha, s.beta
        );
    }

    return 0;
}
