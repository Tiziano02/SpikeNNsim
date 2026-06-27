#ifndef NEURONE_HPP
#define NEURONE_HPP

#include "UnitaSI.hpp"

struct LIF {
    int id;
    double V_0;
    double V_th;
    double V_ThresholdSpikeMax;
    double V_rest;
    double V_reset;
    double R;
    double C;
    double timeAbsolute;
    double timeRelative;
    char tipoIntegratore;
};

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
    double V_;             // potenziale di membrana [V]
    double Vth_;           // soglia di firing dinamica [V]
    double Vth0_;          // soglia di firing fuori dal tempo refrattario relativo [V]
    double VthSpikeMax_;   // soglia massima di firing quando avviene uno spike [V]
    double Vrest_;         // potenziale di riposo [V]
    double Vreset_;        // potenziale di reset post-spike [V]
    double R_;             // resistenza di ingresso [Ohm]
    double C_;             // capacità di membrana [F]
    double tau_;           // costante di tempo tau = R*C [s]
    double timeAbsolute_;  // durata periodo refrattario assoluto [s]
    double timeRelative_;  // durata periodo refrattario relativo [s]
    double tempoRR_;       // tempo refrattario assoluto rimanente [s]
    double tauRelative_;   // tau decadimento Vth_;
    bool fired_;           // true se il neurone ha sparato nell'ultimo step
    char tipoIntegratore_; // tipo di integratore da utilizzare: 'E' = Eulero in avanti, 'R' = Runge-Kutta 4

    // metodi privati di integrazione
    void euleroInAvanti(double correnteTotale, double dt);
    void rungeKutta(double correnteTotale, double dt);

    void update(double correnteTotale, double dt);

    bool hasFired() const { return fired_; }
    double getPotential() const { return V_; }
    int getId() const { return id_; }

  public:
    ~Neurone() = default;

    Neurone(int id, char tipoIntegratore = 'E') : id_(id), V_(-65.0 * mV), Vth_(-50.0 * mV), Vth0_(-50.0 * mV), VthSpikeMax_(-35.0 * mV), Vrest_(-65.0 * mV), Vreset_(-70.0 * mV), R_(1.0 * Mohm), C_(100.0 * p * F), tau_(R_ * C_), timeAbsolute_(5.0 * ms), timeRelative_(15.0 * ms), tempoRR_(0.0), tauRelative_(15.0 / 3 * ms), fired_(false), tipoIntegratore_(tipoIntegratore) {}
    Neurone(int id, double V_0, double V_th, double V_ThresholdSpikeMax, double V_rest, double V_reset, double R, double C, double timeAbsolute, double timeRelative, char tipoIntegratore = 'E') : id_(id), V_(V_0), Vth_(V_th), Vth0_(V_th), VthSpikeMax_(V_ThresholdSpikeMax), Vrest_(V_rest), Vreset_(V_reset), R_(R), C_(C), tau_(R * C), timeAbsolute_(timeAbsolute), timeRelative_(timeRelative), tempoRR_(0.0), tauRelative_(timeRelative / 3), fired_(false), tipoIntegratore_(tipoIntegratore) {}

    friend class Rete;
};

#endif // NEURONE_HPP