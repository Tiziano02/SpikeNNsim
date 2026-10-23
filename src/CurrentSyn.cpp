#include "CurrentSyn.hpp"

void Current::euleroInAvanti(double dt) { Isyn_ += dt * (-Isyn_ / tau_); }

void Current::analiticoEsatto() { Isyn_ *= decayFactor_; }

void Current::rungeKutta(double dt) {
    double alpha = dt / tau_;
    double rk4 =
        1.0 - alpha + (alpha * alpha) / 2.0 - (alpha * alpha * alpha) / 6.0 + (alpha * alpha * alpha * alpha) / 24.0;
    Isyn_ *= rk4;
}

void Current::update(double dt, bool preFired) {

    // 1. Decadimento della corrente sinaptica
    if (isDelta_) {
        // Sinapsi a delta: nessun decadimento esponenziale continuo.
        // La corrente sinaptica dura esattamente uno step (in cui arriva lo spike dal delay ring).
        Isyn_ = delayRing_[presentStep_];
        delayRing_[presentStep_] = 0.0;
    } else {
        // Decadimento continuo secondo il risolutore ODE scelto
        if (tipoIntegratore_ == 'A') {
            analiticoEsatto();
        } else if (tipoIntegratore_ == 'R') {
            rungeKutta(dt);
        } else {
            euleroInAvanti(dt);
        }

        // Raccolta spike dal delayRing
        if (delayRing_[presentStep_] != 0.0) {
            Isyn_ += delayRing_[presentStep_];
            delayRing_[presentStep_] = 0.0;
        }
    }

    // 2. Controllo dello spike pre-sinaptico
    if (preFired) {
        // Inserimento dello spike nel delay ring
        delayRing_[(presentStep_ + delayStep_) % delayRing_.size()] += peso_ * Ipeak_;
    }

    // 3. Avanzamento posizione nel ring
    presentStep_ = (presentStep_ + 1) % delayRing_.size();
}
