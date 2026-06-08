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

    // attributi per I/O
    std::ofstream filePotenziali_;
    std::ofstream fileFiring_;
    std::ofstream fileSinapsi_;
    std::string fileNameV_;
    std::string fileNameF_;
    std::string fileNameS_;
    std::vector<char> bufferV_;
    std::vector<char> bufferF_;
    std::vector<char> bufferS_;
    size_t bytesPerStepV_ = 0;
    size_t bytesPerStepF_ = 0;
    size_t bytesPerStepS_ = 0;
    size_t posizioneBuffer_ = 0;
    size_t stepsPerFlush_ = 0;

    void inizializzaOutput();
    void loadStatoRete(double time);
    void flushBuffer();
    
  public:
    Simulazione(const Rete &rete, double dt, double T) : rete_(rete), dt_(dt), T_(T), stepCorrente_(0), stepTotali_(static_cast<int>(std::round(T / dt))) {}

    // metodi operativi
    void aggiungiInputEsterni(const std::vector<Input> &inputEsterno);
    void avviaSimulazione(const std::string &filenameV, const std::string &filenameF, const std::string &filenameS);

    ~Simulazione() = default;
};

#endif // SIMULAZIONE_HPP