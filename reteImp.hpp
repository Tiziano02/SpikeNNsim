#include "Rete.hpp"
// Implementazione dei metodi della classe ReteNeurale

// metodi per gestire la rete neurale
void Rete::aggiungiNeurone(const Neurone &neurone) {

    neuroni_.push_back(neurone);                       // aggiunge il neurone alla lista
    idToIndex_[neurone.getId()] = neuroni_.size() - 1; // aggiunge ID alla mappa
    
    int newSize = neuroni_.size(); // dimensione aggiornata della rete
    
    connessioni_.resize(newSize); // aggiunge una nuova riga alla matrice di connessioni
    for (auto &row : connessioni_) // aggiunge una nuova colonna a ogni riga esistente
        row.resize(newSize, 0.0);

    inputEsterno_.push_back(0.0); // aggiunge un nuovo input associato al nuovo neurone
}
void Rete::connettiNeuroni(int id1, int id2, double peso) {
    if (idToIndex_.count(id1) && idToIndex_.count(id2))        // verifica che entrambi i neuroni esistano (perche mappa ha ID unici)
        connessioni_[idToIndex_[id1]][idToIndex_[id2]] = peso; // inserisco la connessione da id1 a id2 con il peso specificato
}
void Rete::setInput(int id, double valore) {
    if (idToIndex_.count(id))
        inputEsterno_[idToIndex_[id]] = valore; // imposta l'input associato al neurone con ID specificato
}
void Rete::step(double dt) {

    std::vector<double> inputTotale(neuroni_.size(), 0.0);

    // calcola l'input totale per ogni neurone basato sullo step precedente
    for (size_t i = 0; i < neuroni_.size(); ++i) {
        for (size_t j = 0; j < neuroni_.size(); ++j)
            if (neuroni_[j].hasFired())
                inputTotale[i] += connessioni_[j][i]; // somma i contributi dei neuroni che hanno sparato
        inputTotale[i] += inputEsterno_[i];           // aggiungi l'input esterno se presente
    }

    // aggiorna lo stato di ogni neurone in base all'input totale e alle regole di attivazione
    for (size_t i = 0; i < neuroni_.size(); ++i)
        neuroni_[i].update(inputTotale[i], dt);
}

// metodi getter
std::vector<double> Rete::getPotenziali() const {

    std::vector<double> potenziali;
    potenziali.reserve(neuroni_.size());

    for (const auto &neurone : neuroni_)
        potenziali.push_back(neurone.getPotential());

    return potenziali;
}
std::vector<int> Rete::getFiringStates() const {

    std::vector<int> firingStates;
    firingStates.reserve(neuroni_.size());

    for (const auto &neurone : neuroni_)
        firingStates.push_back(neurone.hasFired() ? 1 : 0); // 1 se ha sparato, 0 altrimenti

    return firingStates;
}

// metodi log
void Rete::salvaStatoRete(std::ofstream &filePotenziali, std::ofstream &fileFiring, double time) {

    std::vector<double> potenziali = getPotenziali();
    std::vector<int> spikes = getFiringStates();

    filePotenziali << time << " ";
    for (double v : potenziali)
        filePotenziali << v << " ";
    filePotenziali << "\n";

    fileFiring << time << " ";
    for (int f : spikes)
        fileFiring << f << " ";
    fileFiring << "\n";
}
