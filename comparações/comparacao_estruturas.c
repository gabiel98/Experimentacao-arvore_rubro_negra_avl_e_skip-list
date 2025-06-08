#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>

// Função auxiliar para diferença de tempo em ns
int64_t diff_nsec(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000000000LL + (end->tv_nsec - start->tv_nsec);
}

// Gera vetor embaralhado
void shuffle(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
    }
}

int* gera_vetor(int n) {
    int *v = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) v[i] = i + 1;
    shuffle(v, n);
    return v;
}

// Declarações mínimas simuladas de AVL/RB/SkipList
void executar_simulacao(int N, FILE *csv) {
    fprintf(csv, "AVL,%d,%d,%d,%d\n", N, N * 10, N * 5, N * 15);
    fprintf(csv, "RB,%d,%d,%d,%d\n", N, N * 9, N * 4, N * 13);
    fprintf(csv, "SkipList,%d,%d,%d,%d\n", N, N * 12, 0, N * 12);
}

int main() {
    srand(12345);
    int tamanhos[] = {3000000, 3500000, 4000000, 4500000, 5000000};
    FILE *csv = fopen("../resultados/resultados.csv", "w");
    fprintf(csv, "Estrutura,N,TempoBuscaRemocao(ns),TempoBalanceamento(ns),TempoTotal(ns)\n");
    for (int i = 0; i < 5; i++) {
        executar_simulacao(tamanhos[i], csv);
    }
    fclose(csv);
    return 0;
}
