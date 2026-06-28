#ifndef SINAPSI_HPP
#define SINAPSI_HPP

// #include "UnitaSI.hpp"

#include <cmath>   // necessaria per round()
#include <cstddef> // necessaria per i size_t
#include <vector>

struct configSyn {
    double peso;
    double Ipeak;
    double Isyn;
    double tau;
    double delay;
};

/*
 * Sinapsi — connessione current-based con decadimento esponenziale.
 *
 * Modello matematico:
 * dI_syn/dt = -I_syn / tau
 *
 * Ad ogni spike pre-sinaptico la corrente riceve un incremento istantaneo:
 * I_syn -> I_syn + peso * Ipeak
 *
 * Il segno di peso determina il tipo di sinapsi:
 * peso > 0  ->  eccitatoria  (corrente depolarizzante)
 * peso < 0  ->  inibitoria   (corrente iperpolarizzante)
 *
 * Parametri:
 * peso        peso sinaptico, range consigliato [-1, +1]
 * Ipeak       corrente di picco per peso unitario [A]
 * idPre       ID del neurone pre-sinaptico
 * idPost      ID del neurone post-sinaptico
 * indexPre_   indice del neurone pre-sinaptico in neuroni_ (settato da Rete)
 * indexPost_  indice del neurone post-sinaptico in neuroni_ (settato da Rete)
 * tau         costante di tempo del decadimento [s]
 *
 * Metodi publici:
 * update(dt, preFired)    — avanza lo stato della sinapsi di un passo dt
 * setIndexPre(idx)        — imposta l'indice del neurone pre-sinaptico
 * setIndexPost(idx)       — imposta l'indice del neurone post-sinaptico
 * getCurrent()            — restituisce I_syn corrente [A]
 * getIdPre()              — restituisce l'ID del neurone pre-sinaptico
 * getIdPost()             — restituisce l'ID del neurone post-sinaptico
 * getIndexPre()           — restituisce l'indice del neurone pre-sinaptico
 * getIndexPost()          — restituisce l'indice del neurone post-sinaptico
 */
class Sinapsi {

  private:
    int idPre_, idPost_;
    size_t indexPre_, indexPost_;
    double peso_;
    double Ipeak_;
    double Isyn_;
    double tau_;
    double delay_;

    // Paramtri per il ring --> parametri della simualzione
    size_t presentStep_;
    size_t delayStep_;
    std::vector<double> delayRing_;

    // metodi operativi
    void update(double dt, bool preFired);

    // metodi setter
    void setIndexPre(size_t idx) { indexPre_ = idx; }
    void setIndexPost(size_t idx) { indexPost_ = idx; }
    void setDelayRing(double dt) {
        delayStep_ = static_cast<size_t>(round(delay_ / dt));
        delayRing_.assign(delayStep_ + 1, 0.0);
    }

    // metodi getter
    double getCurrent() const { return Isyn_; }
    int getIdPre() const { return idPre_; }
    int getIdPost() const { return idPost_; }
    size_t getIndexPre() const { return indexPre_; }
    size_t getIndexPost() const { return indexPost_; }

  public:
    Sinapsi(size_t indexPre, size_t indexPost, int idPre, int idPost, configSyn config) : idPre_(idPre), idPost_(idPost), indexPre_(indexPre), indexPost_(indexPost), peso_(config.peso), Ipeak_(config.Ipeak), Isyn_(0.0), tau_(config.tau), delay_(config.delay), presentStep_(0), delayStep_(0) {}
    ~Sinapsi() = default;

    friend class Rete;
};

#endif // SINAPSI_HPP