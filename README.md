# Solver para o Problema do Caixeiro Viajante (TSP)

**Disciplina:** Algoritmos em Grafos 
**Instituição:** Universidade Federal do Ceará – Campus Crateús  
**Formato de instâncias:** TSPLIB (EUC_2D, simétrico)

---

## Índice

1. [Visão Geral](#visão-geral)
2. [Estrutura de Arquivos](#estrutura-de-arquivos)
3. [Algoritmo Implementado](#algoritmo-implementado)
4. [Como Compilar](#como-compilar)
5. [Como Executar](#como-executar)
6. [Parâmetros Opcionais](#parâmetros-opcionais)
7. [Formato de Entrada](#formato-de-entrada)
8. [Formato de Saída](#formato-de-saída)
9. [Gerando Instâncias de Teste](#gerando-instâncias-de-teste)
10. [Executando o Benchmark](#executando-o-benchmark)
11. [Resultados Obtidos](#resultados-obtidos)
12. [Detalhes Técnicos](#detalhes-técnicos)

---

## Visão Geral

Este projeto implementa um solver para o **Problema do Caixeiro Viajante (TSP)**
em linguagem C. O objetivo é encontrar um ciclo hamiltoniano de custo mínimo em
um grafo ponderado, ou seja, uma rota que visite todas as cidades exatamente uma
vez e retorne à cidade de origem com o menor custo total possível.

O método implementado combina quatro técnicas:

| Etapa | Técnica | Tipo |
|-------|---------|------|
| 1 | Nearest Neighbor (multi-início) | Heurística construtiva |
| 2 | 2-opt | Busca local |
| 3 | Or-opt (segmentos 1, 2, 3) | Busca local |
| 4 | Iterated Local Search com Double Bridge | Metaheurística |

A solução final **vai muito além do Vizinho Mais Próximo isolado**, usando-o
apenas como ponto de partida para as fases de melhoria.

---

## Estrutura de Arquivos

```
.
├── tsp_solver.c          # Código-fonte principal do solver
├── Makefile              # Script de compilação
├── tulio5.tsp            # Instância de teste do enunciado (5 cidades, ótimo = 103)
├── gerar_instancias.py   # Gerador de instâncias aleatórias no formato TSPLIB
├── benchmark.sh          # Script para medir tempos e resultados
└── README.md             # Este arquivo
```

---

## Algoritmo Implementado

O solver executa três fases em sequência:

### Fase 1 — Nearest Neighbor (multi-início)

A heurística do Vizinho Mais Próximo constrói um tour partindo de uma cidade
inicial, sempre escolhendo a cidade não visitada mais próxima até completar o
ciclo.

Para obter uma boa solução inicial, o algoritmo executa essa heurística
**partindo de todos os vértices** como origem (ou até 2.000 para instâncias
muito grandes) e mantém o melhor tour encontrado.

- **Complexidade:** O(n²) por início, O(n³) no total
- **Papel:** Gera uma solução inicial de qualidade para as fases seguintes

---

### Fase 2 — Busca Local: 2-opt + Or-opt

O melhor tour da Fase 1 é refinado alternando dois operadores de melhoria local
até que nenhum dos dois consiga mais reduzir o custo.

#### 2-opt

Remove dois pares de arestas e os reconecta de forma diferente, invertendo o
segmento do tour entre eles. Se o novo custo for menor, a inversão é aceita.

```
Antes:  ... → A → B → ... → C → D → ...
Depois: ... → A → C → ... → B → D → ...
        (segmento B..C invertido)
```

- **Delta (variação de custo):** `dist(A,C) + dist(B,D) − dist(A,B) − dist(C,D)`
- **Aceita se:** delta < 0
- **Complexidade:** O(n²) por passagem

#### Or-opt (relocação de segmentos)

Remove um segmento de **k cidades consecutivas** (k = 1, 2 ou 3) do tour e o
reinsere na melhor posição encontrada, testando também a orientação invertida do
segmento.

```
Antes:  ... → P → [seg₁ → seg₂ → seg₃] → S → ... → J → K → ...
Depois: ... → P → S → ... → J → [seg₁ → seg₂ → seg₃] → K → ...
```

- Prioriza segmentos maiores (k=3 primeiro), pois tendem a gerar maior melhoria
- Para k ≥ 2, testa também a ordem reversa do segmento
- **Complexidade:** O(n²) por passagem

A **busca local combinada** alterna 2-opt e Or-opt em loop até que nenhuma
melhoria seja encontrada por nenhum dos dois:

```
repita:
    custo_anterior = custo_atual
    custo_atual = 2-opt(tour)
    custo_atual = Or-opt(tour)
enquanto custo_atual < custo_anterior
```

---

### Fase 3 — Iterated Local Search (ILS) com Double Bridge

Após atingir um ótimo local na Fase 2, o algoritmo usa ILS para escapar dele e
explorar outras regiões do espaço de soluções.

A cada iteração:

1. **Perturbação Double Bridge:** divide o tour em 4 segmentos A, B, C, D com
   3 cortes aleatórios e os reconecta como **A + C + B + D**. Esse movimento
   é uma 4-opt não-sequencial que não pode ser desfeita por 2-opt, garantindo
   uma perturbação efetiva.

   ```
   Original:  A → B → C → D
   Perturb.:  A → C → B → D
   ```

2. **Busca local** (2-opt + Or-opt) aplicada sobre o tour perturbado.

3. **Critério de aceitação:** o novo tour só substitui o melhor se for
   estritamente melhor (estratégia gulosa).

O processo repete até o tempo limite ser atingido.

---

## Como Compilar

### Linux / macOS / Git Bash (Windows)

```bash
make
```

Ou manualmente:

```bash
gcc -O2 -Wall -Wextra -std=c99 -o tsp_solver tsp_solver.c -lm
```

### Windows (Prompt de Comando com MinGW)

```cmd
gcc -O2 -Wall -std=c99 -o tsp_solver.exe tsp_solver.c -lm
```

> **Flags utilizadas:**
> - `-O2` — otimização de código (execução mais rápida)
> - `-Wall -Wextra` — todos os avisos do compilador ativados
> - `-std=c99` — padrão C99
> - `-lm` — linka a biblioteca matemática (necessária para `sqrt` e `floor`)

---

## Como Executar

O programa lê a instância pela **entrada padrão** e escreve a solução na
**saída padrão**, conforme especificado no enunciado:

```bash
./tsp_solver < instancia.tsp > instancia.tsp.tour
```

### Exemplo com a instância tulio5

```bash
./tsp_solver < tulio5.tsp > tulio5.tsp.tour
```

**Saída esperada em `tulio5.tsp.tour`:**

```
NAME: tulio5
COMMENT: Metodo NN + 2-opt + Or-opt + ILS
TYPE: TOUR
DIMENSION: 5
TOTAL_WEIGHT: 103
TOUR_SECTION
1
2
3
4
5
EOF
```

> **Nota:** Mensagens de diagnóstico (tempo, custo por fase, etc.) são enviadas
> para a **saída de erros** (`stderr`) e **não aparecem** no arquivo `.tour`.
> Para visualizá-las no terminal, redirecione apenas o stdout para o arquivo:
>
> ```bash
> ./tsp_solver < instancia.tsp > instancia.tsp.tour
> # As mensagens de progresso aparecerão normalmente no terminal
> ```

---

## Parâmetros Opcionais

O programa aceita até dois argumentos opcionais na linha de comando:

```bash
./tsp_solver [SEMENTE] [TEMPO_LIMITE_SEG]
```

| Argumento | Padrão | Descrição |
|-----------|--------|-----------|
| `SEMENTE` | `42` | Semente para o gerador de números aleatórios |
| `TEMPO_LIMITE_SEG` | automático | Tempo máximo de execução em segundos |

**Tempo automático por padrão (quando não informado):**

| Tamanho da instância | Tempo limite |
|----------------------|-------------|
| n ≤ 100 cidades | 5 segundos |
| n ≤ 500 cidades | 10 segundos |
| n ≤ 1000 cidades | 20 segundos |
| n > 1000 cidades | 30 segundos |

### Exemplos de uso com parâmetros

```bash
# Semente 7, tempo padrão (automático)
./tsp_solver 7 < instancia.tsp > solucao.tour

# Semente 42, tempo limite de 30 segundos
./tsp_solver 42 30 < instancia.tsp > solucao.tour

# Sem parâmetros (semente=42, tempo automático)
./tsp_solver < instancia.tsp > solucao.tour
```

---

## Formato de Entrada

O programa aceita instâncias no formato **TSPLIB** com `EDGE_WEIGHT_TYPE: EUC_2D`.

**Campos reconhecidos:**

| Campo | Descrição |
|-------|-----------|
| `NAME` | Nome da instância |
| `DIMENSION` | Número de cidades |
| `EDGE_WEIGHT_TYPE` | Tipo de distância (apenas `EUC_2D` suportado) |
| `NODE_COORD_SECTION` | Início da lista de coordenadas |

**Fórmula de distância EUC_2D:**

```
d(i, j) = floor( 0.5 + sqrt( (xᵢ - xⱼ)² + (yᵢ - yⱼ)² ) )
```

**Exemplo de arquivo de entrada:**

```
NAME: tulio5
COMMENT: instancia pequena de teste
TYPE: TSP
DIMENSION: 5
EDGE_WEIGHT_TYPE: EUC_2D
NODE_COORD_SECTION
1 288 149
2 288 129
3 270 133
4 256 141
5 256 157
EOF
```

---

## Formato de Saída

A saída segue rigorosamente o formato TSPLIB TOUR:

```
NAME: <nome_da_instancia>
COMMENT: Metodo NN + 2-opt + Or-opt + ILS
TYPE: TOUR
DIMENSION: <numero_de_cidades>
TOTAL_WEIGHT: <custo_total>
TOUR_SECTION
<cidade_1>
<cidade_2>
...
<cidade_n>
EOF
```

> Os IDs das cidades na saída são **1-indexados**, como no arquivo de entrada.

---

## Gerando Instâncias de Teste

Use o script Python para gerar instâncias aleatórias:

```bash
# Sintaxe
python gerar_instancias.py [N_CIDADES] [COORD_MAX] [SEMENTE]

# Exemplos
python gerar_instancias.py 50  > inst_50.tsp
python gerar_instancias.py 100 10000 42 > inst_100.tsp
python gerar_instancias.py 200 10000 42 > inst_200.tsp
python gerar_instancias.py 500 10000 42 > inst_500.tsp
```

| Parâmetro | Padrão | Descrição |
|-----------|--------|-----------|
| `N_CIDADES` | `100` | Número de cidades |
| `COORD_MAX` | `10000` | Coordenada máxima (x e y entre 0 e COORD_MAX) |
| `SEMENTE` | aleatória | Semente para reprodutibilidade |

---

## Executando o Benchmark

O script `benchmark.sh` gera instâncias de 50, 100, 200 e 500 cidades,
executa o solver em cada uma e exibe custo e tempo:

```bash
chmod +x benchmark.sh
./benchmark.sh
```

**Exemplo de saída:**

```
==========================================
 Benchmark - TSP Solver
==========================================

--- Verificacao: tulio5 (5 cidades, otimo = 103) ---
  Custo obtido: 103
  Status: OK (otimo encontrado)

--- Instancia aleatoria: 50 cidades ---
  Custo: 58670
  Tempo: 3012 ms

--- Instancia aleatoria: 100 cidades ---
  Custo: 78277
  Tempo: 5023 ms
...
```

---

## Resultados Obtidos

Testes realizados em instâncias aleatórias (semente 42, coordenadas 0–10000):

| Cidades | Custo NN puro | Custo final | Melhoria | Iterações ILS | Tempo total |
|---------|--------------|-------------|----------|---------------|-------------|
| 5       | 103          | **103**     | ótimo    | —             | < 1 s       |
| 50      | 67.634       | **58.670**  | −13,2%   | 16.839        | 3 s         |
| 100     | ~85.000      | **78.277**  | ~−8%     | 6.404         | 5 s         |
| 200     | 124.741      | **105.721** | −15,2%   | 1.979         | 10 s        |
| 500     | 202.704      | **169.400** | −16,4%   | 199           | 10 s        |

> A melhoria cresce com o tamanho da instância porque há mais oportunidades
> de reorganização que o Vizinho Mais Próximo não captura.

---

## Detalhes Técnicos

### Biblioteca utilizada

Apenas a **biblioteca padrão do C**:

| Header | Uso |
|--------|-----|
| `<stdio.h>` | Leitura (`fgets`, `sscanf`) e escrita (`printf`) |
| `<stdlib.h>` | Alocação de memória (`malloc`, `calloc`, `free`) |
| `<string.h>` | `memcpy`, `strcmp`, `strncpy` |
| `<math.h>` | `sqrt`, `floor` |
| `<limits.h>` | `INT_MAX` |
| `<time.h>` | Controle de tempo (`clock`) |

Nenhuma biblioteca externa de grafos, TSP ou otimização foi utilizada.

### Estruturas de dados

| Dado | Tipo | Tamanho |
|------|------|---------|
| Coordenadas | `double[]` | O(n) |
| Matriz de distâncias | `int[]` (flat, n×n) | O(n²) |
| Tour atual / melhor / perturbado | `int[]` | O(n) |

A matriz de distâncias é pré-computada uma única vez e acessada em O(1),
evitando recalcular `sqrt` a cada consulta de distância.

### Complexidade resumida

| Fase | Complexidade |
|------|-------------|
| Pré-computação da matriz | O(n²) |
| Nearest Neighbor (n inícios) | O(n³) |
| 2-opt (até convergência) | O(n²) por passagem |
| Or-opt (até convergência) | O(n²) por passagem |
| ILS (por iteração) | O(n²) |

### Representação do tour

O tour é armazenado como um **vetor de inteiros** (índices 0-based), onde a
posição no vetor indica a ordem de visita. A conectividade é implicitamente
circular: `tour[n-1]` conecta de volta a `tour[0]`.
