#ifndef LIF_HPP
#define LIF_HPP

#include "UnitaSI.hpp"
#include <optional>

/**
 * @ingroup publicapi
 * @brief Parametri modificabili per il neurone LIF (Leaky Integrate-and-Fire).
 *
 * @details
 * Utilizzata esclusivamente da Rete::modificaParametriNeurone().
 * Tutti i valori sono in unità SI (vedi UnitaSI.hpp).
 *
 * @note I campi sono opzionali: se non specificati, il valore corrente rimane invariato.
 */
struct patchLIF {
    std::optional<double> V;            ///< Potenziale di membrana iniziale [V]
    std::optional<double> Vth;          ///< Soglia di attivazione iniziale [V]
    std::optional<double> VthMin;       ///< Soglia minima (valore di riposo) [V]
    std::optional<double> VthMax;       ///< Soglia massima (subito dopo uno spike) [V]
    std::optional<double> Vrest;        ///< Potenziale di riposo [V]
    std::optional<double> Vreset;       ///< Potenziale dopo uno spike [V]
    std::optional<double> R;            ///< Resistenza di membrana [Ω]
    std::optional<double> C;            ///< Capacità di membrana [F] (τ = R * C)
    std::optional<double> timeAbsolute; ///< Durata refrattarietà assoluta [s]
    std::optional<double> timeRelative; ///< Durata refrattarietà relativa [s]
};

/**
 * @ingroup internals
 * @brief Neurone Leaky Integrate-and-Fire con refrattarietà assoluta e relativa.
 *
 * @details
 * Modello matematico:
 *   τ * dV/dt = -(V - Vrest) + R * I_tot(t)
 *   con τ = R * C
 *
 * Dinamica della soglia:
 *   - A riposo: Vth = VthMin
 *   - Dopo uno spike: Vth = VthMax (istantaneamente)
 *   - Durante la refrattarietà relativa: Vth decade esponenzialmente verso VthMin
 *     con costante di tempo τ_rel = timeRelative / 3.0
 *
 * Refrattarietà:
 *   1. Assoluta (timeAbsolute): il neurone è sordo, V inchiodato a Vreset
 *   2. Relativa (timeRelative): la soglia decade, il neurone può rispondere
 *      solo a stimoli più forti del normale
 *
 * Integratori disponibili:
 *   - 'E' : Eulero in avanti (veloce, ma instabile per dt > τ/10)
 *   - 'R' : Runge-Kutta 4 (più lento, ma stabile e accurato – consigliato)
 *
 * @warning Non istanziare direttamente – usare Rete::aggiungiNeurone().
 */
class LIF {

    friend class Rete; ///< Concede a Rete l'accesso a tutti i membri privati

    // ── COSTRUTTORE / DISTRUTTORE ──────────────────────────────────────────

  public:
    /**
     * @brief Costruisce un neurone LIF con parametri di default.
     *
     * @param ID               Identificatore univoco del neurone (scelto dall'utente).
     * @param typeIntegratore  Metodo di integrazione: 'E' (Eulero) o 'R' (RK4).
     *
     * @warning Non chiamare direttamente – usare Rete::aggiungiNeurone().
     */
    LIF(int ID, char typeIntegratore) : ID_(ID), tipoIntegratore_(typeIntegratore) {}

    ~LIF() = default;

    // ── ATTRIBUTI PRIVATI ──────────────────────────────────────────────────

  private:
    int ID_;                          ///< Identificatore univoco del neurone
    double V_ = -65.0 * mV;           ///< Potenziale di membrana corrente [V]
    double Vth_ = -50.0 * mV;         ///< Soglia corrente (variabile nel tempo) [V]
    double VthMin_ = -50.0 * mV;      ///< Soglia minima a riposo [V]
    double VthMax_ = -35.0 * mV;      ///< Soglia massima subito dopo uno spike [V]
    double Vrest_ = -65.0 * mV;       ///< Potenziale di riposo [V]
    double Vreset_ = -70.0 * mV;      ///< Potenziale di reset dopo uno spike [V]
    double R_ = 1.0 * Mohm;           ///< Resistenza di membrana [Ω]
    double C_ = 100.0 * p * F;        ///< Capacità di membrana [F]
    double timeAbsolute_ = 5.0 * ms;  ///< Durata refrattarietà assoluta [s]
    double timeRelative_ = 15.0 * ms; ///< Durata refrattarietà relativa [s]
    double tempoRR_ = 0.0 * s;        ///< Tempo residuo di refrattarietà assoluta [s]
    bool fired_ = false;              ///< True se il neurone ha emesso uno spike in questo step
    char tipoIntegratore_ = 'E';      ///< 'E' per Eulero, 'R' per Runge-Kutta 4

    // ── METODI PRIVATI ──────────────────────────────────────────────────────

    /// @name Metodi di integrazione
    /// @{

    /**
     * @brief Integra l'equazione del neurone con il metodo di Eulero in avanti.
     * @param correnteTotale Corrente totale in ingresso [A].
     * @param dt             Passo temporale [s].
     */
    void euleroInAvanti(double correnteTotale, double dt);

    /**
     * @brief Integra l'equazione del neurone con Runge-Kutta del 4° ordine.
     * @param correnteTotale Corrente totale in ingresso [A].
     * @param dt             Passo temporale [s].
     */
    void rungeKutta(double correnteTotale, double dt);

    /// @}

    /// @name Getter
    /// @{

    bool hasFired() const { return fired_; }                             ///< True se ha generato uno spike.
    double getPotential() const { return V_; }                           ///< Potenziale di membrana corrente [V].
    int getId() const { return ID_; }                                    ///< ID del neurone.
    inline double getTau() const { return R_ * C_; }                     ///< Costante di tempo di membrana [s].
    inline double getTauRelative() const { return timeRelative_ / 3.0; } ///< Tau per il decadimento della soglia [s].

    /// @}

    /// @name Motore di evoluzione
    /// @{

    /**
     * @brief Avanza lo stato del neurone di un passo dt.
     *
     * @details
     * Sequenza:
     *   1. Reset di fired_ (flag valido solo per questo step).
     *   2. Se in refrattarietà assoluta: decrementa tempoRR_ e salta l'integrazione.
     *   3. Decadimento della soglia verso VthMin (se tempoRR_ == 0).
     *   4. Integrazione del potenziale con il metodo scelto.
     *   5. Controllo soglia: se V >= Vth → spike.
     *
     * @param correnteTotale Corrente totale in ingresso al neurone [A].
     * @param dt             Passo temporale [s].
     */
    void update(double correnteTotale, double dt);

    /// @}
};

#endif // LIF_HPP