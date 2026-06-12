#include "Neurone.hpp"

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
void Neurone::update(double correnteTotale, double dt) {
    fired_ = false;

    if (tempoRR_ > 0.0) {
        tempoRR_ -= dt;
        if (tempoRR_ < 0.0)
            tempoRR_ = 0.0;
        return;
    }

    V_ += (dt / tau_) * (-(V_ - Vrest_) + R_ * correnteTotale);

    if (V_ >= Vth_) {
        fired_ = true;
        V_ = Vreset_;
        tempoRR_ = tauR_;
    }
}