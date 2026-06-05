# Architecture

This document describes the high-level architecture of HeaderRN and the interaction between its main components.

The goal is to provide an overview of the framework design without entering implementation details.

---

# Design Overview

HeaderRN is organized around five main concepts:

* `Neurone`
* `Sinapsi`
* `Rete`
* `Simulazione`
* `Input`

Each component has a single well-defined responsibility.

This separation keeps the framework modular, easy to understand and extensible.

---

# Main Components

## Neurone

The `Neurone` class represents a single spiking neuron.

Current implementation:

* Leaky Integrate-and-Fire (LIF) model
* Absolute refractory period
* Explicit biological parameters
* State update through numerical integration

### Responsibilities

The neuron is responsible for:

- storing its internal state
- updating its membrane potential
- managing the refractory period
- generating spikes

### Knows

The neuron knows:

- its membrane potential
- its biological parameters
- its refractory state

### Does Not Know

The neuron does not know:

- the network topology
- other neurons
- synaptic connections
- simulation time

---

## Sinapsi

The `Sinapsi` class represents a connection between two neurons.

Current implementation:

* Current-based synapse
* Exponential current decay
* Positive weights for excitatory connections
* Negative weights for inhibitory connections


### Responsibilities

The synapse is responsible for:

- propagating activity between neurons
- generating synaptic current
- storing synaptic parameters

### Knows

The synapse knows:

- presynaptic neuron id
- postsynaptic neuron id
- synaptic weight
- synaptic state variables

### Does Not Know

The synapse does not know:

- global network structure
- simulation management
- external inputs
---

## Rete

The `Rete` class stores the network structure.

The network is represented as:

* a collection of neurons
* a collection of synaptic connections

rather than by an adjacency matrix.

This representation simplifies future extensions such as:

* transmission delays
* plasticity mechanisms
* state-dependent synapses

### Responsibilities

The network is responsible for:

- storing neurons
- storing synaptic connections
- managing connectivity

### Knows

The network knows:

- all neurons
- all synapses
- connectivity relationships

### Does Not Know

The network does not know:

- simulation duration
- output files
- data analysis

---

## Input

The `Input` class represents external stimuli applied to neurons.

Inputs may vary over time and can be assigned independently to different neurons.

This allows the simulation of:

* transient stimuli
* constant currents
* arbitrary time-dependent signals

### Responsibilities

The input object is responsible for:

- storing an external stimulus
- associating the stimulus with a target neuron

### Knows

The input knows:

- target neuron id
- stimulus values

### Does Not Know

The input does not know:

- network topology
- neuron dynamics
- simulation logic

---

## Simulazione

The `Simulazione` class coordinates the simulation process.

### Responsibilities

The simulation is responsible for:

- managing simulation time
- updating network dynamics
- applying external inputs
- storing simulation outputs


### Knows

The simulation knows:

- the network
- the simulation parameters
- the external inputs
- output destinations

### Does Not Know

The simulation does not know:

- the internal implementation details of neurons
- the internal implementation details of synapses

---

# Simulation Workflow

A simulation typically follows the sequence below:

1. Create a network.
2. Add neurons to the network.
3. Create synaptic connections.
4. Create a simulation object.
5. Define external inputs.
6. Run the simulation.
7. Export simulation data.

---

# Current Scope

HeaderRN currently focuses on:

* small and medium-sized spiking neural networks
* educational applications
* rapid prototyping
* experimentation with neural dynamics

The framework prioritizes clarity and extensibility over aggressive optimization.

---

# Future Extensions

The current architecture was designed to support future additions, including:

* transmission delays
* synaptic plasticity
* alternative neuron models
* sparse network optimizations
* large-scale simulations

These extensions can be introduced while preserving the existing high-level structure.
