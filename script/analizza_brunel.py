import numpy as np
import matplotlib.pyplot as plt
import os

def leggi_file_binario(filepath):
    """Legge il formato binario custom strutturato con header int32 (colonne) e dati double."""
    if not os.path.exists(filepath):
        print(f"File non trovato: {filepath}")
        return None, None
        
    with open(filepath, 'rb') as f:
        # Legge il numero di colonne (int32)
        cols_bytes = f.read(4)
        if not cols_bytes:
            return None, None
        num_cols = np.frombuffer(cols_bytes, dtype=np.int32)[0]
        
        # Legge il resto come array di float64 (double)
        data = np.fromfile(f, dtype=np.float64)
        
    # Riorganizza i dati in matrice (num_step, num_cols)
    num_rows = len(data) // num_cols
    matrix = data.reshape((num_rows, num_cols))
    
    times = matrix[:, 0]        # Prima colonna: Tempo
    values = matrix[:, 1:]      # Colonne successive: Neuroni / Sinapsi
    return times, values

def analizza_e_plasma(nome_regime):
    file_f = f"output/F_{nome_regime}.bin"
    times, firing_matrix = leggi_file_binario(file_f)
    
    if times is None:
        return

    print(f"Analisi per il regime: {nome_regime} (Step temporali: {len(times)})")

    # Estraiamo gli spike (dove firing_matrix == 1.0)
    # firing_matrix contiene 1.0 se il neurone ha sparato in quel dt, 0.0 altrimenti
    neuroni_idx, spike_times = [], []
    
    # Campioniamo un sottoinsieme di neuroni se sono troppi (es. max 1000 per il plot)
    num_neuroni = firing_matrix.shape[1]
    step_campione = max(1, num_neuroni // 1000)
    
    for n_idx in range(0, num_neuroni, step_campione):
        spikes = np.where(firing_matrix[:, n_idx] > 0.5)[0]
        if len(spikes) > 0:
            for s in spikes:
                neuroni_idx.append(n_idx)
                spike_times.append(times[s])

    # Calcoliamo il Firing Rate Globale della popolazione (Hz)
    dt = times[1] - times[0] if len(times) > 1 else 0.0001
    spike_counts = np.sum(firing_matrix, axis=1) # totale spike in ogni dt
    # Firing rate medio normalizzato in Hz (spike al secondo per neurone)
    firing_rate_global = (spike_counts / num_neuroni) / dt

    # --- PLOT ---
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

    # 1. Raster Plot
    ax1.scatter(spike_times, neuroni_idx, s=1, color='black', alpha=0.6)
    ax1.set_ylabel("Indice Neurone")
    ax1.set_title(f"Raster Plot - Regime: {nome_regime}")
    ax1.grid(True, linestyle='--', alpha=0.5)

    # 2. Firing Rate Istantaneo Globale
    ax2.plot(times, firing_rate_global, color='firebrick', linewidth=1.2)
    ax2.set_xlabel("Tempo [s]")
    ax2.set_ylabel("Firing Rate Medio [Hz]")
    ax2.grid(True, linestyle='--', alpha=0.5)

    plt.tight_layout()
    output_img = f"Brunel_{nome_regime}.png"
    plt.savefig(output_img, dpi=300)
    print(f"Grafico salvato con successo come: {output_img}\n")
    plt.close()

if __name__ == "__main__":
    regimi = [
        "SR_SynchronousRegular",
        "AI_AsynchronousIrregular",
        "SI_fast_SynchronousFast",
        "SI_slow_SynchronousSlow"
    ]
    
    for r in regimi:
        analizza_e_plasma(r)