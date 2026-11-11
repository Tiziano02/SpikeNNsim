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

struct EventoSinaptico {
    SynapseModel modello;
    std::size_t indiceEngine;
};

struct posizioneSinapsi {
    SynapseModel modello;
    std::size_t indiceEngine;
};

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

    // 1. Parametri totali
    std::size_t numeroNeuroniTotali_; // Numero totale di neuroni nella rete
    std::size_t numeroSinapsiTotali_; // Numero totale di sinapsi nella rete

    // 2. Mappe tra id engine --> id rete
    std::vector<size_t> lifEngineToRete;
    std::vector<size_t> expEngineToRete;
    std::vector<size_t> currentEngineToRete;
    std::vector<size_t> conductanceEngineToRete;

    // 3. Mappe tra id rete --> id engine
    std::vector<posizioneSinapsi> sinapsiReteToEngine; // mappa id rete --> posizione sinapsi interne all'engine

    // 3. Vettori dei neuroni e delle sinapsi divisi per tipo
    std::vector<LIF> lif_;
    std::vector<Exp> exp_;
    std::vector<Current> current_;
    std::vector<size_t> activeCurrent_;
    std::vector<Conductance> conductance_;
    std::vector<size_t> activeConductance_;

    // 4. Lista di adiacenza per la rete (per ogni neurone, quali sinapsi è il neurone pre-sinaptico)
    std::vector<std::vector<posizioneSinapsi>> listaAdiacenza_; // lista adiacenza = neurone i : sinapsi

    // 5. Vettori di input esterno e input totale per ogni neurone della rete
    std::vector<double> stimoli_;     // Correnti esterne calcolate dalla simuazione
    std::vector<double> inputTotale_; // Somma stimoli + correnti sinaptiche

    // i vettori stimoli_ e inputTotale_ potrebbero essere uniti per risparimare memoria, è utile ?

    // 6. Vettori di stato della rete (potenziale di membrana, stato firing, corrente sinaptica)
    std::vector<double> statoNeuroni_; // Potenziale di membrana di tutti i neuroni nella rete
    std::vector<double> statoFiring_;  // Stato dei neuroni di tutta la rete
    std::vector<double> statoSinapsi_; // Corrente sinaptica di tutte le sinapsi nella rete

    // 7. registro circolare degli eventi sinaptici in transito
    std::vector<std::vector<EventoSinaptico>> ringEventi_;

    // 8. Delay sinaptico
    size_t presentStep_ = 0;         // Posizione corrente nel ring buffer degli eventi sinaptici
    std::vector<size_t> delaySteps_; // Delay di ogni sinapsi della rete (in step temporali)

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