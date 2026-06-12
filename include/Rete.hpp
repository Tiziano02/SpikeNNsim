#ifndef RETE_HPP
#define RETE_HPP

#include "Input.hpp"
#include "Neurone.hpp"
#include "Sinapsi.hpp"

#include <vector>
#include <map>

/*
 * Rete — contenitore della topologia e motore di evoluzione della rete neurale.
 *
 * La rete è rappresentata come:
 *   - un vettore di neuroni
 *   - un vettore di sinapsi (ogni sinapsi contiene già i riferimenti ai neuroni pre e post)
 *   - una mappa ID -> indice per accesso rapido ai neuroni
 *   - un buffer di correnti afferenti pre-allocato, riusato ad ogni step
 *   - vettori di stato della rete : statoNeuroni, statoFiring e statoSinapsi
 *
 * La scelta di una lista di sinapsi invece di una matrice di adiacenza
 * semplifica l'aggiunta futura di delay e plasticità sinaptica.
 *
 * Costruttori:
 *   Rete()    — rete vuota
 *   Rete(N)   — N neuroni con parametri biologici di default, ID da 0 a N-1
 *
 * Metodi operativi:
 *   aggiungiNeurone(n)            — aggiunge un neurone alla rete
 *   connettiNeuroni(s)            — aggiunge una sinapsi alla rete
 *   step(dt, inputEsterni)        — avanza lo stato della rete di un passo dt
 *   salvaStatoRete(fV, fF, fS, t) — scrive lo stato corrente sui file di output
 *
 * Metodi getter:
 *   getPotenziali()    — restituisce il vettore statoNeuroni
 *   getFiringStates()  — restituisce il vettore statoFiring
 *   getSinapsi()       — restituisce il vettore statoSinaspi
 *
 * Metodi di controllo:
 *   hasNeurone(id)     — true se un neurone con quell'ID esiste nella rete
 */
class Rete {

  private:
    std::vector<Neurone> neuroni_;              // neuroni della rete
    std::vector<Sinapsi> sinapsi_;              // connessioni sinaptiche
    std::vector<double> inputTotale_;           // buffer correnti afferenti, dimensione = neuroni_.size()
    std::unordered_map<int, size_t> idToIndex_; // mappa ID -> indice in neuroni_, lookup O(1)

    std::vector<double> statoNeuroni_;
    std::vector<double> statoFiring_;
    std::vector<double> statoSinapsi_;

  public:
    Rete() = default;

    Rete(int N) {
        for (int i = 0; i < N; ++i)
            aggiungiNeurone(Neurone(i));
    }

    // metodi operativi
    void aggiungiNeurone(const Neurone &neurone);
    void connettiNeuroni(Sinapsi &s);
    void step(double dt, const std::vector<InputCorrente> &inputEsterni);
    void aggiornaStatoRete();

    // metodi getter
    // std::vector<double> getStatoNeuroni() const;
    // std::vector<double> getStatoFiring() const;
    // std::vector<double> getStatoSinapsi() const;

    const std::vector<double> &getPointerStatoNeuroni() const;
    const std::vector<double> &getPointerStatoFiring() const;
    const std::vector<double> &getPointerStatoSinapsi() const;

    size_t getNumNeuroni() const { return neuroni_.size(); }
    size_t getNumSinapsi() const { return sinapsi_.size(); }

    // metodi di controllo
    bool hasNeurone(int id) const { return idToIndex_.count(id) > 0; }

    ~Rete() = default;
};

#endif // RETE_HPP