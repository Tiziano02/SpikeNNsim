#include "ConductanceSyn.hpp"

void Conductance::euleroInAvanti(double dt) {
    gsyn_ += dt * (-gsyn_ / tau_);
}

void Conductance::analiticoEsatto(double dt) {
    if (decayFactor_ == 0.0) {
        decayFactor_ = std::exp(-dt / tau_);
    }
    gsyn_ *= decayFactor_;
}

void Conductance::rungeKutta(double dt) {
    double alpha = dt / tau_;
    double rk4 = 1.0 - alpha + (alpha * alpha) / 2.0 - (alpha * alpha * alpha) / 6.0 +
                 (alpha * alpha * alpha * alpha) / 24.0;
    gsyn_ *= rk4;
}

void Conductance::update(double dt, bool preFired, double Vpost) {

    // 1. Decadimento della conduttanza in base al risolutore ODE scelto
    if (tipoIntegratore_ == 'A') {
        analiticoEsatto(dt);
    } else if (tipoIntegratore_ == 'R') {
        rungeKutta(dt);
    } else {
        euleroInAvanti(dt);
    }

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
