#ifndef SINAPSI_HPP
#define SINAPSI_HPP

#include "CurrentSyn.hpp"
#include "ConductanceSyn.hpp"
#include <variant>

// ============================================================================
// ENUM E ALIAS (PUBBLICI)
// ============================================================================

/**
 * Label_Type_Syn – tipo di sinapsi (current‑based o conductance‑based).
 */
enum class SynapseModel { Current, Conductance };
/**
 * TypeSyn – variant che può contenere CurrentSyn o ConductanceSyn.
 * È il tipo effettivo memorizzato nel vettore sinapsi_ di Rete.
 */
using TypeSyn = std::variant<Current, Conductance>;

/**
 * TypeConfigSyn – variant che può contenere configCurrentSyn o configConductanceSyn.
 * Usato per passare la configurazione a Rete::connettiNeuroni() e
 * Rete::modificaSinapsi().
 */
using TypePatchSyn = std::variant<patchCurrent, patchConductance>;

#endif // SINAPSI_HPP