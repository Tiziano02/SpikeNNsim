#include "Rete.hpp"
#include <fstream>
#include <iostream>
#include <string>

/*
 * aggiungiNeurone — aggiunge un neurone alla rete.
 *
 * Aggiunge il neurone a neuroni_, registra il suo ID nella mappa idToIndex_
 * e aggiunge una entry a zero nel buffer inputTotale e stimoli_.
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
    stimoli_.push_back(0.0);
    statoNeuroni_.push_back(neurone.getPotential());
    statoFiring_.push_back(0.0);
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
    statoSinapsi_.push_back(s.getCurrent());
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

/*
 * prepare - imposta le sinapsi da punto di vista della simualzione 
 *
 * 
 * */
void Rete::prepare(double dt) {
    // SINAPSI 
    for (auto& s : sinapsi_) {
        s.setDelayRing(dt);
    }
}
void Rete::step(double dt) {

    std::fill(inputTotale_.begin(), inputTotale_.end(), 0.0);

    // Per ogni sinapsi: aggiorna la corrente in base allo spike pre-sinaptico,
    // poi accumula immediatamente il contributo nel buffer del neurone post-sinaptico.
    // I due passi sono uniti in un solo loop perché update() legge hasFired() del
    // passo precedente, che non viene modificato dalla funzione step.
    for (auto &syn : sinapsi_) {
        syn.update(dt, neuroni_[syn.getIndexPre()].hasFired());
        inputTotale_[syn.getIndexPost()] += syn.getCurrent();
    }

    for (size_t i = 0; i < neuroni_.size(); ++i) {
        // aggiungo gli stimoli all'input totale
        inputTotale_[i] += stimoli_[i];

        // avanzamento dinamica di tutti i neuroni
        neuroni_[i].update(inputTotale_[i], dt);
    }
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
void Rete::aggiornaStatoRete() {

    for (size_t i = 0; i < neuroni_.size(); i++) {
        statoNeuroni_[i] = neuroni_[i].getPotential();
        statoFiring_[i] = neuroni_[i].hasFired() ? 1.0 : 0.0;
    }

    for (size_t i = 0; i < sinapsi_.size(); i++)
        statoSinapsi_[i] = sinapsi_[i].getCurrent();
}


/*
 * getPotenziali — restituisce puntatore a vetto cone il potenziale di membrana di tutti i neuroni [V].
 */
const std::vector<double> &Rete::getPointerStatoNeuroni() const { return statoNeuroni_; }
/*
 * getStatoFiring — restituisce il vettore di firing (1 = spike, 0 = no) per tutti i neuroni.
 */
const std::vector<double> &Rete::getPointerStatoFiring() const { return statoFiring_; }

/*
 * getStatoSinapsi — restituisce la corrente sinaptica corrente di tutte le sinapsi [A].
 */
const std::vector<double> &Rete::getPointerStatoSinapsi() const { return statoSinapsi_; }

