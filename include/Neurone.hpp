#ifndef NEURONE_HPP
#define NEURONE_HPP

#include "UnitaSI.hpp"

/*
 * Neurone — modello Leaky Integrate-and-Fire (LIF) con periodo refrattario assoluto.
 *
 * Modello matematico:
 *   tau * dV/dt = -(V - Vrest) + R * I_tot(t)
 *   tau = R * C
 *
 * Quando V raggiunge Vth il neurone emette uno spike:
 * V viene resettato a Vreset e inizia il periodo refrattario assoluto di durata tauR,
 * durante il quale la dinamica è bloccata.
 *
 * Parametri biologici di default (costruttore con solo ID):
 *   Vrest  = -65 mV,  Vth = -50 mV,  Vreset = -70 mV
 *   R = 1 MOhm,  C = 100 pF  ->  tau = 100 ms
 *   tauR = 5 ms
 *
 * Metodi pubblici:
 *   update(I, dt)   — avanza lo stato del neurone di un passo dt
 *
 * Metodi getter:
 *   hasFired()      — true se il neurone ha sparato nell'ultimo step
 *   getPotential()  — restituisce V corrente [V]
 *   getId()         — restituisce l'ID del neurone
 */
class Neurone {

  private:
    int id_;
    double V_;       // potenziale di membrana [V]
    double Vth_;     // soglia di firing [V]
    double Vrest_;   // potenziale di riposo [V]
    double Vreset_;  // potenziale di reset post-spike [V]
    double R_;       // resistenza di ingresso [Ohm]
    double C_;       // capacità di membrana [F]
    double tau_;     // costante di tempo tau = R*C [s]
    double tauR_;    // durata periodo refrattario assoluto [s]
    double tempoRR_; // tempo refrattario rimanente [s]
    bool fired_;     // true se il neurone ha sparato nell'ultimo step

  public:
    Neurone(int id) : id_(id), V_(-65.0 * mV), Vth_(-50.0 * mV), Vrest_(-65.0 * mV), Vreset_(-70.0 * mV), R_(1.0 * Mohm), C_(100.0 * p * F), tau_(R_ * C_), tauR_(5.0 * ms), tempoRR_(0.0), fired_(false) {}

    Neurone(int id, double V_0, double V_th, double V_rest, double V_reset, double R, double C, double tauR) : id_(id), V_(V_0), Vth_(V_th), Vrest_(V_rest), Vreset_(V_reset), R_(R), C_(C), tau_(R * C), tauR_(tauR), tempoRR_(0.0), fired_(false) {}

    void update(double correnteTotale, double dt);

    bool hasFired() const { return fired_; }
    double getPotential() const { return V_; }
    int getId() const { return id_; }

    ~Neurone() = default;
};


#endif // NEURONE_HPP