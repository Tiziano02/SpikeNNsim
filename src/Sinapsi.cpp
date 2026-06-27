#include "Sinapsi.hpp"

/*
 * update — avanza lo stato della sinapsi di un passo dt.
 *
 * Sequenza:
 *  1. decadimento della corrente presente fino a quel momento
 *  2. racolgo la corrente dal ring degli spike e azzero
 *  3. controllo se un neurone ha sparato e in caso aggiorno il ring 
 *  4. vado avanti con la posizione all'interno del ring
 *
 */
void Sinapsi::update(double dt, bool preFired) {

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