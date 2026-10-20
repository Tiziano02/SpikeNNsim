/**
 * @file CurrentSyn.hpp
 * @brief Definizione del modello di sinapsi basato sulla corrente (Current-based).
 * * Questo file contiene sia l'interfaccia pubblica per la modifica dei parametri (patchCurrent),
 * sia l'implementazione interna del modello fisico (Current) utilizzato dal motore del simulatore.
 */

#ifndef CURRENTSYN_HPP
#define CURRENTSYN_HPP

#include <cmath>
#include <cstddef>
#include <vector>
#include <optional>
#include "UnitaSI.hpp"

/**
 * @ingroup publicapi
 * @brief Pacchetto dati per l'aggiornamento dei parametri di una sinapsi Current-based.
 *
 * @details
 * Struttura utilizzata esclusivamente dal metodo `Rete::modificaSinapsi()`.
 * Tutti i campi sono basati su `std::optional`: se un campo non viene esplicitamente
 * impostato dall'utente, il suo valore nella sinapsi bersaglio rimarrà inalterato.
 * * I valori fisici devono essere passati in unità del Sistema Internazionale (SI).
 * * @note Convenzione dei segni:
 * - peso > 0 -> I_syn > 0 -> Inibitoria (abbassa il potenziale post-sinaptico)
 * - peso < 0 -> I_syn < 0 -> Eccitatoria (alza il potenziale post-sinaptico)
 */
struct patchCurrent {
    std::optional<double> Isyn = std::nullopt;  ///< Stato: Corrente sinaptica iniziale/attuale [A]
    std::optional<double> peso = std::nullopt;  ///< Parametro: Fattore di scala adimensionale della sinapsi
    std::optional<double> Ipeak = std::nullopt; ///< Parametro: Corrente di picco iniettata per peso unitario [A]
    std::optional<double> tau = std::nullopt;   ///< Parametro: Scala temporale di decadimento esponenziale [s]
    std::optional<double> delay = std::nullopt; ///< Parametro: Ritardo di conduzione sinaptica (delay) [s]
};

/**
 * @ingroup internals
 * @brief Entità fisica della sinapsi basata sulla corrente.
 *
 * @details
 * Modello matematico:
 * La corrente sinaptica decade esponenzialmente nel tempo:
 * dI_syn/dt = -I_syn / tau
 *
 * Quando il neurone pre-sinaptico emette uno spike, dopo un tempo pari a 'delay',
 * la corrente subisce un incremento istantaneo proporzionale al peso:
 * I_syn = I_syn + (peso * Ipeak)
 *
 * A differenza delle sinapsi Conductance-based, la corrente iniettata nel neurone
 * post-sinaptico non dipende dal suo potenziale di membrana attuale.
 *
 * @warning Non istanziare mai manualmente. Il ciclo di vita di questo oggetto
 * è gestito interamente dal motore di simulazione della classe `Rete`.
 */
class Current {

    friend class Rete;

  public:
    /**
     * @brief Costruttore interno utilizzato da Rete.
     * @param indexPre Indice del neurone pre-sinaptico nel vettore di Rete.
     * @param indexPost Indice del neurone post-sinaptico nel vettore di Rete.
     * @param typeIntegratore Metodo di integrazione: 'A' (Analitico esatto), 'E' (Eulero in avanti), 'R' (Runge-Kutta
     * 4).
     */
    Current(size_t indexPre, size_t indexPost, char typeIntegratore = 'E')
        : indexPre_(indexPre), indexPost_(indexPost), tipoIntegratore_(typeIntegratore) {}

    ~Current() = default;

  private:
    // -- ATTRIBUTI FISICI E DI STATO --------------------------------------------------------------------------

    // Topologia
    size_t indexPre_, indexPost_; // Indici neuroni della sinapsi

    // Stato Dinamico
    double Isyn_ = 0.0 * A;      // Corrente sinaptica calcolata in questo step [A]
    char tipoIntegratore_ = 'E'; // Risolutore ODE ('A' = Analitico, 'E' = Eulero, 'R' = Runge-Kutta)

    // Parametri Fisici
    double peso_ = 1.0;          // Forza adimensionale della sinapsi
    double Ipeak_ = 0.1 * n * A; // Corrente di picco post-spike [A]
    double tau_ = 5.0 * ms;      // Scala temporale di decadimento [s]
    double delay_ = 1.0 * ms;    // Ritardo sinaptico [s]

    // Ring Buffer
    size_t presentStep_ = 0;        // Indice di lettura corrente nel ring buffer
    size_t delayStep_ = 0;          // Numero di step temporali corrispondenti al delay_
    std::vector<double> delayRing_; // Ring buffer per immagazzinare gli spike in transito

    // Coefficienti per il calcolo analitico
    double decayFactor_ = 0.0; // Fattore di decadimento analitico esatto: exp(-dt / tau_)

    // -- METODI PRIVATI --------------------------------------------------------------------------------------

    // 1. Gestione evoluzione temporale
    void update(double dt, bool preFired);
    void euleroInAvanti(double dt);
    void rungeKutta(double dt);
    void analiticoEsatto();

    // 2. Metodi getter
    inline double getCurrent() const { return Isyn_; }
    inline size_t getIndexPre() const { return indexPre_; }
    inline size_t getIndexPost() const { return indexPost_; }

    // 3. Metodi di preparazione e configurazione interna
    inline void setIndexPre(size_t idx) { indexPre_ = idx; }
    inline void setIndexPost(size_t idx) { indexPost_ = idx; }

    /**
     * @brief Inizializza il buffer circolare per il ritardo di conduzione.
     * @param dt Passo di integrazione temporale [s].
     */
    inline void setDelayRing(double dt) {
        delayStep_ = static_cast<size_t>(std::round(delay_ / dt));
        delayRing_.assign(delayStep_ + 1, 0.0);
    }

    /**
     * @brief Prepara la sinapsi prima della simulazione (buffer delay + coefficienti analitici).
     * @param dt Passo di integrazione temporale [s].
     */
    inline void prepare(double dt) {
        setDelayRing(dt);
        decayFactor_ = std::exp(-dt / tau_);
    }
};

#endif // CURRENTSYN_HPP