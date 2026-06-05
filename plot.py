import numpy as np
import matplotlib.pyplot as plt

# ============================================================
# CARICAMENTO DATI
# ============================================================

potenziali = np.loadtxt("potenziali.txt")
firing = np.loadtxt("firing.txt")
sinapsi = np.loadtxt("sinapsi.txt")

# ============================================================
# TEMPO
# ============================================================

time = potenziali[:, 0]

# ============================================================
# DATI NEURONI
# ============================================================

# colonne 1: = neuroni

V_matrix = potenziali[:, 1:]
spike_matrix = firing[:, 1:]
I_matrix = sinapsi[:, 1:]

# numero neuroni
N = V_matrix.shape[1]

# ============================================================
# POTENZIALE MEDIO
# ============================================================

V_mean = np.mean(V_matrix, axis=1)

# ============================================================
# FIRING RATE MEDIO
# ============================================================

dt = time[1] - time[0]

# spike per timestep
spikes_per_step = np.sum(spike_matrix, axis=1)

# firing rate istantaneo (Hz)
firing_rate = spikes_per_step / N / (dt / 1000.0)

# smoothing
window = 50

kernel = np.ones(window) / window

firing_rate_smooth = np.convolve(
    firing_rate,
    kernel,
    mode='same'
)

# ============================================================
# RASTER PLOT
# ============================================================

spike_times = []
spike_neurons = []

n_steps = len(time)

for t_idx in range(n_steps):

    for neuron_id in range(N):

        if spike_matrix[t_idx, neuron_id] > 0:

            spike_times.append(time[t_idx])
            spike_neurons.append(neuron_id)

# ============================================================
# PLOT
# ============================================================

fig, axs = plt.subplots(
    6,
    1,
    figsize=(16, 14)
)

# ============================================================
# 1) Raster plot
# ============================================================

axs[0].scatter(
    spike_times,
    spike_neurons,
    s=2,
    color='black'
)

axs[0].set_title("Raster plot")
axs[0].set_ylabel("Neuron")

# ============================================================
# 2) Potenziali di membrana
# ============================================================

for nid in range(N):
    axs[1].plot(time, V_matrix[:, nid])

axs[1].set_title("Membrane potentials")
axs[1].set_ylabel("V (mV)")

# ============================================================
# 3) Spike trains
# ============================================================

for nid in range(N):

    spikes_n = []

    for t_idx in range(n_steps):

        if spike_matrix[t_idx, nid] > 0:
            spikes_n.append(time[t_idx])

    axs[2].vlines(
        spikes_n,
        nid + 0.5,
        nid + 1.5
    )

axs[2].set_title("Spike trains")
axs[2].set_ylabel("Neuron")

# ============================================================
# 4) Correnti sinaptiche
# ============================================================

for nid in range(N):
    axs[3].plot(time, I_matrix[:, nid])

axs[3].set_title("Synaptic currents")
axs[3].set_ylabel("I (nA)")

# ============================================================
# 5) Potenziale medio
# ============================================================

axs[4].plot(time, V_mean)

axs[4].set_title("Mean membrane potential")
axs[4].set_ylabel("<V> (mV)")

# ============================================================
# 6) Firing rate medio
# ============================================================

axs[5].plot(time, firing_rate_smooth)

axs[5].set_title("Population firing rate")
axs[5].set_ylabel("Rate (Hz)")
axs[5].set_xlabel("Time (ms)")

# ============================================================
# LAYOUT
# ============================================================

plt.tight_layout()
plt.show()
