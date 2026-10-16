# Apêndice A — Reprodutibilidade Experimental: Comparação de Algoritmos de Ordenação (SBC)

Este repositório constitui o pacote de reprodutibilidade científica para avaliação experimental de algoritmos de ordenação operando sobre vetores de inteiros de 32 bits (`int32_t`).

---

## 1. Arquitetura de Instrumentação Dupla

Para impedir que contadores de software desativem otimizações do compilador (como auto-vetorização, alocação em registradores e *branchless moves*) ou distorçam os contadores de hardware da CPU, o orquestrador (`src/main.cpp`) executa **duas passagens independentes** sobre a mesma *seed* determinística:

1. **Passagem 1 — Desempenho Nativo e Hardware (`std::vector<int32_t>`):**
   - Medição de tempo via `std::chrono::steady_clock` e cálculo de *Throughput* ($\text{elementos/s}$).
   - Coleta direta dos registradores PMU do processador via syscall Linux `perf_event_open`: *CPU Cycles*, *Instructions Retired*, *IPC*, *Branch Mispredictions* e *Cache Misses* (com *fallback* automático caso executado em Windows/MINGW64, WSL ou sem permissão de kernel).
2. **Passagem 2 — Instrumentação Algorítmica (`std::vector<TrackedInt32>`):**
   - Contabilização exata de comparações, trocas (*swaps*), escritas em memória (*writes*), pico de memória auxiliar alocada (em bytes) e profundidade máxima da árvore de recursão.

---

## 2. Sementes (Seeds), Distribuições e Baterias

- **Seed Base Fixa:** `BASE_SEED = 20260818U` (definida em `include/generators.hpp`). Para cada repetição $r \in \{0, 1, \dots, R-1\}$, a semente utilizada pelo `std::mt19937` é `BASE_SEED + r * 10007U`.
- **Distribuições Avaliadas:**
  1. `random_uniform`: Inteiros de 32 bits uniformemente distribuídos.
  2. `sorted`: Estritamente crescente.
  3. `reverse_sorted`: Estritamente decrescente.
  4. `nearly_sorted`: Vetor ordenado submetido a trocas aleatórias em exatamente 1% dos elementos ($N/200$ pares trocados).
  5. `sorted_runs`: Subsequências pré-ordenadas de comprimentos variados ($\propto \sqrt{N}$) para avaliar adaptatividade.
  6. `organ_pipe`: Crescente de $0$ até $N/2$ e decrescente de $N/2$ até $N-1$.
- **Baterias Configuradas:**
  - **Bateria 1 (`Bateria_1_Quadratica`):** `BubbleSort` vs `InsertionSort` para $N \in \{100, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000\}$.
  - **Bateria 2 (`Bateria_2_Eficiente`):** `MergeSort` vs `PDQsort` vs `PowerSort` para $N$ de $10^1$ até $10^7$.
  - **Bateria 3 (`Bateria_3_Geral`):** Todos os 5 algoritmos para $N \in \{100, \dots, 100000\}$.

---

## 3. Habilitando Contadores de Hardware (`perf_event_open`) no Linux

Em ambientes Linux nativos, para permitir que processos de usuário leiam contadores de ciclo, *branches* e *cache* sem necessidade de `sudo`, ajuste temporariamente o nível de paranoia do kernel:

```bash
sudo sysctl -w kernel.perf_event_paranoid=1
```

## 4. Como Compilar e Reproduzir Todos os Resultados e Gráficos

### Execução Completa Automatizada (Artigo)

```bash
chmod +x run_all.sh
./run_all.sh
```

### Execução de Validação Rápida (`--quick-test`)

```bash
./run_all.sh --quick-test
```
