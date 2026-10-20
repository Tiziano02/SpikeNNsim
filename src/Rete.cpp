#include "Rete.hpp"
#include "Utility.hpp"
#include <cstddef>
#include <iostream>

// -----------------------------------------------------------------------------
// CONTROLLO STATO E CONGELAMENTO RETE
// -----------------------------------------------------------------------------

void Rete::bloccaRete() {
    isBloccata_ = true;
}

void Rete::sbloccaRete() {
    isBloccata_ = false;
}

void Rete::verificaNonBloccata(const std::string& operazione) const {
    if (isBloccata_) {
        throw std::logic_error("[Rete] Errore: la rete e' bloccata. Impossibile eseguire '" + operazione + "'.");
    }
}

// -----------------------------------------------------------------------------
// POPOLAZIONI
// -----------------------------------------------------------------------------

size_t Rete::allocazioneNeuroni(size_t size, NeuronModel type, char integratore, std::optional<TypePatchNeuron> config,
                                double dispersione, DistType tipoDist) {
    verificaNonBloccata("allocazioneNeuroni");

    // 1. Calcolo indice iniziale blocco di neuroni
    size_t start = neuroni_.size();

    // 2. Riservo lo spazio per i vettori di stato e di topologia
    neuroni_.reserve(neuroni_.size() + size);
    inputTotale_.reserve(inputTotale_.size() + size);
    stimoli_.reserve(stimoli_.size() + size);
    statoNeuroni_.reserve(statoNeuroni_.size() + size);
    statoFiring_.reserve(statoFiring_.size() + size);

    // 3. Aggiungo neuroni e applico patch/dispersione se fornita
    for (size_t i = 0; i < size; ++i) {
        aggiungiNeurone(type, integratore, config, dispersione, tipoDist);
    }
    return start;
}

size_t Rete::addPopulation(size_t size, NeuronModel typeNeuron, char typeIntegratore,
                           std::optional<TypePatchNeuron> config, double dispersione, DistType tipoDist) {
    verificaNonBloccata("addPopulation");

    // 1. Alloco blocco di neuroni
    size_t start = allocazioneNeuroni(size, typeNeuron, typeIntegratore, config, dispersione, tipoDist);

    // 2. Creo la popolazione e la aggiungo alla lista delle popolazioni
    popolazioni_.emplace_back(start, size);

    // 3. Restituisco indice della popolazione nel vettore popolazioni_
    return popolazioni_.size() - 1;
}

void Rete::modificaParametriPopolazione(size_t indicePopolazione, const TypePatchNeuron& patch, double dispersione,
                                        DistType distribuzione) {
    verificaNonBloccata("modificaParametriPopolazione");

    // 1. Ricavo la popolazione dalla lista delle popolazioni
    Popolazione& pop = popolazioni_.at(indicePopolazione);

    // 2. Itero su tutti i neuroni della popolazione e randomizzo i parametri
    for (size_t i = 0; i < pop.size_; ++i) {
        size_t idx = pop.indexStart_ + i;
        modificaParametriNeurone(idx, patch, dispersione, distribuzione);
    }
}

