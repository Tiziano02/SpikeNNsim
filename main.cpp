#include "Rete.hpp"
#include <random>

int main(){
    
    Neurone n1(0, - 49.0 * mV, -50.0 * mV, -65.0 * mV, -70.0 * mV, 10.0 * mS, 5.0 * mS);
    Neurone n2(1, - 65.0 * mV, -50.0 * mV, -65.0 * mV, -70.0 * mV, 10.0 * mS, 5.0 * mS);

    Sinapsi s1(0, 1, 40.0 * mV, 5.0 * mS); // sinapsi da n1 a n2 con peso 0.5 e tau 5 ms

    Rete rete;

    rete.aggiungiNeurone(n1);
    rete.aggiungiNeurone(n2);
    rete.connettiNeuroni(s1); // connetti n1 a n2 con peso 0.5 e tau 5 ms

    // usa le unità di misura corrette per il passo temporale e la durata
    double dt = 0.1 * mS;      // 0.1 millisecondo
    double T = 10.0 * s;    // 10.0 secondi

    rete.simulazione(
        dt,
        T,
        "potenziali.txt",
        "firing.txt",
        "sinapsi.txt"
    );
}