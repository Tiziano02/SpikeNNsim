#ifndef CONDUCTANCESYN_HPP
#define CONDUCTANCESYN_HPP

#include <cmath>
#include <cstddef>
#include <vector>
#include <UnitaSI.hpp>

/**
 * @ingroup publicapi
 * @brief Struct che definisce i parametri biologici di una sinapsi conductance-based
 *
 * @details Per connettere due neuroni tramite il metodo Rete::connettiNeuroni(IDpre, IDpost,configurazioneSinapisi)
 * è necessario utilizzare una configuraione di una sinaspe tra quelle possibili. Una di quelle
 * possibili è la sinapsi di tipo conductance-based. Per definire una sinapsi di questo tipo è necessario passare al
 * metodo Rete::connettiNeuroni una configurazione definita da questa struct.
 *
 * @param gpeak Quando avviene uno spike la conduttanza aumenta in questo modo, dopo il ritardo, gsyn =+ peso * gpeak
 * @param gsyn La dinamica della sinapsi è la seguente d/dt gsyn = - 1/tau * gsyn
 */
struct configConductanceSyn {
    double peso = 1.0;         ///< Forza della sinapsi
    double gpeak = 1 * n * S;  ///< conduttanza di picco dovuto ad uno spike
    double gsyn = 0.0 * S;     ///< conduttanza della sinapsi, variabile dinamica
    double tau = 5 * ms;       ///< scala temporale di decadimento della dinamica della sinapsi
    double delay = 1 * ms;     ///< ritardo sinaptico
    double E_rev = 0.0 * Volt; ///< potenziale di reverse della sinapsi
};

/**
 * @ingroup internals
 * @brief Classe che permette la creazione di oggetti di tipo sinapsi conductance-based
 *
 * @details Questa classe, e quindi il suo costruttuore, non può e non deve essere usata dall'utente. Soltanto il metodo
 * Rete::connettiNeurone chiama il costrutture di questa classe. Oltre ai metodi getter e setter utili alla classe Rete,
 * ha un unico metodo applicativo : update(dt, preFired, V_post). Questo metodo permette alla dinamica della sinapsi di
 * avanzanzare step alla volta
 *
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

    void update(double dt, bool preFired, double V_post);

    void setIndexPre(size_t idx) { indexPre_ = idx; }
    void setIndexPost(size_t idx) { indexPost_ = idx; }
    void setDelayRing(double dt) {
        delayStep_ = static_cast<size_t>(std::round(delay_ / dt));
        delayRing_.assign(delayStep_ + 1, 0.0);
    }

    double getCurrent() const { return Isyn_; }
    int getIdPre() const { return idPre_; }
    int getIdPost() const { return idPost_; }
    size_t getIndexPre() const { return indexPre_; }
    size_t getIndexPost() const { return indexPost_; }

    // ── COSTRUTTORE / DISTRUTTORE (PUBBLICI MA NON USATI DIRETTAMENTE) ──

  public:
    ConductanceSyn(size_t indexPre, size_t indexPost, int idPre, int idPost, configConductanceSyn config)
        : idPre_(idPre), idPost_(idPost), indexPre_(indexPre), indexPost_(indexPost), peso_(config.peso),
          gpeak_(config.gpeak), gsyn_(0.0), Isyn_(0.0), tau_(config.tau), delay_(config.delay), E_rev_(config.E_rev),
          presentStep_(0), delayStep_(0) {}

    ~ConductanceSyn() = default;
};

#endif // CONDUCTANCESYN_HPP