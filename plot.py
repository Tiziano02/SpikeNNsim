import matplotlib.pyplot as plt
import numpy as np

# Load data from potenziali.txt and firing.txt
pot_data = np.loadtxt('potenziali.txt')
fire_data = np.loadtxt('firing.txt')
syn_data = np.loadtxt('sinapsi.txt')

# First column is time, remaining columns are sorted by neuron
time_pot = pot_data[:, 0]
potentials = pot_data[:, 1:]

time_fire = fire_data[:, 0]
spikes = fire_data[:, 1:]

time_syn = syn_data[:, 0]
synapses = syn_data[:, 1:]

if spikes.ndim == 1:
    spikes = spikes[:, np.newaxis]

n_neurons = spikes.shape[1]
spike_times = [time_fire[spikes[:, neuron_idx] == 1] for neuron_idx in range(n_neurons)]

# Average membrane potential and average firing rate over time
avg_potential = np.mean(potentials, axis=1)
avg_synapse = np.mean(synapses, axis=1)
mean_spikes = np.mean(spikes, axis=1)

dt = np.mean(np.diff(time_fire)) if len(time_fire) > 1 else 1.0
if dt == 0:
    dt = 1.0

# Assumes time in milliseconds; convert dt to seconds for Hz
mean_firing_rate = mean_spikes / (dt / 1000.0)


fig, axes = plt.subplots(4, 1, figsize=(12, 10), sharex=True)

# Raster plot
axes[0].eventplot(spike_times, colors='black', linelengths=0.8)
axes[0].set_ylabel('Neuron')
axes[0].set_yticks([])
axes[0].set_title('Raster plot of neural spikes')

# Average membrane potential
axes[1].plot(time_pot, avg_potential, color='tab:blue')
axes[1].set_ylabel('Avg membrane potential (mV)')
axes[1].grid(True, alpha=0.3)

# Mean firing rate
axes[2].plot(time_fire, mean_firing_rate, color='tab:red')
axes[2].set_ylabel('Mean firing rate (Hz)')
axes[2].set_xlabel('Time (ms)')
axes[2].grid(True, alpha=0.3)

# Mean synaptic value
axes[3].plot(time_syn, avg_synapse, color='tab:green')
axes[3].set_ylabel('Mean synaptic value')
axes[3].set_xlabel('Time (ms)')
axes[3].grid(True, alpha=0.3)
plt.tight_layout()
plt.show()