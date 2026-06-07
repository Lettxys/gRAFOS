#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LIMITE_CANDIDATAS 60
#define INTEGRANTES "Leticia Almeida, Isabelli Pinho e Elislandia Aparecida"

// Estrutura da cidade: id e as coordenadas X e Y
typedef struct {
    int id;
    double x;
    double y;
} Cidade;

// Representa uma rota inicial que vai ter VND, só algumas vão ter porque antes fazíamos em todas e levava MUITO tempo 
typedef struct {
    long long peso;
    int *rota;
} RotaCandidata;

// Função que calcula a distância entre duas cidades 
int calDistancia(Cidade a, Cidade b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return (int)floor(0.5 + sqrt(dx * dx + dy * dy));
}

// Função que calcula o peso total de uma rota inteira (usando matriz de distâncias)
long long calcularPesoTotal(int *rota, int n_cidades, int **dist) {
    long long peso = 0;
    for (int i = 0; i < n_cidades - 1; i++) {
        peso += dist[rota[i]][rota[i+1]];
    }
    peso += dist[rota[n_cidades - 1]][rota[0]];
    return peso;
}

// Verifica quantas candidatas vamos otimizar
int quantidadeCandidatas(int n_cidades) {
    if (n_cidades <= 80) return n_cidades;
    return LIMITE_CANDIDATAS;
}

// guarda as 60 melhores
void guardarCandidata(RotaCandidata *candidatas, int limite, int *rota, int n_cidades, long long peso) {
    int posicao = -1;

    for (int i = 0; i < limite; i++) {
        if (candidatas[i].peso == -1 || peso < candidatas[i].peso) {
            posicao = i;
            break;
        }
    }

    if (posicao == -1) return;

    for (int i = limite - 1; i > posicao; i--) {
        candidatas[i].peso = candidatas[i - 1].peso;
        memcpy(candidatas[i].rota, candidatas[i - 1].rota, n_cidades * sizeof(int));
    }

    candidatas[posicao].peso = peso;
    memcpy(candidatas[posicao].rota, rota, n_cidades * sizeof(int));
}

// ---------   VIZINHANÇA 1: 2-Opt (Descruza caminhos)   ---------------------------------------------------------------------

int otimizar2Opt(int *rota, int n_cidades, int **dist) {
    for(int i = 0; i < n_cidades - 1; i++) {
        for(int j = i + 1; j < n_cidades; j++) {
            if (i == 0 && j == n_cidades - 1) continue;

            int A = rota[i];
            int B = rota[(i + 1) % n_cidades];
            int C = rota[j];
            int D = rota[(j + 1) % n_cidades];

            int d_antes = dist[A][B] + dist[C][D];
            int d_depois = dist[A][C] + dist[B][D];

            if(d_depois < d_antes) {
                int inicio = i + 1;
                int fim = j;
                while(inicio < fim) {
                    int temp = rota[inicio];
                    rota[inicio] = rota[fim];
                    rota[fim] = temp;
                    inicio++; fim--;
                }
                return 1; 
            }
        }
    }
    return 0; 
}

// --------- VIZINHANÇA 2: Swap (Troca 2 cidades de lugar) ------------------------------------

int otimizarSwap(int *rota, int n_cidades, int **dist) {
    for (int i = 1; i < n_cidades - 1; i++) {
        for (int j = i + 1; j < n_cidades; j++) {
            
            // Otimização de tempo: calculamos apenas o Delta -> Aqui foi uma sugestão da I.A (Gemini) pra otimização de tempo porque valores maiores demoravam muito
            int ant_i = rota[i - 1];
            int suc_i = rota[i + 1];
            int ant_j = rota[j - 1];
            int suc_j = rota[(j + 1) % n_cidades];
            
            int cidade_i = rota[i];
            int cidade_j = rota[j];

            int d_antes, d_depois;

            if (j == i + 1) {
                d_antes = dist[ant_i][cidade_i] + dist[cidade_j][suc_j];
                d_depois = dist[ant_i][cidade_j] + dist[cidade_i][suc_j];
            } else {
                d_antes = dist[ant_i][cidade_i] + dist[cidade_i][suc_i] + 
                          dist[ant_j][cidade_j] + dist[cidade_j][suc_j];
                d_depois = dist[ant_i][cidade_j] + dist[cidade_j][suc_i] + 
                           dist[ant_j][cidade_i] + dist[cidade_i][suc_j];
            }

            if (d_depois < d_antes) {
                int temp = rota[i];
                rota[i] = rota[j];
                rota[j] = temp;
                
                return 1; 
            } else {
            }
        }
    }
    return 0; 
}

