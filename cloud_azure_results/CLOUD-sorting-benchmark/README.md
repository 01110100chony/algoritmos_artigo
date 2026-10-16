# Apêndice A — Reprodutibilidade Experimental: Comparação de Algoritmos de Ordenação (SBC)

Este repositório constitui o pacote de reprodutibilidade científica para avaliação experimental de algoritmos de ordenação operando sobre vetores de inteiros de 32 bits (`int32_t`).

---

## 1. Tratamento Estatístico: Média Aparada (Sem Melhor e Pior Resultado)

Para eliminar *outliers* causados por interrupções de sistema operacional ou escalonamento de hipervisor em nuvem:
- No modo completo, cada combinação `(Bateria, Algoritmo, Distribuição, N)` é executada por **10 passadas** (ou um número customizado via `--reps K`).
- Os resultados são ordenados de forma crescente e **o melhor resultado (mínimo) e o pior resultado (máximo) são descartados**.
- A média aritmética, mediana e desvio-padrão são calculados estritamente sobre as **8 passadas centrais**.

---

## 2. Documentação Automática do Hardware (Seção 4.1 do Artigo)

Ao final da execução, o pipeline gera automaticamente:
- `data/machine_specs.txt`: Dump bruto do `lscpu`, `/proc/meminfo`, `uname -a` e `g++ --version`.
- `results/tables/hardware_specs_sbc.md` e `results/tables/hardware_specs_sbc.tex`: Parágrafo descritivo e tabela formatada com o modelo exato da CPU, núcleos, caches L1/L2/L3, RAM, Kernel e compilador prontos para colar no artigo.
