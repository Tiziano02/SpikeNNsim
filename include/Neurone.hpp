#ifndef NEURONE_HPP
#define NEURONE_HPP

#include "LIF.hpp"
#include "Exp.hpp"
#include <variant>

// ============================================================================
// ENUM E ALIAS (PUBBLICI)
// ============================================================================

/**
 * NeuronModel – modelli di neurone supportati.
 */
enum class NeuronModel { LIF, Exp };

/**
 * TypeConfig – variant che può contenere configLIF o configExp.
 * Usato per passare la configurazione a Rete::aggiungiNeurone() e
 * Rete::modificaParametriNeurone().
 */
using TypeConfig = std::variant<configLIF, configExp>;

/**
 * TypeNeuron – variant che può contenere LIF o Exp.
 * È il tipo effettivo memorizzato nel vettore neuroni_ di Rete.
 */
using TypeNeuron = std::variant<LIF, Exp>;

#endif // NEURONE_HPP