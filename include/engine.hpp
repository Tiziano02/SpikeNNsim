/**
 * @file engine.hpp
 * @brief Gestore  dell'evoluzione temporale della SpikeNNsim.
 * * Questo file definisce la classe engine, il motore centrale del simulatore.
 * Si occupa di allocare i neurno e le sinapsi in un formato ottimizzato per la dinamica
 */

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Rete.hpp"
#include <cstddef>

/**
 * @ingroup publicapi
 * @brief Classe principale per la costruzione e simulazione della rete neurale.
 *
 * @details
 * L'utente interagisce con `Rete` esclusivamente attraverso i metodi pubblici per:
 * - Aggiungere popolazioni di neuroni (LIF, Exp).
 * - Connettere i neuroni tramite sinapsi (Current-based, Conductance-based).
 * - Modificare i parametri in corso d'opera tramite i pacchetti "Patch".
 * * @note L'evoluzione temporale (`step()`) e la preparazione (`prepare()`)
 * sono gestite internamente dalla classe `Simulazione` (friend class).
 */
class engine {

    friend class Simulazione;

    /**
     * @brief Costruttore classe engine
     *
     * @param rete
     */
    engine(const Rete& rete);

    ~engine() = default;

  private:
    // -- ATTRIBUTI PRIVATI (Topologia e Stato) --------------------------------------------------------------

    std::size_t numeroNeuroniTotali_; // Numero totale di neuroni nella rete
    std::size_t numeroSinapsiTotali_; // Numero totale di sinapsi nella rete

    std::vector<size_t> lifEngineToRete;         // Mappa id engine --> id rete (neuroni lif)
    std::vector<size_t> expEngineToRete;         // Mappa id engine --> id rete (neuroni exp)
    std::vector<size_t> currentEngineToRete;     // Mappa id engine --> id rete (sinapsi current)
    std::vector<size_t> ConductanceEngineToRete; // Mappa id engine --> id rete (sinapsi conductance)

    std::vector<LIF> lif_;                 // Lista dei neuroni
    std::vector<Exp> exp_;                 // Lista dei neuroni
    std::vector<Current> current_;         // Lista delle sinapsi
    std::vector<Conductance> conductance_; // Lista delle sinapsi

    std::vector<std::vector<size_t>> listaAdiacenza_; // lista adiacenza = neurone i : sinapsi

    std::vector<double> stimoli_;     // Correnti esterne calcolate dalla simuazione
    std::vector<double> inputTotale_; // Somma stimoli + correnti sinaptiche

    // i vettori stimoli_ e inputTotale_ potrebbero essere uniti per risparimare memoria, è utile ?

    std::vector<double> statoNeuroni_; // Potenziale di membrana di tutti i neuroni nella rete
    std::vector<double> statoFiring_;  // Stato dei neuroni di tutta la rete
    std::vector<double> statoSinapsi_; // Corrente sinaptica di tutte le sinapsi nella rete

    // -- METODI INTERNI -----------------------------------------------------------------------------------

    // 1. Metodi di preparazione prima di partire la simulazione
    void prepare(double dt);

    // 1. Metodi evoluzione della rete e stato della rete
    void step(double dt);
    void aggiornaStatoRete();

    // 2. Metodi applicativi
    double getMinTau() const;
    bool hasNeurone(size_t idx) const { return idx < numeroNeuroniTotali_; }
    bool hasSinapsi(size_t idx) const { return idx < numeroSinapsiTotali_; }

    // 3. Metodi getter interni
    const std::vector<double>& getPointerStatoNeuroni() const { return statoNeuroni_; };
    const std::vector<double>& getPointerStatoFiring() const { return statoFiring_; };
    const std::vector<double>& getPointerStatoSinapsi() const { return statoSinapsi_; };
};

#endif // ENGINE_HPP