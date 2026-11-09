#include "engine.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <type_traits>

engine::engine(const Rete& rete) {

    // 1. Controllo che la rete sia stata bloccata prima di inziare la simulazione
    if (!rete.isBloccata()) {
        throw std::logic_error(
            "[engine] Errore: la rete non e' bloccata. Chiamare rete.bloccaRete() prima di creare l'engine.");
    }

    // 2. Numero totale di neuroni e sinapsi nella rete
    numeroNeuroniTotali_ = rete.neuroni_.size();
    numeroSinapsiTotali_ = rete.sinapsi_.size();

    // 2. Inizializzazione dei vettori di stato della rete
    statoNeuroni_.assign(numeroNeuroniTotali_, 0.0);
    statoFiring_.assign(numeroNeuroniTotali_, 0.0);
    statoSinapsi_.assign(numeroSinapsiTotali_, 0.0);

    // 3. Inizializzazione dei vettori di input e stimoli
    stimoli_.assign(numeroNeuroniTotali_, 0.0);
    inputTotale_.assign(numeroNeuroniTotali_, 0.0);

    // 4. Copia dei neuroni e delle sinapsi dalla rete all'engine e creazione della mappa (id rete <--> id engine)
    for (size_t i = 0; i < rete.neuroni_.size(); ++i) {
        auto& n = rete.neuroni_[i];

        std::visit(
            [&](auto& neuron) {
                using T = std::decay_t<decltype(neuron)>;
                if constexpr (std::is_same_v<T, LIF>) {
                    lif_.push_back(neuron);
                    lifEngineToRete.push_back(i);
                } else if constexpr (std::is_same_v<T, Exp>) {
                    exp_.push_back(neuron);
                    expEngineToRete.push_back(i);
                }
            },
            n);
    }

    for (size_t i = 0; i < rete.sinapsi_.size(); ++i) {
        auto& s = rete.sinapsi_[i];
        std::visit(
            [&](auto& syn) {
                using T = std::decay_t<decltype(syn)>;
                if constexpr (std::is_same_v<T, Current>) {
                    current_.push_back(syn);
                    currentEngineToRete.push_back(i);
                } else if constexpr (std::is_same_v<T, Conductance>) {
                    conductance_.push_back(syn);
                    ConductanceEngineToRete.push_back(i);
                }
            },
            s);
    }

    // 4. Creazione della lista di adiacenza

    // 1.1 inizializzazione della lista di adiacenza
    listaAdiacenza_.resize(numeroNeuroniTotali_);
    size_t countSinapsi = 0;

    // 1.2 inserimento dati nella lista di adiacenza
    for (auto& s : rete.sinapsi_) {
        std::visit([&](auto& syn) { listaAdiacenza_[syn.getIndexPre()].push_back(countSinapsi); }, s);
        countSinapsi++;
    }

    // Osservazione: potrebbe essere fatto dopo la creazione della struttua dati per l'engine
    // 5. Inserimento dello stato iniziale dei neuroni e delle sinapsi nei vettori di stato della rete
    for (size_t i = 0; i < numeroNeuroniTotali_; i++) {
        std::visit([&](auto& n) { statoNeuroni_[i] = n.getPotential(); }, rete.neuroni_[i]);
        std::visit([&](auto& n) { statoFiring_[i] = n.hasFired() ? 1.0 : 0.0; }, rete.neuroni_[i]);
    }
    for (size_t i = 0; i < rete.sinapsi_.size(); i++) {
        std::visit([&](auto& s) { statoSinapsi_[i] = s.getCurrent(); }, rete.sinapsi_[i]);
    }
};

void engine::prepare(double dt) {

    // 1. Preparazione sinapsi
    for (auto& s : current_) {
        s.prepare(dt);
    }
    for (auto& s : conductance_) {
        s.prepare(dt);
    }

    // 2. Preparazione dei neuroni (precalcolo coefficienti analitici esatti per LIF)
    for (auto& n : lif_) {
        n.prepare(dt);
    }
}

