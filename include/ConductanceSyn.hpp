/**
 * @file ConductanceSyn.hpp
 * @brief Definizione del modello di sinapsi basato sulla conduttanza (Conductance-based).
 * * Questo file contiene sia l'interfaccia pubblica per la modifica dei parametri (patchConductance),
 * sia l'implementazione interna del modello fisico (Conductance) utilizzato dal motore del simulatore.
 */

#ifndef CONDUCTANCESYN_HPP
#define CONDUCTANCESYN_HPP

#include <cmath>
#include <cstddef>
#include <vector>
#include <optional>
#include "UnitaSI.hpp"

/**
 * @ingroup publicapi
 * @brief Pacchetto dati per l'aggiornamento dei parametri di una sinapsi Conductance-based.
 *
 * @details
 * Struttura utilizzata esclusivamente dal metodo `Rete::modificaSinapsi()`.
 * Tutti i campi sono basati su `std::optional`: se un campo non viene esplicitamente
 * impostato dall'utente, il suo valore nella sinapsi bersaglio rimarrà inalterato.
 * * I valori fisici devono essere passati in unità del Sistema Internazionale (SI).
 * * @warning Non inserire MAI valori di default (es. `= 1.0`) in questa struct,
 * altrimenti le modifiche parziali sovrascriveranno i valori esistenti con i default.
 */
struct patchConductance {
    std::optional<double> peso;  ///< Parametro: Forza adimensionale della sinapsi (moltiplicatore)
    std::optional<double> gpeak; ///< Parametro: Conduttanza di picco aggiunta per ogni spike [S]
    std::optional<double> gsyn;  ///< Stato: Conduttanza dinamica attuale [S]
    std::optional<double> tau;   ///< Parametro: Scala temporale di decadimento della conduttanza [s]
    std::optional<double> delay; ///< Parametro: Ritardo di conduzione sinaptica (delay) [s]
    std::optional<double> Erev;  ///< Parametro: Potenziale di inversione (Reversal potential) [V]
};

/**
 * @ingroup internals
 * @brief Entità fisica della sinapsi basata sulla conduttanza.
 *
 * @details
 * Modello matematico:
 * La corrente sinaptica iniettata nel neurone post-sinaptico è calcolata come:
 * I_syn(t) = g_syn(t) * (V_post(t) - E_rev)
 *
 * Dinamica della conduttanza (g_syn):
 * d(g_syn)/dt = -g_syn / tau
 * * Quando il neurone pre-sinaptico emette uno spike, dopo un tempo pari a 'delay',
 * la conduttanza subisce un incremento istantaneo:
 * g_syn = g_syn + (peso * gpeak)
 *
 * @warning Non istanziare mai manualmente. Il ciclo di vita di questo oggetto
 * è gestito interamente dal motore di simulazione della classe `Rete`.
 */
class Conductance {

    friend class Rete;

  public:
    /**
     * @brief Costruttore interno utilizzato da Rete.
     * @param indexPre Indice del neurone pre-sinaptico nel vettore di Rete.
     * @param indexPost Indice del neurone post-sinaptico nel vettore di Rete.
     * @param idPre ID utente del neurone pre-sinaptico.
     * @param idPost ID utente del neurone post-sinaptico.
     */
    Conductance(size_t indexPre, size_t indexPost, int idPre, int idPost)
        : idPre_(idPre), idPost_(idPost), indexPre_(indexPre), indexPost_(indexPost) {}

    ~Conductance() = default;

  private:
    // -- ATTRIBUTI FISICI E DI STATO --------------------------------------------------------------------------

    // Topologia (Routing)
    int idPre_, idPost_;          // Identificatori pubblici scelti dall'utente
    size_t indexPre_, indexPost_; // Indici interni per accesso rapido ai vettori di Rete (Cache)

    // Stato Dinamico
    double Isyn_ = 0.0 * A; // Corrente sinaptica calcolata in questo step [A]
    double gsyn_ = 0.0 * S; // Conduttanza dinamica corrente [S]

    // Parametri Fisici
    double peso_ = 1.0;          // Forza adimensionale della sinapsi
    double gpeak_ = 1.0 * n * S; // Conduttanza di picco post-spike [S]
    double tau_ = 5.0 * ms;      // Scala temporale di decadimento [s]
    double delay_ = 1.0 * ms;    // Ritardo sinaptico [s]
    double Erev_ = 0.0 * Volt;   // Potenziale di inversione [V]

    // Ring Buffer
    size_t presentStep_ = 0;        // Indice di lettura corrente nel ring buffer
    size_t delayStep_ = 0;          // Numero di step temporali corrispondenti al delay_
    std::vector<double> delayRing_; // Ring buffer per immagazzinare gli spike in transito

    // -- METODI PRIVATI --------------------------------------------------------------------------------------

    // 1. Gestione evoluzione temporale
    void update(double dt, bool preFired, double V_post);

    // 2. Metodi getter
    inline double getCurrent() const { return Isyn_; }
    inline int getIdPre() const { return idPre_; }
    inline int getIdPost() const { return idPost_; }
    inline size_t getIndexPre() const { return indexPre_; }
    inline size_t getIndexPost() const { return indexPost_; }

    // 3. Metodi setter
    inline void setIndexPre(size_t idx) { indexPre_ = idx; }
    inline void setIndexPost(size_t idx) { indexPost_ = idx; }
    inline void setDelayRing(double dt) {
        delayStep_ = static_cast<size_t>(std::round(delay_ / dt));
        delayRing_.assign(delayStep_ + 1, 0.0);
    }
};

#endif // CONDUCTANCESYN_HPP