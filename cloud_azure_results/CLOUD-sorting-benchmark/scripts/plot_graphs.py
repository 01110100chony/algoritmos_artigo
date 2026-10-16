#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Script de Visualizacao Cientifica, Tabelas de Resultados e Tabela de Hardware (Padrao SBC - 300 DPI)

import os
import platform
import shutil
import subprocess
import pandas as pd
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import seaborn as sns

def setup_style():
    sns.set_theme(style="whitegrid", context="paper")
    plt.rcParams.update({
        "figure.dpi": 300,
        "savefig.dpi": 300,
        "font.size": 10,
        "axes.titlesize": 11,
        "axes.labelsize": 10,
        "legend.fontsize": 8.5,
        "xtick.labelsize": 9,
        "ytick.labelsize": 9,
    })

def plot_scalability(df, out_dir):
    for bat_name, group in df[df["distribution"] == "random_uniform"].groupby("battery"):
        fig, axes = plt.subplots(1, 2, figsize=(11, 4.5))

        for alg, alg_df in group.groupby("algorithm"):
            alg_df = alg_df.sort_values("n")
            n_vals = alg_df["n"].to_numpy()
            mean_vals = alg_df["time_ms_mean"].to_numpy()
            std_vals = alg_df["time_ms_std"].to_numpy()

            axes[0].plot(n_vals, mean_vals, marker="o", linewidth=1.8, label=alg)
            axes[0].fill_between(
                n_vals,
                np.maximum(0.0, mean_vals - std_vals),
                mean_vals + std_vals,
                alpha=0.18
            )

            axes[1].plot(n_vals, np.maximum(mean_vals, 1e-5), marker="o", linewidth=1.8, label=alg)

        axes[0].set_title(f"{bat_name} - Media Aparada de Tempo vs N (Linear)")
        axes[0].set_xlabel("Tamanho da Entrada (N)")
        axes[0].set_ylabel("Tempo Medio Aparado (ms)")
        axes[0].legend()

        axes[1].set_xscale("log")
        axes[1].set_yscale("log")
        axes[1].set_title(f"{bat_name} - Escalabilidade (Log-Log)")
        axes[1].set_xlabel("Tamanho da Entrada (N) [log]")
        axes[1].set_ylabel("Tempo Medio Aparado (ms) [log]")
        axes[1].legend()

        plt.tight_layout()
        plt.savefig(os.path.join(out_dir, f"1_escalabilidade_{bat_name}.png"), bbox_inches="tight")
        plt.close(fig)

def plot_throughput_and_ipc(df, out_dir):
    b3 = df[df["battery"] == "Bateria_3_Geral"]
    if b3.empty:
        return
    max_n = b3["n"].max()
    sub = b3[b3["n"] == max_n]

    fig, axes = plt.subplots(1, 2, figsize=(12, 4.8))
    sns.barplot(data=sub, x="distribution", y="throughput_eps_mean", hue="algorithm", ax=axes[0])
    axes[0].set_yscale("log")
    axes[0].set_title(f"Throughput Medio Aparado por Distribuicao (N = {max_n})")
    axes[0].set_xlabel("Distribuicao de Entrada")
    axes[0].set_ylabel("Elementos Ordenados / s (log)")
    axes[0].tick_params(axis="x", rotation=25)

    sns.barplot(data=sub, x="distribution", y="ipc_mean", hue="algorithm", ax=axes[1])
    axes[1].set_title(f"Instructions Per Cycle (IPC) por Distribuicao (N = {max_n})")
    axes[1].set_xlabel("Distribuicao de Entrada")
    axes[1].set_ylabel("IPC Medio")
    axes[1].tick_params(axis="x", rotation=25)

    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, "2_throughput_e_ipc_por_distribuicao.png"), bbox_inches="tight")
    plt.close(fig)

