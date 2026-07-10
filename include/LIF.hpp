#ifndef LIF_HPP
#define LIF_HPP

#include "UnitaSI.hpp"
#include <optional>

// ============================================================================
// STRUCT DI CONFIGURAZIONE (PUBBLICA)
// ============================================================================

/**
 * @ingroup publicapi
 * @brief Insieme di parametri modificabili dopo la creazione per il neurone LIF (Leaky Integrate-and-Fire).
 *
 * Utilizzata esclusivamente da Rete::modificaParametriNeurone().
 * Tutti i valori sono in unità SI (vedi UnitaSI.hpp).
 */
struct patchLIF {
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
    int id_;                          // id del neruone
    double V_ = -65.0 * mV;           // potenziale di membrana
    double Vth_ = -50.0 * mV;         // potenziale soglia del neurone, variabile nel tempo
    double VthMin_ = -50.0 * mV;      // potenziale soglia minimo
    double VthMax_ = -35.0 * mV;      // potenziale soglia massima
    double Vrest_ = -65.0 * mV;       // potenziale di membrana a riposo
    double Vreset_ = -70.0 * mV;      // potenziale di membrana dopo uno spike
    double R_ = 1.0 * Mohm;           // resistenza neurone
    double C_ = 100.0 * p * F;        // capacità neurone
    double timeAbsolute_ = 5.0 * ms;  // tempo refrattario assoluto
    double timeRelative_ = 15.0 * ms; // tempo refrattario relativo
    double tempoRR_ = 0.0 * s;        // tempo refrattario assoluto rimanente
    bool fired_ = false;              // stato del neurone
    char tipoIntegratore_ = 'E';

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
    /**
     * Costruisce un neurone LIF.
     * Non chiamare direttamente: usare Rete::aggiungiNeurone().
     */
    LIF(int id, char typeIntegratore) : id_(id), tipoIntegratore_(typeIntegratore) {}

    ~LIF() = default;
};

#endif // LIF_HPP