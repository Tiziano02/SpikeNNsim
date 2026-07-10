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
 * TypeNeuron – variant che può contenere LIF o Exp.
 * È il tipo effettivo memorizzato nel vettore neuroni_ di Rete.
 */
using TypeNeuron = std::variant<LIF, Exp>;
/**
 * TypeConfig – variant che può contenere configLIF o configExp.
 * Usato per passare la configurazione a Rete::aggiungiNeurone() e
 * Rete::modificaParametriNeurone().
 */
using TypePatchNeuron = std::variant<patchLIF, patchExp>;

#endif // NEURONE_HPP