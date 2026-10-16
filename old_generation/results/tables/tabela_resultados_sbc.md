# Tabela Consolidada - Comparacao Geral (N = 5000)

| Distribuicao | Algoritmo | Tempo Medio (ms) | Desvio (ms) | Comparacoes | Escritas (Writes) | Memoria Aux (B) | Prof. Recursao |
|---|---|---:|---:|---:|---:|---:|---:|
| `nearly_sorted` | **PDQsort** | 0.0184 | 0.0025 | 37,353 | 1,241 | 0 | 7 |
| `nearly_sorted` | **InsertionSort** | 0.0508 | 0.0052 | 100,530 | 104,907 | 0 | 1 |
| `nearly_sorted` | **PowerSort** | 0.0738 | 0.0034 | 28,846 | 35,392 | 10,004 | 1 |
| `nearly_sorted` | **MergeSort** | 0.1572 | 0.0553 | 31,948 | 61,024 | 20,000 | 14 |
| `nearly_sorted` | **BubbleSort** | 4.1515 | 0.5553 | 11,935,170 | 286,593 | 0 | 1 |
| `organ_pipe` | **PowerSort** | 0.0226 | 0.0042 | 9,999 | 11,245 | 10,004 | 1 |
| `organ_pipe` | **PDQsort** | 0.1280 | 0.0429 | 89,921 | 55,996 | 0 | 11 |
| `organ_pipe` | **MergeSort** | 0.1450 | 0.0005 | 24,750 | 66,808 | 20,000 | 14 |
| `organ_pipe` | **InsertionSort** | 4.5462 | 1.2376 | 6,252,499 | 6,252,498 | 0 | 1 |
| `organ_pipe` | **BubbleSort** | 7.7277 | 2.7862 | 12,497,500 | 18,742,500 | 0 | 1 |
| `random_uniform` | **PowerSort** | 0.2163 | 0.0089 | 72,376 | 94,381 | 10,004 | 1 |
| `random_uniform` | **MergeSort** | 0.2206 | 0.0061 | 58,636 | 116,566 | 20,000 | 14 |
| `random_uniform` | **PDQsort** | 0.2295 | 0.0268 | 69,234 | 57,102 | 0 | 12 |
| `random_uniform` | **InsertionSort** | 4.2425 | 0.8065 | 6,201,573 | 6,206,559 | 0 | 1 |
| `random_uniform` | **BubbleSort** | 14.9763 | 1.8345 | 12,493,314 | 18,589,743 | 0 | 1 |
| `reverse_sorted` | **PDQsort** | 0.0056 | 0.0002 | 15,032 | 7,554 | 0 | 2 |
| `reverse_sorted` | **PowerSort** | 0.0079 | 0.0006 | 4,999 | 7,500 | 10,004 | 1 |
| `reverse_sorted` | **MergeSort** | 0.1361 | 0.0019 | 37,003 | 123,616 | 20,000 | 14 |
| `reverse_sorted` | **InsertionSort** | 9.2642 | 2.5182 | 12,497,500 | 12,507,498 | 0 | 1 |
| `reverse_sorted` | **BubbleSort** | 9.3766 | 2.4533 | 12,497,500 | 37,492,500 | 0 | 1 |
| `sorted` | **InsertionSort** | 0.0014 | 0.0000 | 4,999 | 0 | 0 | 1 |
| `sorted` | **BubbleSort** | 0.0015 | 0.0001 | 4,999 | 0 | 0 | 1 |
| `sorted` | **PDQsort** | 0.0038 | 0.0003 | 10,010 | 6 | 0 | 1 |
| `sorted` | **PowerSort** | 0.0074 | 0.0003 | 4,999 | 0 | 10,004 | 1 |
| `sorted` | **MergeSort** | 0.0419 | 0.0004 | 4,999 | 0 | 20,000 | 14 |
| `sorted_runs` | **PowerSort** | 0.1568 | 0.0242 | 33,976 | 42,163 | 10,004 | 1 |
| `sorted_runs` | **MergeSort** | 0.2495 | 0.1266 | 40,205 | 75,065 | 20,000 | 14 |
| `sorted_runs` | **PDQsort** | 0.2959 | 0.2254 | 72,919 | 57,023 | 0 | 12 |
| `sorted_runs` | **InsertionSort** | 4.0340 | 1.0990 | 6,086,749 | 6,091,561 | 0 | 1 |
| `sorted_runs` | **BubbleSort** | 7.5641 | 0.1511 | 12,487,489 | 18,245,253 | 0 | 1 |
