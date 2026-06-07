# Trabalho Pratico - Problema do Caixeiro Viajante

<<<<<<< HEAD
Implementacao simples em C para instancias simetricas do TSP no formato TSPLIB, usando coordenadas `EUC_2D`.
=======
**Disciplina:** Algoritmos em Grafos                                             
**Instituição:** Universidade Federal do Ceará – Campus Crateús  
**Formato de instâncias:** TSPLIB (EUC_2D, simétrico)
>>>>>>> d771d1ad89fbf14478675fb5ec49d1e2e5488536

## Arquivos principais

- `tsp.c`: codigo-fonte final em C.
- `medidor_tempos.py`: script para compilar, executar todas as instancias e gerar CSV de tempos.
- `Instancias/`: instancias `.tsp` usadas nos testes.
- `resultados/`: tours gerados e tabelas CSV.
- `relatorio_pcv.pdf`: relatorio tecnico.

## Compilacao

No Windows, com GCC:

```powershell
gcc -O3 tsp.c -o tsp.exe -lm
```

No Linux:

```bash
gcc -O3 tsp.c -o tsp -lm
```

## Execucao

O programa le a instancia pela entrada padrao e escreve o tour na saida padrao.

Windows:

```powershell
Get-Content Instancias\tsp100.tsp | .\tsp.exe > resultados\tsp100.tour
```

Linux:

```bash
./tsp < Instancias/tsp100.tsp > resultados/tsp100.tour
```

## Medicao dos tempos

Execute:

```powershell
python medidor_tempos.py
```

Caso `python` nao esteja no PATH, use o Python instalado na maquina ou informe o caminho completo do executavel.

O script:

1. compila `tsp.c` com `gcc -O3`;
2. executa todas as instancias `.tsp` da pasta `Instancias`;
3. grava os tours em `resultados/`;
4. gera `resultados/relatorio_tempos.csv`.

## Metodo usado

O algoritmo usa:

1. matriz de distancias `EUC_2D`, calculada uma vez;
2. construcao inicial pelo Vizinho Mais Proximo a partir de varias cidades iniciais;
3. filtro das melhores sementes iniciais para evitar aplicar busca local em todas as partidas quando a instancia cresce;
4. VND com duas vizinhancas: `2-Opt` e `Swap`;
5. escolha do melhor tour encontrado.

Essa filtragem reduziu o tempo sem alterar os pesos obtidos nas instancias testadas. Na instancia `tsp436`, por exemplo, o tempo registrado caiu de `23.2357s` para `1.9611s`, mantendo peso `1490`.

## Resultados atuais

| Instancia | Peso total | Tempo (s) |
|---|---:|---:|
| tsp100 | 21046 | 0.1428 |
| tsp150 | 6636 | 0.0898 |
| tsp29 | 27603 | 0.0096 |
| tsp38 | 6656 | 0.0129 |
| tsp436 | 1490 | 1.7340 |
| tsp51 | 428 | 0.0231 |

## Possiveis melhorias

- Para tentar diminuir o peso: aplicar uma perturbacao simples, como double-bridge, e rodar a busca local novamente. Isso transforma o metodo em uma busca local iterada, mas aumenta um pouco o codigo e o numero de parametros.

## Referencias

- Mladenovic, N.; Hansen, P. (1997). *Variable Neighborhood Search*. Computers & Operations Research. DOI: https://doi.org/10.1016/S0305-0548(97)00031-2
- Feo, T. A.; Resende, M. G. C. (1995). *Greedy Randomized Adaptive Search Procedures*. Journal of Global Optimization. DOI: https://doi.org/10.1007/BF01096763
- Helsgaun, K. (2000). *An Effective Implementation of the Lin-Kernighan Traveling Salesman Heuristic*. European Journal of Operational Research. DOI: https://doi.org/10.1016/S0377-2217(99)00284-2
- Reinelt, G. (1991). *TSPLIB - A Traveling Salesman Problem Library*. ORSA Journal on Computing. DOI: https://doi.org/10.1287/ijoc.3.4.376

## Uso de IA

Foram usadas ferramentas de IA (Gemini PRO e ChatGPT/Codex) como apoio para sugerir otimizacoes simples de tempo e na sugestão pra criação do `medidor_tempo.py`. 
