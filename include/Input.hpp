#ifndef INPUT_HPP
#define INPUT_HPP

#include <variant>

// ============================================================================
// TIPI DI STIMOLO (PUBBLICI)
// ============================================================================

/**
 * configConstantStimulus – stimolo costante.
 *
 * Fornisce una corrente costante di ampiezza "ampiezza" nell'intervallo
 * [timeStart, timeEnd] (in secondi).
 */
struct configConstantStimulus {
    double timeStart; // [s]
    double timeEnd;   // [s]
    double ampiezza;  // [A]
};

/**
 * configSinStimulus – stimolo sinusoidale.
 *
 * Fornisce una corrente sinusoidale:
 *   I(t) = ampiezza * sin(2π * frequenza * t + fase)
 * attiva solo per t in [timeStart, timeEnd].
 */
struct configSinStimulus {
    double timeStart; // [s]
    double timeEnd;   // [s]
    double ampiezza;  // [A]
    double frequenza; // [Hz]
    double fase;      // [rad]
};

// ============================================================================
// ALIAS PER LA GESTIONE POLIMORFA DEGLI STIMOLI
// ============================================================================

/**
 * typeParameters – variant che può contenere uno dei tipi di stimolo.
 * Utilizzato per uniformare la gestione degli stimoli esterni.
 */
using typeParameters = std::variant<configConstantStimulus, configSinStimulus>;

/**
 * stimolo – associa uno stimolo a un neurone target.
 *
 * Campi:
 *   IDneurone – ID del neurone nel vettore neuroni_ di Rete
 *   parametri – parametri dello stimolo (uno dei tipi sopra)
 */
struct stimolo {
    int IDneurone;
    typeParameters parametri;
};

struct recordStimolo {
    size_t indexNeurone;
    typeParameters parametri;
};

#endif // INPUT_HPP