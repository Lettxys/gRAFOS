import os
import subprocess
import time
import re
import csv
import sys

# CONFIGURAÇÕES DO SCRIPT
FICHEIRO_C = "tsp.c"
EXECUTAVEL = "tsp.exe" if os.name == 'nt' else "./tsp"
PASTA_INSTANCIAS = "Instancias"
PASTA_RESULTADOS = "resultados"
FICHEIRO_RELATORIO = "relatorio_tempos.csv"

def compilarCodigo():
    print(f"A compilar o ficheiro {FICHEIRO_C}...")
    # flag -O3 pro compilador otimizar o código ao máximo em velocidade     ->  Sugestão da I.A
    comando = ["gcc", "-O3", FICHEIRO_C, "-o", EXECUTAVEL, "-lm"]
    
    try:
        resultado = subprocess.run(comando, capture_output=True, text=True)
        if resultado.returncode != 0:
            print("Erro de compilação:")
            print(resultado.stderr)
            sys.exit(1)
        print("Compilação concluída com sucesso!\n")
    except FileNotFoundError:
        print("Erro: O compilador 'gcc' não foi encontrado no sistema.")
        sys.exit(1)

def criaPasta():
    # Cria as pastas se não existirem
    os.makedirs(PASTA_INSTANCIAS, exist_ok=True)
    os.makedirs(PASTA_RESULTADOS, exist_ok=True)

def extPeso(saida_texto):
    # Procura a linha "TOTAL_WEIGHT: ---" na saída gerada
    match = re.search(r"TOTAL_WEIGHT:\s*(\d+)", saida_texto)
    if match:
        return int(match.group(1))
    return "Erro"

def exTestes():
    # Procura todos os ficheiros terminados em .tsp na pasta instancias
    ficheiros_tsp = sorted(f for f in os.listdir(PASTA_INSTANCIAS) if f.endswith(".tsp"))
    
    if not ficheiros_tsp:
        print(f"Nenhum ficheiro .tsp encontrado na pasta '{PASTA_INSTANCIAS}'.")
        sys.exit(0)

    print(f"Encontradas {len(ficheiros_tsp)} instâncias. A iniciar testes...\n")
    
    resultados_finais = []

    for ficheiro in ficheiros_tsp:
        caminhoEntrada = os.path.join(PASTA_INSTANCIAS, ficheiro)
        nomeInstancia = ficheiro.replace(".tsp", "")
        caminhoSaida = os.path.join(PASTA_RESULTADOS, f"{nomeInstancia}.tour")
        
        print(f"A processar: {ficheiro}...", end="", flush=True)

        # Abre o ficheiro .tsp para redirecionar para o stdin do executável C
        with open(caminhoEntrada, 'r', encoding='utf-8') as f_in:
            # Marca o tempo inicial
            tempoInicio = time.perf_counter()
            
            # Executa o programa 
            proc = subprocess.run([EXECUTAVEL], stdin=f_in, capture_output=True, text=True)
            
            # Marca o tempo final
            tempoFim = time.perf_counter()

        if proc.returncode != 0:
            print("\nErro ao executar o programa:")
            print(proc.stderr)
            sys.exit(proc.returncode)

        tempoDecorrido = tempoFim - tempoInicio
        peso = extPeso(proc.stdout)

        # Guarda a resposta do programa no ficheiro .tour 
        with open(caminhoSaida, 'w', encoding='utf-8') as f_out:
            f_out.write(proc.stdout)

        print(f" Concluído! Peso: {peso} | Tempo: {tempoDecorrido:.4f}s")
        
        resultados_finais.append({
            "Instancia": nomeInstancia,
            "Peso Total": peso,
            "Tempo (segundos)": round(tempoDecorrido, 4)
        })

    return resultados_finais

def relatorioCSV(resultados):
    #ficheiro CSV pra tabela do Relatório 
    caminho_csv = os.path.join(PASTA_RESULTADOS, FICHEIRO_RELATORIO)
    
    with open(caminho_csv, mode='w', newline='', encoding='utf-8') as csvfile:
        campos = ["Instancia", "Peso Total", "Tempo (segundos)"]
        escritor = csv.DictWriter(csvfile, fieldnames=campos)
        
        escritor.writeheader()
        for linha in resultados:
            escritor.writerow(linha)
            
    print(f"\nTabela de resultados guardada em: {caminho_csv}")

if __name__ == "__main__":
    criaPasta()
    compilarCodigo()
    resultados = exTestes()
    if resultados:
        relatorioCSV(resultados)
        print("\nTodos os processos foram finalizados com sucesso!")
