#include "Simulazione.hpp"
#include "Utility.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

// -----------------------------------------------------------------------------------------------------------
// COSTRUTTORE
// -----------------------------------------------------------------------------------------------------------

Simulazione::Simulazione(const Rete& rete, double dt, double T)
    : rete_(rete), dt_(dt), stepCorrente_(0), stepTotali_(static_cast<int>(std::round(T / dt))) {

    // 1. Controllo del tau minimo per il metodo di integrazione Eulero in avanti
    double tauMin = rete_.getMinTau();
    if (dt > tauMin / 10.0) {
        std::cerr << "⚠️  [Simulazione] ATTENZIONE: dt = " << dt << "s è maggiore di tau_min/10 (" << tauMin / 10.0
                  << "s).\n"
                  << "   La simulazione potrebbe essere numericamente instabile.\n"
                  << "   Considera di ridurre dt o aumentare la risoluzione temporale.\n";
    }
}

// -----------------------------------------------------------------------------------------------------------
// STIMOLI ESTERNI
// -----------------------------------------------------------------------------------------------------------

void Simulazione::iniettaStimoli(const std::vector<stimolo>& stimoli) {

    // 1. Accantonamento spazio necessario
    RegistroStimoli_.reserve(RegistroStimoli_.size() + stimoli.size());

    // 2. Aggiungere tutti gli stimoli nel registro degli stimoli
    for (const auto& [id, params] : stimoli) {

        // 2.1 Controllo ID dei neuroni che ricevono gli stimoli
        if (!rete_.hasNeurone(id)) {
            std::cerr << "[Simulazione] errore: neurone ID " << id << " non trovato.\n";
            return;
        }

        // 2.2 Calcolo indice del neurone target
        size_t indexNeurone = rete_.getIndex(id);

        // 2.3 Aggiungo stimolo al registro :  stimolo (ID - parametri ) --> record  registro (indice - parametri)
        RegistroStimoli_.push_back({indexNeurone, params});
    }
}

void Simulazione::valutaStimoli(double t) {

    // 1. Azzero il vector degli stimoli della rete
    std::fill(rete_.stimoli_.begin(), rete_.stimoli_.end(), 0.0);

    // 2. Ispezione del registro degli stimoli
    for (const auto& recordStimolo : RegistroStimoli_) {

        // 2.1 Calcolo valore dello stimolo per il record i-esimo nel registroStimoli
        double valore = std::visit(
            [&](const auto& params) -> double {
                using T = std::decay_t<decltype(params)>;

                // 2.1.1 Controllo intervallo temporale
                if (t < params.timeStart || t > params.timeEnd) {
                    return 0.0;
                }

                // 2.1.2 Calcolo il valore a seconda della tipologia di stimolo
                if constexpr (std::is_same_v<T, configConstantStimulus>) {
                    return params.ampiezza;
                } else if constexpr (std::is_same_v<T, configSinStimulus>) {
                    return params.ampiezza * std::sin(params.frequenza * t + params.fase);
                }
            },
            recordStimolo.parametri);

        // 2.2 Inserimento nel vettore degli stimoli il valore appena calcolato
        rete_.stimoli_[recordStimolo.indexNeurone] += valore;
    }
}

// -----------------------------------------------------------------------------------------------------------
// PREPARAZIONE OUTPUT
// -----------------------------------------------------------------------------------------------------------

void Simulazione::scritturaHeader() {

    // 1. Apertura dei file
    filePotenziali_.open("output/" + fileNameV_, std::ios::binary | std::ios::out);
    fileFiring_.open("output/" + fileNameF_, std::ios::binary | std::ios::out);
    fileSinapsi_.open("output/" + fileNameS_, std::ios::binary | std::ios::out);

    // 2. Controllo della corretta apertura dei file
    if (!filePotenziali_.is_open() || !fileFiring_.is_open() || !fileSinapsi_.is_open()) {
        std::cerr << "[Simulazione] errore: impossibile aprire i file di output.\n";
        return;
    }

    // 3. Calcolo "numero di colonne"
    int32_t colsV = rete_.getNumNeuroni() + 1;
    int32_t colsF = rete_.getNumNeuroni() + 1;
    int32_t colsS = rete_.getNumSinapsi() + 1;

    // 4. Scrittura dell'header nei file
    filePotenziali_.write(reinterpret_cast<const char*>(&colsV), sizeof(int32_t));
    fileFiring_.write(reinterpret_cast<const char*>(&colsF), sizeof(int32_t));
    fileSinapsi_.write(reinterpret_cast<const char*>(&colsS), sizeof(int32_t));
}

