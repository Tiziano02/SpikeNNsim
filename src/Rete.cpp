#include "Rete.hpp"
#include <cstddef>
#include <iostream>

// -----------------------------------------------------------------------------
// Costruttore
// -----------------------------------------------------------------------------

Rete::Rete(int N, NeuronModel typeNeurone, char typeintegratore) {

    // 1. Accantonamento dello spazio necessario
    neuroni_.reserve(N);
    statoNeuroni_.reserve(N);
    inputTotale_.reserve(N);
    stimoli_.reserve(N);
    statoFiring_.reserve(N);

    // 2. Csostruzione singola dei neuroni con parametri di default
    for (int i = 0; i < N; ++i) {
        aggiungiNeurone(i, typeNeurone, typeintegratore);
    }
}

// -----------------------------------------------------------------------------
// Neuroni
// -----------------------------------------------------------------------------

void Rete::aggiungiNeurone(int ID, NeuronModel typeNeurone, char typeIntegratore) {

    // 1. Controllo ID
    if (hasNeurone(ID)) {
        std::cerr << "[Rete] errore: neurone con ID " << ID << " già presente.\n";
        return;
    }

    // 2. Variabile per la condizione inziale dei neuroni
    double initial_V = 0.0;

    // 3. Chiamata costruttore a seconda del tipo di neurone e inserimento nella rete
    switch (typeNeurone) {

    case NeuronModel::LIF: {
        auto& n = neuroni_.emplace_back(std::in_place_type<LIF>, ID, typeIntegratore);
        initial_V = std::get<LIF>(n).V_; // Prende il default dalla classe
        break;
    }
    case NeuronModel::Exp: {
        auto& n = neuroni_.emplace_back(std::in_place_type<Exp>, ID, typeIntegratore);
        initial_V = std::get<Exp>(n).V_; // Prende il default dalla classe
        break;
    }
    default: {
        std::cerr << "[Rete] errore: tipo neurone non supportato.\n";
        return;
    }
    }

    // 4. Inizializzazione dello stato della rete
    statoNeuroni_.push_back(initial_V);
    inputTotale_.push_back(0.0);
    stimoli_.push_back(0.0);
    statoFiring_.push_back(0.0);

    // 5. Aggiornamento mappa indici-ID
    idToIndex_[ID] = neuroni_.size() - 1;
}

void Rete::modificaIntegratoreNeurone(int ID, char typeIntegratore) {

    // 1. Controllo ID neurone
    if (!hasNeurone(ID)) {
        std::cerr << "[Rete] errore: neurone con ID " << ID << " non esiste.\n";
        return;
    }

    // 2. Calcolo indice neurone da modificare
    size_t index = getIndex(ID);

    // 3. Modifica dell'integratore numerico
    std::visit([&](auto& n) { n.tipoIntegratore_ = typeIntegratore; }, neuroni_[index]);
}

