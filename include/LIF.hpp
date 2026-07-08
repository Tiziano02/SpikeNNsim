#ifndef LIF_HPP
#define LIF_HPP

#include "UnitaSI.hpp"

// ============================================================================
// STRUCT DI CONFIGURAZIONE (PUBBLICA)
// ============================================================================

/**
 * configLIF – parametri di configurazione per il neurone LIF.
 *
 * Passata al costruttore di LIF e a Rete::modificaParametriNeurone().
 * Tutti i valori in unità SI.
 *
 * Campi:
 *   V_                  – potenziale iniziale [V]   default: -65 mV
 *   V_th                – soglia a riposo       [V]   default: -50 mV
 *   V_ThresholdSpikeMax – soglia massima post‑spike [V]   default: -35 mV
 *   V_rest              – potenziale di riposo  [V]   default: -65 mV
 *   V_reset             – reset dopo spike      [V]   default: -70 mV
 *   R                   – resistenza di ingresso [Ω]   default: 1 MΩ
 *   C                   – capacità di membrana  [F]   default: 100 pF
 *   timeAbsolute        – refrattarietà assoluta [s]   default: 5 ms
 *   timeRelative        – refrattarietà relativa [s]   default: 15 ms
 */
struct configLIF {
    double V_ = -65.0 * mV;
    double V_th = -50.0 * mV;
    double V_ThresholdSpikeMax = -35.0 * mV;
    double V_rest = -65.0 * mV;
    double V_reset = -70.0 * mV;
    double R = 1.0 * Mohm;
    double C = 100.0 * p * F;
    double timeAbsolute = 5.0 * ms;
    double timeRelative = 15.0 * ms;
};

// ============================================================================
// CLASSE LIF (PRIVATA – USO INTERNO)
// ============================================================================

/**
 * LIF – neurone Leaky Integrate‑and‑Fire con refrattarietà assoluta e relativa.
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