size_t Rete::addPopolazioneEterogenea(size_t size, std::vector<NeuronModel>& typesNeuroni,
                                      std::vector<double>& probabilita, std::vector<char>& typesIntegratori,
                                      std::optional<std::vector<TypePatchNeuron>> configs) {
    verificaNonBloccata("addPopolazioneEterogenea");

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

    // 2.5 Correggo la dimensione totale della popolazione a casua dell'arrotondamento
    size_t sommaCalcolata = std::accumulate(sizes.begin(), sizes.end(), 0);
    int diff = static_cast<int>(size) - static_cast<int>(sommaCalcolata);
    if (diff != 0 && !sizes.empty()) {
        sizes[0] = static_cast<size_t>(static_cast<int>(sizes[0]) + diff);
    }

    // 3. Randomizzo dimensione delle sub-popolazioni
    sizes = generaAttornoVettore(sizes, 0.1, DistType::Gaussian, false, true);

    // 4. Creazione della popolazione eterogena e aggiungo popolazione al vettore popolazioni_
    size_t indexStart = neuroni_.size();
    popolazioni_.emplace_back(indexStart, size);

    // 5. Allocazione blocchi di neuroni e inserimento delle sub-popolazioni
    for (size_t i = 0; i < sizes.size(); i++) {
        // 5.1 allocazione dei neuroni sul vettore neuroni_
        if (configs.has_value()) {
            allocazioneNeuroni(sizes[i], typesNeuroni[i], typesIntegratori[i], configs->at(i));
        } else {
            allocazioneNeuroni(sizes[i], typesNeuroni[i], typesIntegratori[i]);
        }
        // 5.2 Creo sub-popolazione, mi metto sull'ultima popolazione creata e gli aggiungo la sub-popolazione al
        // vettore subPopolazioni_
        popolazioni_.back().subPop_.emplace_back(indexStart, sizes[i]);

        // 5.3 correggo l'indice di partenza per il prossimo blocco di neuroni
        indexStart += sizes[i];
    }

    // 7. Restitiusco indice dell'ultima popolazione
    return popolazioni_.size() - 1;
}

void Rete::modificaParametriSubPopolazione(size_t indicePopolazione, size_t indiceSubPopolazione,
                                           const TypePatchNeuron& patch, double dispersione, DistType distribuzione) {
    verificaNonBloccata("modificaParametriSubPopolazione");

    // 1. Ricavo la popolazione dalla lista delle popolazioni

    Popolazione& pop = popolazioni_.at(indicePopolazione);
    // dovrei mettere erroe se non è stata trovata (cosa restituisci at se non trova ?)

    // 2. Ricavo la sub-popolazione dalla lista delle sub-popolazioni
    Popolazione& subPop = pop.subPop_.at(indiceSubPopolazione);

    // 3. Itero su tutti i neuroni della sub-popolazione e randomizzo i parametri
    for (size_t i = 0; i < subPop.size_; ++i) {
        size_t idx = subPop.indexStart_ + i;
        modificaParametriNeurone(idx, patch, dispersione, distribuzione);
    }
}

// -----------------------------------------------------------------------------
// CONDIZIONI INIZIALI POTENZIALI (SYNTACTIC SUGAR)
// -----------------------------------------------------------------------------

void Rete::setCondizioneInizialeIntervallo(size_t start, size_t end, double V, double dispersione, DistType tipoDist) {
    verificaNonBloccata("setCondizioneInizialeIntervallo");
    if (start >= neuroni_.size()) {
        std::cerr << "[Rete] errore: indice start " << start << " non valido per setCondizioneInizialeIntervallo.\n";
        return;
    }
    size_t actualEnd = std::min(end, neuroni_.size());

    for (size_t idx = start; idx < actualEnd; ++idx) {
        if (std::holds_alternative<LIF>(neuroni_[idx])) {
            modificaParametriNeurone(idx, patchLIF{.V = V}, dispersione, tipoDist);
        } else if (std::holds_alternative<Exp>(neuroni_[idx])) {
            modificaParametriNeurone(idx, patchExp{.V = V}, dispersione, tipoDist);
        }
    }
}

void Rete::setCondizioneInizialeRete(double V, double dispersione, DistType tipoDist) {
    verificaNonBloccata("setCondizioneInizialeRete");
    setCondizioneInizialeIntervallo(0, neuroni_.size(), V, dispersione, tipoDist);
}

void Rete::setCondizioneInizialePopolazione(size_t indicePopolazione, double V, double dispersione, DistType tipoDist) {
    verificaNonBloccata("setCondizioneInizialePopolazione");
    if (indicePopolazione >= popolazioni_.size()) {
        std::cerr << "[Rete] errore: indice popolazione " << indicePopolazione << " non valido per setCondizioneInizialePopolazione.\n";
        return;
    }
    const Popolazione& pop = popolazioni_[indicePopolazione];
    setCondizioneInizialeIntervallo(pop.indexStart_, pop.indexStart_ + pop.size_, V, dispersione, tipoDist);
}

