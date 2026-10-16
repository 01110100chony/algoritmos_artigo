#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Script de Visualizacao Cientifica e Geracao de Tabelas (Padrao SBC - 300 DPI)
# Le data/summary_stats.csv e gera graficos em results/plots/ e tabelas em results/tables/.

import os
import pandas as pd
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

try:
    import seaborn as sns
except ModuleNotFoundError:
    sns = None


def setup_style():
    if sns is not None:
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
            axes[0].plot(alg_df["n"], alg_df["time_ms_mean"], marker="o", linewidth=1.8, label=alg)
            axes[1].plot(alg_df["n"], np.maximum(alg_df["time_ms_mean"], 1e-5), marker="o", linewidth=1.8, label=alg)

        axes[0].set_title(f"{bat_name} - Tempo vs N (Linear)")
        axes[0].set_xlabel("Tamanho da Entrada (N)")
        axes[0].set_ylabel("Tempo Medio (ms)")
        axes[0].legend()

        axes[1].set_xscale("log")
        axes[1].set_yscale("log")
        axes[1].set_title(f"{bat_name} - Escalabilidade (Log-Log)")
        axes[1].set_xlabel("Tamanho da Entrada (N) [log]")
        axes[1].set_ylabel("Tempo Medio (ms) [log]")
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
    if sns is not None:
        sns.barplot(data=sub, x="distribution", y="throughput_eps_mean", hue="algorithm", ax=axes[0])
        sns.barplot(data=sub, x="distribution", y="ipc_mean", hue="algorithm", ax=axes[1])
    else:
        dists = sub["distribution"].unique()
        algorithms = sub["algorithm"].unique()
        x = np.arange(len(dists))
        width = 0.8 / max(1, len(algorithms))
        for idx, alg in enumerate(algorithms):
            vals = sub[sub["algorithm"] == alg].set_index("distribution").reindex(dists)
            offsets = x + (idx - (len(algorithms) - 1) / 2) * width
            axes[0].bar(offsets, vals["throughput_eps_mean"].to_numpy(), width=width, label=alg)
            axes[1].bar(offsets, vals["ipc_mean"].to_numpy(), width=width, label=alg)
        axes[0].set_xticks(x)
        axes[1].set_xticks(x)
        axes[0].set_xticklabels(dists, rotation=25)
        axes[1].set_xticklabels(dists, rotation=25)
    axes[0].set_yscale("log")
    axes[0].set_title(f"Throughput por Distribuicao (N = {max_n}) [Escala Log]")
    axes[0].set_xlabel("Distribuicao de Entrada")
    axes[0].set_ylabel("Elementos Ordenados / s (log)")
    axes[0].tick_params(axis="x", rotation=25)

    axes[1].set_title(f"Instructions Per Cycle (IPC) por Distribuicao (N = {max_n})")
    axes[1].set_xlabel("Distribuicao de Entrada")
    axes[1].set_ylabel("IPC Medio")
    axes[1].tick_params(axis="x", rotation=25)
    axes[1].legend()
    axes[0].legend()

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
    if sns is not None:
        sns.barplot(data=sub, x="distribution", y="comparisons_mean", hue="algorithm", ax=axes[0])
        sns.barplot(data=sub, x="distribution", y="writes_plot", hue="algorithm", ax=axes[1])
    else:
        dists = sub["distribution"].unique()
        algorithms = sub["algorithm"].unique()
        x = np.arange(len(dists))
        width = 0.8 / max(1, len(algorithms))
        for idx, alg in enumerate(algorithms):
            vals_cmp = sub[sub["algorithm"] == alg].set_index("distribution").reindex(dists)
            vals_wrt = sub[sub["algorithm"] == alg].set_index("distribution").reindex(dists)
            offsets = x + (idx - (len(algorithms) - 1) / 2) * width
            axes[0].bar(offsets, vals_cmp["comparisons_mean"].to_numpy(), width=width, label=alg)
            axes[1].bar(offsets, (vals_wrt["writes_plot"]).to_numpy(), width=width, label=alg)
        axes[0].set_xticks(x)
        axes[1].set_xticks(x)
        axes[0].set_xticklabels(dists, rotation=20)
        axes[1].set_xticklabels(dists, rotation=20)
    axes[0].set_yscale("log")
    axes[0].set_title(f"Comparacoes em Cenarios Adaptativos (N = {max_n})")
    axes[0].set_xlabel("Distribuicao")
    axes[0].set_ylabel("Numero de Comparacoes [log]")
    axes[0].tick_params(axis="x", rotation=20)
    axes[0].legend()

    axes[1].set_yscale("log")
    axes[1].set_title(f"Escritas em Memoria (Writes) em Cenarios Adaptativos (N = {max_n})")
    axes[1].set_xlabel("Distribuicao")
    axes[1].set_ylabel("Numero de Escritas (Writes + 1) [log]")
    axes[1].tick_params(axis="x", rotation=20)
    axes[1].legend()

    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, "4_comparacoes_e_writes_adaptativos.png"), bbox_inches="tight")
    plt.close(fig)

def generate_tables(df, tables_dir):
    b3 = df[df["battery"] == "Bateria_3_Geral"]
    if b3.empty:
        return
    max_n = b3["n"].max()
    sub = b3[b3["n"] == max_n][[
        "algorithm", "distribution", "time_ms_mean", "time_ms_std",
        "comparisons_mean", "writes_mean", "peak_aux_bytes", "max_recursion_depth"
    ]].sort_values(["distribution", "time_ms_mean"])

    md_path = os.path.join(tables_dir, "tabela_resultados_sbc.md")
    with open(md_path, "w", encoding="utf-8") as f:
        f.write(f"# Tabela Consolidada - Comparacao Geral (N = {max_n})\n\n")
        f.write("| Distribuicao | Algoritmo | Tempo Medio (ms) | Desvio (ms) | Comparacoes | Escritas (Writes) | Memoria Aux (B) | Prof. Recursao |\n")
        f.write("|---|---|---:|---:|---:|---:|---:|---:|\n")
        for _, r in sub.iterrows():
            f.write(
                f"| `{r['distribution']}` | **{r['algorithm']}** | "
                f"{r['time_ms_mean']:.4f} | {r['time_ms_std']:.4f} | "
                f"{int(r['comparisons_mean']):,} | {int(r['writes_mean']):,} | "
                f"{int(r['peak_aux_bytes']):,} | {int(r['max_recursion_depth'])} |\n"
            )

    tex_path = os.path.join(tables_dir, "tabela_resultados_sbc.tex")
    with open(tex_path, "w", encoding="utf-8") as f:
        f.write("% Tabela gerada automaticamente para artigo no formato SBC\n")
        f.write("\\begin{table}[htbp]\n")
        f.write("\\centering\n")
        f.write(f"\\caption{{Desempenho experimental e metricas algoritmicas para $N = {max_n}$.}}\n")
        f.write("\\label{tab:resultados_gerais}\n")
        f.write("\\resizebox{\\textwidth}{!}{%\n")
        f.write("\\begin{tabular}{llrrrrrr}\n")
        f.write("\\hline\n")
        f.write("\\textbf{Distribuicao} & \\textbf{Algoritmo} & \\textbf{Tempo (ms)} & \\textbf{DP (ms)} & "
                "\\textbf{Comparacoes} & \\textbf{Escritas} & \\textbf{Mem. Aux (B)} & \\textbf{Prof. Rec.} \\\\\n")
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

    print("[SUCESSO] Graficos (300 DPI) gerados em results/plots/ e tabelas em results/tables/")

if __name__ == "__main__":
    main()
