#include "CurrentSyn.hpp"

void Current::update(double dt, bool preFired) {

    // 1. Decadimento esponenziale della corrente sinaptica
    Isyn_ += dt * (-Isyn_ / tau_);

    // 2. Raccolta spike dal delayRing
    if (delayRing_[presentStep_] != 0.0) {
        Isyn_ += delayRing_[presentStep_];
        delayRing_[presentStep_] = 0.0;
    }

    // 3. Controllo dello spike pre-sinaptico
    if (preFired) {
        // 3.1 Inserimento dello spike nel delay ring
        delayRing_[(presentStep_ + delayStep_) % delayRing_.size()] += peso_ * Ipeak_;
    }

    // 5. Avanzamento posizione nel ring
    presentStep_ = (presentStep_ + 1) % delayRing_.size();
}
