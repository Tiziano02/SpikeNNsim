#include "Rete.hpp"
#include "Utility.hpp"
#include <cstddef>
#include <iostream>

// -----------------------------------------------------------------------------
// POPOLAZIONI
// -----------------------------------------------------------------------------

size_t Rete::allocazioneNeuroni(size_t size, NeuronModel type, char integratore,
                                std::optional<TypePatchNeuron> config) {

    // 1. Calcolo indice iniziale blocco di neuroni
    size_t start = neuroni_.size();

    // 2. Riservo lo spazio per i vettori di stato e di topologia
    neuroni_.reserve(neuroni_.size() + size);
    inputTotale_.reserve(inputTotale_.size() + size);
    stimoli_.reserve(stimoli_.size() + size);
    statoNeuroni_.reserve(statoNeuroni_.size() + size);
    statoFiring_.reserve(statoFiring_.size() + size);

    // 3. Aggiungo neuroni e campio i parametri se necessario
    for (size_t i = 0; i < size; ++i) {
        size_t idx = aggiungiNeurone(type, integratore);
        if (config.has_value()) {
            modificaParametriNeurone(idx, config.value());
        }
    }
    return start;
}

Popolazione& Rete::addPopulation(size_t size, NeuronModel typeNeuron, char typeIntegratore,
                                 std::optional<TypePatchNeuron> config) {

    // 1. Alloco blocco di neuroni
    size_t start = allocazioneNeuroni(size, typeNeuron, typeIntegratore, config);

    // 2. Creo la popolazione e la aggiungo alla lista delle popolazioni
    Popolazione pop(start, size);
    popolazioni_.push_back(pop);

    // 4. Restituisco riferimeneto alla popolazione nel vettore popolazioni_
    return popolazioni_.back();
}

void Rete::modificaParametriPopolazione(size_t indicePopolazione, const TypePatchNeuron& patch) {

    // 1. Ricavo la popolazione dalla lista delle popolazioni
    Popolazione& pop = popolazioni_.at(indicePopolazione);

    // 2. Modifico i parametri di tutti i neuroni della popolazione
    for (size_t i = 0; i < pop.getSize(); ++i) {
        size_t idx = pop.getStart() + i;
        modificaParametriNeurone(idx, patch);
    }
}

void Rete::randomizzaParametriPopolazione(size_t indicePopolazione, const TypePatchNeuron& patch,
                                          DistType distribuzione, double dispersione) {

    // 1. Ricavo la popolazione dalla lista delle popolazioni
    Popolazione& pop = popolazioni_.at(indicePopolazione);

    // 2. Itero su tutti i neuroni della popolazione e randomizzo i parametri
    for (size_t i = 0; i < pop.getSize(); ++i) {
        size_t idx = pop.getStart() + i;
        randomizzaParametriNeurone(idx, patch, distribuzione, dispersione);
    }
}

