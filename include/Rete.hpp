/**
 * @file Rete.hpp
 * @brief Gestore principale della topologia e dell'evoluzione temporale della SpikeNNsim.
 * * Questo file definisce la classe Rete, il motore centrale del simulatore.
 * Si occupa di allocare i neuroni, gestire le connessioni sinaptiche e calcolare
 * l'integrazione numerica dell'intero sistema ad ogni step temporale.
 */

#ifndef RETE_HPP
#define RETE_HPP

#include "Neurone.hpp"
#include "Sinapsi.hpp"
#include "Popolazione.hpp"
#include "Utility.hpp"
#include <cstddef>
#include <vector>
#include <optional>
#include <deque>

/**
 * @ingroup publicapi
 * @brief Classe principale per la costruzione e simulazione della rete neurale.
 *
 * @details
 * L'utente interagisce con `Rete` esclusivamente attraverso i metodi pubblici per:
 * - Aggiungere popolazioni di neuroni (LIF, Exp).
 * - Connettere i neuroni tramite sinapsi (Current-based, Conductance-based).
 * - Modificare i parametri in corso d'opera tramite i pacchetti "Patch".
 * * @note L'evoluzione temporale (`step()`) e la preparazione (`prepare()`)
 * sono gestite internamente dalla classe `Simulazione` (friend class).
 */
class Rete {

    friend class Simulazione;

  public:
    /**
     * @brief Costruttore default : crea una rete vuota
     */
    Rete() = default;

    ~Rete() = default;

    // -- GESTIONE SINGOLO NEURONE (API PUBBLICA) ---------------------------------------------------------------------

    /**
     * @brief Inserisce un singolo neurone nella topologia della rete.
     * @param typeNeurone Modello del neurone (es. NeuronModel::Exp).
     * @param typeIntegratore Metodo di risoluzione ODE ('E' o 'R').
     */
    std::size_t aggiungiNeurone(NeuronModel typeNeurone, char typeIntegratore);

    /**
     * @brief Modifica a runtime il risolutore numerico di un neurone specifico.
     * @param idx Identificativo del neurone bersaglio.
     * @param typeIntegratore Nuovo metodo ('E' o 'R').
     */
    void modificaIntegratoreNeurone(size_t idx, char typeIntegratore);

    /**
     * @brief Inietta una modifica parziale (Patch) nei parametri di un neurone.
     * @param idx Identificativo del neurone bersaglio.
     * @param patch Struttura dati contenente solo i parametri da sovrascrivere.
     */
    void modificaParametriNeurone(size_t idx, const TypePatchNeuron& patch);

    /**
     * @brief Randomizza i parametri di un neurone secondo una distribuzione statistica.
     * @param idx Identificativo del neurone bersaglio.
     * @param patch Struttura dati contenente i parametri da randomizzare.
     * @param tipoDist Tipo di distribuzione (Gaussian, Uniform, LogNormal).
     * @param dispersione Ampiezza della dispersione relativa (es. 0.1 = ±10%).
     */
    void randomizzaParametriNeurone(size_t idx, const TypePatchNeuron& patch, DistType tipoDist, double dispersione);

    // -- GESTIONE POPOLAZIONI (API PUBBLICA) -----------------------------------------------------------------

    /**
     * @brief Aggiunge una popolazione di neuroni alla rete
     *
     * @param indexStart Indice di partenza della popolazione di neuroni
     * @param size Numero di neuroni nella popolazione
     * @param typeNeuron Tipologia di neuroni all'interno della popolazione
     * @param typeIntegratore metodo di integrazione
     * @param config configurazione dei parametri dei neuroni della popolazione
     * @return Popolazione
     */
    Popolazione& addPopulation(size_t size, NeuronModel typeNeuron = NeuronModel::LIF, char typeIntegratore = 'E',
                               std::optional<TypePatchNeuron> config = std::nullopt);

    /**
     * @brief Modifica i parametri dei neuroni della popolazione.
     * @param patch Struttura dati contenente i parametri da modificare.
     * @details Per ogni parametro specificato in `patch`, il valore viene applicato
     *          a tutti i neuroni della popolazione.
     */
    void modificaParametriPopolazione(size_t indicePopolazione, const TypePatchNeuron& patch);

    /**
     * @brief Randomizza i parametri dei neuroni della popolazione.
     * @param indicePopolazione Indice della popolazione da modificare.
     * @param patch Struttura dati contenente i parametri da randomizzare.
     * @param distribuzione Distribuzione statistica da utilizzare per la randomizzazione.
     * @details Per ogni parametro specificato in `patch`, viene generato un valore casuale
     *          secondo la distribuzione specificata e applicato a tutti i neuroni della popolazione.
     */
    void randomizzaParametriPopolazione(size_t indicePopolazione, const TypePatchNeuron& patch, DistType distribuzione,
                                        double dispersione);

