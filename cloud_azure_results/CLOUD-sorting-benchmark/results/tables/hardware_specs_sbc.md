# Especificacoes do Ambiente Experimental (Pronto para a Secao 4.1 do Artigo)

## Texto sugerido para colar na Secao 4.1:

> Os experimentos foram conduzidos em um ambiente com processador **Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz** (2 vCPUs), hierarquia de cache **L1d: 48 KiB (1 instance) | L2: 1.3 MiB (1 instance) | L3: 48 MiB (1 instance)** e **7.70 GB RAM**, executando sobre o sistema operacional **Linux 6.17.0-1022-azure (x86_64)**. Todos os algoritmos foram implementados em C++17 e compilados com **g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 (Flags: -std=c++17 -O3 -march=native)**. Para mitigar oscilacoes transientes do sistema operacional, cada cenario foi repetido multiplas vezes, descartando-se o melhor (menor tempo) e o pior (maior tempo) resultado antes do calculo da media aparada e do desvio-padrao.

## Tabela Markdown:

| Componente | Especificacao do Ambiente Experimental |
|---|---|
| **Processador (CPU)** | Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz |
| **Nucleos / vCPUs** | 2 vCPUs |
| **Hierarquia de Cache** | L1d: 48 KiB (1 instance) | L2: 1.3 MiB (1 instance) | L3: 48 MiB (1 instance) |
| **Memoria Principal (RAM)** | 7.70 GB RAM |
| **Sistema Operacional / Kernel** | Linux 6.17.0-1022-azure (x86_64) |
| **Compilador e Flags** | g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 (Flags: -std=c++17 -O3 -march=native) |