Popolazione& Rete::addPopolazioneEterogenea(size_t size, std::vector<NeuronModel> typesNeuroni,
                                            std::vector<double> probabilita, std::vector<char> typesIntegratori,
                                            std::optional<std::vector<TypePatchNeuron>> configs) {

    // 1. Controllo che le dimensioni dei vettori siano coerenti
    if (typesNeuroni.size() != probabilita.size() || typesNeuroni.size() != typesIntegratori.size() ||
        (configs.has_value() && typesNeuroni.size() != configs->size())) {
        throw std::invalid_argument("[Rete] errore: le dimensioni dei vettori di input non sono coerenti.");
    } else if (size == 0) {
        throw std::invalid_argument("[Rete] errore: la dimensione della popolazione deve essere maggiore di zero.");
    } else if (typesNeuroni.empty() || probabilita.empty() || typesIntegratori.empty()) {
        throw std::invalid_argument("[Rete] errore: uno o più vettori di input sono vuoti.");
    }

    // 2. Controllo e normalizzazione delle probabilità - Creazione vettore size per le sub-popolazioni

    // 2.1 Calcolo la somma delle probabilità
    double sumProb = std::accumulate(probabilita.begin(), probabilita.end(), 0.0);

    // 2.2 Inizializzo il vettore delle dimensioni delle sub-popolazioni
    std::vector<size_t> sizes(typesNeuroni.size(), 0);

    // 2.3 Controllo che la somma delle probabilità sia maggiore di zero
    if (sumProb <= 0.0) {
        throw std::invalid_argument("Rete] errore: la somma delle probabilità deve essere maggiore di zero.");
    }

    // 2.4 Normalizzo le probabilità e calcolo le dimensioni delle sub-popolazioni
    for (size_t i = 0; i < probabilita.size(); ++i) {
        // 2.4.1 Normalizzo la probabilità
        probabilita[i] /= sumProb;
        // 2.4.2 Calcolo dimensionid delle sub-popolazione tramite arrotondamento
        sizes[i] = static_cast<size_t>(std::round(probabilita[i] * size));
    }

    // 3. Randomizzo dimensione delle sub-popolazioni
    generaAttornoVettore(sizes, 0.1, DistType::Gaussian, false, true);

    // 4. Creazione della popolazione eterogena
    size_t indexStart = neuroni_.size();
    Popolazione pop(indexStart, size);

    // 5.Aggiungo popolazione al vettore popolazioni_
    popolazioni_.push_back(pop);

    // 6. Allocazione blocchi di neuroni e inserimento degli indici delle sub-popolazioni
    for (size_t i = 0; i < sizes.size(); i++) {
        if (configs.has_value()) {
            allocazioneNeuroni(sizes[i], typesNeuroni[i], typesIntegratori[i], configs->at(i));
        } else {
            allocazioneNeuroni(sizes[i], typesNeuroni[i], typesIntegratori[i]);
        }
        // 6.2 Creo sub-popolazione
        Popolazione subPop(indexStart, sizes[i]);
        indexStart += sizes[i];
        popolazioni_.back().subPop_.push_back(&subPop);
    }

    // 7. Restitiusco riferimeto ad ultima popolazione in popolazioni_
    return popolazioni_.back();
}

// -----------------------------------------------------------------------------
// NEURONI
// -----------------------------------------------------------------------------

size_t Rete::aggiungiNeurone(NeuronModel typeNeurone, char typeIntegratore) {

    // 1. Variabile per la condizione inziale dei neuroni
    double initial_V = 0.0;

    // 2. Chiamata costruttore a seconda del tipo di neurone e inserimento nella rete
    switch (typeNeurone) {

    case NeuronModel::LIF: {
        auto& n = neuroni_.emplace_back(std::in_place_type<LIF>, typeIntegratore);
        initial_V = std::get<LIF>(n).V_; // Prende il valore di default dalla classe
        break;
    }
    case NeuronModel::Exp: {
        auto& n = neuroni_.emplace_back(std::in_place_type<Exp>, typeIntegratore);
        initial_V = std::get<Exp>(n).V_; // Prende il valore di default dalla classe
        break;
    }
    default: {
        std::cerr << "[Rete] errore: tipo neurone non supportato.\n";
        return 0;
    }
    }

    // 3. Inizializzazione dello stato della rete
    statoNeuroni_.push_back(initial_V);
    inputTotale_.push_back(0.0);
    stimoli_.push_back(0.0);
    statoFiring_.push_back(0.0);

    return neuroni_.size() - 1;
}

void Rete::modificaIntegratoreNeurone(size_t idx, char typeIntegratore) {

    // 1. Controllo indice neurone
    if (!hasNeurone(idx)) {
        std::cerr << "[Rete] errore: neurone con indice " << idx << " non esiste.\n";
        return;
    }

    // 2. Modifica dell'integratore numerico
    std::visit([&](auto& n) { n.tipoIntegratore_ = typeIntegratore; }, neuroni_[idx]);
}

void Rete::modificaParametriNeurone(size_t idx, const TypePatchNeuron& patch) {

    // 1. Controllo indice neurone da modificare
    if (!hasNeurone(idx)) {
        std::cerr << "[Rete] errore: neurone con indice " << idx << " non esiste.\n";
        return;
    }

    // 2. Modifica dei parametri
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
        neuroni_[idx]);

    // 4. Aggiornamento dello stato della Rete
    statoNeuroni_[idx] = std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[idx]);
    bool fire = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[idx]);
    statoFiring_[idx] = fire ? 1.0 : 0.0;
}

