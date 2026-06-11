import numpy as np
import matplotlib.pyplot as plt
import struct
import os

def leggi_file_binario(filename):
    """
    Legge i file binari generati dalla Simulazione C++.
    Restituisce l'array dei tempi (1D) e la matrice dei valori (2D).
    """
    if not os.path.exists(filename):
        print(f"File {filename} non trovato.")
        return None, None

    with open(filename, 'rb') as f:
        # 1. Estrae l'header: i primi 4 byte sono un intero a 32 bit (numero di colonne)
        header_bytes = f.read(4)
        num_colonne = struct.unpack('i', header_bytes)[0]
        
        # 2. Legge tutto il resto del file come double a 64 bit (np.float64)
        dati_raw = np.fromfile(f, dtype=np.float64)
        
        # 3. Rimodella l'array 1D in una matrice 2D: (numero_di_step, num_colonne)
        matrice = dati_raw.reshape(-1, num_colonne)
        
        # 4. Separa la colonna del tempo dal resto dei dati
        tempi = matrice[:, 0]
        valori = matrice[:, 1:]
        
        return tempi, valori

def plot_risultati():
    # Nomi dei file generati (modificali se nel main.cpp hai usato nomi diversi)
    file_v = "potenziali.bin"
    file_f = "firing.bin"
    file_s = "sinapsi.bin"

    # Carica i dati
    tempi_v, pot_membrana = leggi_file_binario(file_v)
    tempi_f, firing = leggi_file_binario(file_f)
    tempi_s, sinapsi = leggi_file_binario(file_s)

    if tempi_v is None:
        return

    # Crea una figura con 3 grafici impilati
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 12), sharex=True)

    # --- 1. Grafico Potenziali di Membrana ---
    # Per non intasare il grafico, disegniamo solo i primi 5 neuroni
    # num_neuroni_da_disegnare = min(5, pot_membrana.shape[1])
    for i in range(pot_membrana.shape[1]):
        # Moltiplichiamo per 1000 per convertire da Volt a milliVolt per leggibilità
        ax1.plot(tempi_v, pot_membrana[:, i] * 1000, label=f'Neurone {i}')
    
    ax1.set_ylabel('Potenziale di Membrana (mV)')
    ax1.set_title('Dinamica dei Potenziali (primi 5 neuroni)')
    ax1.legend(loc='upper right', fontsize='small')
    ax1.grid(True, linestyle='--', alpha=0.6)

    # --- 2. Raster Plot dei Firing ---
    if tempi_f is not None and firing is not None:
        # np.where ci trova le coordinate (step, id_neurone) in cui il valore è >= 0.5 (cioè ha sparato)
        spike_steps, spike_neurons = np.where(firing >= 0.5)
        # Sostituiamo gli step con i rispettivi valori temporali
        spike_times = tempi_f[spike_steps]
        
        # Disegniamo dei puntini neri ad ogni spike
        ax2.scatter(spike_times, spike_neurons, s=10, c='black', marker='|')
        ax2.set_ylabel('ID Neurone')
        ax2.set_title('Raster Plot (Spike di tutti i neuroni)')
        ax2.set_ylim(-1, firing.shape[1])
        ax2.grid(True, linestyle='--', alpha=0.6)

    # --- 3. Grafico Correnti Sinaptiche ---
    if tempi_s is not None and sinapsi is not None and sinapsi.shape[1] > 0:
        num_sinapsi_da_disegnare = min(10, sinapsi.shape[1])
        for i in range(num_sinapsi_da_disegnare):
            # Convertiamo da Ampere a nanoAmpere (nA) o picoAmpere (pA) in base alla scala
            ax3.plot(tempi_s, sinapsi[:, i] * 1e9, label=f'Sinapsi {i}')
        
        ax3.set_ylabel('Corrente (nA)')
        ax3.set_title('Correnti Sinaptiche Totali')
        ax3.legend(loc='upper right', fontsize='small')
    else:
        ax3.text(0.5, 0.5, "Nessuna sinapsi registrata", ha='center', va='center')
        ax3.set_ylabel('Corrente')

    ax3.set_xlabel('Tempo (s)')
    ax3.grid(True, linestyle='--', alpha=0.6)

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_risultati()