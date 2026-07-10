#include "LIF.hpp"

void LIF::euleroInAvanti(double correnteTotale, double dt) {
    V_ += (dt / getTau()) * (-(V_ - Vrest_) + R_ * correnteTotale);
}

void LIF::rungeKutta(double correnteTotale, double dt) {
    double k1 = (-(V_ - Vrest_) + R_ * correnteTotale) / getTau();
    double k2 = (-(V_ + 0.5 * dt * k1 - Vrest_) + R_ * correnteTotale) / getTau();
    double k3 = (-(V_ + 0.5 * dt * k2 - Vrest_) + R_ * correnteTotale) / getTau();
    double k4 = (-(V_ + dt * k3 - Vrest_) + R_ * correnteTotale) / getTau();
    V_ += (dt / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

void LIF::update(double correnteTotale, double dt) {

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