// Osservazioni: ogni parametro ha la stessa dispersione relativa, in futuro si potrebbe permettere di specificare
// dispersioni diverse per ogni parametro, "complicato" da implementare ma possibile.
void Rete::randomizzaParametriNeurone(size_t idx, const TypePatchNeuron& patch, DistType tipoDist, double dispersione) {

    // 1. Controllo indice neurone da modificare
    if (!hasNeurone(idx)) {
        std::cerr << "[Rete] errore: neurone con indice " << idx << " non esiste.\n";
        return;
    }

    // 2. Randomizzazione dei parametri
    std::visit(
        [&](auto& n) {
            using T = std::decay_t<decltype(n)>;

            // 2.1 Controllo della tipologia di neurone
            if constexpr (std::is_same_v<T, LIF>) {
                // 2.2 Controllo corrispondenza tra tipo di neurone e patch
                if (auto cfg = std::get_if<patchLIF>(&patch)) {

                    // 2.3 Controllo quali parametri si devono randomizzare e li randomizzo
                    if (cfg->V.has_value())
                        n.V_ = generaAttorno(cfg->V.value(), dispersione, tipoDist);
                    if (cfg->Vth.has_value())
                        n.Vth_ = generaAttorno(cfg->Vth.value(), dispersione, tipoDist);
                    if (cfg->VthMin.has_value())
                        n.VthMin_ = generaAttorno(cfg->VthMin.value(), dispersione, tipoDist);
                    if (cfg->VthMax.has_value())
                        n.VthMax_ = generaAttorno(cfg->VthMax.value(), dispersione, tipoDist);
                    if (cfg->Vrest.has_value())
                        n.Vrest_ = generaAttorno(cfg->Vrest.value(), dispersione, tipoDist);
                    if (cfg->Vreset.has_value())
                        n.Vreset_ = generaAttorno(cfg->Vreset.value(), dispersione, tipoDist);
                    if (cfg->R.has_value())
                        n.R_ = generaAttorno(cfg->R.value(), dispersione, tipoDist);
                    if (cfg->C.has_value())
                        n.C_ = generaAttorno(cfg->C.value(), dispersione, tipoDist);
                    if (cfg->timeAbsolute.has_value())
                        n.timeAbsolute_ = generaAttorno(cfg->timeAbsolute.value(), dispersione, tipoDist);
                }
            }
            if constexpr (std::is_same_v<T, Exp>) {

                if (auto cfg = std::get_if<patchExp>(&patch)) {

                    if (cfg->V.has_value())
                        n.V_ = generaAttorno(cfg->V.value(), dispersione, tipoDist);
                    if (cfg->Vth.has_value())
                        n.Vth_ = generaAttorno(cfg->Vth.value(), dispersione, tipoDist);
                    if (cfg->VthMin.has_value())
                        n.VthMin_ = generaAttorno(cfg->VthMin.value(), dispersione, tipoDist);
                    if (cfg->VthMax.has_value())
                        n.VthMax_ = generaAttorno(cfg->VthMax.value(), dispersione, tipoDist);
                    if (cfg->Vrest.has_value())
                        n.Vrest_ = generaAttorno(cfg->Vrest.value(), dispersione, tipoDist);
                    if (cfg->Vreset.has_value())
                        n.Vreset_ = generaAttorno(cfg->Vreset.value(), dispersione, tipoDist);
                    if (cfg->R.has_value())
                        n.R_ = generaAttorno(cfg->R.value(), dispersione, tipoDist);
                    if (cfg->C.has_value())
                        n.C_ = generaAttorno(cfg->C.value(), dispersione, tipoDist);
                    if (cfg->timeAbsolute.has_value())
                        n.timeAbsolute_ = generaAttorno(cfg->timeAbsolute.value(), dispersione, tipoDist);
                    if (cfg->sharpness.has_value())
                        n.sharpness_ = generaAttorno(cfg->sharpness.value(), dispersione, tipoDist);
                }
            }
        },
        neuroni_[idx]);
}

