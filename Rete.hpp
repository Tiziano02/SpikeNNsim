#ifndef RETE_HPP
#define RETE_HPP

#include "Neurone.hpp"
#include "UnitaSI.hpp"
#include <fstream>
#include <iostream> // --> eliminabile se si toglie la gestione degli errori con i file
#include <map>
#include <string>
#include <vector>

class Rete {

  private:
    std::vector<Neurone> neuroni_;
    std::vector<Sinapsi> sinapsi_; // vettore di sinapsi --> nell'oggetto Sinapsi ci sono già gli ID dei neuroni pre e post sinaptici, quindi non serve una matrice di adiacenza, basta un vettore di sinapsi
    std::vector<double> inputEsterno_;             // input associati a ciascun neurone
    std::map<int, size_t> idToIndex_;              // mappa dagli ID agli indici
  
    public:
    // costruttore di default
    Rete() = default; // --> cosa dovrebbe fare ? 0 neuroni, matrice vuota, input vuoto, mappa vuota

    // costruttore utility --> utilizzabile se si vuole N neuroni standard tutti uguali biologicamente
    Rete(int N) {
        for (int i = 0; i < N; ++i) {
            Neurone n(i);
            aggiungiNeurone(n); // aggiunge il neurone alla rete
        }
    }

    // metodi per gestire la rete neurale
    void aggiungiNeurone(const Neurone &neurone);
    void connettiNeuroni(int id1, int id2, double peso, double tau);
    void setInput(int id, double valore);
    void step(double dt);

    // metodi getter
    std::vector<double> getPotenziali() const;
    std::vector<int> getFiringStates() const;

    // metodo log
    void salvaStatoRete(std::ofstream &filePotenziali, std::ofstream &fileFiring, double time);

    // simualzione della rete 
    void simulazione(double dt, double T, const std::string &filenameV, const std::string &filenameF) {
        
        int steps = static_cast<int>(T / dt);
        double time = 0.0 * s;

        std::ofstream filePotenziali;
        std::ofstream fileFiring;
        filePotenziali.open(filenameV);
        fileFiring.open(filenameF);

        if (!filePotenziali.is_open() || !fileFiring.is_open()) {
            std::cerr << "Errore nell'apertura dei file di output!" << std::endl;
            return;
        }

        for (int n = 0; n < steps; ++n) {
            step(dt);   // esegue un passo di simulazione
            time += dt; // aggiorna il tempo

            salvaStatoRete(filePotenziali, fileFiring, time); // salva lo stato della rete nei file
        }
    }

    ~Rete() = default;
};

#include "reteImp.hpp"

#endif // RETE_HPP
