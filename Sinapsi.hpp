#ifndef SINAPSI_HPP
#define SINAPSI_HPP

#include "UnitaSI.hpp"

class Sinapsi
{
private:
    double peso_; // forza della sinapsi, positiva per eccitatoria, negativa per inibitoria --> [-1,+1] --> in futuro si può fare tipoEccitatoria con A positiva e tipoInibitoria con A negativa e il peso --> [0,1] 
    double Ipeak_; // modulo corrente generata da uno spike pre-sinaptico per peso unitario 
    int idPre_, idPost_;  // ID del neurone pre-sinaptico e post-sinaptico
    double Isyn_; // corrente sinaptica che contribuisce al potenziale del neurone post-sinaptico
    double tau_; // tempo di decadimento della corrente sinaptica

public:
    Sinapsi(double peso, double Ipeak, int idPre, int idPost, double tau) : peso_(peso), Ipeak_(Ipeak), idPre_(idPre), idPost_(idPost), Isyn_(0.0) ,tau_(tau) {};
    
    // metodo update
    void update(double dt, bool preFired) {
        if (preFired) {
            Isyn_ += peso_ * Ipeak_;   
        }
        Isyn_ += dt *  (-Isyn_ /tau_); // decadimento della corrente sinaptica nel tempo
    }

    double getCurrent() const { return Isyn_; }
    int getIdPre() const { return idPre_; }
    int getIdPost() const { return idPost_; }
    
    ~Sinapsi() = default;
};

#endif // SINAPSI_HPP

