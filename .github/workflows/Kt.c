// ============================================================================
// VECTRA TOROIDAL GEOMETRIC DYNAMICS – Invariante + Retroalimentação Quântica
// ============================================================================
// Conceitos integrados:
// - Toro 2D (fases α, β com períodos 56) → superfície hiperforme
// - Vértices (pontos no espaço 6D) e arestas (conexões via diferenças)
// - Paridade geométrica (alternância de sinal por paridade de fase)
// - Sobreposição quântica virtual (termo cos(α-β))
// - Invariante geométrica coerente I(t) = ||x||² - ||prev||²
// - Retroalimentação de observação: pesos adaptativos usam C, H e delta
// - Multifilamento fractal: auto-chamada recursiva com escala reduzida
// - Fórmula de Pitágoras generalizada: delta² = ΣΔx_i²
// - Constantes π, Ω (número de Feigenbaum ≈ 4.669), infinito simbólico
// ============================================================================

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define DIM         6       // 3 coordenadas espaciais + 3 angulares (toro)
#define CYCLES_ALPHA 56     // período da fase α
#define CYCLES_BETA  56     // período da fase β
#define STEPS       200
#define FRACTAL_DEPTH 3     // profundidade da recursão fractal

// Constantes mágicas
#define PI          3.14159265358979323846
#define FEIGENBAUM  4.66920160910299067185   // Ω (constante de Feigenbaum)

typedef struct {
    double x[DIM];          // posição atual (6D)
    double prev[DIM];       // posição anterior
    double prev2[DIM];      // duas posições atrás
    double alpha;           // fase toroidal α [0, CYCLES_ALPHA)
    double beta;            // fase toroidal β [0, CYCLES_BETA)
    double delta;           // norma da diferença (Pitágoras)
    double H;               // entropia
    double C;               // coerência
    double I;               // invariante geométrica I(t) = ||x||² - ||prev||²
} State;

// ========================== DERIVADA (arestas) ==========================
void derivative(const State* s, double out[DIM]) {
    for (int i = 0; i < DIM; i++)
        out[i] = s->x[i] - s->prev[i];
}

// ========================== ANTIDERIVADA ==========================
void antiderivative(const State* s, double out[DIM]) {
    for (int i = 0; i < DIM; i++)
        out[i] = s->x[i] + s->prev[i] + s->prev2[i];
}

// ========================== RECURSÃO INDIRETA (fractal) ==========================
// Termo recursivo que chama a própria dinâmica em escala reduzida
double recursive_fractal_term(const State* s, int dim, int depth) {
    if (depth <= 0) return sin(s->prev[dim] + s->prev2[dim] + s->alpha + s->beta);
    // Auto-similaridade fractal: escala e deslocamento
    double scale = 0.5 / depth;
    State child = *s;
    child.x[dim] *= scale;
    child.prev[dim] *= scale;
    child.prev2[dim] *= scale;
    // Chamada recursiva (simplificada: apenas um termo)
    return sin(child.x[dim] + child.prev[dim]) + 0.1 * recursive_fractal_term(&child, dim, depth-1);
}

void recursive_term(const State* s, double out[DIM]) {
    for (int i = 0; i < DIM; i++)
        out[i] = recursive_fractal_term(s, i, FRACTAL_DEPTH);
}

// ========================== INVERSÃO (proteção contra singularidade) ==========================
void inverse_term(const State* s, double out[DIM]) {
    for (int i = 0; i < DIM; i++)
        out[i] = (fabs(s->x[i]) > 1e-6) ? 1.0 / s->x[i] : 0.0;
}

// ========================== PARIDADE GEOMÉTRICA ==========================
// Alterna sinal conforme a paridade da soma das fases
double parity_term(const State* s) {
    int sum_phase = (int)(s->alpha + s->beta);
    return (sum_phase % 2 == 0) ? 1.0 : -1.0;
}

// ========================== SOBREPOSIÇÃO QUÂNTICA VIRTUAL ==========================
double quantum_interference(const State* s) {
    return cos(s->alpha - s->beta);   // interferência entre as duas fases toroidais
}

