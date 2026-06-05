# HeaderRN

HeaderRN is a header-only C++ framework for simulating spiking neural networks.

The library provides a modular architecture for building networks of neurons and synapses, running time-domain simulations, and exporting data for external analysis.

HeaderRN was originally developed as a personal project to understand how spiking neural network simulations work at a low level.

Instead of relying on existing simulation frameworks, the goal was to build the fundamental components from scratch, including neurons, synapses, network topology and simulation management.

The project later evolved into a reusable framework aimed at experimentation, education and future computational neuroscience applications.

Its design is guided by three main principles:

- clarity and educational value;
- simplicity of use;
- extensibility toward more advanced neural models.

## Features

Current features include:


- Leaky Integrate-and-Fire neurons
- Current-based synapses with exponential decay
- Time-dependent external inputs
- Custom network topologies
- Explicit physical units system
- Time-domain simulations
- Data export for post-processing
- Modular object-oriented architecture

## Design Philosophy

HeaderRN follows a simple object-oriented design.

The main components of a simulation are represented by dedicated classes:

- `Rete`
- `Neurone`
- `Sinapsi`
- `Simulazione`
- `Input`

This separation aims to keep the code readable, extensible, and suitable both for educational purposes and small research projects.

---

## Library Structure

```text
Neurone.hpp      -> neuron models
Sinapsi.hpp      -> synaptic models
Rete.hpp         -> network topology
Simulazione.hpp  -> simulation engine
Input.hpp        -> external stimuli
UnitaSI.hpp      -> physical units
```

## Example

- coming soon

A minimal simulation consists of:

1. Creating a network.
2. Adding neurons.
3. Connecting neurons through synapses.
4. Creating a simulation object.
5. Adding external inputs.
6. Running the simulation.

## Installation

HeaderRN is distributed as a header-only library.

Clone the repository:

```bash
git clone https://github.com/Tiziano02/HeaderRN.git
```
Include the headers in your project and compile with a C++17 compatible compiler.

## Roadmap

Future developments include:

- [ ] Transmission delays
- [ ] Synaptic plasticity
- [ ] Sparse connectivity
- [ ] Additional neuron models
- [ ] Performance optimizations
- [ ] Large-scale simulations

A complete roadmap is available in TODO.md.

## License

This project is distributed under the MIT License.

See [LICENSE](LICENSE) for details.