void Rete::modificaParametriNeurone(int id, const TypePatchNeuron& patch) {

    // 1. Controllo ID neurone da modificare
    if (!hasNeurone(id)) {
        std::cerr << "[Rete] errore: neurone con ID " << id << " non esiste.\n";
        return;
    }

    // 2. Calcolo indice del neurone da modificare
    size_t index = getIndex(id);

    // 3. Modifica dei parametri
    std::visit(
        [&](auto& n) {
            using T = std::decay_t<decltype(n)>;

            // 3.1 Controllo della tipologia di neurone
            if constexpr (std::is_same_v<T, LIF>) {
                // 3.2 Controllo corrispondenza tra tipo di neurone e patch
                if (auto cfg = std::get_if<patchLIF>(&patch)) {

                    // 3.3 Controllo quali parametri si devono modificare e li modifico
                    if (cfg->V.has_value())
                        n.V_ = cfg->V.value();
                    if (cfg->Vth.has_value())
                        n.Vth_ = cfg->Vth.value();
                    if (cfg->VthMin.has_value())
                        n.VthMin_ = cfg->VthMin.value();
                    if (cfg->VthMax.has_value())
                        n.VthMax_ = cfg->VthMax.value();
                    if (cfg->Vrest.has_value())
                        n.Vrest_ = cfg->Vrest.value();
                    if (cfg->Vreset.has_value())
                        n.Vreset_ = cfg->Vreset.value();
                    if (cfg->R.has_value())
                        n.R_ = cfg->R.value();
                    if (cfg->C.has_value())
                        n.C_ = cfg->C.value();
                    if (cfg->timeAbsolute.has_value())
                        n.timeAbsolute_ = cfg->timeAbsolute.value();
                    if (cfg->timeRelative.has_value())
                        n.timeRelative_ = cfg->timeRelative.value();

                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per neurone LIF.\n";
                }
            } else if constexpr (std::is_same_v<T, Exp>) {
                if (auto cfg = std::get_if<patchExp>(&patch)) {

                    if (cfg->V.has_value())
                        n.V_ = cfg->V.value();
                    if (cfg->Vth.has_value())
                        n.Vth_ = cfg->Vth.value();
                    if (cfg->VthMin.has_value())
                        n.VthMin_ = cfg->VthMin.value();
                    if (cfg->VthMax.has_value())
                        n.VthMax_ = cfg->VthMax.value();
                    if (cfg->Vrest.has_value())
                        n.Vrest_ = cfg->Vrest.value();
                    if (cfg->Vreset.has_value())
                        n.Vreset_ = cfg->Vreset.value();
                    if (cfg->R.has_value())
                        n.R_ = cfg->R.value();
                    if (cfg->C.has_value())
                        n.C_ = cfg->C.value();
                    if (cfg->timeAbsolute.has_value())
                        n.timeAbsolute_ = cfg->timeAbsolute.value();
                    if (cfg->timeRelative.has_value())
                        n.timeRelative_ = cfg->timeRelative.value();
                    if (cfg->sharpness.has_value())
                        n.sharpness_ = cfg->sharpness.value();

                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per neurone Exp.\n";
                }
            }
        },
        neuroni_[index]);

    // 4. Aggiornamento dello stato della Rete
    statoNeuroni_[index] = std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[index]);
    bool fire = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[index]);
    statoFiring_[index] = fire ? 1.0 : 0.0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Sinapsi
// ─────────────────────────────────────────────────────────────────────────────

int Rete::connettiNeuroni(int IDpre, int IDpost, SynapseModel typeSynapse) {

    // 1. Controllo ID neuroni da connettere
    if (!hasNeurone(IDpre) || !hasNeurone(IDpost)) {
        std::cerr << "[Rete] errore: uno o entrambi i neuroni (pre=" << IDpre << ", post=" << IDpost
                  << ") non esistono nella rete.\n";
        return -1; // ID invalido come segnale di errore
    }

    // 2. Calcolo indici dei neuroni da connettere
    size_t indexPre = idToIndex_[IDpre];
    size_t indexPost = idToIndex_[IDpost];

    // 3. Variabile per la condizione inizale della sinapsi
    double initial_Isyn = 0.0;

    // 4. Chiamata del costrutture a seconda della tipologia di sinapsi e inserimento nella rete
    switch (typeSynapse) {
    case SynapseModel::Conductance: {
        auto& s = sinapsi_.emplace_back(std::in_place_type<Conductance>, indexPre, indexPost, IDpre, IDpost);
        initial_Isyn = std::get<Conductance>(s).Isyn_;
        break;
    }
    case SynapseModel::Current: {
        auto& s = sinapsi_.emplace_back(std::in_place_type<Current>, indexPre, indexPost, IDpre, IDpost);
        initial_Isyn = std::get<Current>(s).Isyn_;
        break;
    }

    default: {
        std::cerr << "[Rete] errore: tipo sinapsi non supportato.\n";
        return -1;
    }
    }

    // 5. Inizializzazione stato sinapse nela rete
    statoSinapsi_.push_back(initial_Isyn);

    // 6. Assegna e registra nella mappa l'ID della sinapsi
    int idAssegnato = prossimoIdSyn_++;
    idToIndexSyn_[idAssegnato] = sinapsi_.size() - 1;

    // 7. Restituzione dell'ID della sinapsi per eventuali modificiche dei paramtri
    return idAssegnato;
}