    /**
     * @brief Aggiunge una popolazione eterogenea di neuroni alla rete.
     * @param size Numero totale di neuroni nella popolazione.
     * @param typesNeuroni Vettore contenente i tipi di neuroni da inserire.
     * @param probabilita Vettore contenente le probabilità di ciascun tipo di neurone.
     * @param typesIntegratori Vettore contenente i tipi di integratori per ciascun tipo di neurone.
     * @param configs (opzionale) Vettore di configurazioni (Patch) per ciascun tipo di neurone.
     * @return Vettore contenente gli indici di partenza delle sub-popolazioni create.
     * @details
     * La funzione divide la popolazione in sub-popolazioni omogenee secondo le
     * probabilità specificate. Ogni sub-popolazione viene creata con il tipo
     * di neurone e integratore corrispondente, e opzionalmente con la configurazione specificata.
     * Se `configs` non è fornito, i neuroni vengono creati con i parametri di default.
     * @note Le dimensioni dei vettori `typesNeuroni`, `probabilita`, `typesIntegratori` e `configs` devono essere
     * coerenti.
     * @warning La somma delle probabilità deve essere 1.0. In caso contrario, le probabilità verranno normalizzate
     * automaticamente.
     * @warning Se la dimensione della popolazione è 0 o se i vettori di input sono vuoti, la funzione restituirà un
     * vettore vuoto e stamperà un messaggio di errore.
     */
    Popolazione& addPopolazioneEterogenea(size_t size, std::vector<NeuronModel> typesNeuroni,
                                          std::vector<double> probabilita, std::vector<char> typesIntegratori,
                                          std::optional<std::vector<TypePatchNeuron>> configs = std::nullopt);

    // -- GESTIONE CONNESSIONI POPOLAZIONI ---------------------------------------------------------------------

    // -- GESTIONE SINAPSI (API PUBBLICA) ----------------------------------------------------------------------

    /**
     * @brief Crea una connessione unidirezionale tra due neuroni.
     * @param IDXpre indice del neurone sorgente (pre-sinaptico).
     * @param IDXpost indice del neurone bersaglio (post-sinaptico).
     * @param typeSynapse Modello matematico della sinapsi.
     * @return L'ID univoco assegnato automaticamente alla nuova sinapsi, o -1 in caso di errore.
     */
    int connettiNeuroni(size_t IDXpre, size_t IDXpost, SynapseModel typeSynapse);

    /**
     * @brief Aggiorna i parametri di una sinapsi esistente.
     * @param indexSyn Indice univoco della sinapsi (restituito da connettiNeuroni).
     * @param patch Struttura dati contenente le modifiche da applicare.
     * @warning Deve essere chiamata prima dell'avvio della simulazione (prima di prepare()).
     */
    void modificaSinapsi(size_t indexSyn, const TypePatchSyn& patch);

    /**
     * @brief Ricerca tutte le sinapsi che collegano una specifica coppia di neuroni.
     * @param pre indice del neurone sorgente.
     * @param post indice del neurone bersaglio.
     * @return Vettore contenente gli ID delle sinapsi trovate.
     */
    std::vector<int> findSinapsi(size_t pre, size_t post) const;

  private:
    // -- ATTRIBUTI PRIVATI (Topologia e Stato) --------------------------------------------------------------

    std::vector<TypeNeuron> neuroni_;     // Lista dei neuroni
    std::deque<Popolazione> popolazioni_; // Lista delle popolazioni di neuroni
    std::vector<TypeSyn> sinapsi_;        // Lista delle sinapsi

    std::vector<double> stimoli_;     // Correnti esterne calcolate dalla simuazione
    std::vector<double> inputTotale_; // Somma stimoli + correnti sinaptiche

    std::vector<double> statoNeuroni_; // Potenziale di membrana di tutti i neuroni nella rete
    std::vector<double> statoFiring_;  // Stato dei neuroni di tutta la rete
    std::vector<double> statoSinapsi_; // Corrente sinaptica di tutte le sinapsi nella rete

    // -- METODI INTERNI -----------------------------------------------------------------------------------

    // 1. Metodi evoluzione della rete e stato della rete
    void step(double dt);
    void aggiornaStatoRete();

    // 2. Metodi applicativi
    double getMinTau() const;
    void prepare(double dt);
    size_t allocazioneNeuroni(size_t size, NeuronModel type, char integratore,
                              std::optional<TypePatchNeuron> config = {});

    // 3. Metodi getter
    const std::vector<double>& getPointerStatoNeuroni() const { return statoNeuroni_; };
    const std::vector<double>& getPointerStatoFiring() const { return statoFiring_; };
    const std::vector<double>& getPointerStatoSinapsi() const { return statoSinapsi_; };
    int32_t getNumNeuroni() const { return neuroni_.size(); };
    int32_t getNumSinapsi() const { return sinapsi_.size(); };
    // size_t getIndex(int id) const { return idToIndex_.at(id); }

    // 4. Metodi di controllo
    bool hasNeurone(size_t idx) const { return idx < neuroni_.size(); } // vero --> neurone esiste
    bool hasSinapsi(size_t idx) const { return idx < sinapsi_.size(); } // vero --> sinapsi esiste
};

#endif // RETE_HPP