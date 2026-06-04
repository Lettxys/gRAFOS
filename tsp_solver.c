/*
 * Solver para o Problema do Caixeiro Viajante (TSP)
 * Metodo: Nearest Neighbor + 2-opt + Or-opt + Iterated Local Search (ILS)
 *
 * Formato de entrada: TSPLIB (EUC_2D, simetrico)
 * Uso: ./tsp_solver [semente] [tempo_limite_seg] < instancia.tsp > solucao.tour
 *
 * Compilacao: gcc -O2 -Wall -std=c99 -o tsp_solver tsp_solver.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>

/* ============================================================
 * Variaveis globais
 * ============================================================ */
static int n;
static double *coord_x, *coord_y;
static int *dist_matrix;
static char instance_name[512];
static clock_t program_start;

static inline int dist(int i, int j)
{
    return dist_matrix[i * n + j];
}

static double elapsed_seconds(void)
{
    return (double)(clock() - program_start) / CLOCKS_PER_SEC;
}

/* ============================================================
 * Leitura da instancia no formato TSPLIB
 * ============================================================ */
static void parse_tsplib(void)
{
    char buf[1024];
    instance_name[0] = '\0';
    n = 0;

    while (fgets(buf, sizeof buf, stdin)) {
        if (strncmp(buf, "NODE_COORD_SECTION", 18) == 0) {
            if (n <= 0) {
                fprintf(stderr, "Erro: DIMENSION nao definida antes de NODE_COORD_SECTION\n");
                exit(1);
            }
            for (int i = 0; i < n; i++) {
                if (!fgets(buf, sizeof buf, stdin)) break;
                if (strncmp(buf, "EOF", 3) == 0) break;
                int id;
                double x, y;
                if (sscanf(buf, "%d %lf %lf", &id, &x, &y) == 3) {
                    coord_x[i] = x;
                    coord_y[i] = y;
                }
            }
            return;
        }

        char *colon = strchr(buf, ':');
        if (!colon) continue;

        *colon = '\0';
        char *key = buf;
        while (*key == ' ') key++;
        char *ke = colon - 1;
        while (ke > key && *ke == ' ') ke--;
        *(ke + 1) = '\0';

        char *val = colon + 1;
        while (*val == ' ') val++;
        char *ve = val + strlen(val) - 1;
        while (ve > val && (*ve == '\n' || *ve == '\r' || *ve == ' ')) ve--;
        *(ve + 1) = '\0';

        if (strcmp(key, "NAME") == 0) {
            strncpy(instance_name, val, sizeof(instance_name) - 1);
            instance_name[sizeof(instance_name) - 1] = '\0';
        } else if (strcmp(key, "DIMENSION") == 0) {
            n = atoi(val);
            if (n <= 0) {
                fprintf(stderr, "Erro: DIMENSION invalida: %d\n", n);
                exit(1);
            }
            coord_x = (double *)malloc(n * sizeof(double));
            coord_y = (double *)malloc(n * sizeof(double));
            if (!coord_x || !coord_y) {
                fprintf(stderr, "Erro: falha ao alocar coordenadas\n");
                exit(1);
            }
        } else if (strcmp(key, "EDGE_WEIGHT_TYPE") == 0) {
            if (strcmp(val, "EUC_2D") != 0) {
                fprintf(stderr, "Erro: apenas EUC_2D suportado (recebido: %s)\n", val);
                exit(1);
            }
        }
    }
}

/* ============================================================
 * Construcao da matriz de distancias (EUC_2D TSPLIB)
 * d_ij = floor(0.5 + sqrt((xi-xj)^2 + (yi-yj)^2))
 * ============================================================ */
static void build_distance_matrix(void)
{
    dist_matrix = (int *)malloc((size_t)n * n * sizeof(int));
    if (!dist_matrix) {
        fprintf(stderr, "Erro: falha ao alocar matriz de distancias (%d x %d)\n", n, n);
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        dist_matrix[i * n + i] = 0;
        for (int j = i + 1; j < n; j++) {
            double dx = coord_x[i] - coord_x[j];
            double dy = coord_y[i] - coord_y[j];
            int d = (int)floor(0.5 + sqrt(dx * dx + dy * dy));
            dist_matrix[i * n + j] = d;
            dist_matrix[j * n + i] = d;
        }
    }
}

