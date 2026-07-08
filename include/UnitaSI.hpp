#ifndef UNITA_SI_HPP
#define UNITA_SI_HPP

/**
 * UnitaSI – definizione delle unità di misura SI e dei loro multipli.
 *
 * Tutte le costanti sono fattori di scala rispetto alle unità SI base.
 * Moltiplicando un valore numerico per la costante corrispondente lo si esprime
 * nell'unità SI corretta.
 *
 * Esempio:
 *   double V = -65.0 * mV;   // -0.065 V
 *   double t = 5.0 * ms;     // 0.005 s
 *
 * In questo modo l'integrazione numerica lavora sempre con valori coerenti.
 */

// Unità fondamentali del SI
constexpr double s = 1.0;  // secondo      [s]
constexpr double mt = 1.0; // metro        [m]
constexpr double kg = 1.0; // chilogrammo  [kg]
constexpr double A = 1.0;  // ampere       [A]

// Unità derivate
constexpr double Hz = 1.0 / s;              // frequenza         [Hz]
constexpr double v = mt / s;                // velocità          [m/s]
constexpr double a = mt / (s * s);          // accelerazione     [m/s²]
constexpr double N = kg * mt / (s * s);     // forza             [N]
constexpr double rho = kg / (mt * mt * mt); // densità           [kg/m³]
constexpr double Pa = N / (mt * mt);        // pressione         [Pa]
constexpr double J = N * mt;                // energia           [J]
constexpr double W = J / s;                 // potenza           [W]
constexpr double C = A * s;                 // carica elettrica  [C]
constexpr double Volt = J / C;              // potenziale        [V]
constexpr double Ohm = Volt / A;            // resistenza        [Ω]
constexpr double S = A / Volt;              // conduttanza       [S]
constexpr double F = C / Volt;              // capacità          [F]

// Multipli e sottomultipli SI
constexpr double T = 1e12;  // tera  (10^12)
constexpr double G = 1e9;   // giga  (10^9)
constexpr double M = 1e6;   // mega  (10^6)
constexpr double k = 1e3;   // kilo  (10^3)
constexpr double m = 1e-3;  // milli (10^-3)
constexpr double u = 1e-6;  // micro (10^-6)
constexpr double n = 1e-9;  // nano  (10^-9)
constexpr double p = 1e-12; // pico  (10^-12)

// Alias comodi
constexpr double mV = m * Volt;  // millivolt   [mV]
constexpr double ms = m * s;     // millisecondo [ms]
constexpr double ps = p * s;     // picosecondo  [ps]
constexpr double Mohm = M * Ohm; // megaohm     [MΩ]

#endif // UNITA_SI_HPP