// -----------------------------------------------------------------------------
// NEURONI
// -----------------------------------------------------------------------------

size_t Rete::aggiungiNeurone(NeuronModel typeNeurone, char typeIntegratore, std::optional<TypePatchNeuron> config,
                             double dispersione, DistType tipoDist) {
    verificaNonBloccata("aggiungiNeurone");

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

    size_t idx = neuroni_.size() - 1;

    // 4. Modifica parametri tramite patch opzionale
    if (config.has_value()) {
        modificaParametriNeurone(idx, config.value(), dispersione, tipoDist);
    }

    return idx;
}

void Rete::modificaIntegratoreNeurone(size_t idx, char typeIntegratore) {
    verificaNonBloccata("modificaIntegratoreNeurone");

    // 1. Controllo indice neurone
    if (!hasNeurone(idx)) {
        std::cerr << "[Rete] errore: neurone con indice " << idx << " non esiste.\n";
        return;
    }

    // 2. Modifica dell'integratore numerico
    std::visit([&](auto& n) { n.tipoIntegratore_ = typeIntegratore; }, neuroni_[idx]);
}

// Osservazione: chiamando più volte con patch diverse posso modificare la diepsione di ongi siignolo paramteor
void Rete::modificaParametriNeurone(size_t idx, const TypePatchNeuron& patch, double dispersione, DistType tipoDist) {
    verificaNonBloccata("modificaParametriNeurone");

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
                    if (cfg->timeRelative.has_value())
                        n.timeRelative_ = generaAttorno(cfg->timeRelative.value(), dispersione, tipoDist);

                    n.isPrepared_ = false; // Invalida i coefficienti precalcolati per ricalcolo
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
                    if (cfg->timeRelative.has_value())
                        n.timeRelative_ = generaAttorno(cfg->timeRelative.value(), dispersione, tipoDist);
                    if (cfg->sharpness.has_value())
                        n.sharpness_ = generaAttorno(cfg->sharpness.value(), dispersione, tipoDist);
                }
            }
        },
        neuroni_[idx]);

    // 3. Aggiornamento dello stato della rete dopo le modifiche

    // 3.1 Potenziale neurone
    statoNeuroni_[idx] = std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[idx]);

    // 3.2 Firing neurone
    bool fire = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[idx]);
    statoFiring_[idx] = fire ? 1.0 : 0.0;
}

void Rete::modificaParametriNeuroni(const std::vector<size_t>& idxs, const TypePatchNeuron& patch, double dispersione,
                                    DistType tipoDist) {
    verificaNonBloccata("modificaParametriNeuroni");
    for (size_t idx : idxs) {
        modificaParametriNeurone(idx, patch, dispersione, tipoDist);
    }
}
// -----------------------------------------------------------------------------
// Sinapsi
// -----------------------------------------------------------------------------

