import numpy as np
import matplotlib.pyplot as plt

def plot_simulazione():
    # Caricamento dei dati (assumendo il formato: tempo, val1, val2, ... valN)
    try:
        potenziali = np.loadtxt('potenziali.txt')
        firing = np.loadtxt('firing.txt')
        sinapsi = np.loadtxt('sinapsi.txt')
    except OSError as e:
        print(f"Errore nel caricamento dei file: {e}")
        return

    # Estrazione del tempo (prima colonna)
    time = potenziali[:, 0]

    fig, axes = plt.subplots(3, 1, figsize=(12, 12), sharex=True)

    # 1. Raster Plot (Firing)
    # firing[:, 1:] contiene gli stati (0 o 1) per ogni neurone
    neuroni_firing = firing[:, 1:]
    axes[0].set_title("Raster Plot (Spike)")
    for i in range(neuroni_firing.shape[1]):
        spike_times = time[neuroni_firing[:, i] == 1]
        axes[0].scatter(spike_times, np.ones_like(spike_times) * i, s=1, color='black', marker='|')
    axes[0].set_ylabel("ID Neurone")

    # 2. Andamento Potenziali di Membrana
    axes[1].set_title("Potenziali di Membrana")
    # Plottiamo solo alcuni neuroni per evitare di sovraccaricare il grafico
    for i in range(1, potenziali.shape[1]): 
        axes[1].plot(time, potenziali[:, i], label=f'N{i-1}')
    axes[1].set_ylabel("Potenziale [V]")
    axes[1].legend(loc='upper right', fontsize='small')

    # 3. Correnti Sinaptiche
    axes[2].set_title("Correnti Sinaptiche")
    for i in range(1, sinapsi.shape[1]):
        axes[2].plot(time, sinapsi[:, i], label=f'S{i-1}')
    axes[2].set_ylabel("Corrente [A]")
    axes[2].set_xlabel("Tempo [s]")
    axes[2].legend(loc='upper right', fontsize='small')

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_simulazione()