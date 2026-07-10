#include "LIF.hpp"

/*
 * update — avanza lo stato del neurone di un passo dt.
 *
 * Sequenza:
 *  1. Reset del flag fired_.
 *  2. Se il neurone è in periodo refrattario: decrementa tempoRR_ ed esce.
 *  3. Integrazione LIF con metodo di Eulero in avanti:
 *       V(t+dt) = V(t) + (dt/tau) * [-(V(t) - Vrest) + R * I_tot]
 *  4. Se V >= Vth: spike -> fired_ = true, V = Vreset, avvia periodo refrattario.
 *
 * Note:
 *  - fired_ è valido solo per lo step corrente; viene letto da Rete::step()
 *    prima del prossimo update().
 *  - Il metodo di Eulero è stabile se dt << tau (tipicamente dt < tau/10).
 */
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
    fired_ = false;

    // 3. Gestione Refrattarietà Assoluta
    if (tempoRR_ > 0.0) {
        tempoRR_ -= dt;
        if (tempoRR_ < 0.0)
            tempoRR_ = 0.0;
        V_ = Vreset_; // mantiene il potenziale inchiodato a reset
        return;       // Salta l'integrazione fisica: il neurone è sordo
    }

    // 2. Il decadimento della soglia
    Vth_ += dt * ((VthMin_ - Vth_) / getTauRelative());

    // 3. Integrazione del Potenziale (Fase normale o Refrattarietà Relativa)
    if (tipoIntegratore_ == 'E') {
        euleroInAvanti(correnteTotale, dt);
    } else if (tipoIntegratore_ == 'R') {
        rungeKutta(correnteTotale, dt);
    }

    // 4. Controllo del Firing
    if (V_ >= Vth_) {
        fired_ = true;
        V_ = Vreset_;
        tempoRR_ = timeAbsolute_; // Fissa il tempo assoluto
        Vth_ = VthMax_;           // Alza la soglia al massimo istantaneamente!
    }
}