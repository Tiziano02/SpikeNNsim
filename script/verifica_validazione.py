#!/usr/bin/env python3
"""
script/verifica_validazione.py
================================================================================
Suite di verifica analitica automatica per il simulatore SpikeNNsim.

Include:
  1. ANALISI RETE ANULARE IBRIDA (Benchmark Periodo e Hop Delay)
  2. ANALISI DINAMICA ECCITATORI / INIBITORI (Convenzione Segni e Shunting Inhibition)
================================================================================
"""

import struct
import os
import sys

def leggi_file_binario(filepath):
    """Legge un file binario generato da Simulazione.cpp (tempo + colonne dati)."""
    if not os.path.exists(filepath):
        print(f"[ERRORE] File non trovato: {filepath}")
        return None, None

    with open(filepath, "rb") as f:
        raw = f.read()

    if len(raw) < 4:
        print(f"[ERRORE] File troppo piccolo: {filepath}")
        return None, None

    ncols = struct.unpack_from("<i", raw, 0)[0]
    total_doubles = len(raw[4:]) // 8
    n_steps = total_doubles // ncols

    tempi = []
    canali = [[] for _ in range(ncols - 1)]

    for step in range(n_steps):
        offset = 4 + step * ncols * 8
        vals = struct.unpack_from(f"<{ncols}d", raw, offset)
        tempi.append(vals[0] * 1000.0) # ms
        for ch in range(ncols - 1):
            canali[ch].append(vals[1 + ch])

    return tempi, canali

def analizza_spike(tempi, canali_firing):
    """Estrae i tempi di spike (in ms) per ciascun neurone."""
    spikes_per_neurone = {}
    for n, canale in enumerate(canali_firing):
        spike_times = [tempi[i] for i, v in enumerate(canale) if v > 0.5]
        spikes_per_neurone[n] = spike_times
    return spikes_per_neurone

def stampa_ascii_raster(spikes_per_neurone, t_max=35.0, larghezza=65):
    """Stampa un raster plot in caratteri ASCII direttamente su terminale."""
    print("\n  Raster Plot degli Spike (ASCII):")
    print("  " + "-" * (larghezza + 16))
    for n in sorted(spikes_per_neurone.keys()):
        riga = [" "] * larghezza
        for t in spikes_per_neurone[n]:
            if t <= t_max:
                pos = int((t / t_max) * (larghezza - 1))
                riga[pos] = "|"
        ruolo = ""
        if n == 4: ruolo = " [Trigger]"
        elif n == 5: ruolo = " [Readout]"
        elif n in (0, 1, 2, 3): ruolo = f" [Ring P{n}]"
        print(f"  N{n}{ruolo:<12}: {''.join(riga)}")
    print("  " + "-" * (larghezza + 16))
    print(f"  0.0 ms{' ' * (larghezza - 12)}{t_max:.1f} ms\n")