size_t Rete::connettiNeuroni(size_t indexPre, size_t indexPost, SynapseModel typeSynapse,
                             char typeIntegratore,
                             std::optional<TypePatchSyn> config, double dispersione, DistType tipoDist) {
    verificaNonBloccata("connettiNeuroni");
    // 1. Controllo indici neuroni da connettere
    if (!hasNeurone(indexPre) || !hasNeurone(indexPost)) {
        std::cerr << "[Rete] errore: uno o entrambi i neuroni (pre=" << indexPre << ", post=" << indexPost
                  << ") non esistono nella rete.\n";
        return static_cast<size_t>(-1); // IDX invalido come segnale di errore
    }

    // 2. Variabile per la condizione inizale della sinapsi
    double initial_Isyn = 0.0;

    // 3. Chiamata del costrutture a seconda della tipologia di sinapsi e inserimento nella rete
    switch (typeSynapse) {
    case SynapseModel::Conductance: {
        auto& s = sinapsi_.emplace_back(std::in_place_type<Conductance>, indexPre, indexPost, typeIntegratore);
        initial_Isyn = std::get<Conductance>(s).Isyn_;
        break;
    }
    case SynapseModel::Current: {
        auto& s = sinapsi_.emplace_back(std::in_place_type<Current>, indexPre, indexPost, typeIntegratore);
        initial_Isyn = std::get<Current>(s).Isyn_;
        break;
    }

    default: {
        std::cerr << "[Rete] errore: tipo sinapsi non supportato.\n";
        return static_cast<size_t>(-1);
    }
    }

    // 5. Inizializzazione stato sinapse nela rete
    statoSinapsi_.push_back(initial_Isyn);

    size_t idxSyn = sinapsi_.size() - 1;

    // 6. Applicazione eventuale configurazione (patch)
    if (config.has_value()) {
        modificaSinapsi(idxSyn, config.value(), dispersione, tipoDist);
    }

    // 7. Restituzione dell'indice della sinapsi
    return idxSyn;
}

void Rete::modificaIntegratoreSinapsi(size_t idx, char typeIntegratore) {
    verificaNonBloccata("modificaIntegratoreSinapsi");

    if (!hasSinapsi(idx)) {
        std::cerr << "[Rete] errore: sinapsi con ID " << idx << " non esiste.\n";
        return;
    }

    std::visit([&](auto& s) { s.tipoIntegratore_ = typeIntegratore; }, sinapsi_[idx]);
}

void Rete::modificaSinapsi(size_t indexSyn, const TypePatchSyn& patch, double dispersione, DistType tipoDist) {
    verificaNonBloccata("modificaSinapsi");
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
                        syn.Isyn_ = generaAttorno(cfg->Isyn.value(), dispersione, tipoDist);
                        statoSinapsi_[indexSyn] = syn.Isyn_; // a differenza della conductance-based si deve
                                                             // aggiornare lo stato della sinapse nella rete
                    }
                    if (cfg->peso.has_value())
                        syn.peso_ = generaAttorno(cfg->peso.value(), dispersione, tipoDist);
                    if (cfg->Ipeak.has_value())
                        syn.Ipeak_ = generaAttorno(cfg->Ipeak.value(), dispersione, tipoDist);
                    if (cfg->tau.has_value()) {
                        syn.tau_ = generaAttorno(cfg->tau.value(), dispersione, tipoDist);
                        syn.decayFactor_ = 0.0;
                    }
                    if (cfg->delay.has_value())
                        syn.delay_ = generaAttorno(cfg->delay.value(), dispersione, tipoDist);
                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per CurrentSyn "
                              << "(hai passato configConductanceSyn?).\n";
                }
            } else if constexpr (std::is_same_v<TSyn, Conductance>) {
                if (auto cfg = std::get_if<patchConductance>(&patch)) {
                    if (cfg->gsyn.has_value())
                        syn.gsyn_ = generaAttorno(cfg->gsyn.value(), dispersione, tipoDist);
                    if (cfg->peso.has_value() && cfg->peso.value() > 0.0) { // controllo peso positivo
                        double val = generaAttorno(cfg->peso.value(), dispersione, tipoDist);
                        if (val > 0.0)
                            syn.peso_ = val;
                    }
                    if (cfg->gpeak.has_value())
                        syn.gpeak_ = generaAttorno(cfg->gpeak.value(), dispersione, tipoDist);
                    if (cfg->tau.has_value()) {
                        syn.tau_ = generaAttorno(cfg->tau.value(), dispersione, tipoDist);
                        syn.decayFactor_ = 0.0;
                    }
                    if (cfg->delay.has_value())
                        syn.delay_ = generaAttorno(cfg->delay.value(), dispersione, tipoDist);
                    if (cfg->Erev.has_value())
                        syn.Erev_ = generaAttorno(cfg->Erev.value(), dispersione, tipoDist);
                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per ConductanceSyn "
                              << "(hai passato configCurrentSyn?).\n";
                }
            }
        },
        sinapsi_[indexSyn]);
}

