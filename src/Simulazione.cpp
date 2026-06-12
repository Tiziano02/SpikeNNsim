#include "Simulazione.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*
 * aggiungiInputEsterni — associa gli input esterni alla simulazione.
 *
 * Validazione:
 *  - ogni Input deve avere dimensione uguale a stepTotali_
 *  - ogni ID deve corrispondere a un neurone esistente nella rete
 * Se un input non supera la validazione, l'intera operazione viene annullata.
 */
void Simulazione::aggiungiInputEsterni(const std::vector<Input> &inputEsterno) {
    for (const auto &inp : inputEsterno) {
        if (static_cast<int>(inp.valori.size()) != stepTotali_) {
            std::cerr << "[Simulazione] errore: input per neurone " << inp.id << " ha dimensione " << inp.valori.size() << " ma stepTotali = " << stepTotali_ << ".\n";
            return;
        }
        if (!rete_.hasNeurone(inp.id)) {
            std::cerr << "[Simulazione] errore: neurone ID " << inp.id << " non trovato nella rete.\n";
            return;
        }
    }
    inputEsterni_ = inputEsterno; // questa cosa qui va fatta dai metodi setter ? Oppure un metodo setter è un metodo che fa questa cosa qui ma fuori dalla classe e quindi per essere sicuro di modificare un attributo privato lo faccio tramite un metodo setter che è molto espicito
}

/*
 * inizializzaOutput — apre i file, scrive l'header e allora il buffer della simulazione.
 *
 * I file vengono aperti in formato binario (std::ios::binary).Viene scritto un header iniziale di
 * 4 byte per ciascun file contenente il numero totale di colonne (tempo + dati) utile per il parsing.
 *
 *
 * Sequenza :
 * 1. Apre i file e controlla se ci sono errori
 * 1. Ricava il numero di colonne e crea l'header per i tre file di output
 * 3. Controllo la RAM a disposizione della macchina (dovrei fare i casi Windows e Linux)
 * 5. Calcolo grandezza buffer (RAM a disposizione e tipologia di macchina, byte per time step)
 * 6. Imposto il buffer della simulazione (attributo)
 *
 * Potrebbero essre divise in due funzioni : creazioneHeader e inizializzazioneBuffer ?
 *
 */
void Simulazione::inizializzaOutput() {

    // Apertura file e creazione dell'header

    filePotenziali_.open("output/" + fileNameV_, std::ios::binary | std::ios::out);
    fileFiring_.open("output/" + fileNameF_, std::ios::binary | std::ios::out);
    fileSinapsi_.open("output/" + fileNameS_, std::ios::binary | std::ios::out);

    if (!filePotenziali_.is_open() || !fileFiring_.is_open() || !fileSinapsi_.is_open()) {
        std::cerr << "[Simulazione] errore: impossibile aprire i file di output.\n";
        return;
    }

    int32_t colsV = static_cast<int32_t>(rete_.getNumNeuroni() + 1);
    int32_t colsF = static_cast<int32_t>(rete_.getNumNeuroni() + 1);
    int32_t colsS = static_cast<int32_t>(rete_.getNumSinapsi() + 1);

    filePotenziali_.write(reinterpret_cast<const char *>(&colsV), sizeof(int32_t));
    fileFiring_.write(reinterpret_cast<const char *>(&colsF), sizeof(int32_t));
    fileSinapsi_.write(reinterpret_cast<const char *>(&colsS), sizeof(int32_t));

    // creazione del buffer

    bytesPerStepV_ = static_cast<size_t>(colsV) * sizeof(double);
    bytesPerStepF_ = static_cast<size_t>(colsF) * sizeof(double);
    bytesPerStepS_ = static_cast<size_t>(colsS) * sizeof(double);

    size_t ramDisponibile = getAvailableRAM(); /* detection --> scritta interamente da gemini, da controllare ma sembra funzionare*/

    size_t bufferTarget = ramDisponibile / 10 / 3; // 10% della RAM diviso 3 buffer

    // quanti step interi di simulazione entrano nel buffer ?
    size_t stepsPerFlushV = bufferTarget / bytesPerStepV_;
    size_t stepsPerFlushF = bufferTarget / bytesPerStepF_;
    size_t stepsPerFlushS = bufferTarget / bytesPerStepS_;

    size_t stepsPerFlush = std::min({stepsPerFlushV, stepsPerFlushF, stepsPerFlushS});

    // non allocare più del necessario
    stepsPerFlush_ = std::min(stepsPerFlush, static_cast<size_t>(stepTotali_));

    // i buffer hanno dimensioni diverse ma si riempiono tutti allo stesso step
    size_t bufferSizeV = stepsPerFlush_ * bytesPerStepV_;
    size_t bufferSizeF = stepsPerFlush_ * bytesPerStepF_;
    size_t bufferSizeS = stepsPerFlush_ * bytesPerStepS_;

    // imposto la dimensione dei tre buffer
    bufferV_.resize(bufferSizeV);
    bufferF_.resize(bufferSizeF);
    bufferS_.resize(bufferSizeS);

    // Piccolo log a console per vedere quanta RAM sta usando effettivamente (utile per il profiling temporale)
    std::cout << "[Simulazione] RAM allocata per i 3 buffer: " << (bufferSizeV + bufferSizeF + bufferSizeS) / (1024 * 1024) << " MB (" << stepsPerFlush_ << " step per flush)\n";
}

/*
 * flushBuffer - scrive sui file il contenuto dei buffer e poi svuota il buffer
 *
 */