def plot_branches_and_cache(df, out_dir):
    b2 = df[(df["battery"] == "Bateria_2_Eficiente") & (df["distribution"] == "random_uniform")]
    if b2.empty:
        return

    fig, axes = plt.subplots(1, 2, figsize=(11, 4.5))
    for alg, alg_df in b2.groupby("algorithm"):
        alg_df = alg_df.sort_values("n")
        axes[0].plot(alg_df["n"], alg_df["branch_misses_mean"], marker="s", linewidth=1.8, label=alg)
        axes[1].plot(alg_df["n"], alg_df["cache_misses_mean"], marker="^", linewidth=1.8, label=alg)

    axes[0].set_xscale("log")
    axes[0].set_title("Branch Mispredictions vs N (random_uniform)")
    axes[0].set_xlabel("Tamanho da Entrada (N) [log]")
    axes[0].set_ylabel("Branch Misses")
    axes[0].legend()

    axes[1].set_xscale("log")
    axes[1].set_title("Cache Misses vs N (random_uniform)")
    axes[1].set_xlabel("Tamanho da Entrada (N) [log]")
    axes[1].set_ylabel("Cache Misses")
    axes[1].legend()

    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, "3_branch_e_cache_misses.png"), bbox_inches="tight")
    plt.close(fig)

def plot_adaptive_comparisons_and_writes(df, out_dir):
    b3 = df[df["battery"] == "Bateria_3_Geral"]
    if b3.empty:
        return
    max_n = b3["n"].max()
    adaptive_dists = ["sorted", "nearly_sorted", "sorted_runs", "organ_pipe"]
    sub = b3[(b3["n"] == max_n) & (b3["distribution"].isin(adaptive_dists))].copy()
    sub["writes_plot"] = sub["writes_mean"] + 1.0

    fig, axes = plt.subplots(1, 2, figsize=(12, 4.8))
    sns.barplot(data=sub, x="distribution", y="comparisons_mean", hue="algorithm", ax=axes[0])
    axes[0].set_yscale("log")
    axes[0].set_title(f"Comparacoes em Cenarios Adaptativos (N = {max_n})")
    axes[0].set_xlabel("Distribuicao")
    axes[0].set_ylabel("Numero de Comparacoes [log]")
    axes[0].tick_params(axis="x", rotation=20)

    sns.barplot(data=sub, x="distribution", y="writes_plot", hue="algorithm", ax=axes[1])
    axes[1].set_yscale("log")
    axes[1].set_title(f"Escritas em Memoria (Writes) em Cenarios Adaptativos (N = {max_n})")
    axes[1].set_xlabel("Distribuicao")
    axes[1].set_ylabel("Numero de Escritas (Writes + 1) [log]")
    axes[1].tick_params(axis="x", rotation=20)

    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, "4_comparacoes_e_writes_adaptativos.png"), bbox_inches="tight")
    plt.close(fig)

def extract_clean_hardware_summary():
    info = {
        "cpu_model": platform.processor() or "Processador x86_64",
        "cores_threads": f"{os.cpu_count() or 2} vCPUs / Threads",
        "caches": "N/D",
        "ram": "N/D",
        "os_kernel": f"{platform.system()} {platform.release()} ({platform.machine()})",
        "compiler": "GCC / g++ (-std=c++17 -O3 -march=native)",
    }

    if shutil.which("lscpu"):
        try:
            out = subprocess.check_output(["lscpu"], text=True, stderr=subprocess.DEVNULL)
            l1d, l2, l3 = "", "", ""
            for line in out.splitlines():
                if ":" not in line:
                    continue
                k, v = [x.strip() for x in line.split(":", 1)]
                kl = k.lower()
                if "model name" in kl or "nome do modelo" in kl:
                    info["cpu_model"] = v
                elif kl == "cpu(s)":
                    info["cores_threads"] = f"{v} vCPUs"
                elif "l1d cache" in kl or "cache l1d" in kl:
                    l1d = f"L1d: {v}"
                elif "l2 cache" in kl or "cache l2" in kl:
                    l2 = f"L2: {v}"
                elif "l3 cache" in kl or "cache l3" in kl:
                    l3 = f"L3: {v}"
            caches = [c for c in [l1d, l2, l3] if c]
            if caches:
                info["caches"] = " | ".join(caches)
        except Exception:
            pass

    if os.path.exists("/proc/meminfo"):
        try:
            with open("/proc/meminfo", "r", encoding="utf-8") as f:
                for line in f:
                    if line.startswith("MemTotal:"):
                        kb = int(line.split()[1])
                        gb = kb / (1024 * 1024)
                        info["ram"] = f"{gb:.2f} GB RAM"
                        break
        except Exception:
            pass

    if shutil.which("g++"):
        try:
            out = subprocess.check_output(["g++", "--version"], text=True, stderr=subprocess.DEVNULL)
            first_line = out.splitlines()[0].strip()
            info["compiler"] = f"{first_line} (Flags: -std=c++17 -O3 -march=native)"
        except Exception:
            pass

    return info