int main() {
    char linha[256];
    int n_cidades = 0;
    Cidade *cidades = NULL;
    char nomeInstancia[256] = "solucao";
    
    // ---------- 1. LEITURA DA INSTÂNCIA -------------------------------------------------

    while (fgets(linha, sizeof(linha), stdin)) {
        linha[strcspn(linha, "\r\n")] = 0; 

        if(strncmp(linha, "NAME", 4) == 0) {
            char *ptr = strchr(linha, ':');
            if(ptr != NULL) {
                ptr++; 
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                strcpy(nomeInstancia, ptr);
            }
        }
        else if(strncmp(linha, "DIMENSION", 9) == 0) {
            char *pontos = strchr(linha, ':');
            if(pontos != NULL) {
                sscanf(pontos + 1, "%d", &n_cidades);
                cidades = (Cidade *)malloc(n_cidades * sizeof(Cidade));
            }
        }
        else if (strncmp(linha, "NODE_COORD_SECTION", 18) == 0) break;
    }

    if (n_cidades == 0 || cidades == NULL) return 1;

    for(int i = 0; i < n_cidades; i++) {
        scanf("%d %lf %lf", &cidades[i].id, &cidades[i].x, &cidades[i].y);
    }

    // ------------------ 2. MATRIZ DE DISTÂNCIAS (Evita calcular sqrt toda a hora) --------------------------------- 
    // Pra não ficar calculando raízes quadradas várias vezes no 2-opt e no Swap, a gente calcula tudo uma vez só e guarda nessa matriz. -> Sugestão da I.A pra otmização de tempo

    int **dist = (int **)malloc(n_cidades * sizeof(int *));
    for(int i = 0; i < n_cidades; i++) {
        dist[i] = (int *)calloc(n_cidades, sizeof(int));
    }

    for(int i = 0; i < n_cidades; i++) {
        for(int j = i + 1; j < n_cidades; j++) {
            int d = calDistancia(cidades[i], cidades[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    }

    long long melhorPesoGlobal = -1; 
    int *melhorRotaGlobal = (int *)malloc(n_cidades * sizeof(int));
    
    int *visitados = (int *)malloc(n_cidades * sizeof(int));
    int *rota = (int *)malloc(n_cidades * sizeof(int));

    int qtdcandidatas = quantidadeCandidatas(n_cidades);
    RotaCandidata *candidatas = (RotaCandidata *)malloc(qtdcandidatas * sizeof(RotaCandidata));
    for (int i = 0; i < qtdcandidatas; i++) {
        candidatas[i].peso = -1;
        candidatas[i].rota = (int *)malloc(n_cidades * sizeof(int));
    }

    // ------------- 3. META-HEURÍSTICA: MULTI-START --------------------------------------------------
    
    for(int cidadeInicial = 0; cidadeInicial < n_cidades; cidadeInicial++) {
        
        memset(visitados, 0, n_cidades * sizeof(int));
        int cidadeAtual = cidadeInicial;
        visitados[cidadeAtual] = 1;
        rota[0] = cidadeAtual;

        // Passo A: Constrói a rota base usando a heurística (Vizinho Mais Próximo)
        for (int passo = 1; passo < n_cidades; passo++) {
            int proximaCidade = -1;
            int menorDistancia = -1; 
            for(int i = 0; i < n_cidades; i++) {
                if(!visitados[i]) {
                    if(menorDistancia == -1 || dist[cidadeAtual][i] < menorDistancia) {
                        menorDistancia = dist[cidadeAtual][i];
                        proximaCidade = i;
                    }
                }
            }
            cidadeAtual = proximaCidade;
            visitados[cidadeAtual] = 1;
            rota[passo] = cidadeAtual;
        }

        long long pesoInicial = calcularPesoTotal(rota, n_cidades, dist);
        guardarCandidata(candidatas, qtdcandidatas, rota, n_cidades, pesoInicial);
    }

    for (int s = 0; s < qtdcandidatas && candidatas[s].peso != -1; s++) {
        memcpy(rota, candidatas[s].rota, n_cidades * sizeof(int));

        // Passo B: Descida em Vizinhança Variável (VND)
        int k = 1;
        while (k <= 2) {
            if (k == 1) {
                if (otimizar2Opt(rota, n_cidades, dist)) k = 1; else k++;
            } else if (k == 2) {
                if (otimizarSwap(rota, n_cidades, dist)) k = 1; else k++;
            }
        }

        // Passo C: Grava se for o melhor tour encontrado até agora
        long long pesoDestaTentativa = calcularPesoTotal(rota, n_cidades, dist);
        if (melhorPesoGlobal == -1 || pesoDestaTentativa < melhorPesoGlobal) {
            melhorPesoGlobal = pesoDestaTentativa;
            for(int c = 0; c < n_cidades; c++) melhorRotaGlobal[c] = rota[c];
        }
    }

    // --------- 4. IMPRESSÃO DA MELHOR SOLUÇÃO (TSPLIB) --------------
    printf("NAME: %s\n", nomeInstancia); 
    printf("COMMENT: Integrantes: %s; Metodo: VND com multi-start filtrado, Vizinho Mais Proximo, 2-Opt e Swap\n", INTEGRANTES);
    printf("TYPE: TOUR\n");
    printf("DIMENSION: %d\n", n_cidades);
    printf("TOTAL_WEIGHT: %lld\n", melhorPesoGlobal);
    printf("TOUR_SECTION\n");

    for (int i = 0; i < n_cidades; i++) {
        printf("%d\n", cidades[melhorRotaGlobal[i]].id); 
    }
    printf("EOF\n");
    
    for(int i = 0; i < qtdcandidatas; i++) free(candidatas[i].rota);
    free(candidatas);
    for(int i = 0; i < n_cidades; i++) free(dist[i]);
    free(dist); free(visitados); free(rota); free(melhorRotaGlobal); free(cidades);
    
    return 0;
}