// -----------------------------------------------------------------------------
// Sinapsi
// -----------------------------------------------------------------------------

int Rete::connettiNeuroni(size_t indexPre, size_t indexPost, SynapseModel typeSynapse) {
    // 1. Controllo indici neuroni da connettere
    if (!hasNeurone(indexPre) || !hasNeurone(indexPost)) {
        std::cerr << "[Rete] errore: uno o entrambi i neuroni (pre=" << indexPre << ", post=" << indexPost
                  << ") non esistono nella rete.\n";
        return -1; // IDX invalido come segnale di errore
    }

    // 2. Variabile per la condizione inizale della sinapsi
    double initial_Isyn = 0.0;

    // 3. Chiamata del costrutture a seconda della tipologia di sinapsi e inserimento nella rete
    switch (typeSynapse) {
    case SynapseModel::Conductance: {
        auto& s = sinapsi_.emplace_back(std::in_place_type<Conductance>, indexPre, indexPost);
        initial_Isyn = std::get<Conductance>(s).Isyn_;
        break;
    }
    case SynapseModel::Current: {
        auto& s = sinapsi_.emplace_back(std::in_place_type<Current>, indexPre, indexPost);
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

    // 6. Restituzione dell'indice della sinapsi per eventuali modificiche dei paramtri
    return sinapsi_.size() - 1;
}

void Rete::modificaSinapsi(size_t indexSyn, const TypePatchSyn& patch) {
    // 1. Controllo ID della sinapsi
    if (!hasSinapsi(indexSyn)) {
        std::cerr << "[Rete] errore: sinapsi con ID " << indexSyn << " non esiste.\n";
        return;
    }

    // 2. Modifica dei parametri della sinapsi
    std::visit(
        [&](auto& syn) {
            using TSyn = std::decay_t<decltype(syn)>;

            // 3.1 Controllo tipo di sinapsi
            if constexpr (std::is_same_v<TSyn, Current>) {

                // 3.2 Controllo che il tipo di sinapsi individuata dall'ID e il tipo di parametri passati
                // corrispondono
                if (auto cfg = std::get_if<patchCurrent>(&patch)) {

                    // 3.3 Controllo parametri da modificare e modifica

                    if (cfg->Isyn.has_value()) {
                        syn.Isyn_ = cfg->Isyn.value();
                        statoSinapsi_[indexSyn] = syn.Isyn_; // a differenza della conductance-based si deve
                                                             // aggiornare lo stato della sinapse nella rete
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
        sinapsi_[indexSyn]);
}

std::vector<int> Rete::findSinapsi(size_t pre, size_t post) const {
    // 1. vettore di indici di sinapsi tra due ID di neuroni
    std::vector<int> indexes;

    // 2. Scorro tutta lista delle sinapsi
    for (size_t indexSyn = 0; indexSyn < sinapsi_.size(); indexSyn++) {

        // 2.1 estraggo una sinapsi
        const auto& syn = sinapsi_[indexSyn];

        // 2.2 estraggo gli indici dei neuroni della sinapis syn
        size_t indexPre = std::visit([](const auto& s) { return s.getIndexPre(); }, syn);
        size_t indexPost = std::visit([](const auto& s) { return s.getIndexPost(); }, syn);

        // 2.3 se corrispondo entrambi agli ID target signfica che ho trovato una sinapsi
        if (indexPre == pre && indexPost == post) {
            // aggiungo l'indice della sinapsi nella lista di indici di sinaspsi tra due indici di neuroni
            indexes.push_back(indexSyn);
        }
    }

    // 3. Restituisco tutti gli id delle sinapsi cercate
    return indexes;
}

// -----------------------------------------------------------------------------
// Metodi operativi interni
// -----------------------------------------------------------------------------

void Rete::prepare(double dt) {
    // 1. Inizializzo il ringDelay di ogni sinapsi
    for (auto& s : sinapsi_) {
        std::visit([&](auto& syn) { syn.setDelayRing(dt); }, s);
    }

    // ... nel futuro può fare altre cose : chiamato in automatico prima di ogni simulazione
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
