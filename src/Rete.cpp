#include "Rete.hpp"
#include <cstddef>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// Costruttore
// ─────────────────────────────────────────────────────────────────────────────

Rete::Rete(int N, NeuronModel typeNeurone, char typeintegratore) {

    neuroni_.reserve(N);
    statoNeuroni_.reserve(N);
    inputTotale_.reserve(N);
    stimoli_.reserve(N);
    statoFiring_.reserve(N);

    for (int i = 0; i < N; ++i) {
        aggiungiNeurone(i, typeNeurone, typeintegratore);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Neuroni
// ─────────────────────────────────────────────────────────────────────────────

void Rete::aggiungiNeurone(int ID, NeuronModel typeNeurone, char typeIntegratore) {
    if (hasNeurone(ID)) {
        std::cerr << "[Rete] errore: neurone con ID " << ID << " già presente.\n";
        return;
    }

    double initial_V = 0.0;

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

    // Inizializzazione degli stati
    statoNeuroni_.push_back(initial_V);
    idToIndex_[ID] = neuroni_.size() - 1;
    inputTotale_.push_back(0.0);
    stimoli_.push_back(0.0);
    statoFiring_.push_back(0.0);
}

void Rete::modificaIntegratoreNeurone(int ID, char typeIntegratore) {
    if (!hasNeurone(ID)) {
        std::cerr << "[Rete] errore: neurone con ID " << ID << " non esiste.\n";
        return;
    }
    size_t index = getIndex(ID);
    std::visit([&](auto& n) { n.tipoIntegratore_ = typeIntegratore; }, neuroni_[index]);
}

void Rete::modificaParametriNeurone(int id, const TypePatchNeuron& patch) {
    if (!hasNeurone(id)) {
        std::cerr << "[Rete] errore: neurone con ID " << id << " non esiste.\n";
        return;
    }

    size_t index = getIndex(id);

    std::visit(
        [&](auto& n) {
            using T = std::decay_t<decltype(n)>;

            if constexpr (std::is_same_v<T, LIF>) {
                // Controllo corrispondenza tra tipo di neurone e patch
                if (auto cfg = std::get_if<patchLIF>(&patch)) {

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
                // Controllo corrispondenza tra tipo di neurone e patch
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

    // Aggiornamento dello stato nei vettori della Rete per il monitoraggio
    statoNeuroni_[index] = std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[index]);
    bool fire = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[index]);
    statoFiring_[index] = fire ? 1.0 : 0.0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Sinapsi
// ─────────────────────────────────────────────────────────────────────────────

int Rete::connettiNeuroni(int IDpre, int IDpost, SynapseModel typeSynapse) {

    if (!hasNeurone(IDpre) || !hasNeurone(IDpost)) {
        std::cerr << "[Rete] errore: uno o entrambi i neuroni (pre=" << IDpre << ", post=" << IDpost
                  << ") non esistono nella rete.\n";
        return -1; // ID invalido come segnale di errore
    }

    size_t indexPre = idToIndex_[IDpre];
    size_t indexPost = idToIndex_[IDpost];

    double initial_Isyn = 0.0;

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

    statoSinapsi_.push_back(initial_Isyn);

    // Assegna e registra l'ID
    int idAssegnato = prossimoIdSyn_++;
    idToIndexSyn_[idAssegnato] = sinapsi_.size() - 1;

    return idAssegnato;
}

void Rete::modificaSinapsi(int IDsin, const TypePatchSyn& patch) {

    if (!hasSinapsi(IDsin)) {
        std::cerr << "[Rete] errore: sinapsi con ID " << IDsin << " non esiste.\n";
        return;
    }

    size_t index = idToIndexSyn_[IDsin];

    std::visit(
        [&](auto& syn) {
            using TSyn = std::decay_t<decltype(syn)>;

            if constexpr (std::is_same_v<TSyn, Current>) {
                if (auto cfg = std::get_if<patchCurrent>(&patch)) {
                    if (cfg->Isyn.has_value()) {
                        syn.Isyn_ = cfg->Isyn.value();
                        statoSinapsi_[index] = syn.Isyn_;
                    }
                    if (cfg->peso.has_value())
                        syn.peso_ = cfg->peso.value();
                    if (cfg->Ipeak.has_value())
                        syn.Ipeak_ = cfg->Ipeak.value();
                    if (cfg->tau.has_value())
                        syn.tau_ = cfg->tau.value();
                    if (cfg->delay.has_value())
                        syn.delay_ = cfg->delay.value();

                    // prepare() non è ancora stato chiamato, quindi il ring non esiste ancora:
                    // basta aggiornare delay_ e sarà prepare() a costruire il ring con il dt corretto.
                } else {
                    std::cerr << "[Rete] errore: configurazione incompatibile per CurrentSyn "
                              << "(hai passato configConductanceSyn?).\n";
                }
            } else if constexpr (std::is_same_v<TSyn, Conductance>) {
                if (auto cfg = std::get_if<patchConductance>(&patch)) {
                    if (cfg->gsyn.has_value())
                        syn.gsyn_ = cfg->gsyn.value();
                    if (cfg->peso.has_value())
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

std::vector<int> Rete::getSinapsiIds(int pre, int post) const {
    std::vector<int> ids;
    for (const auto& [id, index] : idToIndexSyn_) {
        const auto& syn = sinapsi_[index];
        int IDpre = std::visit([](const auto& s) { return s.getIdPre(); }, syn);
        int IDpost = std::visit([](const auto& s) { return s.getIdPost(); }, syn);
        if (IDpre == pre && IDpost == post) {
            ids.push_back(id);
        }
    }
    return ids;
}

// ─────────────────────────────────────────────────────────────────────────────
// Metodi operativi interni
// ─────────────────────────────────────────────────────────────────────────────

void Rete::prepare(double dt) {
    for (auto& s : sinapsi_) {
        std::visit([&](auto& syn) { syn.setDelayRing(dt); }, s);
    }
}

void Rete::step(double dt) {

    std::fill(inputTotale_.begin(), inputTotale_.end(), 0.0);

    for (auto& synVar : sinapsi_) {

        std::visit(
            [&](auto& syn) {
                using TSyn = std::decay_t<decltype(syn)>;

                bool preFired = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[syn.getIndexPre()]);

                if constexpr (std::is_same_v<TSyn, Current>) {
                    // Current-based: update autonomo, non serve V_post
                    syn.update(dt, preFired);

                } else if constexpr (std::is_same_v<TSyn, Conductance>) {
                    // Conductance-based: serve il potenziale del neurone post-sinaptico
                    double V_post =
                        std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[syn.getIndexPost()]);
                    syn.update(dt, preFired, V_post);
                }
                // sottraggo la corrente sinaptica all'input totale (fisicamente è coerente)
                inputTotale_[syn.getIndexPost()] -= syn.getCurrent();
            },
            synVar);
    }

    for (size_t i = 0; i < neuroni_.size(); ++i) {
        inputTotale_[i] += stimoli_[i];
        std::visit([&](auto& n) { n.update(inputTotale_[i], dt); }, neuroni_[i]);
    }
}

void Rete::aggiornaStatoRete() {

    for (size_t i = 0; i < neuroni_.size(); i++) {
        statoNeuroni_[i] = std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[i]);
        bool fire = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[i]);
        statoFiring_[i] = fire ? 1.0 : 0.0;
    }

    for (size_t i = 0; i < sinapsi_.size(); i++) {
        statoSinapsi_[i] = std::visit([](const auto& s) { return s.getCurrent(); }, sinapsi_[i]);
    }
}

double Rete::getMinTau() const {
    double minTau = std::numeric_limits<double>::max();

    // Controlla i neuroni
    for (const auto& n : neuroni_) {
        double tau = std::visit([](const auto& neuron) { return neuron.getTau(); }, n);
        minTau = std::min(minTau, tau);
    }

    // Controlla le sinapsi (sia Current che Conductance)
    for (const auto& s : sinapsi_) {
        double tau = std::visit([](const auto& syn) { return syn.tau_; }, s);
        minTau = std::min(minTau, tau);
    }

    return minTau;
}
