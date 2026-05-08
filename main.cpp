#include "Rete.hpp"
#include <random>

int main(){

    int N = 100;
    std::vector<Neurone> neuroni;

    for(int i = 0; i < N; ++i){
        Neurone n(i);
        neuroni.push_back(n); // crea N neuroni con ID da 0 a N-1
    }

    Rete rete; // creazione della rete neurale

    for(const auto& neurone : neuroni)
        rete.aggiungiNeurone(neurone); // aggiunge i neuroni alla rete

    // connetti i neuroni in modo casuale con probabilità 0.1
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_real_distribution<> weight_inh(4.0, 6.0);

    for(int i = 0; i < N; ++i){
        for(int j = 0; j < N; ++j){
            if(i != j && prob_dist(gen) < 0.1){
                double peso;
                if(i < 80){ // eccitatore (primi 80)
                    peso = 300.0/N * (1.0 + 0.5 * prob_dist(gen)); // peso random tra 300/N e 450/N
                } else { // inibitore (ultimi 20)
                    peso = -300.0/N * (1.0 + 0.5 * prob_dist(gen)) *  weight_inh(gen);
                }
                rete.connettiNeuroni(i, j, peso);
            }
        }
    }

    // imposta input esterno random tra 18 e 22 mV per tutti i neuroni
    std::uniform_real_distribution<> input_dist(18.0 * mV, 22.0 * mV);
    for(int i = 0; i < N; ++i){
        rete.setInput(i, input_dist(gen));
    }

    // usa le unità di misura corrette per il passo temporale e la durata
    double dt = 0.1 * mS;      // 0.1 millisecondo
    double T = 1000.0 * mS;    // 1000 millisecondi

    rete.simulazione(
        dt,
        T,
        "potenziali.txt",
        "firing.txt"
    );
}