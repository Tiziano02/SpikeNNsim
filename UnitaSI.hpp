// Definizione delle unità di misura, dei multipli e sottomultipli

// unità fondamentali del Sistema Internazionale
const double s =    1.0;                // secondo  
const double m =    1.0;                // metro
const double kg =   1.0;                // chilogrammo 
const double A =    1.0;                // ampere

//unità derivate di nostro utilizzo 
const double Hz = 1.0/s;                // frequenza
const double v = m / s;                 // velocità
const double a = m / (s * s);           // accelerazione
const double N = kg * m / (s * s);      // forza
const double rho = kg / (m * m * m);    // densità
const double Pa = N / (m * m);          // pressione
const double J = N * m;                 // energia
const double W = J / s;                 // potenza
const double C = A * s;                 // carica elettrica
const double V = J / C;                 // potenziale elettrico
const double Ohm = V / A;               // resistenza elettrica
const double S = A / V;                 // conduttanza elettrica
const double F = C / V;                 // capacità elettrica


// multipli e sottomultipli
const double T = 1e12;               // tera
const double G = 1e9;                // giga
const double M = 1e6;                // mega
const double k = 1e3;                // kilo
const double m = 1e-3;                // milli
const double u = 1e-6;                // micro
const double n = 1e-9;                // nano
const double p = 1e-12;               // pico