#ifndef SIMULAZIONE_HPP
#define SIMULAZIONE_HPP

#include "Rete.hpp"
#include "Input.hpp"
#include <fstream>

// ============================================================================
// CLASSE Simulazione (PUBBLICA)
// ============================================================================

/**
 * Simulazione – coordina la rete, gli stimoli esterni e l'output su file.
 *
 * Gestisce il loop temporale e l'applicazione degli stimoli.
 *
 * ============================================================================
 * STIMOLI
 * ============================================================================
 * Utilizza un vettore di stimolo (vedi Input.hpp) per gestire stimoli costanti
 * e sinusoidali. Aggiungere stimoli con iniettaStimoli().
 *
 * ============================================================================
 * OUTPUT
 * ============================================================================
 * I risultati vengono salvati in file binari con un header (int32_t) che
 * indica il numero di colonne (tempo + dati). Usare gli script Python
 * di supporto per la lettura.
 */
class Simulazione {

  private:
    // ── ATTRIBUTI PRIVATI ──────────────────────────────────────────────────

    Rete rete_;                                  // Rete da simulare
    std::vector<recordStimolo> RegistroStimoli_; // Stimoli esterni

    double dt_;        // Passo temporale [s]
    int stepCorrente_; // Step attuale
    int stepTotali_;   // Numero totale di step

    // Output
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

    // ── METODI PRIVATI ──────────────────────────────────────────────────────

    void inizializzaOutput();
    void loadStatoRete(double time);
    void writeFile();

    /**
     * Valuta tutti gli stimoli al tempo t e li applica alla rete.
     * Iterando su stimoli_ e usando std::visit per calcolare il valore corrente.
     */
    void valutaStimoli(double t);

    // ── COSTRUTTORE E METODI PUBBLICI ─────────────────────────────────────

  public:
    /**
     * Costruisce una simulazione.
     * @param rete  Rete da simulare (viene copiata internamente)
     * @param dt    passo temporale [s]
     * @param T     durata totale [s]
     * @warning Se dt > tau_min/10, viene stampato un avviso di instabilità.
     */
    Simulazione(const Rete& rete, double dt, double T);

    /**
     * Inietta una lista di stimoli nella simulazione.
     * @param stimoli vettore di stimolo (coppia indice neurone + parametri)
     * @note I neuroni devono esistere; altrimenti viene stampato un errore.
     */
    void iniettaStimoli(const std::vector<stimolo>& stimoli);

    /**
     * Avvia la simulazione e salva i risultati su file binari.
     * @param filenameV  file per i potenziali
     * @param filenameF  file per i firing
     * @param filenameS  file per le correnti sinaptiche
     */
    void avviaSimulazione(const std::string& filenameV, const std::string& filenameF, const std::string& filenameS);

    /**
     * @return Durata totale della simulazione [s].
     */
    double getTempoTotale() const { return stepTotali_ * dt_; }

    ~Simulazione() = default;
};

#endif // SIMULAZIONE_HPP