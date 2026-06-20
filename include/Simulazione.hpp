#ifndef SIMULAZIONE_HPP
#define SIMULAZIONE_HPP

#include "Input.hpp"
#include "Rete.hpp"
#include "UnitaSI.hpp"
#include "Utility.hpp"

#include <fstream>
#include <memory>

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
 *   aggiungiInputEsterni(inputs)                       — associa gli input esterni alla simulazione
 *   avviaSimulazione(filenameV, filenameF, filenameS)  — esegue la simulazione e salva i risultati
 */
class Simulazione {

  private:
    // rete su cui va fatta la simulazione
    Rete rete_;

    // attributi per gestione Input Esterni
    std::vector<rigaRegistroStimolo> registroStimoli_;
    std::vector<parametriStimoloCostante> databaseStimoloCostante_;
    std::vector<parametriStimoloSeno> databaseStimoloSeno_;

    // attributi per definire una simulazione
    double dt_;
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

    // metodi interni per I/O
    void inizializzaOutput();
    void loadStatoRete(double time);
    void writeFile();

    // metodi interi per gli stimoli
    template <typename tipologiaParametri> bool controlloParametri(std::vector<int> &listID, std::vector<tipologiaParametri> &listaParametri);

  public:
    Simulazione(const Rete &rete, double dt, double T) : rete_(rete), dt_(dt), stepCorrente_(0), stepTotali_(static_cast<int>(std::round(T / dt))) {}

    // tools Simualzione - inserimento degli stimoli
    void iniettaStimoloCostante(std::vector<int> &listaID, std::vector<parametriStimoloCostante> &listaParametri);
    void iniettaStimoloSeno(std::vector<int> &listaID, std::vector<parametriStimoloSeno> &listaParametri);

    // tools Simulazione - aggiorna il vettore stimoli_ dell'oggetto rete_ --> calcola tutti gli stimoli al tempo t
    void valutaStimoli(double t);

    // metodi operativi
    void avviaSimulazione(const std::string &filenameV, const std::string &filenameF, const std::string &filenameS);

    // metodi getter
    double getTempoTotale() const { return stepTotali_ * dt_; }

    ~Simulazione() = default;
};

#endif // SIMULAZIONE_HPP