/* ============================================================
 * Calculo do custo total de um tour
 * ============================================================ */
static int tour_cost(const int *tour)
{
    int cost = 0;
    for (int i = 0; i < n - 1; i++)
        cost += dist(tour[i], tour[i + 1]);
    cost += dist(tour[n - 1], tour[0]);
    return cost;
}

/* ============================================================
 * Heuristica construtiva: Vizinho Mais Proximo
 * Constroi um tour a partir da cidade 'start' escolhendo
 * sempre a cidade nao visitada mais proxima.
 * Complexidade: O(n^2)
 * ============================================================ */
static int nearest_neighbor(int start, int *tour)
{
    int *visited = (int *)calloc(n, sizeof(int));
    tour[0] = start;
    visited[start] = 1;
    int cost = 0;

    for (int i = 1; i < n; i++) {
        int cur = tour[i - 1];
        int best = -1, best_dist = INT_MAX;
        for (int j = 0; j < n; j++) {
            if (!visited[j] && dist(cur, j) < best_dist) {
                best_dist = dist(cur, j);
                best = j;
            }
        }
        tour[i] = best;
        visited[best] = 1;
        cost += best_dist;
    }
    cost += dist(tour[n - 1], tour[0]);

    free(visited);
    return cost;
}

/* ============================================================
 * 2-opt: melhoria local por inversao de segmentos
 *
 * Para cada par (i,j), verifica se inverter o segmento
 * tour[i+1..j] reduz o custo. Se sim, aplica a inversao.
 * Repete ate nao haver mais melhoria.
 * Complexidade: O(n^2) por iteracao
 * ============================================================ */
static void reverse_segment(int *tour, int left, int right)
{
    while (left < right) {
        int tmp = tour[left];
        tour[left] = tour[right];
        tour[right] = tmp;
        left++;
        right--;
    }
}

static int two_opt(int *tour, int cost)
{
    int improved = 1;
    while (improved) {
        improved = 0;
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 2; j < n; j++) {
                /* Pular reversao do tour inteiro (mesma direcao, custo igual) */
                if (i == 0 && j == n - 1) continue;

                int ni = i + 1;
                int nj = (j + 1) % n;
                int delta = dist(tour[i], tour[j]) + dist(tour[ni], tour[nj])
                          - dist(tour[i], tour[ni]) - dist(tour[j], tour[nj]);

                if (delta < 0) {
                    reverse_segment(tour, ni, j);
                    cost += delta;
                    improved = 1;
                }
            }
        }
    }
    return cost;
}

/* ============================================================
 * Or-opt: relocacao de segmentos de tamanho 1, 2 ou 3
 *
 * Remove um segmento de k cidades consecutivas e o insere
 * na melhor posicao do tour restante (testando ambas
 * orientacoes para k >= 2).
 * Complexidade: O(n^2) por iteracao
 * ============================================================ */
