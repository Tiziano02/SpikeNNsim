#ifndef SIMULAZIONE_HPP
#define SIMULAZIONE_HPP

#include "Input.hpp"
#include "Rete.hpp"
#include "UnitaSI.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*
 * Simulazione — coordina la rete, gli input esterni e l'output su file.
 *
 * La simulazione gestisce la griglia temporale e il loop principale:
 *   stepTotali = round(T / dt)
 *   time(k) = k * dt,  k = 1, 2, ..., stepTotali
 *
 * Il primo dato salvato sui file corrisponde a time = dt (dopo il primo step),
 * non a time = 0 (stato iniziale).
 *
 * Formato dei file di output — ogni riga:
 *   time  val1  val2  ...  valN
 * dove i valori sono in unità SI (V, A, s).
 *
 * Attributi:
 *   rete_          rete neurale da simulare (copia interna)
 *   inputEsterni_  stimoli esterni per l'intera simulazione
 *   dt_            passo temporale [s]
 *   T_             durata totale [s]
 *   stepCorrente_  indice dello step corrente
 *   stepTotali_    numero totale di step = round(T/dt)
 *
 * Metodi operativi:
 *   aggiungiInputEsterni(inputs)              — associa gli input esterni alla simulazione
 *   avviaSimulazione(filenameV, filenameF, filenameS) — esegue la simulazione e salva i risultati
 */
class Simulazione {

  private:
    Rete rete_;
    std::vector<Input> inputEsterni_;
    double dt_;
    double T_;
    int stepCorrente_;
    int stepTotali_;

  public:
    Simulazione(const Rete &rete, double dt, double T) : rete_(rete), dt_(dt), T_(T), stepCorrente_(0), stepTotali_(static_cast<int>(std::round(T / dt))) {}

    // metodi operativi
    void aggiungiInputEsterni(const std::vector<Input> &inputEsterno);
    void avviaSimulazione(const std::string &filenameV, const std::string &filenameF, const std::string &filenameS);

    ~Simulazione() = default;
};

/*
 * aggiungiInputEsterni — associa gli input esterni alla simulazione.
 *
 * Validazione:
 *  - ogni Input deve avere dimensione uguale a stepTotali_
 *  - ogni ID deve corrispondere a un neurone esistente nella rete
 * Se un input non supera la validazione, l'intera operazione viene annullata.
 */
void Simulazione::aggiungiInputEsterni(const std::vector<Input> &inputEsterno) {
    for (const auto &inp : inputEsterno) {
        if (static_cast<int>(inp.valori.size()) != stepTotali_) {
            std::cerr << "[Simulazione] errore: input per neurone " << inp.id << " ha dimensione " << inp.valori.size() << " ma stepTotali = " << stepTotali_ << ".\n";
            return;
        }
        if (!rete_.hasNeurone(inp.id)) {
            std::cerr << "[Simulazione] errore: neurone ID " << inp.id << " non trovato nella rete.\n";
            return;
        }
    }
    inputEsterni_ = inputEsterno;
}

/*
 * avviaSimulazione — esegue il loop principale e salva i risultati su file.
 *
 * Sequenza ad ogni step:
 *  1. Estrae il valore corrente di ogni input esterno.
 *  2. Chiama Rete::step(dt, inputEsterniCorrente).
 *  3. Aggiorna il tempo: time += dt.
 *  4. Salva lo stato della rete sui tre file di output.
 *
 * Il vettore inputEsterniCorrente è pre-allocato prima del loop per evitare
 * allocazioni dinamiche durante la simulazione.
 * I file vengono chiusi automaticamente alla distruzione degli ofstream.
 */
void Simulazione::avviaSimulazione(const std::string &filenameV, const std::string &filenameF, const std::string &filenameS) {
    double time = 0.0;
    stepCorrente_ = 0;

    std::vector<InputCorrente> inputEsterniCorrente;
    inputEsterniCorrente.reserve(inputEsterni_.size());
    for (const auto &inp : inputEsterni_)
        inputEsterniCorrente.push_back({inp.id, 0.0});

    std::ofstream filePotenziali(filenameV);
    std::ofstream fileFiring(filenameF);
    std::ofstream fileSinapsi(filenameS);

    if (!filePotenziali.is_open() || !fileFiring.is_open() || !fileSinapsi.is_open()) {
        std::cerr << "[Simulazione] errore: impossibile aprire i file di output.\n";
        return;
    }

    for (; stepCorrente_ < stepTotali_; ++stepCorrente_) {
        for (size_t i = 0; i < inputEsterniCorrente.size(); ++i)
            inputEsterniCorrente[i].valoreCorrente = inputEsterni_[i].valori[stepCorrente_];

        rete_.step(dt_, inputEsterniCorrente);
        time += dt_;

        rete_.salvaStatoRete(filePotenziali, fileFiring, fileSinapsi, time);
    }
}

#endif // SIMULAZIONE_HPP