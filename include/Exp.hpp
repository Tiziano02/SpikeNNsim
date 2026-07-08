#ifndef EXP_HPP
#define EXP_HPP

#include "UnitaSI.hpp"

// ============================================================================
// STRUCT DI CONFIGURAZIONE (PUBBLICA)
// ============================================================================

/**
 * configExp – parametri di configurazione per il neurone Exponential LIF.
 *
 * Passata al costruttore di Exp e a Rete::modificaParametriNeurone().
 * Tutti i valori sono in unità SI (vedi UnitaSI.hpp).
 *
 * Campi (con valori di default):
 *   V_                  – potenziale iniziale di membrana      [V]   default: -65 mV
 *   V_th                – soglia di firing a riposo            [V]   default: -50 mV
 *   V_ThresholdSpikeMax – soglia massima dopo uno spike        [V]   default: -35 mV
 *   V_rest              – potenziale di riposo                 [V]   default: -65 mV
 *   V_reset             – potenziale di reset post‑spike       [V]   default: -70 mV
 *   sharpness           – fattore di sharpness Δ_T             [V]   default: 2.5 mV
 *   R                   – resistenza di ingresso               [Ω]   default: 1 MΩ
 *   C                   – capacità di membrana                 [F]   default: 100 pF  (τ = 100 ms)
 *   timeAbsolute        – durata refrattarietà assoluta        [s]   default: 5 ms
 *   timeRelative        – durata refrattarietà relativa        [s]   default: 15 ms
 */
struct configExp {
    double V_ = -65.0 * mV;
    double V_th = -50.0 * mV;
    double V_ThresholdSpikeMax = -35.0 * mV;
    double V_rest = -65.0 * mV;
    double V_reset = -70.0 * mV;
    double sharpness = 2.5 * mV;
    double R = 1.0 * Mohm;
    double C = 100.0 * p * F;
    double timeAbsolute = 5.0 * ms;
    double timeRelative = 15.0 * ms;
};

// ============================================================================
// CLASSE Exp (PRIVATA – USO INTERNO)
// ============================================================================

/**
 * Exp – neurone Exponential Leaky Integrate‑and‑Fire (versione semplificata di AdEx).
 *
 * Friend di Rete: non istanziare direttamente, usare Rete::aggiungiNeurone().
 *
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
    double sharpness_;    // parametro modello Exp
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
     * Costruisce un neurone Exp.
     * Non chiamare direttamente: usare Rete::aggiungiNeurone().
     *
     * @param id                identificatore univoco
     * @param typeIntegratore   'E' o 'R'
     * @param config            parametri di configurazione
     */
    Exp(int id, char typeIntegratore, configExp config)
        : id_(id), V_(config.V_), Vth_(config.V_th), Vth0_(config.V_th), VthSpikeMax_(config.V_ThresholdSpikeMax),
          Vrest_(config.V_rest), Vreset_(config.V_reset), R_(config.R), C_(config.C), tau_(config.R * config.C),
          timeAbsolute_(config.timeAbsolute), timeRelative_(config.timeRelative), tempoRR_(0.0),
          tauRelative_(config.timeRelative / 3), sharpness_(config.sharpness), fired_(false),
          tipoIntegratore_(typeIntegratore) {}

    ~Exp() = default;
};

#endif // EXP_HPP