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
#include <string>
// #include <stdexcept>

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
     * @param typeIntegratore Metodo di risoluzione ODE ('A' = Analitico esatto per LIF, 'E' = Eulero, 'R' = Runge-Kutta
     * 4).
     * @param config (opzionale) Configurazione (Patch) iniziale per il neurone.
     * @param dispersione Ampiezza della dispersione relativa della distribuzione (default 0.0).
     * @param tipoDist Tipo di distribuzione per la randomizzazione (default Gaussian).
     * @return Indice univoco assegnato al nuovo neurone.
     */
    std::size_t aggiungiNeurone(NeuronModel typeNeurone, char typeIntegratore = 'E',
                                std::optional<TypePatchNeuron> config = std::nullopt, double dispersione = 0.0,
                                DistType tipoDist = DistType::Gaussian);

    /**
     * @brief Modifica a runtime il risolutore numerico di un neurone specifico.
     * @param idx Identificativo del neurone bersaglio.
     * @param typeIntegratore Nuovo metodo ('A' = Analitico esatto, 'E' = Eulero, 'R' = Runge-Kutta 4).
     */
    void modificaIntegratoreNeurone(size_t idx, char typeIntegratore);

    /**
     * @brief Modifica il risolutore numerico di una lista di neuroni.
     * @param idxs Vettore contenente gli indici dei neuroni bersaglio.
     * @param typeIntegratore Nuovo metodo ('A' = Analitico esatto, 'E' = Eulero, 'R' = Runge-Kutta 4).
     */
    void modificaIntegratoreNeuroni(const std::vector<size_t>& idxs, char typeIntegratore);

    /**
     * @brief Modifica i parametri di un neurone attraverso una patch.
     * @param idx Identificativo del neurone bersaglio.
     * @param patch Struttura dati contenente i parametri da randomizzare.
     * @param tipoDist Tipo di distribuzione per la randomizzazione di default ho Gaussian(Gaussian, Uniform,
     * LogNormal).
     * @param dispersione Ampiezza della dispersione relativa della distribuzione se nulla si ha una distribuzione Delta
     * (default)(es. 0.1 = ±10%).
     */
    void modificaParametriNeurone(size_t idx, const TypePatchNeuron& patch, double dispersione = 0.0,
                                  DistType tipoDist = DistType::Gaussian);

    /**
     * @brief Modifica i parametri di una lista di neuroni attraverso una patch.
     * @param idxs Vettore contenente gli indici dei neuroni bersaglio.
     * @param patch Struttura dati contenente i parametri da randomizzare.
     * @param dispersione Ampiezza della dispersione relativa della distribuzione (default 0.0).
     * @param tipoDist Tipo di distribuzione per la randomizzazione (default Gaussian).
     */
    void modificaParametriNeuroni(const std::vector<size_t>& idxs, const TypePatchNeuron& patch,
                                  double dispersione = 0.0, DistType tipoDist = DistType::Gaussian);
    // -- GESTIONE POPOLAZIONI (API PUBBLICA) -----------------------------------------------------------------

    /**
     * @brief Aggiunge una popolazione di neuroni alla rete.
     * @param size Numero di neuroni nella popolazione.
     * @param typeNeuron Tipologia di neuroni all'interno della popolazione.
     * @param typeIntegratore Metodo di integrazione ('A' = Analitico esatto, 'E' = Eulero, 'R' = Runge-Kutta 4).
     * @param config (opzionale) Configurazione (Patch) iniziale per i neuroni della popolazione.
     * @param dispersione Ampiezza della dispersione relativa per la randomizzazione (default 0.0).
     * @param tipoDist Tipo di distribuzione per la randomizzazione (default Gaussian).
     * @return Indice univoco assegnato alla nuova popolazione.
     */
    size_t addPopolazione(size_t size, NeuronModel typeNeuron = NeuronModel::LIF, char typeIntegratore = 'E',
                          std::optional<TypePatchNeuron> config = std::nullopt, double dispersione = 0.0,
                          DistType tipoDist = DistType::Gaussian) {
        return addPopulation(size, typeNeuron, typeIntegratore, config, dispersione, tipoDist);
    }

    // Alias per retrocompatibilità
    size_t addPopulation(size_t size, NeuronModel typeNeuron = NeuronModel::LIF, char typeIntegratore = 'E',
                         std::optional<TypePatchNeuron> config = std::nullopt, double dispersione = 0.0,
                         DistType tipoDist = DistType::Gaussian);

    /**
     * @brief Modifica i parametri dei neuroni della popolazione.
     * @param indicePopolazione Indice della popolazione da modificare.
     * @param patch Struttura dati contenente i parametri da randomizzare.
     * @param distribuzione Distribuzione statistica da utilizzare per la randomizzazione.
     * @param dispersione  Valore di dispersione per la randomizzazione, se è zero (default) inseriesce solo il valore
     * nella patch
     * @details Per ogni parametro specificato in `patch`, viene generato un valore casuale
     *          secondo la distribuzione specificata e applicato a tutti i neuroni della popolazione.
     */
    void modificaParametriPopolazione(size_t indicePopolazione, const TypePatchNeuron& patch, double dispersione = 0.0,
                                      DistType distribuzione = DistType::Gaussian);

    /**
     * @brief Modifica il risolutore numerico di tutti i neuroni di una popolazione.
     * @param indicePopolazione Indice della popolazione da modificare.
     * @param typeIntegratore Nuovo metodo ('A' = Analitico esatto, 'E' = Eulero, 'R' = Runge-Kutta 4).
     */
    void modificaIntegratorePopolazione(size_t indicePopolazione, char typeIntegratore);

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
    size_t addPopolazioneEterogenea(size_t size, std::vector<NeuronModel>& typesNeuroni,
                                    std::vector<double>& probabilita, std::vector<char>& typesIntegratori,
                                    std::optional<std::vector<TypePatchNeuron>> configs = std::nullopt);

    /**
     * @brief Modifica i parametri di una sub-popolazione.
     * @param indicePopolazione Indice della popolazione contenente la sub-popolazione.
     * @param indiceSubPopolazione Indice della sub-popolazione da randomizzare.
     * @param patch Struttura dati contenente le modifiche da applicare.
     * @param distribuzione Tipo di distribuzione per la randomizzazione.
     * @param dispersione Valore di dispersione per la randomizzazione, se è zero (default) inseriesce solo il valore
     * nella patch
     */
    void modificaParametriSubPopolazione(size_t indicePopolazione, size_t indiceSubPopolazione,
                                         const TypePatchNeuron& patch, double dispersione = 0.0,
                                         DistType distribuzione = DistType::Gaussian);

    /**
     * @brief Modifica il risolutore numerico di una specifica sub-popolazione.
     * @param indicePopolazione Indice della popolazione contenente la sub-popolazione.
     * @param indiceSubPopolazione Indice della sub-popolazione da modificare.
     * @param typeIntegratore Nuovo metodo ('A' = Analitico esatto, 'E' = Eulero, 'R' = Runge-Kutta 4).
     */
    void modificaIntegratoreSubPopolazione(size_t indicePopolazione, size_t indiceSubPopolazione, char typeIntegratore);

    // -- CONDIZIONI INIZIALI POTENZIALI (SYNTACTIC SUGAR) ---------------------------------------------------

    /**
     * @brief Imposta la condizione iniziale del potenziale di membrana V(0) su tutta la rete.
     * @param V Valore del potenziale di membrana (in Volt).
     * @param dispersione Ampiezza della dispersione relativa (default 0.0). Se 0.0 la condizione e' deterministica.
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     */
    void setCondizioneInizialeRete(double V, double dispersione = 0.0, DistType tipoDist = DistType::Gaussian);

    /**
     * @brief Imposta la condizione iniziale del potenziale di membrana V(0) per una specifica popolazione.
     * @param indicePopolazione Indice della popolazione bersaglio.
     * @param V Valore del potenziale di membrana (in Volt).
     * @param dispersione Ampiezza della dispersione relativa (default 0.0). Se 0.0 la condizione e' deterministica.
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     */
    void setCondizioneInizialePopolazione(size_t indicePopolazione, double V, double dispersione = 0.0,
                                          DistType tipoDist = DistType::Gaussian);

    /**
     * @brief Imposta la condizione iniziale del potenziale di membrana V(0) su un intervallo contiguo [start, end).
     * @param start Indice del primo neurone dell'intervallo.
     * @param end Indice successivo all'ultimo neurone dell'intervallo (escluso).
     * @param V Valore del potenziale di membrana (in Volt).
     * @param dispersione Ampiezza della dispersione relativa (default 0.0). Se 0.0 la condizione e' deterministica.
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     */
    void setCondizioneInizialeIntervallo(size_t start, size_t end, double V, double dispersione = 0.0,
                                         DistType tipoDist = DistType::Gaussian);

    // -- GESTIONE CONNESSIONI POPOLAZIONI ---------------------------------------------------------------------

    /**
     * @brief Crea connessioni casuali tra tutti i neuroni di una popolazione.
     * @param indicePopolazione Indice della popolazione da connettere.
     * @param probabilitaConnessione Probabilità di connessione tra due neuroni.
     * @param typeSynapse Modello matematico della sinapsi da utilizzare.
     * @param typeIntegratore Metodo di integrazione ('A' = Analitico esatto, 'E' = Eulero, 'R' = Runge-Kutta 4).
     * @param config (opzionale) Configurazione (Patch) iniziale da applicare alle sinapsi create.
     * @param dispersione Ampiezza della dispersione relativa per la randomizzazione dei parametri (default 0.0).
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     * @return Vettore contenente gli ID di tutte le sinapsi create.
     */
    std::vector<size_t> connettiPopolazione(size_t indicePopolazione, double probabilitaConnessione,
                                            SynapseModel typeSynapse, char typeIntegratore = 'E',
                                            std::optional<TypePatchSyn> config = std::nullopt, double dispersione = 0.0,
                                            DistType tipoDist = DistType::Gaussian);

    // Overload per retrocompatibilità quando config viene passata direttamente
    std::vector<size_t> connettiPopolazione(size_t indicePopolazione, double probabilitaConnessione,
                                            SynapseModel typeSynapse, const TypePatchSyn& config,
                                            double dispersione = 0.0, DistType tipoDist = DistType::Gaussian) {
        return connettiPopolazione(indicePopolazione, probabilitaConnessione, typeSynapse, 'E', config, dispersione,
                                   tipoDist);
    }

    /**
     * @brief Modifica i parametri di tutte le sinapsi interne (ricorrenti) di una popolazione.
     * @param indicePopolazione Indice della popolazione bersaglio.
     * @param patch Struttura dati contenente le modifiche da applicare.
     * @param dispersione Ampiezza della dispersione relativa (default 0.0).
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     */
    void modificaSinapsiPopolazione(size_t indicePopolazione, const TypePatchSyn& patch, double dispersione = 0.0,
                                    DistType tipoDist = DistType::Gaussian);

    /**
     * @brief Modifica il risolutore di tutte le sinapsi interne (ricorrenti) di una popolazione.
     * @param indicePopolazione Indice della popolazione bersaglio.
     * @param typeIntegratore Nuovo metodo ('A' = Analitico esatto, 'E' = Eulero in avanti, 'R' = Runge-Kutta 4).
     */
    void modificaIntegratoreSinapsiPopolazione(size_t indicePopolazione, char typeIntegratore);

    /**
     * @brief Modifica il risolutore di tutte le sinapsi interne di una sub-popolazione.
     * @param indicePopolazione Indice della popolazione contenente la sub-popolazione.
     * @param indiceSubPopolazione Indice della sub-popolazione bersaglio.
     * @param typeIntegratore Nuovo metodo ('A' = Analitico esatto, 'E' = Eulero in avanti, 'R' = Runge-Kutta 4).
     */
    void modificaIntegratoreSinapsiSubPopolazione(size_t indicePopolazione, size_t indiceSubPopolazione,
                                                  char typeIntegratore);

    /**
     * @brief Imposta la modalita' a delta (senza decadimento esponenziale) per tutte le sinapsi di una popolazione.
     * @param indicePopolazione Indice della popolazione bersaglio.
     * @param isDelta Se true, le sinapsi erogano impulsi istantanei a delta.
     */
    void modificaDeltaSinapsiPopolazione(size_t indicePopolazione, bool isDelta);

    /**
     * @brief Imposta la modalita' a delta per tutte le sinapsi di una sub-popolazione.
     * @param indicePopolazione Indice della popolazione contenente la sub-popolazione.
     * @param indiceSubPopolazione Indice della sub-popolazione bersaglio.
     * @param isDelta Se true, le sinapsi erogano impulsi istantanei a delta.
     */
    void modificaDeltaSinapsiSubPopolazione(size_t indicePopolazione, size_t indiceSubPopolazione, bool isDelta);

    /**
     * @brief Crea connessioni casuali tra i neuroni di due popolazioni distinte.
     * @param indicePopPre Indice della popolazione sorgente (presinaptica).
     * @param indicePopPost Indice della popolazione bersaglio (postsinaptica).
     * @param probabilitaConnessione Probabilità di connessione tra due neuroni.
     * @param typeSynapse Modello matematico della sinapsi da utilizzare.
     * @param typeIntegratore Metodo di integrazione ('A' = Analitico esatto, 'E' = Eulero, 'R' = Runge-Kutta 4).
     * @param config (opzionale) Configurazione (Patch) iniziale da applicare alle sinapsi create.
     * @param dispersione Ampiezza della dispersione relativa per la randomizzazione dei parametri (default 0.0).
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     * @return Vettore contenente gli ID di tutte le sinapsi create.
     */
    std::vector<size_t> connettiPopolazioni(size_t indicePopPre, size_t indicePopPost, double probabilitaConnessione,
                                            SynapseModel typeSynapse, char typeIntegratore = 'E',
                                            std::optional<TypePatchSyn> config = std::nullopt, double dispersione = 0.0,
                                            DistType tipoDist = DistType::Gaussian);

    // Overload per retrocompatibilità quando config viene passata direttamente
    std::vector<size_t> connettiPopolazioni(size_t indicePopPre, size_t indicePopPost, double probabilitaConnessione,
                                            SynapseModel typeSynapse, const TypePatchSyn& config,
                                            double dispersione = 0.0, DistType tipoDist = DistType::Gaussian) {
        return connettiPopolazioni(indicePopPre, indicePopPost, probabilitaConnessione, typeSynapse, 'E', config,
                                   dispersione, tipoDist);
    }

    // -- GESTIONE SINAPSI (API PUBBLICA) ----------------------------------------------------------------------

    /**
     * @brief Crea una connessione unidirezionale tra due neuroni.
     * @param IDXpre indice del neurone sorgente (pre-sinaptico).
     * @param IDXpost indice del neurone bersaglio (post-sinaptico).
     * @param typeSynapse Modello matematico della sinapsi.
     * @param typeIntegratore Metodo di risoluzione ODE ('A' = Analitico esatto, 'E' = Eulero in avanti, 'R' =
     * Runge-Kutta 4).
     * @param config (opzionale) Configurazione (Patch) iniziale per la sinapsi.
     * @param dispersione Ampiezza della dispersione relativa per la randomizzazione dei parametri (default 0.0).
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     * @return L'ID univoco assegnato automaticamente alla nuova sinapsi, o static_cast<size_t>(-1) in caso di errore.
     */
    size_t connettiNeuroni(size_t IDXpre, size_t IDXpost, SynapseModel typeSynapse, char typeIntegratore = 'E',
                           std::optional<TypePatchSyn> config = std::nullopt, double dispersione = 0.0,
                           DistType tipoDist = DistType::Gaussian);

    // Overload per retrocompatibilità quando config viene passata direttamente
    size_t connettiNeuroni(size_t IDXpre, size_t IDXpost, SynapseModel typeSynapse, const TypePatchSyn& config,
                           double dispersione = 0.0, DistType tipoDist = DistType::Gaussian) {
        return connettiNeuroni(IDXpre, IDXpost, typeSynapse, 'E', config, dispersione, tipoDist);
    }

    /**
     * @brief Modifica a runtime il risolutore numerico/analitico di una specifica sinapsi.
     * @param idx Identificativo della sinapsi bersaglio.
     * @param typeIntegratore Nuovo metodo ('A' = Analitico esatto, 'E' = Eulero in avanti, 'R' = Runge-Kutta 4).
     */
    void modificaIntegratoreSinapsi(size_t idx, char typeIntegratore);

    /**
     * @brief Modifica il risolutore numerico/analitico di una lista di sinapsi.
     * @param idxs Vettore contenente gli indici delle sinapsi da modificare.
     * @param typeIntegratore Nuovo metodo ('A' = Analitico esatto, 'E' = Eulero in avanti, 'R' = Runge-Kutta 4).
     */
    void modificaIntegratoreSinapsi(const std::vector<size_t>& idxs, char typeIntegratore);

    /**
     * @brief Imposta la modalita' a delta (senza decadimento esponenziale) per una specifica sinapsi.
     * @param idx Identificativo della sinapsi bersaglio.
     * @param isDelta Se true, la sinapsi eroga un impulso istantaneo a delta.
     */
    void modificaDeltaSinapsi(size_t idx, bool isDelta);

    /**
     * @brief Imposta la modalita' a delta per una lista di sinapsi.
     * @param idxs Vettore contenente gli indici delle sinapsi da modificare.
     * @param isDelta Se true, le sinapsi erogano impulsi istantanei a delta.
     */
    void modificaDeltaSinapsi(const std::vector<size_t>& idxs, bool isDelta);

    /**
     * @brief Aggiorna i parametri di una sinapsi esistente.
     * @param indexSyn Indice univoco della sinapsi (restituito da connettiNeuroni).
     * @param patch Struttura dati contenente le modifiche da applicare.
     * @param dispersione Ampiezza della dispersione relativa (default 0.0).
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     * @warning Deve essere chiamata prima dell'avvio della simulazione (prima di prepare()).
     */
    void modificaSinapsi(size_t indexSyn, const TypePatchSyn& patch, double dispersione = 0.0,
                         DistType tipoDist = DistType::Gaussian);

    /**
     * @brief Modifica i parametri di una lista di sinapsi attraverso una patch.
     * @param idxs Vettore contenente gli indici delle sinapsi da modificare.
     * @param patch Struttura dati contenente le modifiche da applicare.
     * @param dispersione Ampiezza della dispersione relativa (default 0.0).
     * @param tipoDist Tipo di distribuzione statistica per la randomizzazione (default Gaussian).
     */
    void modificaSinapsi(const std::vector<size_t>& idxs, const TypePatchSyn& patch, double dispersione = 0.0,
                         DistType tipoDist = DistType::Gaussian);

    /**
     * @brief Ricerca tutte le sinapsi che collegano una specifica coppia di neuroni.
     * @param pre indice del neurone sorgente.
     * @param post indice del neurone bersaglio.
     * @return Vettore contenente gli ID delle sinapsi trovate.
     */
    std::vector<size_t> findSinapsi(size_t pre, size_t post) const;

    // -- METODI DI ISPEZIONE (API PUBBLICA) -------------------------------------------------------------------

    // 1. metodi getter
    size_t getNumNeuroni() const { return neuroni_.size(); }
    size_t getNumSinapsi() const { return sinapsi_.size(); }
    size_t getNumPopolazioni() const { return popolazioni_.size(); }
    double getResistenzaNeurone(size_t idx) const;
    double getCapacitaNeurone(size_t idx) const;
    double getSogliaNeurone(size_t idx) const;
    double getValoreSogliaMin(size_t idx) const;
    double getPotenzialeNeurone(size_t idx) const;
    double getPesoSinapsi(size_t idx) const;
    double getDelaySinapsi(size_t idx) const;
    double getTauSinapsi(size_t idx) const;
    char getIntegratoreNeurone(size_t idx) const;
    char getIntegratoreSinapsi(size_t idx) const;
    bool isSinapsiDelta(size_t idx) const;

    // 2. metodi controllo
    bool hasNeurone(size_t idx) const { return idx < neuroni_.size(); }
    bool hasSinapsi(size_t idx) const { return idx < sinapsi_.size(); }

    // -- CONTROLLO STATO E CONGELAMENTO RETE -----------------------------------------------------------------

    /**
     * @brief Blocca (congela) la topologia e i parametri della rete.
     * @details Una volta bloccata la rete, qualsiasi tentativo successivo di aggiungere neuroni,
     *          sinapsi, popolazioni o modificare parametri statici solleva un'eccezione `std::logic_error`.
     *          La chiamata a questo metodo e' obbligatoria prima di creare e avviare la `Simulazione`.
     */
    void bloccaRete();

    /// Alias per brevita' sintattica
    void blocca() { bloccaRete(); }

    /**
     * @brief Sblocca la rete consentendo nuovamente modifiche a topologia e parametri.
     */
    void sbloccaRete();

    /// Alias per brevita' sintattica
    void sblocca() { sbloccaRete(); }

    /**
     * @brief Verifica se la rete e' attualmente bloccata/congelata.
     * @return true se la rete e' bloccata, false altrimenti.
     */
    bool isBloccata() const { return isBloccata_; }

  private:
    // -- ATTRIBUTI PRIVATI (Topologia e Stato) --------------------------------------------------------------

    bool isBloccata_ = false; // Flag che indica se la rete e' bloccata/congelata

    std::vector<TypeNeuron> neuroni_;     // Lista dei neuroni
    std::deque<Popolazione> popolazioni_; // Lista delle popolazioni di neuroni
    std::vector<TypeSyn> sinapsi_;        // Lista delle sinapsi
    std::vector<size_t> sinapsiAttive_;   // Lista di indici delle sinapsi attive (corrente superiore a epsilon)

    std::vector<double> stimoli_;     // Correnti esterne calcolate dalla simuazione
    std::vector<double> inputTotale_; // Somma stimoli + correnti sinaptiche

    std::vector<double> statoNeuroni_; // Potenziale di membrana di tutti i neuroni nella rete
    std::vector<double> statoFiring_;  // Stato dei neuroni di tutta la rete
    // std::vector<double> statoSinapsi_; // Corrente sinaptica di tutte le sinapsi nella rete

    // -- METODI INTERNI -----------------------------------------------------------------------------------

    // 1. Metodi evoluzione della rete e stato della rete
    void step(double dt);
    void aggiornaStatoRete();

    // 2. Metodi applicativi
    double getMinTau() const;
    void prepare(double dt);
    void verificaNonBloccata(const std::string& operazione) const;
    size_t allocazioneNeuroni(size_t size, NeuronModel type, char integratore,
                              std::optional<TypePatchNeuron> config = std::nullopt, double dispersione = 0.0,
                              DistType tipoDist = DistType::Gaussian);

    // 3. Metodi getter interni
    const std::vector<double>& getPointerStatoNeuroni() const { return statoNeuroni_; };
    const std::vector<double>& getPointerStatoFiring() const { return statoFiring_; };
    // const std::vector<double>& getPointerStatoSinapsi() const { return statoSinapsi_; };
};

#endif // RETE_HPP