void engine::step(double dt) {
    // 1. Azzeramento dell'input totale di tutti i neuroni
    std::fill(inputTotale_.begin(), inputTotale_.end(), 0.0);

    // 2. Evoluzione delle sinapsi --> stato precedente dei neuroni si può prendere dai vettori di stato della rete
    for (auto& s : current_) {
        // 2.1 Controllo se il neurone pre-sinaptico ha sparato
        bool preFired = statoFiring_[s.getIndexPre()];
        // bool preFired = std::visit([](const auto& n) { return n.hasFired(); }, neuroni_[s.getIndexPre()]);

        // 2.2 Evoluzione della sinapsi
        s.update(dt, preFired);

        // 2.3 Inserico le correnti sinaptiche all'input totale del neurone post-sinaptico (inserite con il segno meno
        // --> I positive)
        inputTotale_[s.getIndexPost()] -= s.getCurrent();
    }

    for (auto& s : conductance_) {
        // 2.1 Controllo se il neurone pre-sinaptico ha sparato
        bool preFired = statoFiring_[s.getIndexPre()];

        // 2.2 Calcolo del potenziale di membrana del neurone post-sinaptico
        double V_post = statoNeuroni_[s.getIndexPost()];

        // 2.3 Evoluzione della sinapsi
        s.update(dt, preFired, V_post);

        // 2.4 Inserico le correnti sinaptiche all'input totale del neurone post-sinaptico (inserite con il segno meno
        // --> I positive)
        inputTotale_[s.getIndexPost()] -= s.getCurrent();
    }

    // 3. Aggiunta degli stimoli esterni
    for (std::size_t id = 0; id < numeroNeuroniTotali_; ++id) {
        inputTotale_[id] += stimoli_[id];
    }

    // 4. Evoluzione dei neuroni
    for (std::size_t k = 0; k < lif_.size(); ++k) {

        // 4.1 calcolo id rete corrispondente al neurone k nell'engine
        const auto id = lifEngineToRete[k];

        // 4.2 evoluzione del neurone
        lif_[k].update(inputTotale_[id], dt);
    }

    for (std::size_t k = 0; k < exp_.size(); ++k) {

        // 4.3 calcolo id rete corrispondente al neurone k nell'engine
        const auto id = expEngineToRete[k];

        // 4.4 evoluzione del neurone
        exp_[k].update(inputTotale_[id], dt);
    }
}

void engine::aggiornaStatoRete() {

    // 1. Aggirnamento dei potenziali d'azione e dello stato dei neuroni
    for (size_t i = 0; i < lif_.size(); i++) {

        // 1.1 calcolo id rete corrispondente al neurone i nell'engine
        const auto id = lifEngineToRete[i];

        // 1.2 aggiornamento dello stato dei neuroni
        statoNeuroni_[id] = lif_[i].getPotential();
        statoFiring_[id] = lif_[i].hasFired() ? 1.0 : 0.0;
    }

    for (size_t i = 0; i < exp_.size(); i++) {

        // 1.3 calcolo id rete corrispondente al neurone i nell'engine
        const auto id = expEngineToRete[i];

        // 1.4 aggiornamento dello stato dei neuroni
        statoNeuroni_[id] = exp_[i].getPotential();
        statoFiring_[id] = exp_[i].hasFired() ? 1.0 : 0.0;
    }

    // 2. Aggiornamento delle correnti sinaptiche
    /*
    for (std::size_t k = 0; k < current_.size(); ++k)
        statoSinapsi_[currentEngineToRete[k]] = current_[k].getCurrent();

    for (std::size_t k = 0; k < conductance_.size(); ++k)
        statoSinapsi_[ConductanceEngineToRete[k]] = conductance_[k].getCurrent();
    */
}

double engine::getMinTau() const {
    double minTau = std::numeric_limits<double>::max();

    for (const auto& neuron : lif_)
        minTau = std::min(minTau, neuron.getTau());

    for (const auto& neuron : exp_)
        minTau = std::min(minTau, neuron.getTau());

    for (const auto& syn : current_)
        minTau = std::min(minTau, syn.tau_);

    for (const auto& syn : conductance_)
        minTau = std::min(minTau, syn.tau_);

    return minTau;
}