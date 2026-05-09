#ifndef SINAPSI_HPP
#define SINAPSI_HPP

#include "UnitaSI.hpp"

class Sinapsi
{
private:
    double peso_; // forza della sinapsi, positiva per eccitatoria, negativa per inibitoria
    int idPre_, idPost_;  // ID del neurone pre-sinaptico e post-sinaptico
    double Isyn_; // "corrente" sinaptica che contribuisce al potenziale del neurone post-sinaptico
    double tau_; // tempo di decadimento della corrente sinaptica

public:
    Sinapsi(double peso, int idPre, int idPost, double tau) : idPre_(idPre), idPost_(idPost), peso_(peso), Isyn_(0.0) ,tau_(tau) {};
    
    // metodo update
    void update(double dt, bool preFired) {
        if (preFired) {
            Isyn_ += peso_;   
        }
        Isyn_ += dt *  -(Isyn_ /tau_); // decadimento della corrente sinaptica nel tempo
    }

    double getCurrent() const { return Isyn_; }
    int getIdPre() const { return idPre_; }
    int getIdPost() const { return idPost_; }
    
    ~Sinapsi() = default;
};

#endif // SINAPSI_HPP

