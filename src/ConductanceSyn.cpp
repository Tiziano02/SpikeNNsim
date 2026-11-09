#include "ConductanceSyn.hpp"

void Conductance::euleroInAvanti(double dt) { gsyn_ += dt * (-gsyn_ / tau_); }

void Conductance::analiticoEsatto() { gsyn_ *= decayFactor_; }

void Conductance::rungeKutta(double dt) {
    double alpha = dt / tau_;
    double rk4 =
        1.0 - alpha + (alpha * alpha) / 2.0 - (alpha * alpha * alpha) / 6.0 + (alpha * alpha * alpha * alpha) / 24.0;
    gsyn_ *= rk4;
}

void Conductance::update(double dt, double Vpost) {

    // 1. Decadimento della conduttanza già presente
    if (tipoIntegratore_ == 'A') {
        analiticoEsatto();
    } else if (tipoIntegratore_ == 'R') {
        rungeKutta(dt);
    } else {
        euleroInAvanti(dt);
    }

    // 2. Controllo degli eventi sinaptici in arrivo e aggiornamento della conduttanza
    if (eventoArrivato_) {
        gsyn_ += peso_ * gpeak_;
        eventoArrivato_ = false;
    }

    // 3. Calcolo la corrente sinaptica
    Isyn_ = gsyn_ * (Vpost - Erev_);
}
