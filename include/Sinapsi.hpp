/**
 * @file Sinapsi.hpp
 * @brief Registro dei modelli sinaptici e definizione dei tipi polimorfi.
 * * Raggruppa i modelli di sinapsi supportati in SpikeNNsim e
 * definisce le relative varianti per l'allocazione e l'aggiornamento.
 */

#ifndef SINAPSI_HPP
#define SINAPSI_HPP

#include "CurrentSyn.hpp"
#include "ConductanceSyn.hpp"
#include <variant>

/**
 * @ingroup publicapi
 * @brief Etichette identificative della fisica sinaptica supportata.
 * @details Utilizzato dall'utente nella funzione `Rete::connettiNeuroni()`.
 */
enum class SynapseModel {
    Current,    ///< Sinapsi basata sull'iniezione di corrente (I_syn)
    Conductance ///< Sinapsi basata sulla variazione di conduttanza (g_syn)
};

/**
 * @ingroup internals
 * @brief Tipo polimorfo che rappresenta fisicamente una sinapsi in memoria.
 * @details È il tipo effettivo memorizzato nel vettore contiguo `sinapsi_` di `Rete`.
 */
using TypeSyn = std::variant<Current, Conductance>;

/**
 * @ingroup publicapi
 * @brief Tipo polimorfo per l'aggiornamento dei parametri sinaptici.
 * @details Utilizzato dalla funzione `Rete::modificaSinapsi()`.
 */
using TypePatchSyn = std::variant<patchCurrent, patchConductance>;

#endif // SINAPSI_HPP