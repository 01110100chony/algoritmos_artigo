#!/usr/bin/env bash
set -euo pipefail

mkdir -p bin data results/plots results/tables

PYTHON_BIN="${PYTHON:-}"
if [ -z "$PYTHON_BIN" ]; then
    if command -v python3 >/dev/null 2>&1; then
        PYTHON_BIN="python3"
    elif command -v python >/dev/null 2>&1; then
        PYTHON_BIN="python"
    else
        echo "[ERRO] Nenhum interpretador Python encontrado." >&2
        exit 1
    fi
fi

CXX_BIN="${CXX:-}"
if [ -z "$CXX_BIN" ]; then
    if command -v clang++ >/dev/null 2>&1; then
        CXX_BIN="clang++"
    elif command -v g++ >/dev/null 2>&1; then
        CXX_BIN="g++"
    elif command -v c++ >/dev/null 2>&1; then
        CXX_BIN="c++"
    else
        echo "[ERRO] Nenhum compilador C++ encontrado (clang++, g++, c++)." >&2
        exit 1
    fi
fi

echo "===================================================================="
echo " [1/4] Coletando especificacoes da maquina em data/machine_specs.txt"
echo "===================================================================="
{
    echo "=== DATA DA COLETA ==="
    date -u || true
    echo ""
    echo "=== UNAME ==="
    uname -a || true
    echo ""
    echo "=== CPU (lscpu / wmic) ==="
    lscpu 2>/dev/null || wmic cpu get name,NumberOfCores,NumberOfLogicalProcessors 2>/dev/null || true
    echo ""
    echo "=== MEMORIA (free -h) ==="
    free -h 2>/dev/null || true
    echo ""
    echo "=== COMPILADOR (CXX) ==="
    "$CXX_BIN" --version || true
    echo ""
    echo "=== PYTHON ==="
    "$PYTHON_BIN" --version || true
} > data/machine_specs.txt

echo "===================================================================="
echo " [2/4] Compilando src/main.cpp com ${CXX_BIN} -std=c++17 -O3 -march=native"
echo "===================================================================="
"$CXX_BIN" -std=c++17 -O3 -march=native -Wall -Wextra src/main.cpp -o bin/benchmark.exe

echo "===================================================================="
echo " [3/4] Executando bateria de benchmarks C++"
echo "===================================================================="
./bin/benchmark.exe "$@"

echo "===================================================================="
echo " [4/4] Gerando graficos (300 DPI) e tabelas LaTeX/Markdown"
echo "===================================================================="
if ! "$PYTHON_BIN" -c "import seaborn" >/dev/null 2>&1; then
    "$PYTHON_BIN" -m pip install seaborn >/dev/null 2>&1 || true
fi
"$PYTHON_BIN" scripts/plot_graphs.py

echo ""
echo "[CONCLUIDO] Pipeline finalizado com sucesso!"