def test_benchmark_anello():
    print("\n" + "=" * 78)
    print("  PARTE 1: VERIFICA RETE ANULARE IBRIDA (Popolazione + Singoli + Delay)")
    print("=" * 78)

    filepath_f = "output/validazione_F.bin"
    tempi, canali_f = leggi_file_binario(filepath_f)
    if tempi is None:
        return False

    spikes = analizza_spike(tempi, canali_f)

    print(f"\n[DATI ESTRATTI] Neuroni: {len(spikes)}, Durata: {tempi[-1]:.2f} ms")
    for n in sorted(spikes.keys()):
        times_str = ", ".join(f"{t:.2f}" for t in spikes[n])
        print(f"  - Neurone {n}: {len(spikes[n])} spike(s) a t = [{times_str}] ms")

    stampa_ascii_raster(spikes, t_max=tempi[-1])

    successi = 0
    totale = 5

    # Test 1.1: Trigger
    t_trig = spikes.get(4, [])
    if len(t_trig) == 1 and (1.0 <= t_trig[0] <= 2.5):
        print(f"  [PASS] 1.1 Trigger (N4): 1 solo spike a {t_trig[0]:.2f} ms (atteso in [1.0, 2.5] ms)")
        successi += 1
    else:
        print(f"  [FAIL] 1.1 Trigger anomalo: {t_trig}")

    # Test 1.2: Oscillazione periodica anello
    spikes_anello = [spikes.get(i, []) for i in range(4)]
    conteggi = [len(s) for s in spikes_anello]
    if all(c >= 3 for c in conteggi):
        print(f"  [PASS] 1.2 Popolazione anello: 4 neuroni oscillano periodicamente ({conteggi} spikes)")
        successi += 1
    else:
        print(f"  [FAIL] 1.2 Conteggi spike dell'anello non uniformi: {conteggi}")

    # Test 1.3: Ritardo hop inter-nodo
    if all(len(s) >= 1 for s in spikes_anello):
        d01 = spikes_anello[1][0] - spikes_anello[0][0]
        d12 = spikes_anello[2][0] - spikes_anello[1][0]
        d23 = spikes_anello[3][0] - spikes_anello[2][0]
        diffs = [d01, d12, d23]
        media_hop = sum(diffs) / len(diffs)
        if all(1.90 <= d <= 2.30 for d in diffs):
            print(f"  [PASS] 1.3 Ritardo hop inter-nodo delta_t = {media_hop:.2f} ms (atteso ~2.10 ms)")
            successi += 1
        else:
            print(f"  [FAIL] 1.3 Ritardi hop fuori tolleranza: {diffs}")
    else:
        print("  [FAIL] 1.3 Dati insufficienti per calcolo hop.")

    # Test 1.4: Periodo anello
    periodi = [spikes_anello[0][i+1] - spikes_anello[0][i] for i in range(len(spikes_anello[0]) - 1)]
    if periodi and all(8.20 <= p <= 8.60 for p in periodi):
        media_p = sum(periodi) / len(periodi)
        print(f"  [PASS] 1.4 Periodo anello T = {media_p:.2f} ms (atteso 4 * 2.10 = 8.40 ms, tolleranza 0.2 ms)")
        successi += 1
    else:
        print(f"  [FAIL] 1.4 Periodo non costante: {periodi}")

    # Test 1.5: Readout
    spikes_readout = spikes.get(5, [])
    if len(spikes_readout) == len(spikes_anello[2]) and len(spikes_readout) >= 3:
        ritardi_ro = [spikes_readout[i] - spikes_anello[2][i] for i in range(len(spikes_readout))]
        media_ro = sum(ritardi_ro) / len(ritardi_ro)
        if all(2.00 <= r <= 2.30 for r in ritardi_ro):
            print(f"  [PASS] 1.5 Readout N5 (Exp + Conductance): sincrono a N2 con ritardo costante {media_ro:.2f} ms")
            successi += 1
        else:
            print(f"  [FAIL] 1.5 Ritardo readout fuori range: {ritardi_ro}")
    else:
        print(f"  [FAIL] 1.5 Readout spike count anomalo ({len(spikes_readout)}) vs N2 ({len(spikes_anello[2])})")

    return successi == totale

