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

// #include <stdexcept>

struct StatoComune {              // indice = indice interno globale
    std::vector<double> V;        // potenziale
    std::vector<uint8_t> sparato; // non vector<bool>
    std::vector<double> stimolo;  // corrente esterna, riempita da Simulazione
};

struct GruppoLIF { // neuroni LIF = intervallo [ini, fin) dell'indice globale
    uint32_t ini, fin;
    std::vector<double> tauM, Vrip, Vsoglia, Vreset;
    std::vector<double> alpha; // e^(-dt/tauM), calcolato alla compilazione
    std::vector<int32_t> refrattarioRimasto;
    void aggiorna(StatoComune&, const double* inputSin, double dt, uint32_t a, uint32_t b);
};
struct GruppoExp { /* come LIF, più deltaT, Vt, ... */
};
using Gruppo = std::variant<GruppoLIF, GruppoExp>;

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

  public:
    /**
     * @brief Costruttore default : crea una rete vuota
     */
    engine() = default;

    ~engine() = default;

  private:
    // -- ATTRIBUTI PRIVATI (Topologia e Stato) --------------------------------------------------------------

    Rete rete_;

    neuroni

        // -- METODI INTERNI -----------------------------------------------------------------------------------

        // 1. Metodi di preparazione dati
        void prepare(double dt);

    // 1. Metodi evoluzione della rete e stato della rete
    void step(double dt);
    void aggiornaStatoRete();

    // 2. Metodi applicativi
    double getMinTau() const;
};

#endif // ENGINE_HPP