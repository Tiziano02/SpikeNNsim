#ifndef UNITA_SI_HPP
#define UNITA_SI_HPP
// Definizione delle unità di misura, dei multipli e sottomultipli

// unità fondamentali del Sistema Internazionale
const double s = 1.0;                       // secondo  
const double mt = 1.0;                      // metro
const double kg = 1.0;                      // chilogrammo 
const double A = 1.0;                       // ampere

// unità derivate del Sistema Internazionale 
const double Hz = 1.0/s;                    // frequenza
const double v = mt / s;                    // velocità --> non serve
const double a = mt / (s * s);              // accelerazione --> non serve 
const double N = kg * mt / (s * s);         // forza 
const double rho = kg / (mt * mt * mt);     // densità 
const double Pa = N / (mt * mt);            // pressione --> non serve
const double J = N * mt;                    // energia 
const double W = J / s;                     // potenza
const double C = A * s;                     // carica elettrica
const double Volt = J / C;                  // potenziale elettrico
const double Ohm = Volt / A;                // resistenza elettrica
const double S = A / Volt;                  // conduttanza elettrica --> potrebbe conofondere 
const double F = C / Volt;                  // capacità elettrica


// multipli e sottomultipli
const double T = 1e12;                      // tera
const double G = 1e9;                       // giga
const double M = 1e6;                       // mega
const double k = 1e3;                       // kilo
const double m = 1e-3;                      // milli
const double u = 1e-6;                      // micro
const double n = 1e-9;                      // nano
const double p = 1e-12;                     // pico

// unità più comode 
const double mV = m * Volt;                 // millivolt
const double ms = m * s;                    // millisecondo
const double ps = p * s;                    // picosecondo
const double Mohm = M *Ohm;                 // megaOhm 
// parametri utili per la simulazione


#endif // UNITA_SI_HPP