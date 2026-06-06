#ifndef RETE_HPP
#define RETE_HPP

#include "Input.hpp"
#include "Neurone.hpp"
#include "Sinapsi.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

/*
 * Rete — contenitore della topologia e motore di evoluzione della rete neurale.
 *
 * La rete è rappresentata come:
 *   - un vettore di neuroni
 *   - un vettore di sinapsi (ogni sinapsi contiene già i riferimenti ai neuroni pre e post)
 *   - una mappa ID -> indice per accesso rapido ai neuroni
 *   - un buffer di correnti afferenti pre-allocato, riusato ad ogni step
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
 *   getPotenziali()    — potenziali di membrana di tutti i neuroni [V]
 *   getFiringStates()  — stati di firing (0/1) di tutti i neuroni
 *   getSinapsi()       — correnti sinaptiche di tutte le sinapsi [A]
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
    void salvaStatoRete(std::ofstream &filePotenziali, std::ofstream &fileFiring, std::ofstream &fileSinapsi, double time);

    // metodi getter
    std::vector<double> getPotenziali() const;
    std::vector<int> getFiringStates() const;
    std::vector<double> getSinapsi() const;

    // metodi di controllo
    bool hasNeurone(int id) const { return idToIndex_.count(id) > 0; }

    ~Rete() = default;
};

#include "reteImp.hpp"

#endif // RETE_HPP