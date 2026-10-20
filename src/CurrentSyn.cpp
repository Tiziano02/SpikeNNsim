#include "CurrentSyn.hpp"

void Current::euleroInAvanti(double dt) {
    Isyn_ += dt * (-Isyn_ / tau_);
}

void Current::analiticoEsatto(double dt) {
    if (decayFactor_ == 0.0) {
        decayFactor_ = std::exp(-dt / tau_);
    }
    Isyn_ *= decayFactor_;
}

void Current::rungeKutta(double dt) {
    double alpha = dt / tau_;
    double rk4 = 1.0 - alpha + (alpha * alpha) / 2.0 - (alpha * alpha * alpha) / 6.0 +
                 (alpha * alpha * alpha * alpha) / 24.0;
    Isyn_ *= rk4;
}

void Current::update(double dt, bool preFired) {

    // 1. Decadimento della corrente sinaptica in base al risolutore ODE scelto
    if (tipoIntegratore_ == 'A') {
        analiticoEsatto(dt);
    } else if (tipoIntegratore_ == 'R') {
        rungeKutta(dt);
    } else {
        euleroInAvanti(dt);
    }

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

    // 4. Avanzamento posizione nel ring
    presentStep_ = (presentStep_ + 1) % delayRing_.size();
}
