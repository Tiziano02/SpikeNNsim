#ifndef CONDUCTANCESYN_HPP
#define CONDUCTANCESYN_HPP

#include <cmath>
#include <cstddef>
#include <vector>
#include <optional>
#include <UnitaSI.hpp>

/**
 * @ingroup publicapi
 * @brief Parametri biologici per configurare una sinapsi conductance-based
 *
 * @details Per connettere due neuroni tramite il metodo Rete::connettiNeuroni(IDpre, IDpost,configurazioneSinapisi)
 * è necessario fornire al metodo determinati parametri biologici a seconda della tipologia di sinapsi che si vuole
 * creare. Per creare una sinapsi conductance-based è necessario fornire i parametri all'inteno di questa struct.
 * Inoltre
 *
 * @param gpeak Quando avviene uno spike la conduttanza aumenta in questo modo, dopo il ritardo, gsyn =+ peso * gpeak
 * @param gsyn La dinamica della sinapsi è la seguente d/dt gsyn = - 1/tau * gsyn
 */
struct patchConductance {
    std::optional<double> peso = 1.0;        ///< Forza della sinapsi
    std::optional<double> gpeak = 1 * n * S; ///< conduttanza di picco dovuto ad uno spike
    std::optional<double> gsyn = 0.0 * S;    ///< conduttanza della sinapsi, variabile dinamica
    std::optional<double> tau = 5 * ms;      ///< scala temporale di decadimento della dinamica della sinapsi
    std::optional<double> delay = 1 * ms;    ///< ritardo sinaptico
    std::optional<double> Erev = 0.0 * Volt; ///< potenziale di reverse della sinapsi
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
class Conductance {

    friend class Rete; // Consente a Rete di accedere a tutti i membri privati

    // ── ATTRIBUTI PRIVATI ────────────────────────────────────────────────────

  private:
    int idPre_, idPost_;            // ID dei neuroni pre e post
    size_t indexPre_, indexPost_;   // indici interni nei vettori di Rete
    double Isyn_ = 0 * A;           // corrente sinaptica corrente [A]  (= gsyn_ * (V_post - E_rev))
    double peso_ = 1.0;             // Forza della sinapsi
    double gpeak_ = 1 * n * S;      // conduttanza di picco dovuto ad uno spike
    double gsyn_ = 0.0 * S;         // conduttanza della sinapsi, variabile dinamica
    double tau_ = 5 * ms;           // scala temporale di decadimento della dinamica della sinapsi
    double delay_ = 1 * ms;         // ritardo sinaptico
    double Erev_ = 0.0 * Volt;      // potenziale di reverse della sinapsi
    size_t presentStep_ = 0;        // posizione corrente nel ring buffer
    size_t delayStep_ = 0;          // numero di passi corrispondenti al delay
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
    Conductance(size_t indexPre, size_t indexPost, int idPre, int idPost)
        : idPre_(idPre), idPost_(idPost), indexPre_(indexPre), indexPost_(indexPost) {}

    ~Conductance() = default;
};

#endif // CONDUCTANCESYN_HPP