void Simulazione::writeFile() {

    // scrivo i buffer sui file
    filePotenziali_.write(bufferV_.data(), posizioneBuffer_ * bytesPerStepV_);
    fileFiring_.write(bufferF_.data(), posizioneBuffer_ * bytesPerStepF_);
    fileSinapsi_.write(bufferS_.data(), posizioneBuffer_ * bytesPerStepS_);

    // rimetto l'indice nella posizione zero così da sovrascrivere, senza svuotarlo, il buffer.
    // non crea problemi perchè quando scrivo su disco utilizzo la poszioneBuffer_
    posizioneBuffer_ = 0;
}

/*
 * loadStatoRete — carica lo stato della rete nel buffer
 *
 * Se è il buffer è pieno prima chiamo writeFile, che scrive sul file la parte di buffer corretta e poi
 * mette la poszione del buffer a zero
 *
 * siccome lo stato della rete sono vettori di double, mente il buffer sono in char devo fare prima una
 * conversione. l'offset serve per capire a che punto del buffer sono arrivato
 *
 */
void Simulazione::loadStatoRete(double time) {

    if (posizioneBuffer_ == stepsPerFlush_)
        writeFile();

    // a che punto sono nel buffer ?
    size_t offsetV = posizioneBuffer_ * bytesPerStepV_;
    size_t offsetF = posizioneBuffer_ * bytesPerStepF_;
    size_t offsetS = posizioneBuffer_ * bytesPerStepS_;

    // conversione da double a char della variabile tempo
    const char *src = reinterpret_cast<const char *>(&time);

    // inserisco il tempo nei tre buffer
    std::copy(src, src + sizeof(time), bufferV_.begin() + offsetV);
    std::copy(src, src + sizeof(time), bufferF_.begin() + offsetF);
    std::copy(src, src + sizeof(time), bufferS_.begin() + offsetS);

    // sposto l'offset dei byte del tempo
    offsetV += sizeof(time);
    offsetF += sizeof(time);
    offsetS += sizeof(time);

    // 1. Calcolo quanti BYTE totali occupano gli array
    size_t byteNeuroni = bytesPerStepV_ - sizeof(time);
    size_t byteFiring = bytesPerStepF_ - sizeof(time);
    size_t byteSinapsi = bytesPerStepS_ - sizeof(time);

    // 2. copio, in blocco, lo stato della rete (neuroni, firing e sinapsi)

    // Prendo il puntatore all'inizio dell'array e lo converto in puntatore a char
    const char *srcV = reinterpret_cast<const char *>(rete_.getPointerStatoNeuroni().data());
    std::copy(srcV, srcV + byteNeuroni, bufferV_.begin() + offsetV);

    const char *srcF = reinterpret_cast<const char *>(rete_.getPointerStatoFiring().data());
    std::copy(srcF, srcF + byteFiring, bufferF_.begin() + offsetF);

    const char *srcS = reinterpret_cast<const char *>(rete_.getPointerStatoSinapsi().data());
    std::copy(srcS, srcS + byteSinapsi, bufferS_.begin() + offsetS);

    // ho copiato nel buffer uno step, avanzo di possizione.
    posizioneBuffer_++;
}

/*
 * avviaSimulazione — esegue il loop principale e salva i risultati su file.
 *
 * I file vengono aperti in formato binario (std::ios::binary). Prima di iniziare
 * l'integrazione, viene scritto un header iniziale di 32 bit per ciascun file
 * contenente il numero totale di colonne (tempo + dati) utile per il parsing.
 *
 * Sequenza ad ogni step:
 * 1. inizializza il tempo della simulazione e il passo attuale a zero
 * 2. Estrae il valore corrente di ogni input esterno.
 * 3. Crea l'header per i tre file di output inserendo nei primi 4 byte il numero di collone
 * 4. Chiama Rete::step(dt, inputEsterniCorrente).
 * 5. Aggiorna il tempo trascorso nella simulazione: time += dt.
 * 6. Salva lo stato della rete sui tre file di output in binario.
 *
 * Il vettore inputEsterniCorrente è pre-allocato prima del loop per evitare
 * allocazioni dinamiche durante la simulazione.
 * I file vengono chiusi automaticamente alla distruzione degli ofstream.
 */
void Simulazione::avviaSimulazione(const std::string &filenameV, const std::string &filenameF, const std::string &filenameS) {
    double time = 0.0;
    stepCorrente_ = 0;

    // setter privato per chiarezza ?
    fileNameV_ = filenameV;
    fileNameF_ = filenameF;
    fileNameS_ = filenameS;

    // questa parte va insierita in un metodo privato che prepara l'input esterno
    std::vector<InputCorrente> inputEsterniCorrente;
    inputEsterniCorrente.reserve(inputEsterni_.size());
    for (const auto &inp : inputEsterni_)
        inputEsterniCorrente.push_back({inp.id, 0.0});

    inizializzaOutput();

    for (; stepCorrente_ < stepTotali_; ++stepCorrente_) {
        for (size_t i = 0; i < inputEsterniCorrente.size(); ++i)
            inputEsterniCorrente[i].valoreCorrente = inputEsterni_[i].valori[stepCorrente_];

        rete_.step(dt_, inputEsterniCorrente);
        time += dt_;

        rete_.aggiornaStatoRete();

        loadStatoRete(time); // in realtà gestisce tutto l'output : salva sul buffer, se piengo chiama flushBuffer che svuota il buffer e scrive su file
    }
    if (posizioneBuffer_ > 0)
        writeFile();
}
