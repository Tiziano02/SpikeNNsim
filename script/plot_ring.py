#!/usr/bin/env python3
"""Grafici per SpikeNNsim: uint32 ncol, seguito da double (tempo SI + dati).

Dipendenze: numpy, matplotlib.
Esempio: python3 plot_ring.py --voltage output/ring_V.bin --firing output/ring_F.bin
Salva PNG e PDF; --no-show per esecuzioni senza finestra grafica.
I potenziali sono quelli registrati: non vengono aggiunti picchi artificiali.
Gli intervalli fra spike sono diagnostici, non una prova generale del motore.
"""
import argparse
from pathlib import Path
import struct
import numpy as np


def load_binary(path):
    """Formato little-endian prodotto da Mac ARM64 / Fedora x86_64."""
    path = Path(path)
    with path.open('rb') as stream:
        header = stream.read(4)
        if len(header) != 4:
            raise ValueError(f'{path}: header incompleto')
        columns = struct.unpack('<I', header)[0]
        if columns < 2:
            raise ValueError(f'{path}: numero di colonne non valido: {columns}')
        payload_bytes = path.stat().st_size - 4
        if payload_bytes == 0 or payload_bytes % (8 * columns):
            raise ValueError(f'{path}: payload vuoto o righe incomplete')
        data = np.fromfile(stream, dtype='<f8').reshape(-1, columns)
    if not np.isfinite(data).all():
        raise ValueError(f'{path}: valori NaN o infiniti')
    if np.any(np.diff(data[:, 0]) <= 0):
        raise ValueError(f'{path}: tempi non strettamente crescenti')
    return data[:, 0] * 1e3, data[:, 1:]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--voltage', type=Path, default=Path('ring_V.bin'))
    parser.add_argument('--firing', type=Path, default=Path('ring_F.bin'))
    parser.add_argument('--out', type=Path, default=Path('grafici/ring'))
    parser.add_argument('--tmin', type=float, default=None, help='Inizio finestra, ms')
    parser.add_argument('--tmax', type=float, default=None, help='Fine finestra, ms')
    parser.add_argument('--delays-ms', type=float, nargs='+', default=None,
                        help='Ritardi configurati 0->1, 1->2, ..., ultimo->0. Per il test: 0 1 2')
    parser.add_argument('--no-show', action='store_true')
    args = parser.parse_args()
    import matplotlib
    if args.no_show:
        matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    tv, volts = load_binary(args.voltage)
    tf, firing = load_binary(args.firing)
    if volts.shape != firing.shape or not np.allclose(tv, tf, rtol=0, atol=1e-9):
        raise ValueError('I due file devono avere stessi neuroni, righe e tempi')
    if not np.isin(firing, [0.0, 1.0]).all():
        raise ValueError('Il file firing deve contenere soltanto 0 e 1')
    n = firing.shape[1]
    if n < 2:
        raise ValueError('Questo grafico richiede almeno due neuroni')
    if args.delays_ms is not None and len(args.delays_ms) != n:
        raise ValueError('--delays-ms richiede un valore per ogni collegamento dell’anello')
    lo = tf[0] if args.tmin is None else args.tmin
    hi = tf[-1] if args.tmax is None else args.tmax
    mask = (tf >= lo) & (tf <= hi)
    if hi <= lo or not mask.any():
        raise ValueError('Finestra temporale vuota o non valida')
    dt = np.median(np.diff(tf)) if len(tf) > 1 else None
    spikes = [tf[firing[:, k] == 1] for k in range(n)]
    colors = [plt.get_cmap('tab10')(k % 10) for k in range(n)]
    plt.rcParams.update({'font.size': 10, 'axes.spines.top': False,
                         'axes.spines.right': False, 'axes.titleweight': 'bold',
                         'savefig.facecolor': 'white', 'figure.facecolor': 'white'})
    fig = plt.figure(figsize=(12, 5.3 + 1.05 * n), layout='constrained')
    grid = fig.add_gridspec(n + 2, 1, height_ratios=[2] + [1] * n + [1.8])
    raster = fig.add_subplot(grid[0])
    raster.eventplot(spikes, colors=colors, lineoffsets=np.arange(n),
                     linelengths=0.65, linewidths=1.8)
    raster.set(xlim=(lo, hi), ylim=(-0.7, n - 0.3),
               yticks=np.arange(n), ylabel='Neurone',
               title='Spike registrati — un impulso percorre l’anello')
    raster.set_yticklabels([str(k) for k in range(n)])
    raster.grid(axis='x', alpha=0.18)
    raster.tick_params(labelbottom=False)
    fig.suptitle('SpikeNNsim | Verifica dell’anello', fontsize=19, fontweight='bold')

    for k in range(n):
        ax = fig.add_subplot(grid[k + 1], sharex=raster)
        vm = volts[:, k] * 1e3
        ax.plot(tv, vm, color=colors[k], linewidth=1.6)
        selected = spikes[k][(spikes[k] >= lo) & (spikes[k] <= hi)]
        ax.plot(selected, np.interp(selected, tv, vm), linestyle='none',
                marker='o', markersize=4, color=colors[k])
        ax.set_ylabel(f'V{k} [mV]')
        ax.grid(alpha=0.18)
        if np.ptp(vm[mask]) < 1e-9:
            center = float(vm[mask][0])
            ax.set_ylim(center - 1, center + 1)
            ax.text(0.99, 0.75, f'Campioni costanti: {center:g} mV',
                    transform=ax.transAxes, ha='right', color='#555555', fontsize=9)
        if k == n - 1:
            ax.set_xlabel('Tempo [ms] — punti: spike registrati, senza picchi ricostruiti')
        else:
            ax.tick_params(labelbottom=False)

    intervals = fig.add_subplot(grid[-1])
    print(f'{len(tf)} campioni, {n} neuroni; dt = {dt} ms')
    for k, times in enumerate(spikes):
        print(f'Neurone {k}: spike [ms] = {np.round(times, 6).tolist()}')
        if len(times) > 1:
            print(f'  Intervalli propri [ms]: {np.round(np.diff(times), 6).tolist()}')
        target = spikes[(k + 1) % n]
        # Associa ciascuno spike al primo successivo del prossimo neurone.
        # È una misura temporale; non dimostra causalità in reti generiche.
        origins = times[(times >= lo) & (times <= hi)]
        idx = np.searchsorted(target, origins, side='right')
        valid = idx < len(target)
        arrivals = target[idx[valid]]
        gaps = (arrivals - origins[valid])[arrivals <= hi]
        intervals.scatter(np.full(gaps.size, k), gaps, s=45,
                          color=colors[k], alpha=0.7, zorder=3)
        if gaps.size:
            intervals.annotate(f'{np.median(gaps):.3g} ms',
                               (k, np.median(gaps)), xytext=(12, 6),
                               textcoords='offset points', color=colors[k])
            print(f'{k} -> {(k+1)%n}: intervalli [ms] = {np.round(gaps, 6).tolist()}')
    if args.delays_ms is not None and dt is not None:
        # Convenzione concordata: spike letto nella chiamata seguente;
        # vale se ritardi multipli di dt e risposta postsinaptica immediata.
        expected = np.asarray(args.delays_ms) + dt
        intervals.scatter(np.arange(n), expected, marker='_', s=650,
                          color='#202938', linewidth=2, label='delay configurato + dt')
        intervals.legend(frameon=False, loc='upper left')
    intervals.set(xticks=np.arange(n), xlim=(-0.5, n - 0.5),
                  ylabel='Intervallo [ms]',
                  title='Distanza dal primo spike successivo del neurone seguente')
    intervals.set_xticklabels([f'{k} → {(k+1)%n}' for k in range(n)])
    intervals.grid(axis='y', alpha=0.2)
    intervals.set_ylim(bottom=0)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    for ext in ('png', 'pdf'):
        destination = args.out.parent / f'{args.out.name}.{ext}'
        fig.savefig(destination, dpi=200)
        print(f'Salvato: {destination}')
    if not args.no_show:
        plt.show()
    plt.close(fig)


if __name__ == '__main__':
    main()
