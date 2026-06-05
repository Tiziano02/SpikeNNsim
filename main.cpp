#include "Input.hpp"
#include "Rete.hpp"
#include "Simulazione.hpp"
#include "Sinapsi.hpp"
#include "UnitaSI.hpp"

#include <iostream>
#include <random>
#include <vector>

double rumoreGaussiano(double media, double deviazioneStandard) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::normal_distribution<double> distribuzione(media, deviazioneStandard);

    return distribuzione(gen);
}

double rumoreUniforme(double centro, double ampiezza) {

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution<double> distribuzione(centro - ampiezza, centro + ampiezza);

    return distribuzione(gen);
}

int main() {

    // =========================================================
    // PARAMETRI RETE
    // =========================================================

    const int N = 5000;

    const int Ne = N * 0.8;
    const int Ni = N * 0.2;

    // Neurone eccitatorio (tipo piramidale)
    const double eV_rest = -65.0 * mV;
    const double eV_reset = -65.0 * mV;
    const double eV_th = -50.0 * mV;
    const double eR = 100 * Mohm;
    const double eC = 200 * p * F;
    const double eTau_m = eR * eC;
    const double eTau_ref = 3.5 * ms;

    // neurone inibitorio (interneuroni fast-spiking)
    const double iV_rest = -65.0 * mV;
    const double iV_reset = -65.0 * mV;
    const double iV_th = -50.0 * mV;
    const double iR = 50 * Mohm;
    const double iC = 200 * p * F;
    const double iTau_m = iR * iC;
    const double iTau_ref = 0.5 * ms;

    Rete rete;

    // creo i neuroni eccitatori e inibitori
    for (int i = 0; i < N; ++i) {
        double V_0 = eV_reset + rumoreGaussiano(0, 2 * mV);
        // std :: cout << "Potenziale iniziale : " << V_0 << " = " << eV_reset << " + " << V_0-eV_reset << "\n";
        if (i < Ne) {
            Neurone neurone(i, V_0, eV_th, eV_rest, eV_reset, eR, eC, eTau_ref);
            rete.aggiungiNeurone(neurone);
        } else {
            Neurone neurone(i, V_0, iV_th, iV_rest, iV_reset, iR, iC, iTau_ref);
            rete.aggiungiNeurone(neurone);
        }
    }

    // sinapsi eccitatorie
    const double eWeight = +0.2;
    const double eIpeak = 0.1 * n * A;
    const double eTau_syn = 5 * ms;

    // sinapsi inibitorie
    const double iWeight = -1.0;
    const double iIpeak = 0.1 * n * A;
    const double iTau_syn = 10 * ms;

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; j++) {
            double pCorrente = rumoreUniforme(0.5, 0.5);
            if (pCorrente < 0.2) {
                if (i < Ne) {
                    Sinapsi s(eWeight, eIpeak, i, j, eTau_syn);
                    rete.connettiNeuroni(s);
                } else {
                    Sinapsi s(iWeight, iIpeak, i, j, iTau_syn);
                    rete.connettiNeuroni(s);
                }
            }
        }
    }

    // dati simulazione
    double dt = 0.1 * ms;
    double T = 500.0 * ms;

    Simulazione sim(rete, dt, T);

    // input esterno 

    int stepTotali = static_cast<int>(T / dt);

    std::vector<Input> inputEsterno;

    double inputCorrente = 0.0;
    for(int l=0;l< N; l++){    
        Input i;
        std::vector<double> v;
        i.id = l;
       
        if (l < Ne)
            inputCorrente = 0.15 * n*A + rumoreGaussiano(0,0.02*n*A);
        else
            inputCorrente = 0.30 * n * A + rumoreGaussiano(0,0.02*n*A);
           
        for(int k=0;k<stepTotali;k++)
            v.push_back(inputCorrente);

        i.valori = v;
        v.clear();
        inputEsterno.push_back(i);
    }

    sim.aggiungiInputEsterni(inputEsterno);


    // =========================================================
    // AVVIO SIMULAZIONE
    // =========================================================

    sim.avviaSimulazione("potenziali.txt", "firing.txt", "sinapsi.txt");

    std::cout << "Simulazione completata.\n";
    return 0;
}