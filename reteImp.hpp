#ifndef RETEIMP_HPP
#define RETEIMP_HPP

#include "Rete.hpp"

/*
 * aggiungiNeurone — aggiunge un neurone alla rete.
 *
 * Aggiunge il neurone a neuroni_, registra il suo ID nella mappa idToIndex_
 * e aggiunge una entry a zero nel buffer inputTotale_.
 * Se un neurone con lo stesso ID esiste già, stampa un errore e non lo aggiunge.
 */
void Rete::aggiungiNeurone(const Neurone &neurone) {
    if (hasNeurone(neurone.getId())) {
        std::cerr << "[Rete] errore: neurone con ID " << neurone.getId() << " già presente.\n";
        return;
    }
    neuroni_.push_back(neurone);
    idToIndex_[neurone.getId()] = neuroni_.size() - 1;
    inputTotale_.push_back(0.0);
}

/*
 * connettiNeuroni — aggiunge una sinapsi alla rete.
 *
 * Verifica che entrambi i neuroni (pre e post) esistano nella rete.
 * Imposta gli indici diretti indexPre_ e indexPost_ direttamente sulla sinapsi
 * originale tramite i setter, poi la aggiunge a sinapsi_.
 * Gli indici diretti evitano il lookup sulla mappa ad ogni step in step().
 */
void Rete::connettiNeuroni(Sinapsi &s) {
    if (!hasNeurone(s.getIdPre()) || !hasNeurone(s.getIdPost())) {
        std::cerr << "[Rete] errore: uno o entrambi i neuroni (pre=" << s.getIdPre() << ", post=" << s.getIdPost() << ") non esistono nella rete.\n";
        return;
    }
    s.setIndexPre(idToIndex_[s.getIdPre()]);
    s.setIndexPost(idToIndex_[s.getIdPost()]);
    sinapsi_.push_back(s);
}

/*
 * step — avanza lo stato dell'intera rete di un passo dt.
 *
 * Sequenza (l'ordine è importante per la correttezza causale):
 *  1. Reset del buffer inputTotale_ a zero.
 *  2. Aggiornamento di tutte le sinapsi: usano il flag hasFired() dei neuroni
 *     relativo al passo precedente. Usa indexPre_ per accesso diretto senza lookup.
 *  3. Accumulo delle correnti sinaptiche in inputTotale_. Usa indexPost_ per
 *     accesso diretto senza lookup.
 *  4. Aggiunta delle correnti esterne a inputTotale_.
 *  5. Aggiornamento di ogni neurone con la corrente totale afferente.
 *
 * Nota: uno spike emesso al passo t aggiorna la sinapsi a partire dal passo t+1
 * (latenza minima di un passo). Questo è il comportamento causale corretto.
 */
void Rete::step(double dt, const std::vector<InputCorrente> &inputEsterni) {

    std::fill(inputTotale_.begin(), inputTotale_.end(), 0.0);

    // Per ogni sinapsi: aggiorna la corrente in base allo spike pre-sinaptico,
    // poi accumula immediatamente il contributo nel buffer del neurone post-sinaptico.
    // I due passi sono uniti in un solo loop perché update() legge hasFired() del
    // passo precedente, che non viene modificato dalla funzione step.
    for (auto &syn : sinapsi_) {
        syn.update(dt, neuroni_[syn.getIndexPre()].hasFired());
        inputTotale_[syn.getIndexPost()] += syn.getCurrent();
    }

    for (const auto &inp : inputEsterni)
        inputTotale_[idToIndex_[inp.id]] += inp.valoreCorrente;

    for (size_t i = 0; i < neuroni_.size(); ++i)
        neuroni_[i].update(inputTotale_[i], dt);
}

/*
 * getPotenziali — restituisce il potenziale di membrana di tutti i neuroni [V].
 */
std::vector<double> Rete::getPotenziali() const {
    std::vector<double> potenziali;
    potenziali.reserve(neuroni_.size());
    for (const auto &n : neuroni_)
        potenziali.push_back(n.getPotential());
    return potenziali;
}

/*
 * getFiringStates — restituisce il vettore di firing (1 = spike, 0 = no) per tutti i neuroni.
 */
std::vector<int> Rete::getFiringStates() const {
    std::vector<int> states;
    states.reserve(neuroni_.size());
    for (const auto &n : neuroni_)
        states.push_back(n.hasFired() ? 1 : 0);
    return states;
}

/*
 * getSinapsi — restituisce la corrente sinaptica corrente di tutte le sinapsi [A].
 */
std::vector<double> Rete::getSinapsi() const {
    std::vector<double> correnti;
    correnti.reserve(sinapsi_.size());
    for (const auto &s : sinapsi_)
        correnti.push_back(s.getCurrent());
    return correnti;
}

/*
 * salvaStatoRete — scrive lo stato corrente della rete su tre file di output.
 *
 * I dati sono salvati in formato puramente binario per massimizzare le prestazioni
 * di I/O. Per garantire coerenza durante il parsing (matrice omogenea), tutti i 
 * valori, inclusi i flag booleani di firing, vengono salvati come double a 64 bit.
 *
 * Sequenza di scrittura per step temporale:
 * 1. Scrittura del tempo corrente su ciascun file.
 * 2. Scrittura dei dati dei neuroni o delle sinapsi relativi a quel tempo.
 *
 * Scrive direttamente dai contenitori originali usando file.write()
 * senza allocare vettori temporanei.
 */
void Rete::salvaStatoRete(std::ofstream &filePotenziali, std::ofstream &fileFiring, std::ofstream &fileSinapsi, double time) {

    filePotenziali.write(reinterpret_cast<const char*>(&time), sizeof(double));
    for (const auto &n : neuroni_) {
        double v = n.getPotential();
        filePotenziali.write(reinterpret_cast<const char*>(&v), sizeof(double));
    }

    fileFiring.write(reinterpret_cast<const char*>(&time), sizeof(double));
    for (const auto &n : neuroni_) {
        double fired = n.hasFired() ? 1.0 : 0.0;
        fileFiring.write(reinterpret_cast<const char*>(&fired), sizeof(double));
    }

    fileSinapsi.write(reinterpret_cast<const char*>(&time), sizeof(double));
    for (const auto &s : sinapsi_) {
        double c = s.getCurrent();
        fileSinapsi.write(reinterpret_cast<const char*>(&c), sizeof(double));
    }
}

#endif // RETEIMP_HPP