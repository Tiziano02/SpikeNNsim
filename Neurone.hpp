#include "UnitaSI.hpp"

class Neurone
{

private:
    int id_;
    double V_;          // stato dinamico del neurone
    double Vth_;        // soglia di attivazione
    double Vrest_;      // potenziale di riposo
    double Vreset_;     // potenziale di reset dopo il firing
    double tau_;        // tempo di rilassamento del potenziale di membrana
    double tauR_;       // tempo refrattario del neurone
    double tempoRR_;    // tempo refrattario rimanente
    bool fired = false; // stato di firing del neuroneù

public:
    
// costrutture di default e con parametri per inizializzare i neuroni con i parametri biologici e lo stato dinamico iniziale
    Neurone(int id) : id_(id), V_(-65.0 * mV), Vth_(-50.0 * mV), Vrest_(-65.0 * mV), Vreset_(-70.0 * mV),
                      tau_(10.0 * mS), tauR_(5.0 * mS), tempoRR_(0.0 * mS), fired(false) {}

                      
    // costruttore per inizializzare i neuroni con i parametri biologici e lo stato dinamico iniziale
    Neurone(int id, double V_0, double V_th, double V_rest, double V_reset, double tau, double tauR)
        : id_(id), V_(V_0), Vth_(V_th), Vrest_(V_rest), Vreset_(V_reset), tau_(tau), tauR_(tauR),
          tempoRR_(0.0 * mS), fired(false) {}

    // Metodi della classe Neurone
    void update(double inputTotale, double dt)
    {
        fired = false;

        // gestione refrattario
        if (tempoRR_ > 0)
        {
            tempoRR_ -= dt;
            if (tempoRR_ < 0)
                tempoRR_ = 0;
            return;
        }

        // dinamica del potenziale
        V_ += dt * (-(V_ - Vrest_) + inputTotale) / tau_;

        // firing
        if (V_ >= Vth_)
        {
            fired = true;
            V_ = Vreset_;
            tempoRR_ = tauR_;
        }
    }

    bool hasFired() const { return fired; }
    double getPotential() const { return V_; }
    int getId() const { return id_; }

    ~Neurone() = default;
};