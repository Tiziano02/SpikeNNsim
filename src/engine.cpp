#include "engine.hpp"
/*
void engine::prepare(double dt) {

    // 1. Creazione della lista di adiacenza e preparazione delle sinapsi

    // 1.1 inizializzazione della lista di adiacenza
    size_t numNeuroniTotali = statoNeuroni_.size(); // o il totale dei tuoi neuroni
    listaAdiacenza_.clear();
    listaAdiacenza_.resize(numNeuroniTotali);
    size_t countSinapsi = 0;

    // 1.1 inserimento dati nella lista di adiacenza e preparazione sinapsi
    for (auto& s : sinapsi_) {
        std::visit(
            [&](auto& syn) {
                listaAdiacenza_[syn.getIndexPre()].push_back(countSinapsi);
                syn.prepare(dt);
            },
            s);
        countSinapsi++;
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

void engine::step(double dt) {
    // 1. Azzeramento dell'input totale di tutti i neuroni
    std::fill(inputTotale_.begin(), inputTotale_.end(), 0.0);

    // 2. Evoluzione delle sinapsi --> stato precedente dei neuroni si può prendere dai vettori di stato della rete
    for (auto& synVar : sinapsi_) {

        std::visit(
            [&](auto& syn) {
                using TSyn = std::decay_t<decltype(syn)>;

                // 2.1 Controllo se il neurone pre-sinaptico ha sparato
                bool preFired = statoFiring_[syn.getIndexPre()];
                // bool preFired = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[syn.getIndexPre()]);

                // 2.2 Metodo di evoluzione delle sinapsi a seconda del tipo di sinapsi
                if constexpr (std::is_same_v<TSyn, Current>) {
                    syn.update(dt, preFired);

                } else if constexpr (std::is_same_v<TSyn, Conductance>) {

                    // 2.2.1 Calcolo del potenziale di membrana del neurone post-sinaptico
                    double V_post = statoNeuroni_[syn.getIndexPost()];

                    syn.update(dt, preFired, V_post);
                }

                // 2.3 Inserico le correnti sinaptiche all'input totale del neurone post-sinaptico (inserite con il
                // segno meno --> I positive)
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

void engine::aggiornaStatoRete() {
    // 1. Aggirnamento dei potenziali d'azione e dello stato dei neuroni
    for (size_t i = 0; i < neuroni_.size(); i++) {
        statoNeuroni_[i] = std::visit([](const auto& n) { return n.getPotential(); }, neuroni_[i]);
        bool fire = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[i]);
        statoFiring_[i] = fire ? 1.0 : 0.0;
    }

    // 2. Aggiornamento delle correnti sinaptiche
    // for (size_t i = 0; i < sinapsi_.size(); i++) {
    //    statoSinapsi_[i] = std::visit([](const auto& s) { return s.getCurrent(); }, sinapsi_[i]);
    //}
}
*/

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