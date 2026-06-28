#ifndef CURRENTSYN_HPP
#define CURRENTSYN_HPP

#include <cmath>
#include <cstddef>
#include <vector>

struct configCurrentSyn {
    double peso   = 1.0;
    double Ipeak  = 1e-10;  // [A]
    double Isyn   = 0.0;    // [A]
    double tau    = 5e-3;   // [s]
    double delay  = 1e-3;   // [s]
};

/*
 * CurrentSyn — sinapsi current-based con decadimento esponenziale.
 *
 * Modello matematico:
 *   dI_syn/dt = -I_syn / tau
 *
 * Ad ogni spike pre-sinaptico:
 *   I_syn -> I_syn + peso * Ipeak
 *
 * Il segno di peso determina il tipo:
 *   peso > 0  ->  eccitatoria
 *   peso < 0  ->  inibitoria
 *
 * Firma di update:
 *   update(dt, preFired)
 *   (non dipende dal potenziale post-sinaptico)
 */
class CurrentSyn {

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

    // metodi operativi
    void update(double dt, bool preFired);

    // metodi setter
    void setIndexPre(size_t idx)  { indexPre_  = idx; }
    void setIndexPost(size_t idx) { indexPost_ = idx; }
    void setDelayRing(double dt) {
        delayStep_ = static_cast<size_t>(std::round(delay_ / dt));
        delayRing_.assign(delayStep_ + 1, 0.0);
    }

    // metodi getter
    double getCurrent()    const { return Isyn_; }
    int    getIdPre()      const { return idPre_;    }
    int    getIdPost()     const { return idPost_;   }
    size_t getIndexPre()   const { return indexPre_; }
    size_t getIndexPost()  const { return indexPost_;}

  public:
    CurrentSyn(size_t indexPre, size_t indexPost, int idPre, int idPost,
               configCurrentSyn config)
        : idPre_(idPre), idPost_(idPost),
          indexPre_(indexPre), indexPost_(indexPost),
          peso_(config.peso), Ipeak_(config.Ipeak),
          Isyn_(0.0), tau_(config.tau), delay_(config.delay),
          presentStep_(0), delayStep_(0) {}

    ~CurrentSyn() = default;

    friend class Rete;
};

#endif // CURRENTSYN_HPP
