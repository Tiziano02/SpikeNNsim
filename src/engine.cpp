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
    delaySteps_.resize(rete.sinapsi_.size());
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
                    sinapsiReteToEngine.push_back({SynapseModel::Current, current_.size() - 1});
                    currentEngineToRete.push_back(i);
                } else if constexpr (std::is_same_v<T, Conductance>) {
                    conductance_.push_back(syn);
                    sinapsiReteToEngine.push_back({SynapseModel::Conductance, conductance_.size() - 1});
                    conductanceEngineToRete.push_back(i);
                }
            },
            s);
    }

    // 4. Creazione della lista di adiacenza

    // 1.1 inizializzazione della lista di adiacenza
    listaAdiacenza_.resize(numeroNeuroniTotali_);
    size_t countSinapsi = 0;

    // 1.2 inserimento dati nella lista di adiacenza
    for (size_t i = 0; i < rete.sinapsi_.size(); ++i) {
        auto& s = rete.sinapsi_[i];
        std::visit(
            [&](auto& syn) {
                using T = std::decay_t<decltype(syn)>;
                if constexpr (std::is_same_v<T, Current>) {
                    listaAdiacenza_[syn.getIndexPre()].push_back(
                        {SynapseModel::Current, sinapsiReteToEngine[countSinapsi].indiceEngine});
                    countSinapsi++;
                } else if constexpr (std::is_same_v<T, Conductance>) {
                    listaAdiacenza_[syn.getIndexPre()].push_back(
                        {SynapseModel::Conductance, sinapsiReteToEngine[countSinapsi].indiceEngine});
                    countSinapsi++;
                }
            },
            s);
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

    // 1. Preparazione sinapsi e inzializzazione vettore delaySteps_

    // 1.1 Preparazione sinapsi di tipo Current
    for (size_t i = 0; i < current_.size(); ++i) {

        Current& s = current_[i];

        // 1.1.1 Preparazione della sinapsi (coefficienti analitici + soglia)
        s.prepare(dt);

        // 1.1.2 Calcolo del delay in step temporali e inserimento nel vettore delaySteps_
        delaySteps_[currentEngineToRete[i]] = static_cast<size_t>(std::round(s.getDelay() / dt));

        // 1.1.3 Controllo condizione inziale della sinapsi e inserimento nell'array activeCurrent_
        if (std::abs(s.Isyn_) > s.soglia_) {
            s.statoSinapsi_ = true;
            activeCurrent_.push_back(i);
        } else {
            s.Isyn_ = 0.0;
            s.statoSinapsi_ = false;
        }
    }

    // 1.2 Preparazione sinapsi di tipo Conductance
    for (size_t i = 0; i < conductance_.size(); ++i) {

        auto& s = conductance_[i];

        // 1.2.1 Preparazione della sinapsi (coefficienti analitici + soglia)
        s.prepare(dt);

        // 1.2.2 Calcolo del delay in step temporali e inserimento nel vettore delaySteps_
        delaySteps_[conductanceEngineToRete[i]] = static_cast<size_t>(std::round(s.getDelay() / dt));
    }

    // 2. Preparazione del ring buffer degli eventi sinaptici (risolvere problema di rete senza sinapsi)
    size_t maxDelaySteps = 0;
    if (!delaySteps_.empty()) {
        maxDelaySteps = *std::max_element(delaySteps_.begin(), delaySteps_.end());
    }
    ringEventi_.resize(maxDelaySteps + 1);

    // 3. Preparazione dei neuroni (precalcolo coefficienti analitici esatti per LIF)
    for (auto& n : lif_) {
        n.prepare(dt);
    }
}

