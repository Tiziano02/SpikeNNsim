#ifndef SINAPSI_HPP
#define SINAPSI_HPP

#include "UnitaSI.hpp"

class Sinapsi
{
private:
    double peso_; // forza della sinapsi, positiva per eccitatoria, negativa per inibitoria
    int idPre_, idPost_;  // ID del neurone pre-sinaptico e post-sinaptico
    double Vsyn_; // "corrente" sinaptica che contribuisce al potenziale del neurone post-sinaptico
    double tau_; // tempo di decadimento della corrente sinaptica
    // double R_; // resistenza sinaptica inserie dopo , può essere regolata per modulare l'efficacia della sinapsi
public:
    Sinapsi(int idPre, int idPost, double peso, double tau) : idPre_(idPre), idPost_(idPost), peso_(peso), tau_(tau),  Vsyn_(0.0) {};
    
    // metodo update
    void update(double dt, bool preFired) {
        if (preFired) {
            Vsyn_ += peso_;  // (unità di musra  = V) aggiunge il contributo della sinapsi se il neurone pre-sinaptico ha sparato
        }
        Vsyn_ += dt *  -Vsyn_ / tau_; // decadimento della corrente sinaptica nel tempo
    }

    double getCurrent() const { return Vsyn_; }
    int getIdPre() const { return idPre_; }
    int getIdPost() const { return idPost_; }
    
    ~Sinapsi() = default;
};

#endif // SINAPSI_HPP