def generate_hardware_table(tables_dir):
    hw = extract_clean_hardware_summary()

    md_path = os.path.join(tables_dir, "hardware_specs_sbc.md")
    with open(md_path, "w", encoding="utf-8") as f:
        f.write("# Especificacoes do Ambiente Experimental (Pronto para a Secao 4.1 do Artigo)\n\n")
        f.write("## Texto sugerido para colar na Secao 4.1:\n\n")
        f.write(
            f"> Os experimentos foram conduzidos em um ambiente com processador **{hw['cpu_model']}** "
            f"({hw['cores_threads']}), hierarquia de cache **{hw['caches']}** e **{hw['ram']}**, "
            f"executando sobre o sistema operacional **{hw['os_kernel']}**. "
            f"Todos os algoritmos foram implementados em C++17 e compilados com **{hw['compiler']}**. "
            f"Para mitigar oscilacoes transientes do sistema operacional, cada cenario foi repetido multiplas vezes, "
            f"descartando-se o melhor (menor tempo) e o pior (maior tempo) resultado antes do calculo da media aparada e do desvio-padrao.\n\n"
        )
        f.write("## Tabela Markdown:\n\n")
        f.write("| Componente | Especificacao do Ambiente Experimental |\n")
        f.write("|---|---|\n")
        f.write(f"| **Processador (CPU)** | {hw['cpu_model']} |\n")
        f.write(f"| **Nucleos / vCPUs** | {hw['cores_threads']} |\n")
        f.write(f"| **Hierarquia de Cache** | {hw['caches']} |\n")
        f.write(f"| **Memoria Principal (RAM)** | {hw['ram']} |\n")
        f.write(f"| **Sistema Operacional / Kernel** | {hw['os_kernel']} |\n")
        f.write(f"| **Compilador e Flags** | {hw['compiler']} |\n")

    tex_path = os.path.join(tables_dir, "hardware_specs_sbc.tex")
    with open(tex_path, "w", encoding="utf-8") as f:
        f.write("% Tabela de Hardware pronta para a Secao 4.1 do artigo no formato SBC\n")
        f.write("\\begin{table}[htbp]\n")
        f.write("\\centering\n")
        f.write("\\caption{Especificações do ambiente de hardware e software utilizado nos experimentos.}\n")
        f.write("\\label{tab:ambiente_experimental}\n")
        f.write("\\begin{tabular}{ll}\n")
        f.write("\\hline\n")
        f.write("\\textbf{Componente} & \\textbf{Especificação} \\\\\n")
        f.write("\\hline\n")
        f.write(f"Processador (CPU) & {hw['cpu_model'].replace('_', '\\_')} \\\\\n")
        f.write(f"Núcleos / vCPUs & {hw['cores_threads'].replace('_', '\\_')} \\\\\n")
        f.write(f"Caches (L1/L2/L3) & {hw['caches'].replace('_', '\\_')} \\\\\n")
        f.write(f"Memória RAM & {hw['ram'].replace('_', '\\_')} \\\\\n")
        f.write(f"Sistema Operacional & {hw['os_kernel'].replace('_', '\\_')} \\\\\n")
        f.write(f"Compilador e Flags & {hw['compiler'].replace('_', '\\_')} \\\\\n")
        f.write("\\hline\n")
        f.write("\\end{tabular}\n")
        f.write("\\end{table}\n")

