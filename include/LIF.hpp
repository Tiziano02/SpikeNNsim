#ifndef LIF_HPP
#define LIF_HPP

#include "UnitaSI.hpp"

// ============================================================================
// STRUCT DI CONFIGURAZIONE (PUBBLICA)
// ============================================================================

/**
 * @ingroup publicapi
 * @brief Parametri di configurazione per il neurone LIF (Leaky Integrate-and-Fire).
 *
 * Passata al costruttore di LIF e a Rete::modificaParametriNeurone().
 * Tutti i valori sono in unità SI (vedi UnitaSI.hpp).
 */
struct configLIF {
    double V_ = -65.0 * mV;                  ///< Potenziale di membrana iniziale [V]
    double V_th = -50.0 * mV;                ///< Soglia di attivazione a riposo [V]
    double V_ThresholdSpikeMax = -35.0 * mV; ///< Soglia massima raggiunta subito dopo uno spike [V]
    double V_rest = -65.0 * mV;              ///< Potenziale di riposo [V]
    double V_reset = -70.0 * mV;             ///< Potenziale a cui il neurone torna dopo lo spike [V]
    double R = 1.0 * Mohm;                   ///< Resistenza di ingresso [Ω]
    double C = 100.0 * p * F;                ///< Capacità di membrana [F] (tau = R * C)
    double timeAbsolute = 5.0 * ms;          ///< Durata della refrattarietà assoluta [s]
    double timeRelative = 15.0 * ms;         ///< Durata della refrattarietà relativa (decadimento soglia) [s]
};

// ============================================================================
// CLASSE LIF (PRIVATA – USO INTERNO)
// ============================================================================

/**
 * @ingroup internals
 * @brief Neurone Leaky Integrate-and-Fire con refrattarietà assoluta e relativa.
 *
 * Friend di Rete: usare Rete::aggiungiNeurone().
 *
 * Equazione:
 *   τ * dV/dt = -(V - Vrest) + R * I_tot(t)
 *
 * Quando V ≥ Vth: spike, reset a Vreset, alza la soglia a VthSpikeMax
 * e inizia il periodo refrattario assoluto. Durante la refrattarietà relativa
 * la soglia decade esponenzialmente verso Vth0.
 *
 * Integratori: 'E' (Eulero) o 'R' (Runge‑Kutta 4).
 */
class LIF {

    friend class Rete;

    // ── ATTRIBUTI PRIVATI ────────────────────────────────────────────────────

  private:
    int id_;              // id del neruone
    double V_;            // potenziale di membrana
    double Vth_;          // potenziale soglia del neurone, variabile nel tempo
    double Vth0_;         // potenziale soglia a riposo --> no spike --> soglia più bassa possibile
    double VthSpikeMax_;  // potenziale soglia appena il neurone emette uno spike --> soglia più alta possibile
    double Vrest_;        // potenziale di membrana a riposo
    double Vreset_;       // potenziale di membrana dopo uno spike
    double R_;            // resistenza neurone
    double C_;            // capacità neurone
    double tau_;          // tempo scala di decadimento del neurone verso il potenziale di riposo : tau_ = R_* C_
    double timeAbsolute_; // tempo refrattario assoluto
    double timeRelative_; // tempo refrattario relativo
    double tempoRR_;      // tempo refrattario assoluto rimanente
    double tauRelative_;  // tempo scala di decadimento della soglia
    bool fired_;          // stato del neurone
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

    // ── COSTRUTTORE / DISTRUTTORE ──────────────────────────────────────────

  public:
    /**
     * Costruisce un neurone LIF.
     * Non chiamare direttamente: usare Rete::aggiungiNeurone().
     */
    LIF(int id, char typeIntegratore, configLIF config)
        : id_(id), V_(config.V_), Vth_(config.V_th), Vth0_(config.V_th), VthSpikeMax_(config.V_ThresholdSpikeMax),
          Vrest_(config.V_rest), Vreset_(config.V_reset), R_(config.R), C_(config.C), tau_(config.R * config.C),
          timeAbsolute_(config.timeAbsolute), timeRelative_(config.timeRelative), tempoRR_(0.0),
          tauRelative_(config.timeRelative / 3), fired_(false), tipoIntegratore_(typeIntegratore) {}

    ~LIF() = default;
};

#endif // LIF_HPP