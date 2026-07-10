#ifndef CURRENTSYN_HPP
#define CURRENTSYN_HPP

#include <cmath>
#include <cstddef>
#include <vector>
#include <optional>
#include <UnitaSI.hpp>

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
struct patchCurrent {
    std::optional<double> Isyn; // [A]
    std::optional<double> peso;
    std::optional<double> Ipeak; // [A]
    std::optional<double> tau;   // [s]
    std::optional<double> delay; // [s]
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
class Current {

    friend class Rete;

    // ── ATTRIBUTI PRIVATI ────────────────────────────────────────────────────

  private:
    int idPre_, idPost_;
    size_t indexPre_, indexPost_;
    double Isyn_ = 0.0 * A; // [A]
    double peso_ = 1.0;
    double Ipeak_ = 0.1 * n * A; // [A]
    double tau_ = 5 * ms;        // [s]
    double delay_ = 1 * ms;      // [s]
    size_t presentStep_ = 0;
    size_t delayStep_ = 0;
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
    Current(size_t indexPre, size_t indexPost, int idPre, int idPost)
        : idPre_(idPre), idPost_(idPost), indexPre_(indexPre), indexPost_(indexPost) {}

    ~Current() = default;
};

#endif // CURRENTSYN_HPP