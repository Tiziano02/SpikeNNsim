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
#include <cstddef>
#include <unordered_map>
#include <vector>

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
     * @brief Costruttore principale: alloca e inizializza una popolazione omogenea.
     * @param N Numero di neuroni da creare.
     * @param typeNeurone Modello fisico dei neuroni (es. NeuronModel::LIF).
     * @param typeintegratore Metodo numerico: 'E' (Eulero) o 'R' (Runge-Kutta 4). Default 'E'.
     */
    Rete(int N, NeuronModel typeNeurone, char typeintegratore = 'E');

    ~Rete() = default;

    // -- GESTIONE NEURONI (API PUBBLICA) ---------------------------------------------------------------------

    /**
     * @brief Inserisce un singolo neurone nella topologia della rete.
     * @param ID Identificativo univoco definito dall'utente.
     * @param typeNeurone Modello del neurone (es. NeuronModel::Exp).
     * @param typeIntegratore Metodo di risoluzione ODE ('E' o 'R').
     */
    void aggiungiNeurone(int ID, NeuronModel typeNeurone, char typeIntegratore);

    /**
     * @brief Modifica a runtime il risolutore numerico di un neurone specifico.
     * @param ID Identificativo del neurone bersaglio.
     * @param typeIntegratore Nuovo metodo ('E' o 'R').
     */
    void modificaIntegratoreNeurone(int ID, char typeIntegratore);

    /**
     * @brief Inietta una modifica parziale (Patch) nei parametri di un neurone.
     * @param id Identificativo del neurone bersaglio.
     * @param patch Struttura dati contenente solo i parametri da sovrascrivere.
     */
    void modificaParametriNeurone(int id, const TypePatchNeuron& patch);

    // -- GESTIONE SINAPSI (API PUBBLICA) ----------------------------------------------------------------------

    /**
     * @brief Crea una connessione unidirezionale tra due neuroni.
     * @param IDpre ID del neurone sorgente (pre-sinaptico).
     * @param IDpost ID del neurone bersaglio (post-sinaptico).
     * @param typeSynapse Modello matematico della sinapsi.
     * @return L'ID univoco assegnato automaticamente alla nuova sinapsi, o -1 in caso di errore.
     */
    int connettiNeuroni(int IDpre, int IDpost, SynapseModel typeSynapse);

    /**
     * @brief Aggiorna i parametri di una sinapsi esistente.
     * @param IDsin ID univoco della sinapsi (restituito da connettiNeuroni).
     * @param patch Struttura dati contenente le modifiche da applicare.
     * @warning Deve essere chiamata prima dell'avvio della simulazione (prima di prepare()).
     */
    void modificaSinapsi(int IDsin, const TypePatchSyn& patch);

    /**
     * @brief Ricerca tutte le sinapsi che collegano una specifica coppia di neuroni.
     * @param pre ID del neurone sorgente.
     * @param post ID del neurone bersaglio.
     * @return Vettore contenente gli ID delle sinapsi trovate.
     */
    std::vector<int> findSinapsi(int pre, int post) const;

  private:
    // -- ATTRIBUTI PRIVATI (Topologia e Stato) --------------------------------------------------------------

    std::vector<TypeNeuron> neuroni_; // Lista dei neuroni
    std::vector<TypeSyn> sinapsi_;    // Lista delle sinapsi

    std::unordered_map<int, size_t> idToIndex_;    // Mappa: ID Utente -> Indice Vettore Neuroni
    std::unordered_map<int, size_t> idToIndexSyn_; // Mappa: ID Generato -> Indice Vettore Sinapsi
    int prossimoIdSyn_ = 0;                        // Contatore auto-incrementante per gli ID sinapsi

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

    // 3. Metodi getter
    const std::vector<double>& getPointerStatoNeuroni() const { return statoNeuroni_; }
    const std::vector<double>& getPointerStatoFiring() const { return statoFiring_; }
    const std::vector<double>& getPointerStatoSinapsi() const { return statoSinapsi_; }
    int32_t getNumNeuroni() const { return neuroni_.size(); }
    int32_t getNumSinapsi() const { return sinapsi_.size(); }
    size_t getIndex(int id) const { return idToIndex_.at(id); }

    // 4. Metodi di controllo
    bool hasNeurone(int id) const { return idToIndex_.count(id) > 0; }
    bool hasSinapsi(int id) const { return idToIndexSyn_.count(id) > 0; }
};

#endif // RETE_HPP