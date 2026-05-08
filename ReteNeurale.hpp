#include <vector>
#include <map> 
#include <fstream>
#include <string>
#include <iostream>

class Neurone{

    private:
        int id_;
        double V_; // stato dinamico del neurone
        double Vth_; // soglia di attivazione
        double Vrest_; // potenziale di riposo
        double Vreset_; // potenziale di reset dopo il firing
        double tau_; // tempo di rilassamento del potenziale di membrana
        double tauR_; // tempo refrattario del neurone
        double tempoRR_; // tempo refrattario rimanente
        bool fired = false; // stato di firing del neuroneù

    public:
        // costrutture di default e con parametri per inizializzare i neuroni con i parametri biologici e lo stato dinamico iniziale
        Neurone(int id) : id_(id), V_(-65.0), Vth_(-50.0), Vrest_(-65.0), Vreset_(-70.0), tau_(10.0), tauR_(5.0), tempoRR_(0.0), fired(false) {}
        // costruttore per inizializzare i neuroni con i parametri biologici e lo stato dinamico iniziale
        Neurone(int id, double V_0, double V_th, double V_rest, double V_reset, double tau, double tauR) 
            : id_(id), V_(V_0), Vth_(V_th), Vrest_(V_rest), Vreset_(V_reset), tau_(tau), tauR_(tauR), tempoRR_(0.0), fired(false) {}
        

        // Metodi della classe Neurone    
        void update(double inputTotale, double dt){
            fired = false;

            // gestione refrattario
            if(tempoRR_ > 0){
                tempoRR_ -= dt;
                if(tempoRR_ < 0)
                    tempoRR_ = 0;
                return;
            }

            // dinamica del potenziale
            V_ += dt * (-(V_ - Vrest_) + inputTotale)/tau_;

            // firing
            if(V_ >= Vth_){
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


class ReteNeurale{

    private:
        std::vector<Neurone> neuroni_;
        std::vector<std::vector<double>> connessioni_; // matrice di connessioni con pesi
        std::vector<double> inputEsterno_; // input associati a ciascun neurone
        std::map<int, size_t> idToIndex_; // mappa dagli ID agli indici
    public:

        void aggiungiNeurone(const Neurone& neurone){
            neuroni_.push_back(neurone); // aggiunge il neurone alla lista
            idToIndex_[neurone.getId()] = neuroni_.size() - 1; // aggiunge ID alla mappa
            
            int newSize = neuroni_.size();  // dimensione aggiornata della rete

            connessioni_.resize(newSize);   // aggiunge una nuova riga alla matrice di connessioni
            for(auto& row : connessioni_)   // aggiunge una nuova colonna a ogni riga esistente 
                row.resize(newSize, 0.0); 

            inputEsterno_.push_back(0.0); // aggiunge un nuovo input associato al nuovo neurone
        }

        void connettiNeuroni(int id1, int id2, double peso){
            if(idToIndex_.count(id1) && idToIndex_.count(id2)){ // verifica che entrambi i neuroni esistano (perche mappa ha ID unici)
                connessioni_[idToIndex_[id1]][idToIndex_[id2]] = peso; // inserisco la connessione da id1 a id2 con il peso specificato
            }
        }
        void setInput(int id, double valore){
            if(idToIndex_.count(id)){
                inputEsterno_[idToIndex_[id]] = valore; // imposta l'input associato al neurone con ID specificato
            }
        }

        void step(double dt){
            std::vector<double> inputTotale(neuroni_.size(), 0.0);

            // calcola l'input totale per ogni neurone basato sullo step precedente
            for(size_t i = 0; i < neuroni_.size(); ++i){
                for(size_t j = 0; j < neuroni_.size(); ++j){
                    if(neuroni_[j].hasFired()){
                        inputTotale[i] += connessioni_[j][i]; // somma i contributi dei neuroni che hanno sparato
                    }
                }
                inputTotale[i] += inputEsterno_[i]; // aggiungi l'input esterno se presente
            }

            // aggiorna lo stato di ogni neurone in base all'input totale e alle regole di attivazione
            for(size_t i = 0; i < neuroni_.size(); ++i){
                neuroni_[i].update(inputTotale[i], dt);
            }
        }
        
        std::vector<double> getPotenziali() const {
            std::vector<double> potenziali;
            potenziali.reserve(neuroni_.size());
            for(const auto& neurone : neuroni_)
                potenziali.push_back(neurone.getPotential());

            return potenziali;
        }
        
        std::vector<int> getFiringStates() const {
            std::vector<int> firingStates;
            firingStates.reserve(neuroni_.size());
            for(const auto& neurone : neuroni_)
                firingStates.push_back(neurone.hasFired() ? 1 : 0); // 1 se ha sparato, 0 altrimenti

            return firingStates;
        }

        void simulazione(double dt, double T, const std::string& filenameV, const std::string& filenameF){
            int steps = static_cast<int>(T / dt);
            double time = 0.0;

            std::ofstream filePotenziali(filenameV); // apre il filePotenziali 
            std::ofstream fileFiring(filenameF); // apre il fileFiring
            if(!filePotenziali || !fileFiring){ //controlla se i file sono stati aperti correttamente
                std::cerr << "Errore apertura file\n";
                return;
            }

            for(int n = 0; n < steps; ++n){
                step(dt); // esegue un passo di simulazione
                std::vector<double> potenziali = getPotenziali();
                std::vector<int> spikes = getFiringStates();
                time += dt; // aggiorna il tempo

                // salva lo stato della rete in due file
                filePotenziali << time << " ";
                for(double v : potenziali)
                    filePotenziali << v << " ";
                filePotenziali << "\n";

                fileFiring << time << " ";
                for(int f : spikes)
                    fileFiring << f << " ";
                fileFiring << "\n";
            }

            filePotenziali.close(); // chiude il filePotenziali
            fileFiring.close(); // chiude il fileFiring
        }


        ~ReteNeurale() = default;
};
/*
    Documentazione: 
    1) ordine delle conessioni nella matrice di connessioni: 
        - connessioni_[i][j] rappresenta la connessione da neurone i a neurone j, con il peso specificato
        - peso positivo indica una connessione eccitatoria, peso negativo indica una connessione inibitoria, peso zero indica nessuna connessione
    2) gestione dell'input esterno: 
        - inputEsterno_[i] rappresenta l'input esterno associato al neurone i, che viene aggiunto all'input totale calcolato dalle connessioni sinaptiche
    3) dinamica del potenziale di membrana:
        - il potenziale di membrana V_ viene aggiornato in base alla formula: dV/dt = (-(V_ - Vrest_) + inputTotale)/tau_
        - quando V_ supera la soglia Vth_, il neurone spara (fired = true), il potenziale viene resettato a Vreset_ e inizia il periodo refrattario
    4) gestione del periodo refrattario:
        - tempoRR_ rappresenta il tempo refrattario rimanente, durante il quale il neurone non può sparare
        - quando un neurone spara, tempoRR_ viene impostato a tauR_, e viene decrementato ad ogni passo di simulazione fino a tornare a zero
    5) spike interviene immediatamente nel passo successivo senza ritardo di trasmissione 
    6) simulazione:
        - il metodo simulazione() esegue la simulazione della rete per un tempo totale T con passo dt, salvando i potenziali di membrana e gli stati di firing in due file separati
    7) estendibilità futura: 
    ....
    


*/
