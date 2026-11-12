#include "CurrentSyn.hpp"
#include <cmath>

void Current::euleroInAvanti(double dt) { Isyn_ += dt * (-Isyn_ / tau_); }

void Current::analiticoEsatto() { Isyn_ *= decayFactor_; }

void Current::rungeKutta(double dt) {
    double alpha = dt / tau_;
    double rk4 =
        1.0 - alpha + (alpha * alpha) / 2.0 - (alpha * alpha * alpha) / 6.0 + (alpha * alpha * alpha * alpha) / 24.0;
    Isyn_ *= rk4;
}

void Current::update(double dt) {

    // 1. Controllo tipologia sinapsi
    if (isDelta_) {
        if (eventoArrivato_) {
            Isyn_ = peso_ * Ipeak_;
        } else {
            Isyn_ = 0.0;
        }
    } else {

        // 1.1 Decadimento corrente già presente
        if (tipoIntegratore_ == 'A') {
            analiticoEsatto();
        } else if (tipoIntegratore_ == 'R') {
            rungeKutta(dt);
        } else {
            euleroInAvanti(dt);
        }

        // 1.2 Controllo evento
        if (eventoArrivato_) {

            // 1.2.1 aggiungo la corrente di picco alla corrente sinaptica
            Isyn_ += peso_ * Ipeak_;
        }
    }

    // 2. Reset del flag dell'evento
    eventoArrivato_ = false;
}

void Current::prepare(double dt) {
    decayFactor_ = std::exp(-dt / tau_);
    setSoglia();
}

inline void Current::setSoglia() { soglia_ = 1e-12 * A; }