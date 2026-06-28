#include "ConductanceSyn.hpp"

/*
 * update — avanza lo stato della sinapsi di un passo dt.
 *
 * Sequenza:
 *  1. Decadimento della conduttanza
 *  2. Raccolta dell'incremento di conduttanza dal ring e azzeramento
 *  3. Controllo spike pre-sinaptico e aggiornamento del ring
 *  4. Calcolo della corrente: I = g * (V_post - E_rev)
 *  5. Avanzamento della posizione nel ring
 *
 * Nota: il calcolo di Isyn_ avviene DOPO il decadimento e l'eventuale
 * kick, così getCurrent() restituisce sempre il valore aggiornato
 * coerente con la g corrente e il V_post di questo step.
 */
void ConductanceSyn::update(double dt, bool preFired, double V_post) {

    // 1. Decadimento esponenziale della conduttanza
    gsyn_ += dt * (-gsyn_ / tau_);

    // 2. Raccolta dal ring del delay
    if (delayRing_[presentStep_] != 0.0) {
        gsyn_ += delayRing_[presentStep_];
        delayRing_[presentStep_] = 0.0;
    }

    // 3. Spike pre-sinaptico: incremento nel ring al posto giusto
    if (preFired) {
        delayRing_[(presentStep_ + delayStep_) % delayRing_.size()] += peso_ * gpeak_;
    }

    // 4. Corrente sinaptica: dipende dal potenziale post-sinaptico corrente
    //    Il segno è fisicamente corretto per costruzione:
    //    se V_post > E_rev  -> I > 0 (depolarizzante, eccitatoria)
    //    se V_post < E_rev  -> I < 0 (iperpolarizzante, inibitoria)
    Isyn_ = gsyn_ * (V_post - E_rev_);

    // 5. Avanzamento posizione nel ring
    presentStep_ = (presentStep_ + 1) % delayRing_.size();
}
