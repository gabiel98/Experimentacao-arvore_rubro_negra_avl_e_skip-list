# plotar_graficos.py
# Execute com: python3 plotar_graficos.py

import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("../resultados/resultados.csv")
sizes = sorted(df["N"].unique())

def plot_metric(metric, ylabel, filename, skip_zero=False):
    plt.figure()
    for estrutura in ["AVL", "RB", "SkipList"]:
        if skip_zero and estrutura == "SkipList":
            continue
        dados = df[df["Estrutura"] == estrutura]
        plt.plot(sizes, dados[metric], label=estrutura, marker='o')
    plt.xlabel("N")
    plt.ylabel(ylabel)
    plt.title(ylabel)
    plt.legend()
    plt.grid(True)
    plt.savefig(filename)
    plt.close()

plot_metric("TempoBuscaRemocao(ns)", "Tempo Busca+Remocao (ns)", "../graficos/grafico_busca_remocao.png")
plot_metric("TempoBalanceamento(ns)", "Tempo Balanceamento (ns)", "../graficos/grafico_balanceamento.png", skip_zero=True)
plot_metric("TempoTotal(ns)", "Tempo Total (ns)", "../graficos/grafico_total.png")
print("Gráficos gerados.")
