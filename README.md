# Trabalho Prático – Problema do Caixeiro Viajante (TSP)

Implementação em linguagem **C** para resolução de instâncias simétricas do **Problema do Caixeiro Viajante (Travelling Salesman Problem - TSP)** no formato **TSPLIB**, utilizando coordenadas **EUC_2D**.

**Disciplina:** Algoritmos em Grafos
**Instituição:** Universidade Federal do Ceará (UFC) – Campus Crateús

---

## Formato das Instâncias

O projeto trabalha com instâncias da biblioteca TSPLIB que atendem às seguintes características:

* Tipo: TSP Simétrico
* Formato de distância: EUC_2D
* Entrada: Arquivos `.tsp`
* Saída: Arquivos `.tour`

---

## Estrutura do Projeto

```text
.
├── tsp.c
├── medidor_tempos.py
├── Instancias/
│   ├── tsp29.tsp
│   ├── tsp38.tsp
│   ├── ...
│
├── resultados/
│   ├── *.tour
│   └── relatorio_tempos.csv
│
└── relatorio_pcv.pdf
```

### Arquivos Principais

| Arquivo             | Descrição                                                 |
| ------------------- | --------------------------------------------------------- |
| `tsp.c`             | Implementação principal do algoritmo em C                 |
| `medidor_tempos.py` | Script para automação dos testes e geração dos relatórios |
| `Instancias/`       | Conjunto de instâncias TSPLIB utilizadas nos experimentos |
| `resultados/`       | Diretório para armazenamento dos tours e tabelas CSV      |
| `relatorio_pcv.pdf` | Relatório técnico do projeto                              |

---

# Compilação

## Windows (GCC)

```bash
gcc -O3 tsp.c -o tsp.exe -lm
```

## Linux

```bash
gcc -O3 tsp.c -o tsp -lm
```

---

# Execução

O programa recebe uma instância pela entrada padrão (`stdin`) e imprime o tour encontrado na saída padrão (`stdout`).

## Windows (PowerShell)

```powershell
Get-Content Instancias\tsp100.tsp | .\tsp.exe > resultados\tsp100.tour
```

## Linux

```bash
./tsp < Instancias/tsp100.tsp > resultados/tsp100.tour
```

---

# Medição de Tempos

Para executar todas as instâncias automaticamente:

```bash
python medidor_tempos.py
```

> Caso o comando `python` não esteja disponível no sistema, utilize `python3` ou informe o caminho completo para o interpretador.

---

## Funcionalidades do Script

O script `medidor_tempos.py` realiza automaticamente:

1. Compilação do arquivo `tsp.c` utilizando a flag `-O3`;
2. Execução de todas as instâncias `.tsp` presentes na pasta `Instancias`;
3. Geração dos arquivos `.tour` na pasta `resultados`;
4. Criação do relatório consolidado `resultados/relatorio_tempos.csv`.

---

# Metodologia Utilizada

O algoritmo foi desenvolvido utilizando uma combinação de heurísticas e busca local.

## 1. Matriz de Distâncias

As distâncias EUC_2D entre todas as cidades são calculadas apenas uma vez e armazenadas em memória.

**Vantagem:** evita milhares de chamadas repetidas à função de raiz quadrada durante a execução.

---

## 2. Construção Inicial – Vizinho Mais Próximo (Multi-Start)

Uma rota inicial é construída utilizando a heurística do Vizinho Mais Próximo.

O procedimento é repetido partindo de diferentes cidades para gerar múltiplas soluções iniciais.

---

## 3. Filtro de Candidatas

Após gerar as soluções iniciais, apenas as **60 melhores rotas candidatas** são mantidas para a etapa de refinamento.

**Objetivo:**

* reduzir significativamente o tempo de execução;
* evitar busca local em soluções claramente inferiores;
* aumentar a escalabilidade para instâncias maiores.

---

## 4. Busca Local (VND)

A melhoria das soluções é realizada por meio de **Variable Neighborhood Descent (VND)**, utilizando:

### 2-Opt

Remove cruzamentos de arestas e reduz o comprimento da rota através da inversão de segmentos.

### Swap

Troca a posição de duas cidades da rota em busca de melhorias adicionais.

O algoritmo alterna entre essas estruturas até atingir um ótimo local.

---

## 5. Seleção Global

Ao final da execução, é selecionada a melhor solução encontrada entre todas as rotas avaliadas.

---

# Ganho Obtido com o Filtro de Sementes

A introdução da filtragem reduziu drasticamente o tempo de execução sem alterar a qualidade das soluções obtidas nos testes realizados.

### Exemplo

| Instância | Peso | Tempo Antes | Tempo Depois |
| --------- | ---- | ----------- | ------------ |
| tsp436    | 1490 | 23.2357 s   | 1.9611 s     |

**Redução aproximada de 91% no tempo de execução**, mantendo exatamente o mesmo peso da solução.

---

# Resultados Obtidos

| Instância | Peso Total | Tempo (s) |
| --------- | ---------- | --------- |
| tsp29     | 27603      | 0.0096    |
| tsp38     | 6656       | 0.0129    |
| tsp51     | 428        | 0.0231    |
| tsp100    | 21046      | 0.1428    |
| tsp150    | 6636       | 0.0898    |
| tsp436    | 1490       | 1.9611    |

---

# Possíveis Melhorias Futuras

## Redução de Tempo

Implementação de **Candidate Lists** na vizinhança 2-Opt.

A ideia consiste em restringir a busca apenas às cidades mais próximas, reduzindo o custo para aproximadamente:

[
O(N \cdot K)
]

onde:

* (N) é o número de cidades;
* (K) é o número de candidatos considerados para cada cidade.

---

## Redução de Peso

Implementação de uma perturbação do tipo **Double-Bridge**, transformando o método em uma **Iterated Local Search (ILS)**.

Essa abordagem permite escapar com maior facilidade de ótimos locais e explorar novas regiões do espaço de busca.

---

# Referências Bibliográficas

* Alvesa, D. J. F., Avelino, B., de Faria, V. C. N., Macedo, E. A., Martini, V. G., Vieira, B. S., & Chaves, A. A. *Estudo comparativo de metaheurísticas aplicadas ao problema do caixeiro viajante múltiplo.*

* Bentley, J. J. (1992). *Fast Algorithms for Geometric Traveling Salesman Problems*. ORSA Journal on Computing, 4(4), 387–411.

* da Cunha, C. B., de Oliveira Bonasser, U., & Abrahão, F. T. M. (2002). *Experimentos Computacionais com Heurísticas de Melhorias para o Problema do Caixeiro Viajante*. XVI Congresso da ANPET.

* Diadelmo, M. V., Batista, L. S., & Bessani, M. (2024). *Uma Análise Estatística de Estruturas de Vizinhança para o Problema do Caixeiro Viajante*. Congresso Brasileiro de Automática (CBA).

* Goldbarg, E., Goldbarg, M., & Luna, H. (2017). *Otimização Combinatória e Metaheurísticas: Algoritmos e Aplicações*. Elsevier.

* Reinelt, G. (1991). *TSPLIB – A Traveling Salesman Problem Library*. ORSA Journal on Computing, 3(4), 376–384.

---

# Uso de Inteligência Artificial

Foram utilizadas ferramentas de Inteligência Artificial como apoio pedagógico e auxílio no desenvolvimento do projeto, incluindo:

* Google Gemini;
* ChatGPT/Codex.

As ferramentas foram utilizadas principalmente para:

* sugestão de otimizações de desempenho (matriz de distâncias e cálculo incremental de custos);
* apoio na estruturação lógica do script de automação `medidor_tempos.py`;

