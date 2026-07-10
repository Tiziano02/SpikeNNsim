#ifndef LIF_HPP
#define LIF_HPP

#include "UnitaSI.hpp"
#include <optional>

/**
 * @ingroup publicapi
 *
 * @brief
 * Insieme di parametri modificabili dopo la creazione per il neurone LIF (Leaky Integrate-and-Fire).
 *
 * @details
 * Utilizzata esclusivamente da Rete::modificaParametriNeurone().
 * Tutti i valori sono in unità SI (vedi UnitaSI.hpp).
 */
struct patchLIF {
    std::optional<double> V;            ///< Potenziale di membrana iniziale [V]
    std::optional<double> Vth;          ///< Soglia di attivazione iniziale [V]
    std::optional<double> VthMin;       ///< Soglia minima, la dinamica di Vth tende a questa soglia [V]
    std::optional<double> VthMax;       ///< Soglia massima raggiunta subito dopo uno spike [V]
    std::optional<double> Vrest;        ///< Potenziale di riposo [V]
    std::optional<double> Vreset;       ///< Potenziale a cui il neurone torna dopo lo spike [V]
    std::optional<double> R;            ///< Resistenza di ingresso [Ω]
    std::optional<double> C;            ///< Capacità di membrana [F] (tau = R * C)
    std::optional<double> timeAbsolute; ///< Durata della refrattarietà assoluta [s]
    std::optional<double> timeRelative; ///< Durata della refrattarietà relativa (decadimento soglia) [s]
};

/**
 * @ingroup internals
 *
 * @brief
 * Neurone Leaky Integrate-and-Fire con refrattarietà assoluta e relativa.
 *
 */
class LIF {

    friend class Rete;

    // -- COSTRUTTORE / DISTRUTTORE -----------------------------------------

  public:
    /**
     * @brief Costruisce un neurone di default di tipo LIF
     *
     * @details Non chiamare direttamente: usare Rete::aggiungiNeurone().
     * @param ID Identificatore univoco del neurone
     * @param typeIntegratore Metodo di integrazione: 'E' per Eulero in avanti, 'R' per Runge-Kutta 4 (consigliato).
     */
    LIF(int ID, char typeIntegratore) : ID_(ID), tipoIntegratore_(typeIntegratore) {}

    ~LIF() = default;

    // ── ATTRIBUTI PRIVATI --------------------------------------------------

  private:
    int ID_;                          ///< id del neruone
    double V_ = -65.0 * mV;           ///< potenziale di membrana
    double Vth_ = -50.0 * mV;         ///< potenziale soglia del neurone, variabile nel tempo
    double VthMin_ = -50.0 * mV;      ///< potenziale soglia minimo
    double VthMax_ = -35.0 * mV;      ///< potenziale soglia massima
    double Vrest_ = -65.0 * mV;       ///< potenziale di membrana a riposo
    double Vreset_ = -70.0 * mV;      ///< potenziale di membrana dopo uno spike
    double R_ = 1.0 * Mohm;           ///< resistenza neurone
    double C_ = 100.0 * p * F;        ///< capacità neurone
    double timeAbsolute_ = 5.0 * ms;  ///< tempo refrattario assoluto
    double timeRelative_ = 15.0 * ms; ///< tempo refrattario relativo
    double tempoRR_ = 0.0 * s;        ///< tempo refrattario assoluto rimanente
    bool fired_ = false;              ///< stato del neurone
    char tipoIntegratore_ = 'E';      ///< tipologia di intgratore per la dinamica del potenziale di membrana

    // -- METODI PRIVATI ---------------------------------------------------

    // 1. metodi di integrazione
    void euleroInAvanti(double correnteTotale, double dt);
    void rungeKutta(double correnteTotale, double dt);

    // 2. metodi getter
    bool hasFired() const { return fired_; }
    double getPotential() const { return V_; }
    int getId() const { return ID_; }
    inline double getTau() const { return R_ * C_; }
    inline double getTauRelative() const { return timeRelative_ / 3.0; }

    // 3. metodi di evoluzione della dinamica
    void update(double correnteTotale, double dt);
};

#endif // LIF_HPP