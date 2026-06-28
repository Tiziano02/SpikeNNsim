#include "CurrentSyn.hpp"

/*
 * update — avanza lo stato della sinapsi di un passo dt.
 *
 * Sequenza:
 *  1. Decadimento della corrente presente
 *  2. Raccolta della corrente dal ring degli spike e azzeramento
 *  3. Controllo spike pre-sinaptico e aggiornamento del ring
 *  4. Avanzamento della posizione nel ring
 */
void CurrentSyn::update(double dt, bool preFired) {

    Isyn_ += dt * (-Isyn_ / tau_);

    if (delayRing_[presentStep_] != 0.0) {
        Isyn_ += delayRing_[presentStep_];
        delayRing_[presentStep_] = 0.0;
    }

    if (preFired) {
        delayRing_[(presentStep_ + delayStep_) % delayRing_.size()] += peso_ * Ipeak_;
    }

    presentStep_ = (presentStep_ + 1) % delayRing_.size();
}
