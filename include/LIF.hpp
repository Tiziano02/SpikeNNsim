/**
 * @file LIF.hpp
 * @brief Definizione del modello neurale Leaky Integrate-and-Fire (LIF).
 * * Questo file contiene sia l'interfaccia pubblica per la modifica dei parametri (PatchLIF),
 * sia l'implementazione interna del modello fisico (LIF) utilizzato dal motore del simulatore.
 */

#ifndef LIF_HPP
#define LIF_HPP

#include "UnitaSI.hpp"
#include <optional>

/**
 * @ingroup publicapi
 * @brief Pacchetto dati per l'aggiornamento dei parametri di un neurone LIF.
 *
 * @details
 * Struttura utilizzata esclusivamente dal metodo `Rete::modificaParametriNeurone()`.
 * Tutti i campi sono basati su `std::optional`: se un campo non viene esplicitamente
 * impostato dall'utente, il suo valore nel neurone bersaglio rimarrà inalterato.
 * * I valori fisici devono essere passati in unità del Sistema Internazionale (SI).
 * Si consiglia l'uso dei moltiplicatori definiti in `UnitaSI.hpp` (es. `-65.0 * mV`).
 */
struct patchLIF {
    std::optional<double> V = std::nullopt;            ///< Potenziale di membrana attuale [V]
    std::optional<double> Vth = std::nullopt;          ///< Soglia dinamica attuale [V]
    std::optional<double> VthMin = std::nullopt;       ///< Parametro: Soglia minima (asintoto di riposo) [V]
    std::optional<double> VthMax = std::nullopt;       ///< Parametro: Soglia massima (picco post-spike) [V]
    std::optional<double> Vrest = std::nullopt;        ///< Parametro: Potenziale di riposo [V]
    std::optional<double> Vreset = std::nullopt;       ///< Parametro: Potenziale di reset post-spike [V]
    std::optional<double> R = std::nullopt;            ///< Parametro: Resistenza di membrana [Ω]
    std::optional<double> C = std::nullopt;            ///< Parametro: Capacità di membrana [F]
    std::optional<double> timeAbsolute = std::nullopt; ///< Parametro: Durata del periodo refrattario assoluto [s]
    std::optional<double> timeRelative = std::nullopt; ///< Parametro: Durata del periodo refrattario relativo [s]
};

/**
 * @ingroup internals
 * @brief Entità fisica del neurone Leaky Integrate-and-Fire (LIF) con refrattarietà.
 *
 * @details
 * Modello matematico dell'evoluzione del potenziale:
 * τ * dV/dt = -(V - Vrest) + R * I_tot(t)
 * (con τ = R * C)
 *
 * Dinamica della soglia adattiva (Adaptive Threshold):
 * - A riposo: Vth = VthMin
 * - Immediatamente dopo uno spike: Vth = VthMax
 * - Durante la refrattarietà relativa: Vth decade esponenzialmente verso VthMin
 * secondo la costante di tempo τ_rel = timeRelative / 3.0
 *
 * Gestione della refrattarietà:
 * 1. **Assoluta** (timeAbsolute): Il neurone è bloccato (V = Vreset) e non integra correnti.
 * 2. **Relativa** (timeRelative): Il neurone integra correnti, ma la soglia è inibitoriamente alta.
 *
 * @warning Non istanziare mai manualmente. Il ciclo di vita di questo oggetto
 * è gestito interamente dal motore di simulazione della classe `Rete`.
 */
class LIF {

    friend class Rete;

  public:
    /**
     * @brief Costruttore interno utilizzato da Rete.
     * @param typeIntegratore Metodo di integrazione: 'A' (Analitico esatto), 'E' (Eulero in avanti) o 'R' (Runge-Kutta 4).
     */
    LIF(char typeIntegratore) : tipoIntegratore_(typeIntegratore) {}
    ~LIF() = default;

  private:
    // -- ATTRIBUTI FISICI E DI STATO -----------------------------------------

    double V_ = -65.0 * mV;           // Stato: Potenziale di membrana corrente
    double Vth_ = -50.0 * mV;         // Stato: Soglia adattiva corrente
    double VthMin_ = -50.0 * mV;      // Parametro: Soglia di riposo
    double VthMax_ = -35.0 * mV;      // Parametro: Soglia di picco
    double Vrest_ = -65.0 * mV;       // Parametro: Potenziale di riposo
    double Vreset_ = -70.0 * mV;      // Parametro: Potenziale post-spike
    double R_ = 1.0 * Mohm;           // Parametro: Resistenza di membrana
    double C_ = 100.0 * p * F;        // Parametro: Capacità di membrana
    double timeAbsolute_ = 5.0 * ms;  // Parametro: Refrattarietà assoluta
    double timeRelative_ = 15.0 * ms; // Parametro: Refrattarietà relativa

    double tempoRR_ = 0.0 * s;   // Stato interno: Timer refrattarietà residua
    bool fired_ = false;         // Stato interno: Flag di spike in questo dt
    char tipoIntegratore_ = 'E'; // Impostazione: Tipo di risolutore ODE ('A', 'E', 'R')

    // Coefficienti precalcolati per l'integrazione analitica esatta ('A')
    double P_ = 0.0;            // Fattore di decadimento passivo: exp(-dt / tau)
    double C1_ = 0.0;           // Termine di riposo: Vrest * (1 - P)
    double C2_ = 0.0;           // Termine di corrente: R * (1 - P)
    double P_th_ = 0.0;         // Decadimento della soglia adattiva: exp(-dt / tau_rel)
    bool isPrepared_ = false;   // Flag di validità dei coefficienti analitici

    // -- METODI INTERNI ------------------------------------------------------

    // 1. Integratori numerici e analitici
    void euleroInAvanti(double correnteTotale, double dt);
    void rungeKutta(double correnteTotale, double dt);
    void analiticoEsatto(double correnteTotale, double dt);
    void prepare(double dt);

    // 2. Metodi getter
    bool hasFired() const { return fired_; }
    double getPotential() const { return V_; }
    inline double getTau() const { return R_ * C_; }
    inline double getTauRelative() const { return timeRelative_ / 3.0; }

    // 3. Gestione dell'evoluzione temporale
    void update(double correnteTotale, double dt);
};

#endif // LIF_HPP