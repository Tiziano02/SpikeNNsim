#include "Exp.hpp"
#include <cmath>

void Exp::euleroInAvanti(double correnteTotale, double dt) {

    double termExp = sharpness_ * std::exp((V_ - Vth_) / sharpness_);
    double dV = (-(V_ - Vrest_) + termExp + R_ * correnteTotale) / getTau();
    V_ += dV * dt;
}

void Exp::rungeKutta(double correnteTotale, double dt) {
    // Funzione lambda locale per calcolare f(V, I) in modo pulito
    auto f = [this, correnteTotale](double v) {
        double termExp = sharpness_ * std::exp((v - Vth_) / sharpness_);
        return (-(v - Vrest_) + termExp + R_ * correnteTotale) / getTau();
    };

    // Calcolo dei 4 coefficienti di Runge-Kutta
    double k1 = f(V_);
    double k2 = f(V_ + 0.5 * dt * k1);
    double k3 = f(V_ + 0.5 * dt * k2);
    double k4 = f(V_ + dt * k3);

    // Aggiornamento finale del potenziale di membrana
    V_ += (dt / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

void Exp::update(double correnteTotale, double dt) {

    // 1. Reset flag stato del neurono
    fired_ = false;

    // 2. Gestione Refrattarietà Assoluta
    if (tempoRR_ > 0.0) {

        // 2.1 diminuizione del tempo refrattario assoluto
        tempoRR_ -= dt;

        // 2.2 controllo del tempo refrattario assoluto
        if (tempoRR_ < 0.0)
            tempoRR_ = 0.0;

        // 2.3 Potenziale fisso a reset
        V_ = Vreset_;

        // 2.4 Salto integrazione fisica
        return;
    }

    // 3. Il decadimento della soglia
    Vth_ += dt * ((VthMin_ - Vth_) / getTauRelative());

    // 4. Integrazione del Potenziale
    if (tipoIntegratore_ == 'E') {
        euleroInAvanti(correnteTotale, dt);
    } else if (tipoIntegratore_ == 'R') {
        rungeKutta(correnteTotale, dt);
    }

    // 5. Controllo superametro soglia
    if (V_ >= Vth_) {
        // 5.1 Aggiornameto soglia
        fired_ = true;

        // 5.2 Setting potenziale di membrana
        V_ = Vreset_;

        // 5.3 Setting tempo relativo assoluto rimanente
        tempoRR_ = timeAbsolute_;

        // 5.4 Setting soglia al valore massimo
        Vth_ = VthMax_;
    }
}