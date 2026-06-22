import numpy as np
import matplotlib.pyplot as plt
import os

# ============================================================
# CONFIGURAZIONE FILE DI INPUT
# ============================================================
# Assicurati che i file si trovino nella cartella corretta (es. "output/")
PATH_CARTELLA = "output/" 
FILE_POTENZIALI = os.path.join(PATH_CARTELLA, "test_potenziali.bin")
FILE_FIRING = os.path.join(PATH_CARTELLA, "test_firing.bin")
FILE_SINAPSI = os.path.join(PATH_CARTELLA, "test_sinapsi.bin")

def carica_dati_binari_con_header(filepath):
    """
    Legge il file binario personalizzato:
    - Primi 4 byte: int32_t (numero di colonne della matrice)
    - Byte successivi: sequenza di double (float64) organizzati in righe
    """
    if not os.path.exists(filepath):
        raise FileNotFoundError(f"Il file '{filepath}' non esiste. Controlla il percorso.")
        
    with open(filepath, 'rb') as f:
        # 1. Leggi l'header di 4 byte (int32_t)
        header_bytes = f.read(4)
        if len(header_bytes) < 4:
            raise ValueError(f"Il file {filepath} è corrotto o vuoto (manca l'header).")
            
        # Convertiamo i 4 byte in un intero a 32 bit
        num_colonne = np.frombuffer(header_bytes, dtype=np.int32)[0]
        
        # 2. Leggi tutto il resto del file come double (float64 in Python)
        # np.fromfile continua a leggere dal punto in cui f.read(4) si è fermato
        dati_flat = np.fromfile(f, dtype=np.float64)
        
        # 3. Reshape della matrice basandoci sul numero di colonne letto dall'header
        matrice_dati = dati_flat.reshape(-1, num_colonne)
        return matrice_dati, num_colonne

# ============================================================
# CARICAMENTO DEI DATI
# ============================================================
try:
    potenziali, cols_V = carica_dati_binari_con_header(FILE_POTENZIALI)
    firing, cols_F = carica_dati_binari_con_header(FILE_FIRING)
    sinapsi, cols_S = carica_dati_binari_con_header(FILE_SINAPSI)
    
    num_neuroni = cols_V - 1
    num_sinapsi = cols_S - 1
    print(f"Lettura completata con successo!")
    print(f" -> Trovati {num_neuroni} neuroni e {num_sinapsi} sinapsi.")
    print(f" -> Rilevati {potenziali.shape[0]} step temporali.")
except Exception as e:
    print(f"Errore durante il parsing dei file binari: {e}")
    print("Verifica di aver eseguito la simulazione e che i file binari siano dentro la cartella specificata.")
    exit(1)

# Estrazione del tempo (prima colonna di uno qualsiasi dei file)
time_ms = potenziali[:, 0] * 1000.0  # da secondi a millisecondi

# Separazione delle matrici dei dati (colonne successive al tempo)
V_matrix = potenziali[:, 1:]    # Stato dei potenziali
spike_matrix = firing[:, 1:]    # Stato dei firing (0 o 1)
I_syn_matrix = sinapsi[:, 1:]   # Stato delle correnti sinaptiche

# ============================================================
# PLOT DI VERIFICA (Configurato per il test a 3 neuroni)
# ============================================================
fig, axs = plt.subplots(3, 1, figsize=(12, 10), sharex=True)

# Definiamo dei colori fissi per i primi 3 neuroni del test
colors = ['#1f77b4', '#ff770e', '#2ca02c'] 

# 1) Potenziali di Membrana
for nid in range(num_neuroni):
    label = f"Neurone {nid}"
    color = colors[nid] if nid < len(colors) else None
    axs[0].plot(time_ms, V_matrix[:, nid] * 1000.0, color=color, label=label, alpha=0.8)

axs[0].axvline(10, color='gray', linestyle='--', alpha=0.5, label="Inizio I_cost (N0)")
axs[0].axvline(40, color='purple', linestyle='--', alpha=0.5, label="Inizio I_seno (N2)")
axs[0].set_title("Potenziali di Membrana (Unità: mV)")
axs[0].set_ylabel("V (mV)")
axs[0].legend(loc="upper right")
axs[0].grid(True, alpha=0.3)

# 2) Raster Plot degli Spike
for nid in range(num_neuroni):
    spike_indices = np.where(spike_matrix[:, nid] > 0)[0]
    spike_times_n = time_ms[spike_indices]
    color = colors[nid] if nid < len(colors) else 'black'
    
    axs[1].scatter(spike_times_n, np.ones_like(spike_times_n) * nid, 
                    color=color, marker='|', s=500, linewidths=2)

axs[1].set_title("Raster Plot (Spike del Sistema)")
axs[1].set_ylabel("ID Neurone")
axs[1].set_yticks(range(num_neuroni))
axs[1].grid(True, alpha=0.3)

# 3) Correnti Afferenti
# Se il numero di colonne del file sinapsi è pari al numero di neuroni, plottiamo per neurone.
# Altrimenti plottiamo l'andamento di ogni singola sinapsi registrata.
for i in range(I_syn_matrix.shape[1]):
    label = f"Indice {i}"
    color = colors[i] if i < len(colors) else None
    axs[2].plot(time_ms, I_syn_matrix[:, i] * 1e9, color=color, label=label, alpha=0.8)

axs[2].set_title("Correnti Registrate (Unità: nA)")
axs[2].set_ylabel("I (nA)")
axs[2].set_xlabel("Tempo (ms)")
axs[2].legend(loc="upper right")
axs[2].grid(True, alpha=0.3)

plt.tight_layout()
plt.show()