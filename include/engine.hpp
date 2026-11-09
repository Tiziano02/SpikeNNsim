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

    engine(const Rete& rete) {

        // 1. Controllo che la rete sia stata bloccata prima di inziare la simulazione
        if (!rete.isBloccata()) {
            throw std::logic_error(
                "[engine] Errore: la rete non e' bloccata. Chiamare rete.bloccaRete() prima di creare l'engine.");
        }

        // 1. Recupero dei neuroni e delle sinapsi dalla rete
        neuroni_ = rete.neuroni_;
        sinapsi_ = rete.sinapsi_;

        // 2. Inizializzazione dei vettori di stato della rete
        statoNeuroni_.resize(neuroni_.size(), 0.0);
        statoFiring_.resize(neuroni_.size(), 0.0);
        statoSinapsi_.resize(sinapsi_.size(), 0.0);

        // 3. Inizializzazione dei vettori di input e stimoli
        stimoli_.resize(neuroni_.size(), 0.0);
        inputTotale_.resize(neuroni_.size(), 0.0);

        // 4. Creazione della lista di adiacenza

        // 1.1 inizializzazione della lista di adiacenza
        size_t numNeuroniTotali = neuroni_.size();
        listaAdiacenza_.resize(numNeuroniTotali);
        size_t countSinapsi = 0;

        // 1.2 inserimento dati nella lista di adiacenza
        for (auto& s : sinapsi_) {
            std::visit([&](auto& syn) { listaAdiacenza_[syn.getIndexPre()].push_back(countSinapsi); }, s);
            countSinapsi++;
        }

        // Osservazione: potrebbe essere fatto dopo la creazione della struttua dati per l'engine
        // 5. Inserimento dello stato iniziale dei neuroni e delle sinapsi nei vettori di stato della rete
        for (size_t i = 0; i < neuroni_.size(); i++) {
            std::visit([&](auto& n) { statoNeuroni_[i] = n.getPotential(); }, neuroni_[i]);
            std::visit([&](auto& n) { statoFiring_[i] = n.hasFired() ? 1.0 : 0.0; }, neuroni_[i]);
        }
        for (size_t i = 0; i < sinapsi_.size(); i++) {
            std::visit([&](auto& s) { statoSinapsi_[i] = s.getCurrent(); }, sinapsi_[i]);
        }
    };

    ~engine() = default;

  private:
    // -- ATTRIBUTI PRIVATI (Topologia e Stato) --------------------------------------------------------------

    std::vector<TypeNeuron> neuroni_; // Lista dei neuroni
    std::vector<TypeSyn> sinapsi_;    // Lista delle sinapsi

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
    bool hasNeurone(size_t idx) const { return idx < neuroni_.size(); }
    bool hasSinapsi(size_t idx) const { return idx < sinapsi_.size(); }

    // 3. Metodi getter interni
    const std::vector<double>& getPointerStatoNeuroni() const { return statoNeuroni_; };
    const std::vector<double>& getPointerStatoFiring() const { return statoFiring_; };
    const std::vector<double>& getPointerStatoSinapsi() const { return statoSinapsi_; };
};

#endif // ENGINE_HPP