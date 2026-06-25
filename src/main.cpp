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
    std::cout << "=========================================================" << std::endl;
    std::cout << "--- BENCHMARK PER SAMPLY: Rete Biologica Sparsa (15k) ---" << std::endl;
    std::cout << "=========================================================" << std::endl;

    // 1. Parametri di Scala della Rete
    const int N = 150; 
    const int Ne = static_cast<int>(N * 0.8);
    //const int Ni = N - Ne;

    // Parametri Neuroni
    const double eV_th = -50.0 * mV,  eV_rest = -65.0 * mV,  eV_reset = -65.0 * mV;
    const double eR = 100.0 * Mohm,  eC = 200.0 * p * F,    eTau_ref = 3.5 * ms;
    
    const double iV_th = -50.0 * mV,  iV_rest = -65.0 * mV,  iV_reset = -65.0 * mV;
    const double iR = 50.0 * Mohm,   iC = 200.0 * p * F,    iTau_ref = 0.5 * ms;

    Rete rete;
    
    std::cout << "[1/3] Allocazione di " << N << " neuroni..." << std::endl;
    for (int i = 0; i < N; ++i) {
        double V_0 = eV_reset + rumoreGaussiano(0, 2.0 * mV);
        if (i < Ne) {
            rete.aggiungiNeurone(Neurone(i, V_0, eV_th, eV_rest, eV_reset, eR, eC, eTau_ref));
        } else {
            rete.aggiungiNeurone(Neurone(i, V_0, iV_th, iV_rest, iV_reset, iR, iC, iTau_ref));
        }
    }

    // 2. Generazione Connessioni Sparse (Probabilità 10%)
    std::cout << "[2/3] Generazione sinapsi sparse (Connettivita' 10%)..." << std::endl;
    const double eWeight = +1.0, eIpeak = 1.5 * n * A, eTau_syn = 5.0 * ms;
    const double iWeight = -3.0, iIpeak = 1.5 * n * A, iTau_syn = 10.0 * ms;

    std::mt19937 gen(12345); // Seed fisso per replicabilità
    std::uniform_real_distribution<double> dist_conn(0.0, 1.0);

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (dist_conn(gen) < 0.10) { // 10% di probabilità di connessione
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
    
    std::cout << " -> Totale Sinapsi Istanziate: " << rete.getNumSinapsi() << std::endl;

    // 3. Configurazione Simulazione
    double dt = 0.1 * ms;
    double T = 2.0 * ms; // 5000 step temporali
    Simulazione sim(rete, dt, T);

    // Stimolo iniziale per attivare la dinamica (sui primi 100 neuroni eccitatori)
    std::vector<int> idStimolati;
    std::vector<parametriStimoloCostante> paramStimoli;
    for(int i = 0; i < 100; ++i) {
        idStimolati.push_back(i);
        parametriStimoloCostante p;
        p.timeStart = 0.0 * ms;
        p.timeEnd = 20.0 * ms;
        p.ampiezza = 5.0 * n * A;
        paramStimoli.push_back(p);
    }
    sim.iniettaStimoloCostante(idStimolati, paramStimoli);

    std::cout << "[3/3] Setup completato. Avvio del calcolo numerico..." << std::endl;

    // =========================================================
    // ESECUZIONE SIMULAZIONE (IL LOOP COMPLETO SOTTO PROFILING)
    // =========================================================
    sim.avviaSimulazione("potenziali.bin", "firing.bin", "sinapsi.bin");

    std::cout << "=========================================================" << std::endl;
    std::cout << " COMPILATION COMPLETED " << std::endl;
    std::cout << "=========================================================" << std::endl;
    /*
    std::cout << "--- Test Ring Network (Propagazione ad Anello) ---" << std::endl;

    // 1. Creiamo una rete con 5 neuroni (ID da 0 a 4)
    const int numNeuroni = 50;
    Rete rete(numNeuroni); 

    // 2. Connettiamo i neuroni in un anello chiuso (Ring Topology)
    std::cout << "[CONFIG] Creazione delle connessioni sinaptiche ad anello..." << std::endl;
    
    // Connettiamo 0->1, 1->2, 2->3, 3->4
    for (int i = 0; i < numNeuroni - 1; ++i) {
        Sinapsi s(1.0, 30.0 * n * A, i, i + 1, 4.0 * ms);
        rete.connettiNeuroni(s);
    }
    // Chiudiamo l'anello connettendo l'ultimo (4) al primo (0)
    Sinapsi s_chiusura(1.0, 30.0 * n * A, numNeuroni - 1, 0, 4.0 * ms);
    rete.connettiNeuroni(s_chiusura);

    // 3. Inizializziamo i parametri della simulazione
    double dt = 0.1 * ms; 
    double T = 500.0 * ms; // Durata sufficiente per vedere più giri dell'anello
    Simulazione sim(rete, dt, T);

    // 4. CONFIGURAZIONE STIMOLO COSTANTE — SOLO SUL NEURONE 0
    std::vector<int> idCostante = {0};
    std::vector<parametriStimoloSeno> paramSeno;
    
    parametriStimoloSeno pCost;
    pCost.timeStart = 50.0 * ms;   // Lo stimolo parte a 5 ms
    pCost.timeEnd = 500.0 * ms;    // Lo stimolo si interrompe a 25 ms
    pCost.ampiezza = 40.0 * n * A; // Corrente forte sul primo neurone per innescare il ciclo
    pCost.fase = 0.0;  
    pCost.frequenza = 40 * Hz;
    paramSeno.push_back(pCost);

    std::cout << "[TEST] Iniezione stimolo costante SOLO sul neurone 0..." << std::endl;
    sim.iniettaStimoloSeno(idCostante, paramSeno);

    // 5. ESECUZIONE SIMULAZIONE
    std::cout << "\n[TEST] Avvio della simulazione ad anello..." << std::endl;
    sim.avviaSimulazione("test_potenziali.bin", "test_firing.bin", "test_sinapsi.bin");

    std::cout << "--- Simulazione completata! ---" << std::endl;
    */
    // =========================================================
    // PARAMETRI RETE
    // =========================================================
    /*
    const int N = 1000;

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

    sim.avviaSimulazione("potenziali.bin", "firing.bin", "sinapsi.bin");

    std::cout << "Simulazione completata.\n";
    return 0;

    
    
    // rete e neuroni
    const int N = 500;
    Rete rete(N);

    // sinapsi
    for (int i = 0; i < N - 1; ++i) {
        Sinapsi s(1, 20 * n * A, i, i + 1, 5 * ms);
        rete.connettiNeuroni(s);
    }

    Sinapsi s(1, 20 * n * A, N - 1, 0, 5 * ms);
    rete.connettiNeuroni(s);

    // simulazione
    double dt = 0.1 * ms;
    double T = 500.0 * ms;

    Simulazione sim(rete, dt, T);

    // input esterno
    int stepTotali = static_cast<int>(T / dt);

    //std::vector<Input> inputEsterno;

    std::vector<double> stimolo(stepTotali, 0.0);

    std::fill(stimolo.begin() + 300, stimolo.begin() + 300 + 1, 20 * n * A);

    //Input i;

    //i.id = 0;
    //i.valori = stimolo;

    //inputEsterno.push_back(i);

    //sim.aggiungiInputEsterni(inputEsterno);

    sim.avviaSimulazione("potenziali.bin", "firing.bin", "sinapsi.bin");

    */
   return 0;
}