def test_eccitatori_inibitori():
    print("\n" + "=" * 78)
    print("  PARTE 2: VERIFICA DINAMICA ECCITATORI / INIBITORI E CONVENZIONE DEI SEGNI")
    print("=" * 78)

    filepath_f = "output/verifica_ei_F.bin"
    filepath_v = "output/verifica_ei_V.bin"

    tempi_f, canali_f = leggi_file_binario(filepath_f)
    tempi_v, canali_v = leggi_file_binario(filepath_v)

    if tempi_f is None or tempi_v is None:
        return False

    spikes = analizza_spike(tempi_f, canali_f)

    # Calcolo Vmin e Vmax per ciascun neurone (in mV)
    v_min = [min(ch) * 1000.0 for ch in canali_v]
    v_max = [max(ch) * 1000.0 for ch in canali_v]

    nomi_ruoli = {
        0: "Target 0 (Current Eccitatorio, peso < 0)",
        1: "Target 1 (Current Inibitorio, peso > 0)",
        2: "Target 2 (Misto: Eccitazione + Inibizione Shunting)",
        3: "Target 3 (Conductance Inibitorio, Erev = -75 mV)",
        4: "PreExc 4 (Generatore di spike eccitatorio)",
        5: "PreInh 5 (Generatore di spike inibitorio)"
    }

    print("\n[PROFILO DI VOLTAGGIO E FIRING]")
    for i in range(len(canali_f)):
        ruolo = nomi_ruoli.get(i, "")
        n_spk = len(spikes.get(i, []))
        print(f"  - N{i} [{ruolo}]:")
        print(f"      Spike count: {n_spk} | V_min: {v_min[i]:.2f} mV | V_max: {v_max[i]:.2f} mV")

    successi = 0
    totale = 4

    # TEST 2.1: Eccitazione Current (peso < 0) genera spike
    if len(spikes.get(0, [])) >= 1:
        print(f"\n  [PASS] 2.1 Eccitazione Current (peso = -1.0): Neurone 0 ha depolarizzato e SPARATO ({len(spikes[0])} spike)")
        successi += 1
    else:
        print(f"\n  [FAIL] 2.1 Eccitazione non ha prodotto spike sul bersaglio 0")

    # TEST 2.2: Inibizione Current (peso > 0) iperpolarizza e non spara
    if len(spikes.get(1, [])) == 0 and v_min[1] < -66.0:
        print(f"  [PASS] 2.2 Inibizione Current (peso = +1.0): Neurone 1 IPERPOLARIZZATO a {v_min[1]:.2f} mV (< Vrest -65mV), 0 spike")
        successi += 1
    else:
        print(f"  [FAIL] 2.2 Inibizione Current non ha iperpolarizzato correttamente: Vmin = {v_min[1]:.2f} mV")

    # TEST 2.3: Shunting Inhibition (Eccitatorio + Inibitorio sincrono sopprime lo spike)
    if len(spikes.get(2, [])) == 0:
        print(f"  [PASS] 2.3 Shunting Inhibition: Neurone 2 (PreExc + PreInh) NON ha sparato! L'inibizione ha cancellato l'eccitazione")
        successi += 1
    else:
        print(f"  [FAIL] 2.3 Shunting fallito: Neurone 2 ha sparato nonostante l'inibizione sincrona")

    # TEST 2.4: Conductance Inibitorio (Erev = -75 mV) iperpolarizza verso Erev
    if len(spikes.get(3, [])) == 0 and v_min[3] < -67.0:
        print(f"  [PASS] 2.4 Inibizione Conductance (Erev = -75 mV): Neurone 3 iperpolarizzato a {v_min[3]:.2f} mV verso Erev, 0 spike")
        successi += 1
    else:
        print(f"  [FAIL] 2.4 Inibizione Conductance anomala: Vmin = {v_min[3]:.2f} mV")

    return successi == totale

