#include "ReteNeurale.hpp"
#include <random>

int main(){

    int N = 100;
    std::vector<Neurone> neuroni;

    for(int i = 0; i < N; ++i){
        Neurone n(i);
        neuroni.push_back(n); // crea N neuroni con ID da 0 a N-1
    }

    ReteNeurale rete; // creazione della rete neurale

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
                    peso = 30.0;
                } else { // inibitore (ultimi 20)
                    peso = -30.0 *  weight_inh(gen);
                }
                rete.connettiNeuroni(i, j, peso);
            }
        }
    }

    // imposta input esterno random tra 18 e 22 per tutti i neuroni
    std::uniform_real_distribution<> input_dist(18.0, 22.0);
    for(int i = 0; i < N; ++i){
        rete.setInput(i, input_dist(gen));
    }

rete.simulazione(
    0.1,
    1000.0,
    "potenziali.txt",
    "firing.txt"
);
}