// ========================== DELTA (norma Pitagórica) ==========================
double compute_delta(const State* s) {
    double sum_sq = 0.0;
    for (int i = 0; i < DIM; i++) {
        double diff = s->x[i] - s->prev[i];
        sum_sq += diff * diff;
    }
    return sqrt(sum_sq);   // raiz quadrada = norma euclidiana (Pitágoras)
}

// ========================== INVARIANTE GEOMÉTRICA COERENTE ==========================
double compute_invariant(const State* s) {
    double norm_x = 0.0, norm_prev = 0.0;
    for (int i = 0; i < DIM; i++) {
        norm_x += s->x[i] * s->x[i];
        norm_prev += s->prev[i] * s->prev[i];
    }
    return norm_x - norm_prev;   // I(t) = ||x(t)||² - ||x(t-1)||²
}

// ========================== PESOS ADAPTATIVOS (retroalimentação de observação) ==========================
void update_weights(double W[5], double delta, double C, double H) {
    double eta = 0.01;
    // Regras baseadas em delta, coerência e entropia
    if (delta > 1.0 || C < 0.3) {
        W[1] += eta;   // reforça memória (antiderivada)
        W[3] += eta;   // reforça inversão (estabiliza)
    }
    if (H > 1.5) {
        W[2] += eta;   // reforça recursão fractal (explora caos)
    }
    if (delta < 0.1 && C > 0.8) {
        W[0] += eta;   // reforça derivada (regime estacionário)
    }
    // Normalização para soma = 1
    double sum = 0.0;
    for (int i = 0; i < 5; i++) sum += W[i];
    for (int i = 0; i < 5; i++) W[i] /= sum;
}

// ========================== PASSO PRINCIPAL (dinâmica toroidal) ==========================
void step(State* s, double W[5]) {
    double D[DIM], A[DIM], R[DIM], I[DIM];
    derivative(s, D);
    antiderivative(s, A);
    recursive_term(s, R);
    inverse_term(s, I);

    double parity = parity_term(s);
    double q_intf = quantum_interference(s);

    double new_x[DIM];
    for (int i = 0; i < DIM; i++) {
        new_x[i] = parity * (
            W[0] * D[i] +
            W[1] * A[i] +
            W[2] * R[i] +
            W[3] * I[i] +
            W[4] * s->delta
        ) + q_intf * 0.1;   // acoplamento da interferência quântica
    }

    // Atualiza histórico (shift)
    for (int i = 0; i < DIM; i++) {
        s->prev2[i] = s->prev[i];
        s->prev[i]  = s->x[i];
        s->x[i]     = new_x[i];
    }

    // Atualiza fases toroidais (movimento circular)
    s->alpha = fmod(s->alpha + 1.0, CYCLES_ALPHA);
    s->beta  = fmod(s->beta  + 1.0, CYCLES_BETA);

    // Recalcula delta, coerência, entropia e invariante
    s->delta = compute_delta(s);
    s->C = 1.0 / (1.0 + s->delta);
    s->H = log(1.0 + s->delta);
    s->I = compute_invariant(s);
}

// ========================== MAIN ==========================
int main() {
    State s = {0};

    // Condições iniciais suaves (vértices do hipercubo)
    for (int i = 0; i < DIM; i++) {
        s.x[i] = 0.1 * (i+1);
        s.prev[i] = 0.0;
        s.prev2[i] = 0.0;
    }
    s.alpha = 0.0;
    s.beta = 0.0;
    s.delta = 0.0;
    s.C = 0.5;
    s.H = 0.5;
    s.I = 0.0;

    // Pesos iniciais igualitários
    double W[5] = {0.2, 0.2, 0.2, 0.2, 0.2};

    // Cabeçalho CSV
    printf("t,x0,x1,x2,x3,x4,x5,delta,C,H,alpha,beta,invariant\n");

    for (int t = 0; t < STEPS; t++) {
        step(&s, W);
        update_weights(W, s.delta, s.C, s.H);

        // Imprime 6 coordenadas, delta, C, H, fases e invariante
        printf("%d", t);
        for (int i = 0; i < DIM; i++) printf(",%.5f", s.x[i]);
        printf(",%.5f,%.5f,%.5f,%.0f,%.0f,%.5f\n",
               s.delta, s.C, s.H, s.alpha, s.beta, s.I);
    }

    return 0;
}
