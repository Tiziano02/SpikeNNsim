#include "Rete.hpp"
#include "Sinapsi.hpp"
// #include <fstream>
#include <cstddef>
#include <iostream>
#include <type_traits>

Rete::Rete(int N, Label_Type_Neuron typeNeurone, char typeintegratore) {

    // 1. PRE-ALLOCAZIONE: Salviamo sempre le performance del vettore
    neuroni_.reserve(N);

    // 2. SWITCH ESTERNO: Decidiamo il tipo una sola volta
    switch (typeNeurone) {

    case Label_Type_Neuron::LIF: {
        // Prepariamo la configurazione base una sola volta
        configLIF configBase;

        for (int i = 0; i < N; ++i) {
            aggiungiNeurone(i, typeintegratore, configBase);
        }
        break;
    }

    case Label_Type_Neuron::Exp: {
        // Prepariamo la configurazione base AdExp
        configExp configBase;
        for (int i = 0; i < N; ++i) {
            aggiungiNeurone(i, typeintegratore, configBase);
        }
        break;
    }

        // Nessun caso di default se usi enum class (il compilatore ti avviserà se ne mancano)
    }
}

/*
 * aggiungiNeurone — aggiunge un neurone alla rete.
 *
 * Aggiunge il neurone a neuroni_, registra il suo ID nella mappa idToIndex_
 * e aggiunge una entry a zero nel buffer inputTotale e stimoli_.
 * Se un neurone con lo stesso ID esiste già, stampa un errore e non lo aggiunge.
 */
void Rete::aggiungiNeurone(int id, char typeIntegratore, const TypeConfig &configurazione) {

    std::visit(
        [&](const auto &config) {
            // 1. Controllo neuroen (Identico per tutti --> tutte le struct con parametro id )
            if (hasNeurone(id)) {
                std::cerr << "[Rete] errore: neurone con ID " << id << " già presente.\n";
                return; //  interrompendo l'aggiunta
            }

            // 2. Costruttori "tipizzati": Creazione del neurone e inserimento nel vector di neuroni --> tutto tramite emplace_back e in_place_type
            using T = std::decay_t<decltype(config)>; // "incastro" in T il vero tipo di configurazione

            if constexpr (std::is_same_v<T, configLIF>) { // Costrutture  LIF
                neuroni_.emplace_back(std::in_place_type<LIF>, id, typeIntegratore, config);
            } else if constexpr (std::is_same_v<T, configExp>) { // Costruttre AdEXP
                neuroni_.emplace_back(std::in_place_type<Exp>, id, typeIntegratore, config);
            }

            // 3. Aggiornamento attributi della rete // importante V_ = Vmebrana deve avere stesso nome per tutte le scruct
            statoNeuroni_.push_back(config.V_);
            idToIndex_[id] = neuroni_.size() - 1;

            inputTotale_.push_back(0.0);
            stimoli_.push_back(0.0);
            statoFiring_.push_back(0.0);
        },
        configurazione);
}

/*
 * Modifica integratore del neurone — cambia solo il metodo di integrazione.
 */
void Rete::modificaIntegratoreNeurone(int id, char typeIntegratore) {
    if (!hasNeurone(id)) {
        std::cerr << "[Rete] errore: neurone con ID " << id << " non esiste.\n";
        return;
    }

    size_t index = getIndex(id);
    std::visit([&](auto &n) { n.tipoIntegratore_ = typeIntegratore; }, neuroni_[index]);
}

/*
 * Modifica parametri biologici del neurone — mantiene lo stesso integratore.
 */
