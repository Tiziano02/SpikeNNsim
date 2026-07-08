#ifndef CONDUCTANCESYN_HPP
#define CONDUCTANCESYN_HPP

#include <cmath>
#include <cstddef>
#include <vector>

// ============================================================================
// STRUCT DI CONFIGURAZIONE (PUBBLICA)
// ============================================================================

/**
 * configConductanceSyn – parametri di configurazione per una sinapsi conductance‑based.
 *
 * Questa struct viene passata a Rete::connettiNeuroni() e a Rete::modificaSinapsi()
 * per specificare le proprietà della sinapsi. Tutti i valori sono in unità SI
 * (vedi UnitaSI.hpp).
 *
 * Campi (con valori di default):
 *   peso   – fattore di scala del kick di conduttanza ad ogni spike   [-]   default: 1.0
 *   gpeak  – conduttanza di picco per peso unitario                   [S]   default: 1 nS
 *   gsyn   – conduttanza iniziale (di solito lasciare a 0)            [S]   default: 0
 *   tau    – costante di tempo del decadimento esponenziale           [s]   default: 5 ms
 *   delay  – ritardo sinaptico                                        [s]   default: 1 ms
 *   E_rev  – potenziale di inversione                                 [V]   default: 0 V
 */
struct configConductanceSyn {
    double peso = 1.0;   // [-]
    double gpeak = 1e-9; // [S]
    double gsyn = 0.0;   // [S]
    double tau = 5e-3;   // [s]
    double delay = 1e-3; // [s]
    double E_rev = 0.0;  // [V]
};

// ============================================================================
// CLASSE ConductanceSyn (PRIVATA – USO INTERNO)
// ============================================================================

/**
 * ConductanceSyn – sinapsi conductance‑based con decadimento esponenziale e ritardo.
 *
 * Questa classe è friend di Rete e NON deve essere istanziata direttamente
 * dall'utente: usare sempre Rete::connettiNeuroni().
 *
 * Modello matematico:
 *   dg_syn/dt = -g_syn / tau
 *
 * Ad ogni spike pre‑sinaptico (dopo il ritardo delay):
 *   g_syn += peso * gpeak
 *
 * La corrente sinaptica è:
 *   I_syn = g_syn * (V_post - E_rev)
 *
 * In Rete::step() la corrente viene sottratta all'input totale del neurone post.
 * Il segno dell'effetto dipende da V_post rispetto a E_rev:
 *   - se V_post > E_rev  -> I_syn > 0  -> depolarizzante (eccitatoria)
 *   - se V_post < E_rev  -> I_syn < 0  -> iperpolarizzante (inibitoria)
 *
 * A differenza di CurrentSyn, la corrente dipende dal potenziale post‑sinaptico,
 * rendendo il modello più realistico.
 */
class ConductanceSyn {

    friend class Rete; // Consente a Rete di accedere a tutti i membri privati

    // ── ATTRIBUTI PRIVATI ────────────────────────────────────────────────────

  private:
    int idPre_, idPost_;            // ID dei neuroni pre e post
    size_t indexPre_, indexPost_;   // indici interni nei vettori di Rete
    double peso_;                   // scalare del kick di conduttanza [-]
    double gpeak_;                  // conduttanza di picco per peso unitario [S]
    double gsyn_;                   // conduttanza corrente [S]
    double Isyn_;                   // corrente sinaptica corrente [A]  (= gsyn_ * (V_post - E_rev))
    double tau_;                    // costante di tempo del decadimento [s]
    double delay_;                  // ritardo sinaptico [s]
    double E_rev_;                  // potenziale di inversione [V]
    size_t presentStep_;            // posizione corrente nel ring buffer
    size_t delayStep_;              // numero di passi corrispondenti al delay
    std::vector<double> delayRing_; // ring buffer per il ritardo

    // ── METODI PRIVATI ──────────────────────────────────────────────────────

    /**
     * Avanza lo stato della sinapsi di un passo temporale dt.
     * Viene chiamato da Rete::step().
     *
     * @param dt       passo temporale [s]
     * @param preFired true se il neurone pre ha generato uno spike in questo step
     * @param V_post   potenziale del neurone post (necessario per la corrente)
     */
    void update(double dt, bool preFired, double V_post);

    // Setter per gli indici interni (usati da Rete)
    void setIndexPre(size_t idx) { indexPre_ = idx; }
    void setIndexPost(size_t idx) { indexPost_ = idx; }

    /**
     * Alloca il ring buffer per il ritardo in base al passo temporale dt.
     * Viene chiamato da Rete::prepare() prima dell'avvio.
     */
    void setDelayRing(double dt) {
        delayStep_ = static_cast<size_t>(std::round(delay_ / dt));
        delayRing_.assign(delayStep_ + 1, 0.0);
    }

    // Getter per l'accesso da parte di Rete
    double getCurrent() const { return Isyn_; }
    int getIdPre() const { return idPre_; }
    int getIdPost() const { return idPost_; }
    size_t getIndexPre() const { return indexPre_; }
    size_t getIndexPost() const { return indexPost_; }

    // ── COSTRUTTORE / DISTRUTTORE (PUBBLICI MA NON USATI DIRETTAMENTE) ──

  public:
    /**
     * Costruisce una sinapsi conductance‑based.
     * Non chiamare direttamente: usare Rete::connettiNeuroni().
     * Il ring buffer viene allocato successivamente da Rete::prepare().
     */
    ConductanceSyn(size_t indexPre, size_t indexPost, int idPre, int idPost, configConductanceSyn config)
        : idPre_(idPre), idPost_(idPost), indexPre_(indexPre), indexPost_(indexPost), peso_(config.peso),
          gpeak_(config.gpeak), gsyn_(0.0), Isyn_(0.0), tau_(config.tau), delay_(config.delay), E_rev_(config.E_rev),
          presentStep_(0), delayStep_(0) {}

    ~ConductanceSyn() = default;
};

#endif // CONDUCTANCESYN_HPP