static int or_opt(int *tour, int cost)
{
    int *temp = (int *)malloc(n * sizeof(int));
    int seg[3];
    int improved = 1;

    while (improved) {
        improved = 0;

        for (int seg_size = 3; seg_size >= 1 && !improved; seg_size--) {
            for (int i = 0; i <= n - seg_size && !improved; i++) {
                int pred = (i == 0) ? n - 1 : i - 1;
                int succ = (i + seg_size) % n;
                int seg_first = tour[i];
                int seg_last = tour[i + seg_size - 1];

                /* Variacao no custo ao remover o segmento */
                int remove_delta = dist(tour[pred], tour[succ])
                                 - dist(tour[pred], seg_first)
                                 - dist(seg_last, tour[succ]);

                /* Salva segmento e constroi tour reduzido */
                for (int m = 0; m < seg_size; m++)
                    seg[m] = tour[i + m];

                int tp = 0;
                for (int m = 0; m < n; m++)
                    if (m < i || m >= i + seg_size)
                        temp[tp++] = tour[m];

                int reduced_n = n - seg_size;
                int best_delta = 0, best_pos = -1, best_reversed = 0;

                /* Testa todas as posicoes de insercao no tour reduzido */
                for (int j = 0; j < reduced_n; j++) {
                    int jnext = (j + 1) % reduced_n;
                    int edge_cost = dist(temp[j], temp[jnext]);

                    /* Orientacao normal */
                    int ins_delta = dist(temp[j], seg[0])
                                  + dist(seg[seg_size - 1], temp[jnext])
                                  - edge_cost;
                    int total = remove_delta + ins_delta;
                    if (total < best_delta) {
                        best_delta = total;
                        best_pos = j;
                        best_reversed = 0;
                    }

                    /* Orientacao reversa (para segmentos >= 2) */
                    if (seg_size >= 2) {
                        ins_delta = dist(temp[j], seg[seg_size - 1])
                                  + dist(seg[0], temp[jnext])
                                  - edge_cost;
                        total = remove_delta + ins_delta;
                        if (total < best_delta) {
                            best_delta = total;
                            best_pos = j;
                            best_reversed = 1;
                        }
                    }
                }

                if (best_pos >= 0 && best_delta < 0) {
                    /* Inverte segmento se a orientacao reversa for melhor */
                    if (best_reversed) {
                        for (int m = 0; m < seg_size / 2; m++) {
                            int tmp_val = seg[m];
                            seg[m] = seg[seg_size - 1 - m];
                            seg[seg_size - 1 - m] = tmp_val;
                        }
                    }

                    /* Reconstroi tour com segmento na nova posicao */
                    tp = 0;
                    for (int m = 0; m <= best_pos; m++)
                        tour[tp++] = temp[m];
                    for (int m = 0; m < seg_size; m++)
                        tour[tp++] = seg[m];
                    for (int m = best_pos + 1; m < reduced_n; m++)
                        tour[tp++] = temp[m];

                    cost += best_delta;
                    improved = 1;
                }
            }
        }
    }

    free(temp);
    return cost;
}

/* ============================================================
 * Busca local combinada: alterna 2-opt e Or-opt ate
 * nao haver mais melhoria em nenhum dos dois.
 * ============================================================ */
static int local_search(int *tour, int cost)
{
    int prev_cost;
    do {
        prev_cost = cost;
        cost = two_opt(tour, cost);
        cost = or_opt(tour, cost);
    } while (cost < prev_cost);
    return cost;
}

/* ============================================================
 * Perturbacao Double Bridge (para ILS)
 *
 * Divide o tour em 4 segmentos A, B, C, D usando 3 cortes
 * aleatorios e reconecta como A + C + B + D. Essa perturbacao
 * nao pode ser desfeita por 2-opt, permitindo escapar de
 * otimos locais.
 * ============================================================ */
static void double_bridge(const int *src, int *dst)
{
    if (n < 8) {
        memcpy(dst, src, n * sizeof(int));
        if (n >= 2) {
            int a = rand() % n;
            int b;
            do { b = rand() % n; } while (b == a);
            int tmp = dst[a];
            dst[a] = dst[b];
            dst[b] = tmp;
        }
        return;
    }

    /* 3 pontos de corte distintos e ordenados em [1, n-1] */
    int cuts[3];
    cuts[0] = 1 + rand() % (n - 1);
    do { cuts[1] = 1 + rand() % (n - 1); } while (cuts[1] == cuts[0]);
    do { cuts[2] = 1 + rand() % (n - 1); } while (cuts[2] == cuts[0] || cuts[2] == cuts[1]);

    /* Ordena os cortes */
    if (cuts[0] > cuts[1]) { int t = cuts[0]; cuts[0] = cuts[1]; cuts[1] = t; }
    if (cuts[1] > cuts[2]) { int t = cuts[1]; cuts[1] = cuts[2]; cuts[2] = t; }
    if (cuts[0] > cuts[1]) { int t = cuts[0]; cuts[0] = cuts[1]; cuts[1] = t; }

    /* Reconecta: A=[0,c0) + C=[c1,c2) + B=[c0,c1) + D=[c2,n) */
    int p = 0;
    for (int i = 0; i < cuts[0]; i++)         dst[p++] = src[i];
    for (int i = cuts[1]; i < cuts[2]; i++)    dst[p++] = src[i];
    for (int i = cuts[0]; i < cuts[1]; i++)    dst[p++] = src[i];
    for (int i = cuts[2]; i < n; i++)          dst[p++] = src[i];
}