def generate_tables(df, tables_dir):
    b3 = df[df["battery"] == "Bateria_3_Geral"]
    if b3.empty:
        return
    max_n = b3["n"].max()
    sub = b3[b3["n"] == max_n][[
        "algorithm", "distribution", "reps_total", "reps_valid_trimmed",
        "time_ms_mean", "time_ms_std", "time_ms_best_removed", "time_ms_worst_removed",
        "comparisons_mean", "writes_mean", "peak_aux_bytes", "max_recursion_depth"
    ]].sort_values(["distribution", "time_ms_mean"])

    md_path = os.path.join(tables_dir, "tabela_resultados_sbc.md")
    with open(md_path, "w", encoding="utf-8") as f:
        f.write(f"# Tabela Consolidada (Media Aparada sem Melhor e Pior) - N = {max_n}\n\n")
        f.write("| Distribuicao | Algoritmo | Tempo Medio Aparado (ms) | Desvio (ms) | Melhor Descartado (ms) | Pior Descartado (ms) | Comparacoes | Escritas (Writes) | Mem. Aux (B) | Prof. Rec. |\n")
        f.write("|---|---|---:|---:|---:|---:|---:|---:|---:|---:|\n")
        for _, r in sub.iterrows():
            f.write(
                f"| `{r['distribution']}` | **{r['algorithm']}** | "
                f"{r['time_ms_mean']:.4f} | {r['time_ms_std']:.4f} | "
                f"{r['time_ms_best_removed']:.4f} | {r['time_ms_worst_removed']:.4f} | "
                f"{int(r['comparisons_mean']):,} | {int(r['writes_mean']):,} | "
                f"{int(r['peak_aux_bytes']):,} | {int(r['max_recursion_depth'])} |\n"
            )

    tex_path = os.path.join(tables_dir, "tabela_resultados_sbc.tex")
    with open(tex_path, "w", encoding="utf-8") as f:
        f.write("% Tabela gerada automaticamente para artigo no formato SBC (Media Aparada)\n")
        f.write("\\begin{table}[htbp]\n")
        f.write("\\centering\n")
        f.write(f"\\caption{{Média aparada (descartando o melhor e o pior resultado) e métricas algorítmicas para $N = {max_n}$.}}\n")
        f.write("\\label{tab:resultados_gerais}\n")
        f.write("\\resizebox{\\textwidth}{!}{%\n")
        f.write("\\begin{tabular}{llrrrrrr}\n")
        f.write("\\hline\n")
        f.write("\\textbf{Distribuição} & \\textbf{Algoritmo} & \\textbf{Tempo Médio (ms)} & \\textbf{DP (ms)} & "
                "\\textbf{Comparações} & \\textbf{Escritas} & \\textbf{Mem. Aux (B)} & \\textbf{Prof. Rec.} \\\\\n")
        f.write("\\hline\n")
        for _, r in sub.iterrows():
            dist_tex = str(r["distribution"]).replace("_", "\\_")
            f.write(
                f"{dist_tex} & {r['algorithm']} & "
                f"{r['time_ms_mean']:.4f} & {r['time_ms_std']:.4f} & "
                f"{int(r['comparisons_mean'])} & {int(r['writes_mean'])} & "
                f"{int(r['peak_aux_bytes'])} & {int(r['max_recursion_depth'])} \\\\\n"
            )
        f.write("\\hline\n")
        f.write("\\end{tabular}%\n}\n")
        f.write("\\end{table}\n")

def main():
    csv_path = "data/summary_stats.csv"
    if not os.path.exists(csv_path):
        print(f"[ERRO] Arquivo {csv_path} nao encontrado. Execute o binario C++ primeiro.")
        return

    out_plots = "results/plots"
    out_tables = "results/tables"
    os.makedirs(out_plots, exist_ok=True)
    os.makedirs(out_tables, exist_ok=True)

    setup_style()
    df = pd.read_csv(csv_path)

    plot_scalability(df, out_plots)
    plot_throughput_and_ipc(df, out_plots)
    plot_branches_and_cache(df, out_plots)
    plot_adaptive_comparisons_and_writes(df, out_plots)
    generate_tables(df, out_tables)
    generate_hardware_table(out_tables)

    print("[SUCESSO] Graficos (300 DPI) gerados em results/plots/")
    print("[SUCESSO] Tabelas de Resultados e de Hardware geradas em results/tables/")

if __name__ == "__main__":
    main()