def test_confronto_analitico_vs_eulero():
    print("\n" + "=" * 78)
    print("  PARTE 3: VERIFICA EQUIVALENZA FISICA ANALITICO ('A') vs EULERO ('E')")
    print("=" * 78)

    filepath_v = "output/confronto_ae_V.bin"
    filepath_f = "output/confronto_ae_F.bin"
    filepath_s = "output/confronto_ae_S.bin"

    tempi_v, canali_v = leggi_file_binario(filepath_v)
    tempi_f, canali_f = leggi_file_binario(filepath_f)
    tempi_s, canali_s = leggi_file_binario(filepath_s)

    if tempi_v is None or tempi_f is None or tempi_s is None:
        return False

    spikes = analizza_spike(tempi_f, canali_f)

    spk_A = spikes.get(0, [])
    spk_E = spikes.get(1, [])

    # Differenza massima potenziale prima degli spike (in mV)
    v_diff = [abs(v_a - v_e) * 1000.0 for v_a, v_e in zip(canali_v[0], canali_v[1])]
    max_v_diff = max(v_diff)

    # Differenza massima corrente sinaptica (in nA)
    s_diff = [abs(s_a - s_e) * 1e9 for s_a, s_e in zip(canali_s[0], canali_s[1])]
    max_s_diff = max(s_diff)

    print(f"\n[CONFRONTO NUMERICO]")
    print(f"  - Spike N0 (Analitico): {len(spk_A)} spike a t = {['%.2f' % t for t in spk_A]} ms")
    print(f"  - Spike N1 (Eulero):    {len(spk_E)} spike a t = {['%.2f' % t for t in spk_E]} ms")
    print(f"  - Max Delta V di membrana: {max_v_diff:.4f} mV (tolleranza < 0.15 mV)")
    print(f"  - Max Delta I sinaptica:   {max_s_diff:.4f} nA (tolleranza < 0.05 nA)")

    successi = 0
    totale = 4

    # TEST 3.1: Conteggio spike identico
    if len(spk_A) == len(spk_E) and len(spk_A) > 0:
        print(f"\n  [PASS] 3.1 Conteggio spike identico: entrambi hanno sparato esattamente {len(spk_A)} volte")
        successi += 1
    else:
        print(f"\n  [FAIL] 3.1 Conteggio spike divergente: A={len(spk_A)} vs E={len(spk_E)}")

    # TEST 3.2: Timing spike identico
    if len(spk_A) == len(spk_E):
        diff_tempi = [abs(ta - te) for ta, te in zip(spk_A, spk_E)]
        max_dt = max(diff_tempi) if diff_tempi else 0.0
        if max_dt < 1e-4:
            print(f"  [PASS] 3.2 Timing spike coincidente: delta t massimo = {max_dt:.6f} ms (zero jitter)")
            successi += 1
        else:
            print(f"  [FAIL] 3.2 Timing spike sfalsato: {diff_tempi}")
    else:
        print("  [FAIL] 3.2 Impossibile confrontare i tempi di spike (conteggi diversi)")

    # TEST 3.3: Fedeltà traiettoria potenziale
    if max_v_diff < 0.15:
        print(f"  [PASS] 3.3 Traiettoria potenziale equivalente: deviazione massima = {max_v_diff:.4f} mV (< 0.15 mV)")
        successi += 1
    else:
        print(f"  [FAIL] 3.3 Deviazione potenziale eccessiva: {max_v_diff:.4f} mV")

    # TEST 3.4: Fedeltà decadimento sinaptico
    if max_s_diff < 0.05:
        print(f"  [PASS] 3.4 Decadimento sinaptico equivalente: deviazione massima = {max_s_diff:.4f} nA (< 0.05 nA)")
        successi += 1
    else:
        print(f"  [FAIL] 3.4 Deviazione corrente sinaptica eccessiva: {max_s_diff:.4f} nA")

    return successi == totale

def main():
    print("\n" + "#" * 78)
    print("   SUITE DI VERIFICA COMPLETA DEL SIMULATORE NEURONALE SpikeNNsim")
    print("#" * 78)

    ok1 = test_benchmark_anello()
    ok2 = test_eccitatori_inibitori()
    ok3 = test_confronto_analitico_vs_eulero()

    print("\n" + "=" * 78)
    if ok1 and ok2 and ok3:
        print("  CONFERMA FINALE: TUTTI I TEST FISICI E ANALITICI SONO STATI SUPERATI CON SUCCESSO!")
        print("  - Convenzione dei segni (Current e Conductance):     VERIFICATA")
        print("  - Dinamica Eccitatoria / Inibitoria / Shunting:      VERIFICATA")
        print("  - Delay Ring Buffer e Trasmissione:                 VERIFICATA")
        print("  - Popolazioni e Neuroni Singoli Ibridi:             VERIFICATA")
        print("  - Equivalenza Fisica Soluzione Analitica vs Eulero: VERIFICATA")
    else:
        print("  ATTENZIONE: Uno o piu' test hanno riscontrato anomalie.")
    print("=" * 78 + "\n")

if __name__ == "__main__":
    main()