void Rete::modificaSinapsi(int IDsin, const TypePatchSyn& patch) {

    // 1. Controllo ID della sinapsi
    if (!hasSinapsi(IDsin)) {
        std::cerr << "[Rete] errore: sinapsi con ID " << IDsin << " non esiste.\n";
        return;
    }

    // 2. Calcolo indice della sinapsi
    size_t index = idToIndexSyn_[IDsin];

    // 3. Modifica dei parametri della sinapsi
    std::visit(
        [&](auto& syn) {
            using TSyn = std::decay_t<decltype(syn)>;

            // 3.1 Controllo tipo di sinapsi
            if constexpr (std::is_same_v<TSyn, Current>) {

                // 3.2 Controllo che il tipo di sinapsi individuata dall'ID e il tipo di parametri passati corrispondono
                if (auto cfg = std::get_if<patchCurrent>(&patch)) {

                    // 3.3 Controllo parametri da modificare e modifica

                    if (cfg->Isyn.has_value()) {
                        syn.Isyn_ = cfg->Isyn.value();
                        statoSinapsi_[index] = syn.Isyn_; // a differenza della conductance-based si deve aggiornare lo
                                                          // stato della sinapse nella rete
                    }
                    if (cfg->peso.has_value())
                        syn.peso_ = cfg->peso.value();
                    if (cfg->Ipeak.has_value())
                        syn.Ipeak_ = cfg->Ipeak.value();
                    if (cfg->tau.has_value())
                        syn.tau_ = cfg->tau.value();
                    if (cfg->delay.has_value())
                        syn.delay_ = cfg->delay.value();
                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per CurrentSyn "
                              << "(hai passato configConductanceSyn?).\n";
                }
            } else if constexpr (std::is_same_v<TSyn, Conductance>) {
                if (auto cfg = std::get_if<patchConductance>(&patch)) {
                    if (cfg->gsyn.has_value())
                        syn.gsyn_ = cfg->gsyn.value();
                    if (cfg->peso.has_value() && cfg->peso.value() > 0.0) // controllo peso positivo
                        syn.peso_ = cfg->peso.value();
                    if (cfg->gpeak.has_value())
                        syn.gpeak_ = cfg->gpeak.value();
                    if (cfg->tau.has_value())
                        syn.tau_ = cfg->tau.value();
                    if (cfg->delay.has_value())
                        syn.delay_ = cfg->delay.value();
                    if (cfg->Erev.has_value())
                        syn.Erev_ = cfg->Erev.value();
                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per ConductanceSyn "
                              << "(hai passato configCurrentSyn?).\n";
                }
            }
        },
        sinapsi_[index]);
}

std::vector<int> Rete::findSinapsi(int pre, int post) const {
    // 1. vettore di indici di sinapsi tra due ID di neuroni
    std::vector<int> ids;

    // 2. Scorro tutta la mappa [indice - ID] delle sinapsi
    for (const auto& [id, index] : idToIndexSyn_) {

        // 2.1 seleziono una sinapsi
        const auto& syn = sinapsi_[index];

        // 2.2 estraggo gli ID dei neuroni
        int IDpre = std::visit([](const auto& s) { return s.getIdPre(); }, syn);
        int IDpost = std::visit([](const auto& s) { return s.getIdPost(); }, syn);

        // 2.3 se corrispondo entrambi agli ID target signfica che ho trovato una sinapsi
        if (IDpre == pre && IDpost == post) {
            // aggiungo l'ID della sinapsi nella lista di ID di sinaspsi tra due ID di neuroni
            ids.push_back(id);
        }
    }

    // 3. Restituisco tutti gli id delle sinapsi cercate
    return ids;
}