void Simulazione::creazioneBuffer() {

    // 1. Calcolo "numero di colonne"
    int32_t colsV = rete_.getNumNeuroni() + 1;
    int32_t colsF = rete_.getNumNeuroni() + 1;
    int32_t colsS = rete_.getNumSinapsi() + 1;

    // 2. Calcolo numero di byte da scrviere ad ogni step
    bytesPerStepV_ = static_cast<size_t>(colsV) * sizeof(double);
    bytesPerStepF_ = static_cast<size_t>(colsF) * sizeof(double);
    bytesPerStepS_ = static_cast<size_t>(colsS) * sizeof(double);

    // 3. Calcolo la RAM disponibile sulla macchina
    size_t ramDisponibile = getAvailableRAM(); // detection --> scritta interamente da gemini, da controllare

    // 4. Calcolo la grandezza massima uguale per i tre buffer
    size_t bufferTarget = ramDisponibile / 10 / 3;

    // 5. Calcolo il numero di step di simulazione che entrano nel buffer di dimensione massima
    size_t stepsPerFlushV = bufferTarget / bytesPerStepV_;
    size_t stepsPerFlushF = bufferTarget / bytesPerStepF_;
    size_t stepsPerFlushS = bufferTarget / bytesPerStepS_;

    // 6. Considero il numero di step minore
    stepsPerFlush_ = std::min({stepsPerFlushV, stepsPerFlushF, stepsPerFlushS, static_cast<size_t>(stepTotali_)});

    // 7. Controllo che il numero di step più piccolo non sia nullo
    if (stepsPerFlush_ == 0) {
        std::cerr << "[Simulazione] Warning: RAM disponibile insufficiente o non rilevata. "
                  << "Forzo stepsPerFlush a 1 per evitare il crash.\n";
        stepsPerFlush_ = 1;
    }

    // 8. Calcolo la dimensione di ciascun buffer in base al numero di byte scritti in uno step
    // Osservazione : i buffer hanno dimensioni diverse ma si riempiono tutti allo stesso step (stepPerFlush_)
    size_t bufferSizeV = stepsPerFlush_ * bytesPerStepV_;
    size_t bufferSizeF = stepsPerFlush_ * bytesPerStepF_;
    size_t bufferSizeS = stepsPerFlush_ * bytesPerStepS_;

    // 9. Imposto la dimensione dei tre buffer
    bufferV_.resize(bufferSizeV);
    bufferF_.resize(bufferSizeF);
    bufferS_.resize(bufferSizeS);

    // 10. Log per vedere quanta RAM si sta usando effettivamente
    std::cout << "[Simulazione] RAM allocata per i 3 buffer: "
              << (bufferSizeV + bufferSizeF + bufferSizeS) / (1024 * 1024) << " MB (" << stepsPerFlush_
              << " step per flush)\n";
}

void Simulazione::preparazioneOutput() {

    // 1. Apertura dei file di output e inseriemto dell'header
    scritturaHeader();

    // 2. Creazione del buffer
    creazioneBuffer();
}

// -----------------------------------------------------------------------------------------------------------
// SCRITTURA OUTPUT
// -----------------------------------------------------------------------------------------------------------

void Simulazione::writeFile() {

    // 1. Scrivo i buffer sui file
    filePotenziali_.write(bufferV_.data(), posizioneBuffer_ * bytesPerStepV_);
    fileFiring_.write(bufferF_.data(), posizioneBuffer_ * bytesPerStepF_);
    fileSinapsi_.write(bufferS_.data(), posizioneBuffer_ * bytesPerStepS_);

    // 2. Reset dell'indice
    // Sovrascrivo il buffer senza svuotarlo
    // non crea problemi perchè quando scrivo su disco utilizzo la posizioneBuffer_ e non scrivo tutto il buffer
    // in quel momento (quando la simualzione si interrope e il buffer enon è stato riempito tutto)
    posizioneBuffer_ = 0;
}

