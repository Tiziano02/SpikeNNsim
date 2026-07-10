/**
 * @file Neurone.hpp
 * @brief Registro dei modelli neurali e definizione dei tipi polimorfi.
 * * Raggruppa tutti i modelli di neuroni disponibili in SpikeNNsim
 * e definisce le varianti (std::variant) utilizzate dal motore di simulazione.
 */

#ifndef NEURONE_HPP
#define NEURONE_HPP

#include "LIF.hpp"
#include "Exp.hpp"
#include <variant>

/**
 * @ingroup publicapi
 * @brief Etichette identificative dei modelli neurali supportati.
 * @details Utilizzato dall'utente nella funzione `Rete::aggiungiNeurone()`.
 */
enum class NeuronModel {
    LIF, ///< Leaky Integrate-and-Fire standard
    Exp  // /< Integrate-and-Fire Esponenziale (AdEx semplificato)
};

/**
 * @ingroup internals
 * @brief Tipo polimorfo che rappresenta fisicamente un neurone in memoria.
 * @details È il tipo effettivo memorizzato nel vettore contiguo `neuroni_` di `Rete`.
 */
using TypeNeuron = std::variant<LIF, Exp>;

/**
 * @ingroup publicapi
 * @brief Tipo polimorfo per l'aggiornamento dei parametri neurali.
 * @details Permette alla funzione `Rete::modificaParametriNeurone()` di accettare
 * dinamicamente la Patch corretta per il neurone bersaglio.
 */
using TypePatchNeuron = std::variant<patchLIF, patchExp>;

#endif // NEURONE_HPP