// ─────────────────────────────────────────────────────────────────────────────
// Metodi operativi interni
// ─────────────────────────────────────────────────────────────────────────────

void Rete::prepare(double dt) {

    // 1. Inizializzo il ringDelay di ogni sinapsi
    for (auto& s : sinapsi_) {
        std::visit([&](auto& syn) { syn.setDelayRing(dt); }, s);
    }

    // ... nel futuro può fare altre cose : chiamato in automatico prima di ogni simulazione
    // aggiungere un metodo di blocco rete --> dopo aver chiamato blocco rete l'utente non può più modificare la rete
    // la simulazione invece può modificare la rete
    // una simulazione non può partire se blocco rete non è attivo
    // tutte le modifiche alla rete non vengono eseguite se il blocco rete è attivo
}

void Rete::step(double dt) {

    // 1. Azzeramento dell'input totale di tutti i neuroni
    std::fill(inputTotale_.begin(), inputTotale_.end(), 0.0);

    // 2. Evoluzione delle sinapsi
    for (auto& synVar : sinapsi_) {

        std::visit(
            [&](auto& syn) {
                using TSyn = std::decay_t<decltype(syn)>;

                // 2.1 Controllo se il neurone pre-sinaptico ha sparato
                bool preFired = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[syn.getIndexPre()]);

                // 2.2 Metodo di evoluzione delle sinapsi a seconda del tipo di sinapsi
                if constexpr (std::is_same_v<TSyn, Current>) {
                    syn.update(dt, preFired);

                } else if constexpr (std::is_same_v<TSyn, Conductance>) {

                    // 2.2.1 Calcolo del potenziale di membrana del neurone post-sinaptico
                    double V_post =
                        std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[syn.getIndexPost()]);

                    syn.update(dt, preFired, V_post);
                }

                // 2.3 Inserico le correnti sinaptiche all'input totale del neurone post-sinaptico
                inputTotale_[syn.getIndexPost()] -= syn.getCurrent();
            },
            synVar);
    }

    // 3. Inserimento degli stimoli esterni ed evoluzione dinamica dei neuroni
    for (size_t i = 0; i < neuroni_.size(); ++i) {

        // 3.1 Inserimento stimoli esterni
        inputTotale_[i] += stimoli_[i];

        // 3.2 Evoluzione dinamica
        std::visit([&](auto& n) { n.update(inputTotale_[i], dt); }, neuroni_[i]);
    }
}

void Rete::aggiornaStatoRete() {

    // 1. Aggirnamento dei potenziali d'azione e dello stato dei neuroni
    for (size_t i = 0; i < neuroni_.size(); i++) {
        statoNeuroni_[i] = std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[i]);
        bool fire = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[i]);
        statoFiring_[i] = fire ? 1.0 : 0.0;
    }

    // 2. Aggiornamento delle correnti sinaptiche
    for (size_t i = 0; i < sinapsi_.size(); i++) {
        statoSinapsi_[i] = std::visit([](const auto& s) { return s.getCurrent(); }, sinapsi_[i]);
    }
}

double Rete::getMinTau() const {

    // 1. Inizializzazione
    double minTau = std::numeric_limits<double>::max();

    // 2. Controllo le scali temporali dei neuroni
    for (const auto& n : neuroni_) {
        double tau = std::visit([](const auto& neuron) { return neuron.getTau(); }, n);
        minTau = std::min(minTau, tau);
    }

    // 3. Controllo le scali temporali delle sinapsi
    for (const auto& s : sinapsi_) {
        double tau = std::visit([](const auto& syn) { return syn.tau_; }, s);
        minTau = std::min(minTau, tau);
    }

    // 4. Restitusico la scala temporale più piccola
    return minTau;

    // ...questo è un metodo che andrebbe in prepare()
}
