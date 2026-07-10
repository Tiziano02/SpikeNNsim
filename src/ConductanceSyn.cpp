#include "ConductanceSyn.hpp"

void Conductance::update(double dt, bool preFired, double Vpost) {

    // 1. Decadimento esponenziale della conduttanza
    gsyn_ += dt * (-gsyn_ / tau_);

    // 2. Raccolta spike dal delayRing
    if (delayRing_[presentStep_] != 0.0) {
        gsyn_ += delayRing_[presentStep_];
        delayRing_[presentStep_] = 0.0;
    }

    // 3. Controllo dello spike pre-sinaptico
    if (preFired) {
        // 3.1 Inserimento dello spike nel delay ring
        delayRing_[(presentStep_ + delayStep_) % delayRing_.size()] += peso_ * gpeak_;
    }

    // 4. Calcolo corrente sinaptica
    Isyn_ = gsyn_ * (Vpost - Erev_);

    // 5. Avanzamento posizione nel ring
    presentStep_ = (presentStep_ + 1) % delayRing_.size();
}
