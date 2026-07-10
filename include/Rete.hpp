#ifndef RETE_HPP
#define RETE_HPP

#include "Neurone.hpp"
#include "Sinapsi.hpp"
#include <algorithm>
#include <cstddef>
#include <unordered_map>
#include <vector>

// ============================================================================
// CLASSE Rete (PUBBLICA)
// ============================================================================

/**
 * Rete – contenitore della topologia e motore di evoluzione della rete neurale.
 *
 * La rete è composta da:
 *   - un vettore di neuroni (TypeNeuron)
 *   - un vettore di sinapsi (TypeSyn)
 *   - mappe per accesso rapido ID → indice
 *   - buffer pre‑allocati per correnti e stato
 *
 * L'utente interagisce con Rete attraverso i metodi pubblici per aggiungere
 * neuroni, connetterli, modificare parametri e avviare la simulazione
 * (tramite la classe Simulazione).
 *
 * ============================================================================
 * COSTRUTTORE
 * ============================================================================
 * Rete(N, neuronModel, integratore)
 *   - Crea N neuroni del tipo specificato (LIF o Exp) con parametri di default.
 *   - L'integratore può essere 'E' (Eulero) o 'R' (Runge‑Kutta 4).
 *
 * ============================================================================
 * METODI PUBBLICI – NEURONI
 * ============================================================================
 * aggiungiNeurone(id, integratore, config)
 *   - Aggiunge un neurone con ID unico.
 *   - La config deve essere dello stesso tipo del modello scelto.
 *
 * modificaIntegratoreNeurone(id, integratore)
 *   - Cambia il metodo di integrazione di un neurone esistente.
 *
 * modificaParametriNeurone(id, config)
 *   - Aggiorna i parametri di un neurone esistente.
 *   - La config deve corrispondere al tipo del neurone.
 *
 * ============================================================================
 * METODI PUBBLICI – SINAPSI
 * ============================================================================
 * connettiNeuroni(IDpre, IDpost, configSyn) -> int
 *   - Crea una sinapsi tra due neuroni e restituisce un ID univoco.
 *   - Supporta sinapsi multiple tra la stessa coppia.
 *
 * modificaSinapsi(IDsin, configSyn)
 *   - Modifica i parametri di una sinapsi esistente.
 *   - Deve essere chiamata prima di prepare() (ovvero prima di avviare la simulazione).
 *
 * getSinapsiIds(pre, post) -> vector<int>
 *   - Restituisce tutti gli ID delle sinapsi tra due neuroni.
 *
 * ============================================================================
 * METODI INTERNI (accessibili solo a Simulazione)
 * ============================================================================
 * step(dt)             – avanza la rete di un passo
 * prepare(dt)          – prepara la rete (ring buffer per i delay)
 * aggiornaStatoRete()  – sincronizza i vettori di stato
 * getPointerStato...() – puntatori ai vettori di stato per l'output
 * hasNeurone(id)       – verifica esistenza neurone
 * hasSinapsi(id)       – verifica esistenza sinapsi
 */
class Rete {
  private:
    // ── ATTRIBUTI PRIVATI ──────────────────────────────────────────────────

    // Topologia
    std::vector<TypeNeuron> neuroni_;
    std::vector<TypeSyn> sinapsi_;
    std::unordered_map<int, size_t> idToIndex_;    // ID neurone → indice
    std::unordered_map<int, size_t> idToIndexSyn_; // ID sinapsi → indice
    int prossimoIdSyn_ = 0;

    // Correnti e stimoli
    std::vector<double> stimoli_;
    std::vector<double> inputTotale_;

    // Stati (per output)
    std::vector<double> statoNeuroni_;
    std::vector<double> statoFiring_;
    std::vector<double> statoSinapsi_;

    // ── METODI PRIVATI ──────────────────────────────────────────────────────

    // Gestione stimoli
    void resetStimoli() { std::fill(stimoli_.begin(), stimoli_.end(), 0.0); }
    void addStimolo(size_t i, double value) { stimoli_[i] += value; }

    // Getter per lo stato (usati da Simulazione)
    const std::vector<double>& getPointerStatoNeuroni() const { return statoNeuroni_; }
    const std::vector<double>& getPointerStatoFiring() const { return statoFiring_; }
    const std::vector<double>& getPointerStatoSinapsi() const { return statoSinapsi_; }

    size_t getNumNeuroni() const { return neuroni_.size(); }
    size_t getNumSinapsi() const { return sinapsi_.size(); }
    size_t getIndex(int id) const { return idToIndex_.at(id); }

    // Controllo esistenza
    bool hasNeurone(int id) const { return idToIndex_.count(id) > 0; }
    bool hasSinapsi(int id) const { return idToIndexSyn_.count(id) > 0; }

    // Motore di evoluzione (chiamati da Simulazione)
    void step(double dt);
    void aggiornaStatoRete();
    void prepare(double dt);
    double getMinTau() const;

    // ── COSTRUTTORE E METODI PUBBLICI ─────────────────────────────────────

  public:
    /// Crea N neuroni del tipo specificato con parametri di default.
    ///
    /// @param N Il numero iniziale di neuroni.
    /// @param neuronModel Il modello da utilizzare (es. LIF o Exp).
    /// @param integratore 'E' (Eulero) o 'R' (Runge‑Kutta 4).
    Rete(int N, NeuronModel typeNeurone, char typeintegratore = 'E');

    // -------------------- Neuroni --------------------

    /// Aggiunge un nuovo neurone con un ID unico.
    ///
    /// @param id Identificativo univoco del neurone.
    /// @param integratore 'E' o 'R'.
    /// @param config I parametri (deve corrispondere al modello scelto).
    void aggiungiNeurone(int id, NeuronModel typeNeurone, char typeIntegratore);

    /**
     * Cambia il metodo di integrazione di un neurone.
     * @param ID              ID del neurone
     * @param typeIntegratore 'E' o 'R'
     */
    void modificaIntegratoreNeurone(int ID, char typeIntegratore);

    /**
     * Modifica i parametri di un neurone.
     * @param ID              ID del neurone
     * @param configurazione  configLIF o configExp (stesso tipo del neurone)
     */
    void modificaParametriNeurone(int ID, const TypePatchNeuron& patch);

    // -------------------- Sinapsi --------------------

    /**
     * Crea una sinapsi tra due neuroni.
     * @param IDpre                ID del neurone pre‑sinaptico
     * @param IDpost               ID del neurone post‑sinaptico
     * @param configurazioneSinapsi configCurrentSyn o configConductanceSyn
     * @return ID univoco della sinapsi, oppure -1 in caso di errore
     */
    int connettiNeuroni(int IDpre, int IDpost, SynapseModel typeSynapse);

    /**
     * Modifica i parametri di una sinapsi esistente.
     * @param IDsin                ID della sinapsi
     * @param configurazioneSinapsi nuova configurazione (stesso tipo dell'originale)
     * @warning Deve essere chiamata prima di prepare() (ovvero prima di avviare la simulazione).
     */
    void modificaSinapsi(int IDsin, const TypePatchSyn& patch);

    /**
     * Restituisce tutti gli ID delle sinapsi tra due neuroni.
     * @param pre  ID neurone pre
     * @param post ID neurone post
     * @return vettore di ID (può essere vuoto)
     */
    std::vector<int> getSinapsiIds(int pre, int post) const;

    ~Rete() = default;

    // Concede a Simulazione l'accesso ai metodi privati
    friend class Simulazione;
};

#endif // RETE_HPP