void Rete::modificaParametriNeurone(int id, const TypeConfig &configurazione) {
    if (!hasNeurone(id)) {
        std::cerr << "[Rete] errore: neurone con ID " << id << " non esiste.\n";
        return;
    }

    size_t index = getIndex(id);

    std::visit(
        [&](auto &n) {
            using T = std::decay_t<decltype(n)>;
            if constexpr (std::is_same_v<T, LIF>) {
                if (auto cfg = std::get_if<configLIF>(&configurazione)) {
                    n.V_ = cfg->V_;
                    n.Vth_ = cfg->V_th;
                    n.Vth0_ = cfg->V_th;
                    n.VthSpikeMax_ = cfg->V_ThresholdSpikeMax;
                    n.Vrest_ = cfg->V_rest;
                    n.Vreset_ = cfg->V_reset;
                    n.R_ = cfg->R;
                    n.C_ = cfg->C;
                    n.tau_ = cfg->R * cfg->C;
                    n.timeAbsolute_ = cfg->timeAbsolute;
                    n.timeRelative_ = cfg->timeRelative;
                    n.tauRelative_ = cfg->timeRelative / 3;
                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per neurone LIF.\n";
                    return;
                }
            } else if constexpr (std::is_same_v<T, Exp>) {
                if (auto cfg = std::get_if<configExp>(&configurazione)) {
                    n.V_ = cfg->V_;
                    n.Vth_ = cfg->V_th;
                    n.Vth0_ = cfg->V_th;
                    n.VthSpikeMax_ = cfg->V_ThresholdSpikeMax;
                    n.Vrest_ = cfg->V_rest;
                    n.Vreset_ = cfg->V_reset;
                    n.R_ = cfg->R;
                    n.C_ = cfg->C;
                    n.tau_ = cfg->R * cfg->C;
                    n.timeAbsolute_ = cfg->timeAbsolute;
                    n.timeRelative_ = cfg->timeRelative;
                    n.tauRelative_ = cfg->timeRelative / 3;
                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per neurone Exp.\n";
                    return;
                }
            }
        },
        neuroni_[index]);

    statoNeuroni_[index] = std::visit([](const auto &n) { return n.getPotential(); }, neuroni_[index]);
    bool fire = std::visit([](const auto &n) { return n.hasFired(); }, neuroni_[index]);
    statoFiring_[index] = fire ? 1.0 : 0.0;
}

/*
 * connettiNeuroni — aggiunge una sinapsi alla rete.
 *
 * Verifica che entrambi i neuroni (pre e post) esistano nella rete.
 * Imposta gli indici diretti indexPre_ e indexPost_ direttamente sulla sinapsi
 * originale tramite i setter, poi la aggiunge a sinapsi_.
 * Gli indici diretti evitano il lookup sulla mappa ad ogni step in step().
 */
void Rete::connettiNeuroni(int IDpre, int IDpost, configSyn s) {

    if (!hasNeurone(IDpre) || !hasNeurone(IDpost)) {
        std::cerr << "[Rete] errore: uno o entrambi i neuroni (pre=" << IDpre << ", post=" << IDpost << ") non esistono nella rete.\n";
        return;
    }

    size_t indexPre = idToIndex_[IDpre];
    size_t indexPost = idToIndex_[IDpost];
    sinapsi_.emplace_back(indexPre, indexPost, IDpre, IDpost, s);
    statoSinapsi_.push_back(sinapsi_.back().getCurrent()); // alternativamente : statoSinapsi_.push_back(s.Isyn_);
}

void Rete::modificaSinapsi(int IDpre, int IDpost, configSyn) {
    if (!hasNeurone(IDpre) || !hasNeurone(IDpost)) {
        std::cerr << "[Rete] errore: uno o entrambi i neuroni (pre=" << IDpre << ", post=" << IDpost << ") non esistono nella rete.\n";
        return;
    }

    sinapsi_.

    statoSinapsi_.push_back(sinapsi_.back().getCurrent()); // alternativamente : statoSinapsi_.push_back(s.Isyn_);

}

/*
 * prepare - imposta le sinapsi da punto di vista della simualzione
 *
 *
 * */
void Rete::prepare(double dt) {
    // SINAPSI
    for (auto &s : sinapsi_) {
        s.setDelayRing(dt);
    }
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
void Rete::step(double dt) {

    std::fill(inputTotale_.begin(), inputTotale_.end(), 0.0);

    // Per ogni sinapsi: aggiorna la corrente in base allo spike pre-sinaptico,
    // poi accumula immediatamente il contributo nel buffer del neurone post-sinaptico.
    // I due passi sono uniti in un solo loop perché update() legge hasFired() del
    // passo precedente, che non viene modificato dalla funzione step.
    for (auto &syn : sinapsi_) {

        bool preFired = std::visit([](const auto &n) { return n.hasFired(); }, neuroni_[syn.getIndexPre()]);

        syn.update(dt, preFired);
        inputTotale_[syn.getIndexPost()] += syn.getCurrent();
    }

    for (size_t i = 0; i < neuroni_.size(); ++i) {
        inputTotale_[i] += stimoli_[i];

        std::visit([&](auto &n) { n.update(inputTotale_[i], dt); }, neuroni_[i]);
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

        statoNeuroni_[i] = std::visit([](const auto &n) { return n.getPotential(); }, neuroni_[i]);

        bool fire = std::visit([](const auto &n) { return n.hasFired(); }, neuroni_[i]);

        statoFiring_[i] = fire ? 1.0 : 0.0;
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
