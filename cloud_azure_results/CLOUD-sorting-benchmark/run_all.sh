#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

mkdir -p bin data results/plots results/tables

find_cpp_compiler() {
    local candidates=(
        "${CXX:-}"
        "clang++"
        "g++"
        "c++"
        "/c/msys64/ucrt64/bin/clang++.exe"
        "/c/msys64/mingw64/bin/clang++.exe"
        "/c/msys64/ucrt64/bin/g++.exe"
        "/c/msys64/mingw64/bin/g++.exe"
        "F:/MinGW/bin/g++.exe"
        "C:/MinGW/bin/g++.exe"
        "C:/msys64/ucrt64/bin/clang++.exe"
        "C:/msys64/mingw64/bin/clang++.exe"
        "C:/msys64/ucrt64/bin/g++.exe"
        "C:/msys64/mingw64/bin/g++.exe"
    )

    local compiler test_cpp test_exe
    for compiler in "${candidates[@]}"; do
        [ -n "${compiler}" ] || continue
        test_cpp="$(mktemp /tmp/sbc_cpp_check.XXXXXX.cpp 2>/dev/null || printf '%s' /tmp/sbc_cpp_check.cpp)"
        test_exe="${test_cpp%.cpp}"
        cat > "${test_cpp}" <<'CPP'
#include <iostream>
int main() {
    std::cout << "ok\\n";
    return 0;
}
CPP
        if "${compiler}" -std=c++17 "${test_cpp}" -o "${test_exe}" >/dev/null 2>&1; then
            rm -f "${test_cpp}" "${test_exe}"
            echo "${compiler}"
            return 0
        fi
        rm -f "${test_cpp}" "${test_exe}"
    done

    return 1
}

CPP_BIN="$(find_cpp_compiler || true)"
if [ -z "${CPP_BIN}" ]; then
    echo "[ERRO] Nenhum compilador C++ funcional foi encontrado no PATH ou nos diretórios comuns do sistema."
    exit 1
fi

echo "===================================================================="
echo " [1/4] Coletando especificacoes completas do Hardware da Maquina/VM"
echo "===================================================================="
{
    echo "=== DATA DA COLETA ==="
    date -u || true
    echo ""
    echo "=== SISTEMA OPERACIONAL E KERNEL ==="
    uname -a || true
    cat /etc/os-release 2>/dev/null || true
    echo ""
    echo "=== PROCESSADOR E CACHES (lscpu) ==="
    lscpu 2>/dev/null || wmic cpu get name,NumberOfCores,NumberOfLogicalProcessors 2>/dev/null || true
    echo ""
    echo "=== MEMORIA RAM (free -h) ==="
    free -h 2>/dev/null || true
    echo ""
    echo "=== COMPILADOR C++ ==="
    "${CPP_BIN}" --version || true
} > data/machine_specs.txt

echo "===================================================================="
echo " [2/4] Compilando src/main.cpp com ${CPP_BIN} -std=c++17 -O3 -march=native"
echo "===================================================================="
"${CPP_BIN}" -std=c++17 -O3 -march=native -Wall -Wextra src/main.cpp -o bin/benchmark

echo "===================================================================="
echo " [3/4] Executando bateria de benchmarks C++ (Com Media Aparada)"
echo "===================================================================="
if [ -f "./bin/benchmark.exe" ]; then
    ./bin/benchmark.exe "$@"
else
    ./bin/benchmark "$@"
fi

echo "===================================================================="
echo " [4/4] Gerando graficos (300 DPI), tabelas de resultados e de Hardware"
echo "===================================================================="
python3 scripts/plot_graphs.py 2>/dev/null || python scripts/plot_graphs.py

echo ""
echo "===================================================================="
echo " RESUMO DO HARDWARE PARA COLAR NO SEU ARTIGO (Secao 4.1):"
echo "===================================================================="
cat results/tables/hardware_specs_sbc.md || true
echo "===================================================================="
echo "[CONCLUIDO] Pipeline finalizado com sucesso!"
