#ifndef EXP_HPP
#define EXP_HPP

#include "UnitaSI.hpp"
#include <optional>

/**
 * @ingroup publicapi
 * @brief Insieme di parametri modificabili dei neuroni di tipo Exp
 *
 * Utilizzata esclusivamente da Rete::modificaParametriNeurone()
 * Tutti i valori sono in unità SI (vedi UnitaSI.hpp).
 */
struct patchExp {
    std::optional<double> V;            ///< Potenziale di membrana iniziale [V]
    std::optional<double> Vth;          ///< Soglia di attivazione iniziale [V]
    std::optional<double> VthMin;       ///< Soglia minima, la dinamica di Vth tende a questa soglia [V]
    std::optional<double> VthMax;       ///< Soglia massima raggiunta subito dopo uno spike [V]
    std::optional<double> Vrest;        ///< Potenziale di riposo [V]
    std::optional<double> Vreset;       ///< Potenziale a cui il neurone torna dopo lo spike [V]
    std::optional<double> R;            ///< Resistenza di ingresso [Ω]
    std::optional<double> C;            ///< Capacità di membrana [F] (tau = R * C)
    std::optional<double> timeAbsolute; ///< Durata della refrattarietà assoluta [s]
    std::optional<double> timeRelative; ///< Durata della refrattarietà relativa (decadimento soglia) [s]
    std::optional<double> sharpness;    ///< Fattore di pendenza dell'esponenziale (Delta_T) [V]
};

/**
 * @ingroup internals
 * @brief neurone Exponential Leaky Integrate‑and‑Fire
 *
 * Friend di Rete: non istanziare direttamente, usare costrutture di Rete oppure il metodo aggiungiNeurone()
 * (sconsigliato).
 *
 * @details
 * Equazione:
 *   τ * dV/dt = -(V - Vrest) + Δ_T * exp((V - Vth)/Δ_T) + R * I_tot(t)
 *   τ = R * C
 *
 * Il termine esponenziale ammorbidisce la soglia, rendendo il firing più graduale.
 * La refrattarietà (assoluta e relativa) è gestita come nel LIF.
 *
 * Integratori disponibili (passati al costruttore):
 *   'E' – Eulero in avanti (instabile per dt grandi)
 *   'R' – Runge‑Kutta 4 (consigliato)
 */
class Exp {

    friend class Rete;

    // ── ATTRIBUTI PRIVATI ────────────────────────────────────────────────────

  private:
    int id_;                     // id del neruone
    double V_ = -65.0 * mV;      // potenziale di membrana
    double Vth_ = -50.0 * mV;    // potenziale soglia del neurone, variabile nel tempo
    double VthMin_ = -50.0 * mV; // potenziale soglia a riposo --> no spike --> soglia più bassa possibile
    double VthMax_ = -35.0 * mV; // potenziale soglia appena il neurone emette uno spike --> soglia più alta possibile
    double Vrest_ = -65.0 * mV;  // potenziale di membrana a riposo
    double Vreset_ = -70.0 * mV; // potenziale di membrana dopo uno spike
    double R_ = 1.0 * Mohm;      // resistenza neurone
    double C_ = 100.0 * p * F;   // capacità neurone
    double timeAbsolute_ = 5.0 * ms;  // tempo refrattario assoluto
    double timeRelative_ = 15.0 * ms; // tempo refrattario relativo
    double tempoRR_ = 0.0;            // tempo refrattario assoluto rimanente
    double sharpness_ = 2.5 * mV;     // parametro modello Exp
    bool fired_ = false;              // stato del neurone
    char tipoIntegratore_;

    // ── METODI PRIVATI ──────────────────────────────────────────────────────

    void euleroInAvanti(double correnteTotale, double dt);
    void rungeKutta(double correnteTotale, double dt);

    /**
     * Avanza lo stato del neurone di un passo dt.
     * Chiamato da Rete::step().
     */
    void update(double correnteTotale, double dt);

    // Getter per Rete
    bool hasFired() const { return fired_; }
    double getPotential() const { return V_; }
    int getId() const { return id_; }

    inline double getTau() const { return R_ * C_; }

    inline double getTauRelative() const { return timeRelative_ / 3.0; }

    // ── COSTRUTTORE / DISTRUTTORE ──────────────────────────────────────────

  public:
    /// Costruisce un neurone Exp.
    /// Non chiamare direttamente: usare Rete::aggiungiNeurone().
    ///
    /// \param id Identificatore univoco del neurone.
    /// \param typeIntegratore Metodo di integrazione: 'E' per Eulero in avanti, 'R' per Runge-Kutta 4 (consigliato).
    /// \param config Struttura configExp con i parametri di configurazione.
    Exp(int id, char typeIntegratore) : id_(id), tipoIntegratore_(typeIntegratore) {}

    ~Exp() = default;
};

#endif // EXP_HPP