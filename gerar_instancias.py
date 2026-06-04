#!/usr/bin/env python3
"""
Gerador de instancias aleatorias para o TSP no formato TSPLIB.
Uso: python3 gerar_instancias.py [n_cidades] [coord_max] [semente]
"""
import random
import sys

n = int(sys.argv[1]) if len(sys.argv) > 1 else 100
max_coord = int(sys.argv[2]) if len(sys.argv) > 2 else 10000
seed = int(sys.argv[3]) if len(sys.argv) > 3 else None

if seed is not None:
    random.seed(seed)

print(f"NAME: random{n}")
print(f"COMMENT: Instancia aleatoria com {n} cidades (max_coord={max_coord})")
print("TYPE: TSP")
print(f"DIMENSION: {n}")
print("EDGE_WEIGHT_TYPE: EUC_2D")
print("NODE_COORD_SECTION")
for i in range(1, n + 1):
    x = random.randint(0, max_coord)
    y = random.randint(0, max_coord)
    print(f"{i} {x} {y}")
print("EOF")