void Simulazione::loadStatoRete(double time) {

    // 1. Controllo se il buffer è pieno, nel caso scrivo su disco
    if (posizioneBuffer_ == stepsPerFlush_)
        writeFile();

    // 2. Calcolo in che poszione sono nel buffer
    size_t offsetV = posizioneBuffer_ * bytesPerStepV_;
    size_t offsetF = posizioneBuffer_ * bytesPerStepF_;
    size_t offsetS = posizioneBuffer_ * bytesPerStepS_;

    // 3. "Converto" il tempo attuale, inserisco il tempo nei tre buffer, vado avanti nella pozione del buffer attuale
    const char* src = reinterpret_cast<const char*>(&time);

    // 3.1 Inserisco il tempo nei tre buffer
    std::copy(src, src + sizeof(time), bufferV_.begin() + offsetV);
    std::copy(src, src + sizeof(time), bufferF_.begin() + offsetF);
    std::copy(src, src + sizeof(time), bufferS_.begin() + offsetS);

    // 3.2 Sposto l'offset pari a nuemro di byte della "variabile" tempo
    offsetV += sizeof(time);
    offsetF += sizeof(time);
    offsetS += sizeof(time);

    // 4. Calcolo quanti BYTE totali occupano gli array
    size_t byteNeuroni = bytesPerStepV_ - sizeof(time);
    size_t byteFiring = bytesPerStepF_ - sizeof(time);
    size_t byteSinapsi = bytesPerStepS_ - sizeof(time);

    // 5. Copio, in blocco, lo stato della rete (neuroni, firing e sinapsi)

    // 5.1 Prendo il puntatore all'inizio dell'array e lo converto in puntatore a char
    const char* srcV = reinterpret_cast<const char*>(rete_.getPointerStatoNeuroni().data());
    std::copy(srcV, srcV + byteNeuroni, bufferV_.begin() + offsetV);

    const char* srcF = reinterpret_cast<const char*>(rete_.getPointerStatoFiring().data());
    std::copy(srcF, srcF + byteFiring, bufferF_.begin() + offsetF);

    const char* srcS = reinterpret_cast<const char*>(rete_.getPointerStatoSinapsi().data());
    std::copy(srcS, srcS + byteSinapsi, bufferS_.begin() + offsetS);

    // 6. Avanzo la pozione nel buffer dopo aver salvato uno step di simulazione
    posizioneBuffer_++;
}

// -----------------------------------------------------------------------------------------------------------
// GESTIONE SIMULAZIONE
// -----------------------------------------------------------------------------------------------------------

void Simulazione::avviaSimulazione(const std::string& filenameV, const std::string& filenameF,
                                   const std::string& filenameS) {

    // 1. Isnizializza il tempo della simulazione e il passo attuale a zero
    double time = 0.0;
    stepCorrente_ = 0;

    // 2. Inializzo il nome del file dove salvare l'output della simulazione
    fileNameV_ = filenameV;
    fileNameF_ = filenameF;
    fileNameS_ = filenameS;

    // 3. Preparo il necessario per poter salvare su disco l'output
    preparazioneOutput();

    // 4. Per adesso : creazione della dimensione dei ring per il ritardo sinaptico
    rete_.prepare(dt_);

    // 5. Avvio della simualzione
    for (; stepCorrente_ < stepTotali_; ++stepCorrente_) {

        // 1. Applica gli stimoli al tempo corrente
        valutaStimoli(time);

        // 2. Avanza la rete di un passo
        rete_.step(dt_);
        time += dt_;

        // 3. Aggiorna lo stato e salva
        rete_.aggiornaStatoRete();
        loadStatoRete(time);
    }

    // 6. Svuoto il buffer a fine simulazione
    if (posizioneBuffer_ > 0) {
        writeFile();
    }
}
