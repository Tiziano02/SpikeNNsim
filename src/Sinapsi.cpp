#include "Sinapsi.hpp"

/*
 * update — avanza lo stato della sinapsi di un passo dt.
 *
 * Sequenza:
 *  1. Se il neurone pre-sinaptico ha sparato: aggiunge l'impulso peso * Ipeak.
 *  2. Decadimento esponenziale con Eulero in avanti:
 *       I_syn(t+dt) = I_syn(t) + dt * (-I_syn(t) / tau)
 *
 * Nota sull'ordine impulso/decadimento: nello stesso step in cui arriva lo
 * spike, l'impulso viene aggiunto e poi immediatamente decresce di un dt.
 * Fisicamente sarebbe più corretto far partire il decadimento dal passo
 * successivo, ma l'errore introdotto è dell'ordine dt * Ipeak / tau,
 * trascurabile per dt << tau.
 */
void Sinapsi::update(double dt, bool preFired) {
    if (preFired)
        Isyn_ += peso_ * Ipeak_;

    Isyn_ += dt * (-Isyn_ / tau_);
}