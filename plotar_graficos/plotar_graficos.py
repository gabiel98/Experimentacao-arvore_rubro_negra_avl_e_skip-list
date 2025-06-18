import pandas as pd
import matplotlib.pyplot as plt

# Lê os dados do CSV
df = pd.read_csv("../resultados/resultados.csv")

# Define os pontos que você quer visualizar no eixo X
sizes = [
    100000, 120000, 150000, 180000, 210000,
    240000, 270000, 300000, 330000, 360000,
    390000, 420000, 450000, 480000, 500000
]

def plot_metric(metric, ylabel, filename, skip_zero=False):
    plt.figure()
    for estrutura in ["AVL", "RB", "SkipList"]:
        if skip_zero and estrutura == "SkipList":
            continue
        dados = df[(df["Estrutura"] == estrutura) & (df["N"].isin(sizes))]
        plt.plot(dados["N"], dados[metric], label=estrutura, marker='o')

    plt.xlabel("N")
    plt.ylabel(ylabel)
    plt.title(ylabel)
    plt.xticks(sizes, rotation=45)     # Força mostrar todos os valores de N
    plt.yscale("log")                  # Corrige achatamento visual (opcional)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()                 # Ajusta para evitar corte de rótulos
    plt.savefig(filename)
    plt.close()

# Gera os gráficos
plot_metric("TempoBuscaRemocao(ns)", "Tempo Busca+Remocao (ns)", "../graficos/grafico_busca_remocao.png")
plot_metric("TempoBalanceamento(ns)", "Tempo Balanceamento (ns)", "../graficos/grafico_balanceamento.png", skip_zero=True)
plot_metric("TempoTotal(ns)", "Tempo Total (ns)", "../graficos/grafico_total.png")

print("Gráficos gerados")