void Rete::modificaSinapsi(const std::vector<size_t>& idxs, const TypePatchSyn& patch, double dispersione,
                           DistType tipoDist) {
    verificaNonBloccata("modificaSinapsi");
    for (size_t idx : idxs) {
        modificaSinapsi(idx, patch, dispersione, tipoDist);
    }
}

std::vector<size_t> Rete::findSinapsi(size_t pre, size_t post) const {
    // 1. vettore di indici di sinapsi tra due ID di neuroni
    std::vector<size_t> indexes;

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

std::vector<size_t> Rete::connettiPopolazione(size_t indicePopolazione, double probabilitaConnessione,
                                              SynapseModel typeSynapse, char typeIntegratore,
                                              std::optional<TypePatchSyn> config,
                                              double dispersione, DistType tipoDist) {
    verificaNonBloccata("connettiPopolazione");
    // 1. Controllo che l'indice della popolazione sia valido
    if (indicePopolazione >= popolazioni_.size()) {
        std::cerr << "[Rete] errore: indice popolazione " << indicePopolazione << " non valido.\n";
        return {};
    }

    // 2. Ricavo la popolazione dalla lista delle popolazioni
    Popolazione& pop = popolazioni_[indicePopolazione];

    // 3. Inizializzo vettore di indici da restituire
    std::vector<size_t> sinapsiCreate;

    // 4. Itero su tutti i neuroni della popolazione e creo connessioni casuali
    for (size_t i = 0; i < pop.size_; ++i) {
        size_t indexPre = pop.indexStart_ + i;

        for (size_t j = 0; j < pop.size_; ++j) {
            size_t indexPost = pop.indexStart_ + j;

            // Evito di connettere un neurone a se stesso
            if (indexPre != indexPost) {
                // Genero un numero casuale tra 0 e 1
                double randomValue = generaNumero<double>(0.0, 1.0, DistType::Uniform);

                // Se il numero casuale è minore della probabilità di connessione, creo la sinapsi
                if (randomValue < probabilitaConnessione) {
                    size_t id = connettiNeuroni(indexPre, indexPost, typeSynapse, typeIntegratore, config, dispersione, tipoDist);
                    if (id != static_cast<size_t>(-1)) {
                        sinapsiCreate.push_back(id);
                    }
                }
            }
        }
    }

    // 5. Memorizzo le sinapsi create nella popolazione
    pop.sinapsiInterne_.insert(pop.sinapsiInterne_.end(), sinapsiCreate.begin(), sinapsiCreate.end());

    return sinapsiCreate;
}

std::vector<size_t> Rete::connettiPopolazioni(size_t indicePopPre, size_t indicePopPost,
                                              double probabilitaConnessione, SynapseModel typeSynapse,
                                              char typeIntegratore,
                                              std::optional<TypePatchSyn> config,
                                              double dispersione, DistType tipoDist) {
    verificaNonBloccata("connettiPopolazioni");
    // 1. Se le due popolazioni coincidono, delega a connettiPopolazione per tracciare le sinapsi interne
    if (indicePopPre == indicePopPost) {
        return connettiPopolazione(indicePopPre, probabilitaConnessione, typeSynapse, typeIntegratore, config, dispersione, tipoDist);
    }

    // 2. Controllo validità degli indici delle popolazioni
    if (indicePopPre >= popolazioni_.size() || indicePopPost >= popolazioni_.size()) {
        std::cerr << "[Rete] errore: uno o entrambi gli indici di popolazione (pre=" << indicePopPre
                  << ", post=" << indicePopPost << ") non sono validi.\n";
        return {};
    }

    // 3. Ricavo le due popolazioni
    Popolazione& popPre = popolazioni_[indicePopPre];
    Popolazione& popPost = popolazioni_[indicePopPost];

    // 4. Inizializzo vettore di indici da restituire
    std::vector<size_t> sinapsiCreate;

    // 5. Itero su tutti i neuroni delle due popolazioni e creo connessioni casuali
    for (size_t i = 0; i < popPre.size_; ++i) {
        size_t indexPre = popPre.indexStart_ + i;

        for (size_t j = 0; j < popPost.size_; ++j) {
            size_t indexPost = popPost.indexStart_ + j;

            if (indexPre != indexPost) {
                double randomValue = generaNumero<double>(0.0, 1.0, DistType::Uniform);
                if (randomValue < probabilitaConnessione) {
                    size_t id = connettiNeuroni(indexPre, indexPost, typeSynapse, typeIntegratore, config, dispersione, tipoDist);
                    if (id != static_cast<size_t>(-1)) {
                        sinapsiCreate.push_back(id);
                    }
                }
            }
        }
    }

    return sinapsiCreate;
}

void Rete::modificaSinapsiPopolazione(size_t indicePopolazione, const TypePatchSyn& patch, double dispersione,
                                      DistType tipoDist) {
    verificaNonBloccata("modificaSinapsiPopolazione");
    // 1. Controllo che l'indice della popolazione sia valido
    if (indicePopolazione >= popolazioni_.size()) {
        std::cerr << "[Rete] errore: indice popolazione " << indicePopolazione << " non valido.\n";
        return;
    }

    // 2. Modifica di tutte le sinapsi interne della popolazione
    modificaSinapsi(popolazioni_[indicePopolazione].sinapsiInterne_, patch, dispersione, tipoDist);
}

// -----------------------------------------------------------------------------
// Metodi operativi interni
// -----------------------------------------------------------------------------

void Rete::prepare(double dt) {
    // 1. Preparazione delle sinapsi (allocazione ring buffer delay + precalcolo decadimento analitico)
    for (auto& s : sinapsi_) {
        std::visit([&](auto& syn) { syn.prepare(dt); }, s);
    }

    // 2. Preparazione dei neuroni (precalcolo coefficienti analitici esatti per LIF)
    for (auto& n : neuroni_) {
        std::visit(
            [&](auto& neuron) {
                using T = std::decay_t<decltype(neuron)>;
                if constexpr (std::is_same_v<T, LIF>) {
                    neuron.prepare(dt);
                }
            },
            n);
    }
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

// -----------------------------------------------------------------------------
// Metodi getter
// -----------------------------------------------------------------------------

double Rete::getResistenzaNeurone(size_t idx) const {
    if (!hasNeurone(idx))
        return 0.0;
    return std::visit([](const auto& n) { return n.R_; }, neuroni_[idx]);
}

double Rete::getCapacitaNeurone(size_t idx) const {
    if (!hasNeurone(idx))
        return 0.0;
    return std::visit([](const auto& n) { return n.C_; }, neuroni_[idx]);
}

double Rete::getSogliaNeurone(size_t idx) const {
    if (!hasNeurone(idx))
        return 0.0;
    return std::visit([](const auto& n) { return n.Vth_; }, neuroni_[idx]);
}

double Rete::getPesoSinapsi(size_t idx) const {
    if (!hasSinapsi(idx))
        return 0.0;
    return std::visit([](const auto& s) { return s.peso_; }, sinapsi_[idx]);
}

double Rete::getDelaySinapsi(size_t idx) const {
    if (!hasSinapsi(idx))
        return 0.0;
    return std::visit([](const auto& s) { return s.delay_; }, sinapsi_[idx]);
}

double Rete::getTauSinapsi(size_t idx) const {
    if (!hasSinapsi(idx))
        return 0.0;
    return std::visit([](const auto& s) { return s.tau_; }, sinapsi_[idx]);
}