/* ============================================================
 * Saida no formato TSPLIB TOUR
 * ============================================================ */
static void output_solution(const int *tour, int cost)
{
    printf("NAME: %s\n", instance_name);
    printf("COMMENT: Metodo NN + 2-opt + Or-opt + ILS\n");
    printf("TYPE: TOUR\n");
    printf("DIMENSION: %d\n", n);
    printf("TOTAL_WEIGHT: %d\n", cost);
    printf("TOUR_SECTION\n");
    for (int i = 0; i < n; i++)
        printf("%d\n", tour[i] + 1);
    printf("EOF\n");
}

/* ============================================================
 * Programa principal
 * ============================================================ */
int main(int argc, char *argv[])
{
    /* Parametros opcionais: semente e tempo limite */
    unsigned int seed = 42;
    double time_budget = -1.0;
    if (argc > 1) seed = (unsigned int)atoi(argv[1]);
    if (argc > 2) time_budget = atof(argv[2]);
    srand(seed);
    program_start = clock();

    /* Leitura e preprocessamento */
    parse_tsplib();
    if (n <= 0) {
        fprintf(stderr, "Erro: nenhuma cidade lida\n");
        return 1;
    }
    build_distance_matrix();
    fprintf(stderr, "Instancia: %s (%d cidades)\n", instance_name, n);

    int *tour      = (int *)malloc(n * sizeof(int));
    int *best      = (int *)malloc(n * sizeof(int));
    int *perturbed = (int *)malloc(n * sizeof(int));
    int best_cost  = INT_MAX;

    /* --------------------------------------------------------
     * Fase 1: Construcao inicial com Nearest Neighbor
     * Testa todos os vertices como ponto de partida e
     * mantem o melhor tour encontrado.
     * -------------------------------------------------------- */
    int max_starts = (n <= 2000) ? n : 2000;
    for (int s = 0; s < max_starts; s++) {
        int start_city = (max_starts < n) ? (rand() % n) : s;
        int c = nearest_neighbor(start_city, tour);
        if (c < best_cost) {
            best_cost = c;
            memcpy(best, tour, n * sizeof(int));
        }
    }
    fprintf(stderr, "Melhor NN: %d (%.2fs)\n", best_cost, elapsed_seconds());

    /* --------------------------------------------------------
     * Fase 2: Busca local (2-opt + Or-opt) no melhor tour NN
     * -------------------------------------------------------- */
    best_cost = local_search(best, best_cost);
    fprintf(stderr, "Apos busca local: %d (%.2fs)\n", best_cost, elapsed_seconds());

    /* --------------------------------------------------------
     * Fase 3: Iterated Local Search (ILS)
     * Perturba o tour com Double Bridge e aplica busca local.
     * Mantem a melhor solucao encontrada.
     * -------------------------------------------------------- */
    if (time_budget < 0) {
        if      (n <= 100)  time_budget = 5.0;
        else if (n <= 500)  time_budget = 10.0;
        else if (n <= 1000) time_budget = 20.0;
        else                time_budget = 30.0;
    }

    int ils_iters = 0;
    while (elapsed_seconds() < time_budget) {
        double_bridge(best, perturbed);
        int pc = tour_cost(perturbed);
        pc = local_search(perturbed, pc);

        if (pc < best_cost) {
            best_cost = pc;
            memcpy(best, perturbed, n * sizeof(int));
            fprintf(stderr, "  ILS melhoria #%d: %d (%.2fs)\n",
                    ils_iters + 1, best_cost, elapsed_seconds());
        }
        ils_iters++;
    }
    fprintf(stderr, "ILS concluido: %d iteracoes em %.2fs\n",
            ils_iters, elapsed_seconds());

    /* Verificacao final de consistencia */
    best_cost = tour_cost(best);
    fprintf(stderr, "Custo final verificado: %d\n", best_cost);

    /* Saida da solucao */
    output_solution(best, best_cost);

    /* Liberacao de memoria */
    free(tour);
    free(best);
    free(perturbed);
    free(dist_matrix);
    free(coord_x);
    free(coord_y);

    return 0;
}
