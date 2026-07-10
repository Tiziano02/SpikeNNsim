/**
 * @file Simulazione.hpp
 * @brief Direttore d'orchestra del simulatore: loop temporale, stimoli e I/O.
 * * Questo file definisce la classe Simulazione, che prende in carico una Rete
 * configurata, le applica eventuali correnti esterne nel tempo e ne calcola
 * l'evoluzione, serializzando i risultati su file binari ad alte prestazioni.
 */

#ifndef SIMULAZIONE_HPP
#define SIMULAZIONE_HPP

#include "Rete.hpp"
#include "Stimoli.hpp"
#include <fstream>
#include <string>
#include <vector>

/**
 * @ingroup publicapi
 * @brief Motore di esecuzione della simulazione e gestione dell'output.
 *
 * @details
 * La classe gestisce l'intero ciclo di vita dell'integrazione temporale:
 * 1. Clona la topologia della rete passata dall'utente.
 * 2. Gestisce il ring buffer temporale e la validazione numerica.
 * 3. Applica gli stimoli (es. correnti costanti o sinusoidali) istante per istante.
 * 4. Salva i risultati (potenziali, spike, correnti sinaptiche) in file binari
 * tramite un sistema a doppio buffer per massimizzare le prestazioni di I/O.
 * * * I file binari generati contengono un header iniziale (int32_t) che indica
 * il numero di colonne (1 colonna per il tempo + N colonne dati).
 */
class Simulazione {

  public:
    /**
     * @brief Costruisce e inizializza l'ambiente di simulazione.
     * @param rete Oggetto Rete configurato (viene copiato e blindato internamente).
     * @param dt Passo di integrazione temporale [s] (es. 0.1 * ms).
     * @param T Durata fisica totale della simulazione [s].
     * @warning Il costruttore verificherà la stabilità numerica. Se `dt > tau_min/10`,
     * verrà stampato un avviso di potenziale instabilità per i metodi di Eulero.
     */
    Simulazione(const Rete& rete, double dt, double T);

    ~Simulazione() = default;

    // -- METODI PUBBLICI (API) ----------------------------------------------------------------------------------

    /**
     * @brief Registra una lista di stimoli esterni da applicare durante la simulazione.
     * @param stimoli Vettore di strutture `stimolo` (contenente ID neurone bersaglio e parametri della corrente).
     * @note Se uno stimolo è indirizzato a un ID neurone non esistente, verrà ignorato
     * e verrà stampato un errore su stderr.
     */
    void iniettaStimoli(const std::vector<stimolo>& stimoli);

    /**
     * @brief Avvia il loop di calcolo temporale e la scrittura su disco.
     * @param filenameV Percorso del file binario di output per i Potenziali di Membrana.
     * @param filenameF Percorso del file binario di output per la matrice di Firing (Spike).
     * @param filenameS Percorso del file binario di output per le Correnti Sinaptiche.
     */
    void avviaSimulazione(const std::string& filenameV, const std::string& filenameF, const std::string& filenameS);

  private:
    // -- ATTRIBUTI PRIVATI ----------------------------------------------------------------------------------------

    // Topologia e Dati
    Rete rete_;                                  // Copia isolata della rete da simulare
    std::vector<recordStimolo> RegistroStimoli_; // Registro degli stimoli da iniettare

    // Controllo Temporale
    double dt_;        // Passo temporale di integrazione [s]
    int stepCorrente_; // Indice dello step attuale (t = stepCorrente_ * dt_)
    int stepTotali_;   // Numero totale di step previsti

    // Gestione Nomi File Output
    std::string fileNameV_;
    std::string fileNameF_;
    std::string fileNameS_;

    // Stream di Scrittura
    std::ofstream filePotenziali_;
    std::ofstream fileFiring_;
    std::ofstream fileSinapsi_;

    // Buffer
    std::vector<char> bufferV_; // Buffer per array potenziali
    std::vector<char> bufferF_; // Buffer per array spike
    std::vector<char> bufferS_; // Buffer per array correnti

    size_t bytesPerStepV_ = 0; // Dimensione in byte di una singola riga di V
    size_t bytesPerStepF_ = 0; // Dimensione in byte di una singola riga di Firing
    size_t bytesPerStepS_ = 0; // Dimensione in byte di una singola riga di Sinapsi

    size_t posizioneBuffer_ = 0; // Indice di riempimento attuale dei buffer
    size_t stepsPerFlush_ = 0;   // Numero di step da accumulare prima di scrivere su disco

    // -- METODI PRIVATI  ------------------------------------------------------------------------

    // 1. Setup I/O e Buffer
    void preparazioneOutput();
    void scritturaHeader();
    void creazioneBuffer();

    // 2. Scrittura buffer e disco
    void loadStatoRete(double time);
    void writeFile();

    // 3. Gesione stimoli
    void valutaStimoli(double t); // Calcola lo stimolo da inserire al neurone in base al registro
};

#endif // SIMULAZIONE_HPP