void engine::step(double dt) {
    // 1. Azzeramento dell'input totale di tutti i neuroni
    std::fill(inputTotale_.begin(), inputTotale_.end(), 0.0);

    // 2. Controllo spike pre-sinaptici e inserimento degli eventi nel ring buffer
    // (quali neuroni hanno fatto spike nel passo precedente)
    for (std::size_t id = 0; id < numeroNeuroniTotali_; ++id) {

        if (statoFiring_[id] > 0.0) {

            // 2.1 Inserisco eventi nel ring per tutte le sinapsi che hanno il neurone id come pre-sinaptico
            for (const auto& s : listaAdiacenza_[id]) {

                if (s.modello == SynapseModel::Current) {

                    // 2.1.1 Calcolo il delayStep corrispondente alla sinapsi s
                    size_t delayStep = delaySteps_[currentEngineToRete[s.indiceEngine]];

                    // 2.1.2 Inserisco l'evento nel ring buffer alla posizione corretta
                    ringEventi_[(presentStep_ + delayStep) % ringEventi_.size()].push_back(
                        {SynapseModel::Current, s.indiceEngine});

                } else if (s.modello == SynapseModel::Conductance) {

                    // 2.2.1 Calcolo il delayStep corrispondente alla sinapsi s
                    size_t delayStep = delaySteps_[conductanceEngineToRete[s.indiceEngine]];

                    // 2.2.2 Inserisco l'evento nel ring buffer alla posizione corretta
                    ringEventi_[(presentStep_ + delayStep) % ringEventi_.size()].push_back(
                        {SynapseModel::Conductance, s.indiceEngine});
                }
            }
        }
    }

    // 3. Elaborazione degli eventi sinaptici nello step corrente
    for (const auto& evento : ringEventi_[presentStep_]) {

        if (evento.modello == SynapseModel::Current) {
            auto& s = current_[evento.indiceEngine];
            s.eventoArrivato_ = true;
            if (s.statoSinapsi_ == false) {
                s.statoSinapsi_ = true;
                activeCurrent_.push_back(evento.indiceEngine);
            }
        } else if (evento.modello == SynapseModel::Conductance) {
            auto& s = conductance_[evento.indiceEngine];
            s.eventoArrivato_ = true;
            // activeConductance_.push_back(evento.indiceEngine);
        }
    }

    // 3.1 Pulizia del ring buffer degli eventi nello step corrente
    ringEventi_[presentStep_].clear();

    // 2. Evoluzione delle sinapsi
    for (size_t i = 0; i < activeCurrent_.size();) {

        size_t index = activeCurrent_[i];
        bool avvenutoSwap = false;

        // 2.1 Avanzamento dinamica sinapsi
        current_[index].update(dt);

        // 2.2 Aggiungo le correnti appena calcolate all'input totale del neurone post-sinaptico
        inputTotale_[current_[index].getIndexPost()] -=
            current_[index].getCurrent(); // inserite con il segno meno per convenzione

        // 2.3 Controllo se la sinapsi è ancora attiva, altrimenti la rimuovo dall'array activeCurrent_
        // Se la sinapsi è delta può essere rimossa appena è stata processata (altrimenti viene chiamato update nello
        // step successivo)
        if (current_[index].isDelta() || std::abs(current_[index].Isyn_) <= current_[index].soglia_) {
            current_[index].Isyn_ = 0.0;
            current_[index].statoSinapsi_ = false;
            std::swap(activeCurrent_[i], activeCurrent_.back());
            activeCurrent_.pop_back();
            avvenutoSwap = true;
        }
        if (!avvenutoSwap)
            ++i;
    }

    for (auto& s : conductance_) {

        // 2.1 Calcolo del potenziale di membrana del neurone post-sinaptico
        double V_post = statoNeuroni_[s.getIndexPost()];

        // 2.2 Avanzamento dinamica sinapsi
        s.update(dt, V_post);

        // 2.3 Aggiungo le correnti appena calcolate all'input totale del neurone post-sinaptico
        inputTotale_[s.getIndexPost()] -= s.getCurrent(); // inserite con il segno meno per convenzione
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

    // 5. Avanzamento posizione nel ring buffer degli eventi sinaptici
    presentStep_ = (presentStep_ + 1) % ringEventi_.size();
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