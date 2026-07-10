/**
 * @file Exp.hpp
 * @brief Definizione del modello neurale Esponenziale (Exp / AdEx semplificato).
 * * Questo file contiene sia l'interfaccia pubblica per la modifica dei parametri (patchExp),
 * sia l'implementazione interna del modello fisico (Exp) utilizzato dal motore del simulatore.
 */

#ifndef EXP_HPP
#define EXP_HPP

#include "UnitaSI.hpp"
#include <optional>

/**
 * @ingroup publicapi
 * @brief Pacchetto dati per l'aggiornamento dei parametri di un neurone Exp.
 *
 * @details
 * Struttura utilizzata esclusivamente dal metodo `Rete::modificaParametriNeurone()`.
 * Tutti i campi sono basati su `std::optional`: se un campo non viene esplicitamente
 * impostato dall'utente, il suo valore nel neurone bersaglio rimarrà inalterato.
 * * I valori fisici devono essere passati in unità del Sistema Internazionale (SI).
 */
struct patchExp {
    std::optional<double> V;            ///< Potenziale di membrana attuale [V]
    std::optional<double> Vth;          ///< Soglia dinamica attuale [V]
    std::optional<double> VthMin;       ///< Parametro: Soglia minima (asintoto di riposo) [V]
    std::optional<double> VthMax;       ///< Parametro: Soglia massima (picco post-spike) [V]
    std::optional<double> Vrest;        ///< Parametro: Potenziale di riposo [V]
    std::optional<double> Vreset;       ///< Parametro: Potenziale di reset post-spike [V]
    std::optional<double> sharpness;    ///< Parametro: Fattore di pendenza esponenziale (Delta T) [V]
    std::optional<double> R;            ///< Parametro: Resistenza di membrana [Ω]
    std::optional<double> C;            ///< Parametro: Capacità di membrana [F]
    std::optional<double> timeAbsolute; ///< Parametro: Durata del periodo refrattario assoluto [s]
    std::optional<double> timeRelative; ///< Parametro: Durata del periodo refrattario relativo [s]
};

/**
 * @ingroup internals
 * @brief Entità fisica del neurone Integrate-and-Fire Esponenziale con refrattarietà.
 *
 * @details
 * Modello matematico dell'evoluzione del potenziale:
 * τ * dV/dt = -(V - Vrest) + sharpness * exp((V - Vth)/sharpness) + R * I_tot(t)
 * (con τ = R * C)
 *
 * Rispetto al LIF classico, l'aggiunta del termine esponenziale modella
 * in modo più realistico la rapida depolarizzazione (upstroke) del potenziale
 * d'azione quando V si avvicina alla soglia Vth.
 *
 * @warning Non istanziare mai manualmente. Il ciclo di vita di questo oggetto
 * è gestito interamente dal motore di simulazione della classe `Rete`.
 */
class Exp {

    friend class Rete;

  public:
    /**
     * @brief Costruttore interno utilizzato da Rete.
     * @param ID Identificatore univoco del neurone nella rete.
     * @param typeIntegratore Metodo di integrazione: 'E' (Eulero in avanti) o 'R' (Runge-Kutta 4).
     */
    Exp(int ID, char typeIntegratore) : ID_(ID), tipoIntegratore_(typeIntegratore) {}
    ~Exp() = default;

  private:
    // -- ATTRIBUTI FISICI E DI STATO -------------------------------------------------

    int ID_;                          // Identificatore univoco
    double V_ = -65.0 * mV;           // Stato: Potenziale di membrana corrente
    double Vth_ = -50.0 * mV;         // Stato: Soglia adattiva corrente
    double VthMin_ = -50.0 * mV;      // Parametro: Soglia di riposo
    double VthMax_ = -35.0 * mV;      // Parametro: Soglia di picco
    double Vrest_ = -65.0 * mV;       // Parametro: Potenziale di riposo
    double Vreset_ = -70.0 * mV;      // Parametro: Potenziale post-spike
    double sharpness_ = 2.0 * mV;     // Parametro: Pendenza dell'esponenziale (Delta T)
    double R_ = 1.0 * Mohm;           // Parametro: Resistenza di membrana
    double C_ = 100.0 * p * F;        // Parametro: Capacità di membrana
    double timeAbsolute_ = 5.0 * ms;  // Parametro: Refrattarietà assoluta
    double timeRelative_ = 15.0 * ms; // Parametro: Refrattarietà relativa

    double tempoRR_ = 0.0 * s;   // Stato interno: Timer refrattarietà residua
    bool fired_ = false;         // Stato interno: Flag di spike in questo dt
    char tipoIntegratore_ = 'E'; // Impostazione: Tipo di risolutore ODE

    // -- METODI INTERNI ------------------------------------------------------

    // 1. Integratori numerici
    void euleroInAvanti(double correnteTotale, double dt);
    void rungeKutta(double correnteTotale, double dt);

    // 2. Metodi getter
    bool hasFired() const { return fired_; }
    double getPotential() const { return V_; }
    int getId() const { return ID_; }
    inline double getTau() const { return R_ * C_; }
    inline double getTauRelative() const { return timeRelative_ / 3.0; }

    // 3. Gestione evoluzione temporale
    void update(double correnteTotale, double dt);
};

#endif // EXP_HPP