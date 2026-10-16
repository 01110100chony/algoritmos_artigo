# Especificacoes do Ambiente Experimental (Pronto para a Secao 4.1 do Artigo)

## Texto sugerido para colar na Secao 4.1:

> Os experimentos foram conduzidos em um ambiente com processador **AMD64 Family 23 Model 24 Stepping 1, AuthenticAMD** (8 vCPUs / Threads), hierarquia de cache **N/D** e **N/D**, executando sobre o sistema operacional **Windows 10 (AMD64)**. Todos os algoritmos foram implementados em C++17 e compilados com **g++ (Rev13, Built by MSYS2 project) 15.2.0 (Flags: -std=c++17 -O3 -march=native)**. Para mitigar oscilacoes transientes do sistema operacional, cada cenario foi repetido multiplas vezes, descartando-se o melhor (menor tempo) e o pior (maior tempo) resultado antes do calculo da media aparada e do desvio-padrao.

## Tabela Markdown:

| Componente | Especificacao do Ambiente Experimental |
|---|---|
| **Processador (CPU)** | AMD64 Family 23 Model 24 Stepping 1, AuthenticAMD |
| **Nucleos / vCPUs** | 8 vCPUs / Threads |
| **Hierarquia de Cache** | N/D |
| **Memoria Principal (RAM)** | N/D |
| **Sistema Operacional / Kernel** | Windows 10 (AMD64) |
| **Compilador e Flags** | g++ (Rev13, Built by MSYS2 project) 15.2.0 (Flags: -std=c++17 -O3 -march=native) |
