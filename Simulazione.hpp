#ifndef SIMULAZIONE_HPP
#define SIMULAZIONE_HPP

#include "Input.hpp"
#include "Rete.hpp"
#include "UnitaSI.hpp"

class Simulazione {
  private:
    Rete rete_;
    std::vector<Input> inputEsterni_;
    double dt_;
    double T_;
    int stepCorrente_;
    int stepTotali_;

  public:
    // costruttore per inizializzare i neuroni con i parametri biologici e lo stato dinamico iniziale
    Simulazione(const Rete& rete, double dt, double T) : rete_(rete), dt_(dt), T_(T), stepCorrente_(0), stepTotali_(static_cast<int>(std::round(T / dt))) {}

    // Metodi della classe Simulazione
    // aggiungi InputEsterni
    void aggiungiInputEsterni( const std::vector<Input>& inputEsterno) {
        // controllo che l'input ha la stessa dimensione della simulazione
        for (size_t i = 0; i < inputEsterno.size(); i++)
            if (stepTotali_ != inputEsterno[i].valori.size() || !rete_.hasNeurone(inputEsterno[i].id)) {
                std::cerr << "errore nell'input esterno o dimensione errata o il neurone assegnato non essite\n";
                return;
            }

        // imposto l'input esterno
        inputEsterni_ = inputEsterno;
    }

    void avviaSimulazione(const std::string &filenameV, const std::string &filenameF, const std::string &filenameS) {

        double time = 0.0 * s;
        std::vector<InputCorrente> inputEsterniCorrente;
        stepCorrente_ = 0;

        for (size_t i = 0; i < inputEsterni_.size(); i++) {
            InputCorrente tmp = {inputEsterni_[i].id, 0.0};
            inputEsterniCorrente.push_back(tmp);
        }

        std::ofstream filePotenziali;
        std::ofstream fileFiring;
        std::ofstream fileSinapsi;
        filePotenziali.open(filenameV);
        fileFiring.open(filenameF);
        fileSinapsi.open(filenameS);

        if (!filePotenziali.is_open() || !fileFiring.is_open() || !fileSinapsi.is_open()) {
            std::cerr << "Errore nell'apertura dei file di output!" << std::endl;
            return;
        }

        for (; stepCorrente_ < stepTotali_; stepCorrente_++) {

            for (size_t i = 0; i < inputEsterniCorrente.size(); i++) {
                inputEsterniCorrente[i].valoreCorrente = inputEsterni_[i].valori[stepCorrente_];
            }
            rete_.step(dt_, inputEsterniCorrente); // esegue un passo di simulazione
            time += dt_;                           // aggiorna il tempo

            rete_.salvaStatoRete(filePotenziali, fileFiring, fileSinapsi, time); // salva lo stato della rete nei file
        }
    }

    ~Simulazione() = default;
};

#endif // SIMULAZIONE_HPP