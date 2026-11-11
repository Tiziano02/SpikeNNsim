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

    # Escludiamo i primi 0.2 secondi di transitorio.
    maschera = times >= 0.2
    tempi = times[maschera]

    cv_neuroni = np.full(firing_matrix.shape[1], np.nan)

    for indice in range(firing_matrix.shape[1]):
        spike_times = tempi[firing_matrix[maschera, indice] > 0.5]
        isi = np.diff(spike_times)

        # Soglia pratica: con pochi intervalli il CV è poco affidabile.
        if isi.size >= 10:
            cv_neuroni[indice] = np.std(isi, ddof=1) / np.mean(isi)

    validi = cv_neuroni[np.isfinite(cv_neuroni)]

    if validi.size:
        print(f"Neuroni con almeno 10 ISI: {validi.size}/{cv_neuroni.size}")
        print(f"CV medio:   {validi.mean():.3f}")
        print(f"CV mediano: {np.median(validi):.3f}")

        plt.figure(figsize=(7, 4))
        plt.hist(validi, bins=40, color="steelblue", edgecolor="white")
        plt.xlabel("CV degli intervalli tra spike")
        plt.ylabel("Numero di neuroni")
        plt.tight_layout()
        plt.show()
    else:
        print("Spike insufficienti per stimare il CV.")

    if times is None or len(times) < 2:
        return

    num_neuroni = firing_matrix.shape[1]
    dt = times[1] - times[0]

    # Finestra dopo il transitorio, in secondi.
    t_start, t_end = 0.5, 0.7
    finestra = (times >= t_start) & (times < t_end)

    if not np.any(finestra):
        print(f"{nome_regime}: nessun dato nella finestra scelta.")
        return

    tempi = times[finestra]
    firing = firing_matrix[finestra] > 0.5

    # Stesso campione riproducibile nei diversi regimi.
    rng = np.random.default_rng(42)
    selezionati = np.sort(
        rng.choice(num_neuroni, size=min(50, num_neuroni), replace=False)
    )

    fig, (ax1, ax2) = plt.subplots(
        2, 1, figsize=(11, 7), sharex=True
    )

    # Posizioni ordinate nel campione; ogni riga è un neurone.
    for posizione, indice in enumerate(selezionati):
        spike_times = tempi[firing[:, indice]]
        ax1.scatter(
            spike_times,
            np.full(spike_times.size, posizione),
            marker="|", s=18, linewidths=0.6, color="black"
        )

    ax1.set_ylabel("Neurone nel campione")
    ax1.set_title(f"Regime obiettivo: {nome_regime}")
    ax1.set_ylim(-1, len(selezionati))

    # Rate su TUTTI i neuroni, raggruppando timestep completi.
    passi_bin = max(1, int(round(1e-3 / dt)))
    n_bin = len(tempi) // passi_bin

    if n_bin == 0:
        plt.close(fig)
        print(f"{nome_regime}: finestra troppo breve per il bin scelto.")
        return

    n_utili = n_bin * passi_bin
    durata_bin = passi_bin * dt

    conteggi = firing[:n_utili].sum(axis=1)
    conteggi_bin = conteggi.reshape(n_bin, passi_bin).sum(axis=1)
    tempi_bin = tempi[:n_utili].reshape(n_bin, passi_bin).mean(axis=1)

    rate = conteggi_bin / (num_neuroni * durata_bin)
    rate_medio = conteggi.sum() / (num_neuroni * n_utili * dt)

    ax2.plot(tempi_bin, rate, color="firebrick", linewidth=1)
    ax2.axhline(
        rate_medio, color="black", linestyle="--",
        label=f"Media nella finestra: {rate_medio:.1f} Hz"
    )
    ax2.set_xlabel("Tempo [s]")
    ax2.set_ylabel("Rate di popolazione [Hz]")
    ax2.legend()

    for ax in (ax1, ax2):
        ax.set_xlim(t_start, t_end)
        ax.grid(True, linestyle="--", alpha=0.3)

    fig.tight_layout()
    output_img = f"Brunel_{nome_regime}.png"
    fig.savefig(output_img, dpi=300)
    plt.close(fig)

    print(
        f"{nome_regime}: rate medio = {rate_medio:.2f} Hz, "
        f"bin = {durata_bin * 1000:.2f} ms. "
        f"Salvato: {output_img}"
    )

if __name__ == "__main__":
    regimi = [
        "SR_SynchronousRegular",
        "AI_AsynchronousIrregular",
        "SI_fast_SynchronousFast",
        "SI_slow_SynchronousSlow"
    ]
    
    for r in regimi:
        analizza_e_plasma(r)