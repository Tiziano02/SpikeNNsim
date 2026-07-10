/**
 * @file Input.hpp
 * @brief Definizione degli stimoli esterni (correnti iniettabili) per la rete.
 * * Fornisce le strutture per creare correnti nel dominio del tempo
 * (costanti o sinusoidali) da applicare a specifici neuroni bersaglio.
 */

#ifndef INPUT_HPP
#define INPUT_HPP

#include <variant>
#include <cstddef>

// -- PARAMETRI DEGLI STIMOLI ------------------------------------------------------------------------------

/**
 * @ingroup publicapi
 * @brief Generatore di corrente continua (DC) per una finestra temporale.
 */
struct configConstantStimulus {
    double timeStart; ///< [s] Istante di accensione dello stimolo
    double timeEnd;   ///< [s] Istante di spegnimento dello stimolo
    double ampiezza;  ///< [A] Intensità della corrente iniettata
};

/**
 * @ingroup publicapi
 * @brief Generatore di corrente alternata (AC) sinusoidale.
 * @details Equazione: I(t) = ampiezza * sin(2π * frequenza * t + fase)
 */
struct configSinStimulus {
    double timeStart; ///< [s] Istante di accensione
    double timeEnd;   ///< [s] Istante di spegnimento
    double ampiezza;  ///< [A] Ampiezza massima dell'oscillazione
    double frequenza; ///< [Hz] Frequenza dell'onda
    double fase;      ///< [rad] Fase iniziale dell'onda
};

/**
 * @ingroup publicapi
 * @brief Variante polimorfa per uniformare il passaggio dei parametri degli stimoli.
 */
using typeParameters = std::variant<configConstantStimulus, configSinStimulus>;

// -- STRUTTURE DI ROUTING DEGLI STIMOLI ----------------------------------------------------------

/**
 * @ingroup publicapi
 * @brief Struttura utente per mappare uno stimolo a un neurone.
 * @details L'utente crea vettori di questo tipo per passarli a `Simulazione::iniettaStimoli()`.
 */
struct stimolo {
    int IDneurone;            ///< ID pubblico del neurone bersaglio
    typeParameters parametri; ///< Forma d'onda e parametri della corrente
};

/**
 * @ingroup internals
 * @brief Struttura interna (Cache-friendly) per l'elaborazione degli stimoli.
 * @details Traduce l'ID pubblico in un indice di vettore diretto, evitando
 * lente ricerche in mappa durante il loop temporale.
 */
struct recordStimolo {
    size_t indexNeurone;      ///< Indice diretto nel vettore neuroni_
    typeParameters parametri; ///< Forma d'onda e parametri della corrente
};

#endif // INPUT_HPP