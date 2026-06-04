#!/bin/bash
# Script para gerar instancias aleatorias e medir tempos de execucao
# Uso: ./benchmark.sh

SOLVER=./tsp_solver
GENERATOR=gerar_instancias.py

# Compila se necessario
if [ ! -f "$SOLVER" ]; then
    echo "Compilando solver..."
    make
fi

echo "=========================================="
echo " Benchmark - TSP Solver"
echo "=========================================="
echo ""

# Testa instancia tulio5 (verificacao basica)
if [ -f "tulio5.tsp" ]; then
    echo "--- Verificacao: tulio5 (5 cidades, otimo = 103) ---"
    $SOLVER < tulio5.tsp > tulio5.tsp.tour 2>/dev/null
    COST=$(grep "TOTAL_WEIGHT" tulio5.tsp.tour | awk '{print $2}')
    echo "  Custo obtido: $COST"
    if [ "$COST" = "103" ]; then
        echo "  Status: OK (otimo encontrado)"
    else
        echo "  Status: ATENCAO (otimo nao encontrado)"
    fi
    echo ""
fi

# Gera e testa instancias de diferentes tamanhos
for N in 50 100 200 500; do
    INST="random_${N}.tsp"
    TOUR="${INST}.tour"

    echo "--- Instancia aleatoria: $N cidades ---"

    # Gera instancia com semente fixa para reproducibilidade
    python3 "$GENERATOR" $N 10000 42 > "$INST"

    # Executa e mede tempo (usa time do bash)
    START_TIME=$(date +%s%3N 2>/dev/null || python3 -c "import time; print(int(time.time()*1000))")
    $SOLVER < "$INST" > "$TOUR" 2>/dev/null
    END_TIME=$(date +%s%3N 2>/dev/null || python3 -c "import time; print(int(time.time()*1000))")

    ELAPSED=$((END_TIME - START_TIME))
    COST=$(grep "TOTAL_WEIGHT" "$TOUR" | awk '{print $2}')

    echo "  Custo: $COST"
    echo "  Tempo: ${ELAPSED} ms"
    echo ""
done

echo "=========================================="
echo " Benchmark concluido"
echo "=========================================="
