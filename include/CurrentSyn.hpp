#ifndef CURRENTSYN_HPP
#define CURRENTSYN_HPP

#include <cmath>
#include <cstddef>
#include <vector>

// ============================================================================
// STRUCT DI CONFIGURAZIONE (PUBBLICA)
// ============================================================================

/**
 * configCurrentSyn – parametri di configurazione per una sinapsi current‑based.
 *
 * Passata a Rete::connettiNeuroni() e a Rete::modificaSinapsi().
 * Tutti i valori sono in unità SI (vedi UnitaSI.hpp).
 *
 * Campi:
 *   peso   – fattore di scala del kick di corrente ad ogni spike   [-]   default: 1.0
 *   Ipeak  – corrente di picco per peso unitario                   [A]   default: 100 pA
 *   Isyn   – corrente iniziale (di solito 0)                       [A]   default: 0
 *   tau    – costante di tempo del decadimento esponenziale        [s]   default: 5 ms
 *   delay  – ritardo sinaptico                                     [s]   default: 1 ms
 *
 * Convenzione dei segni:
 *   peso > 0  -> I_syn > 0  -> inputTotale diminuisce  -> inibitoria
 *   peso < 0  -> I_syn < 0  -> inputTotale aumenta     -> eccitatoria
 */
struct configCurrentSyn {
    double peso = 1.0;
    double Ipeak = 1e-10; // [A]
    double Isyn = 0.0;    // [A]
    double tau = 5e-3;    // [s]
    double delay = 1e-3;  // [s]
};

// ============================================================================
// CLASSE CurrentSyn (PRIVATA – USO INTERNO)
// ============================================================================

/**
 * CurrentSyn – sinapsi current‑based con decadimento esponenziale e ritardo.
 *
 * Friend di Rete: non istanziare direttamente, usare Rete::connettiNeuroni().
 *
 * Modello matematico:
 *   dI_syn/dt = -I_syn / tau
 *
 * Ad ogni spike pre‑sinaptico (dopo delay):
 *   I_syn += peso * Ipeak
 *
 * La corrente non dipende dal potenziale post‑sinaptico: il segno è
 * determinato dal peso e da Ipeak (vedi configCurrentSyn).
 *
 * Il ritardo è gestito con un ring buffer.
 */
class CurrentSyn {

    friend class Rete;

    // ── ATTRIBUTI PRIVATI ────────────────────────────────────────────────────

  private:
    int idPre_, idPost_;
    size_t indexPre_, indexPost_;
    double peso_;
    double Ipeak_;
    double Isyn_;
    double tau_;
    double delay_;
    size_t presentStep_;
    size_t delayStep_;
    std::vector<double> delayRing_;

    // ── METODI PRIVATI ──────────────────────────────────────────────────────

    /**
     * Avanza lo stato della sinapsi di un passo dt.
     * Chiamato da Rete::step().
     */
    void update(double dt, bool preFired);

    // Setter per indici (usati da Rete)
    void setIndexPre(size_t idx) { indexPre_ = idx; }
    void setIndexPost(size_t idx) { indexPost_ = idx; }

    /**
     * Alloca il ring buffer per il ritardo.
     * Chiamato da Rete::prepare().
     */
    void setDelayRing(double dt) {
        delayStep_ = static_cast<size_t>(std::round(delay_ / dt));
        delayRing_.assign(delayStep_ + 1, 0.0);
    }

    // Getter per Rete
    double getCurrent() const { return Isyn_; }
    int getIdPre() const { return idPre_; }
    int getIdPost() const { return idPost_; }
    size_t getIndexPre() const { return indexPre_; }
    size_t getIndexPost() const { return indexPost_; }

    // ── COSTRUTTORE / DISTRUTTORE ──────────────────────────────────────────

  public:
    /**
     * Costruisce una sinapsi current‑based.
     * Non chiamare direttamente: usare Rete::connettiNeuroni().
     */
    CurrentSyn(size_t indexPre, size_t indexPost, int idPre, int idPost, configCurrentSyn config)
        : idPre_(idPre), idPost_(idPost), indexPre_(indexPre), indexPost_(indexPost), peso_(config.peso),
          Ipeak_(config.Ipeak), Isyn_(0.0), tau_(config.tau), delay_(config.delay), presentStep_(0), delayStep_(0) {}

    ~CurrentSyn() = default;
};

#endif // CURRENTSYN_HPP