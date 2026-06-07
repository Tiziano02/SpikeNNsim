#ifndef INPUT_HPP
#define INPUT_HPP

#include <vector>

/*
 * Input — stimolo esterno applicato a un neurone per l'intera simulazione.
 *
 * Attributi:
 *   id      ID del neurone destinatario (deve esistere nella Rete)
 *   valori  vettore di correnti [A] in unità SI, una per step temporale
 *
 * La lunghezza di valori deve coincidere con stepTotali della Simulazione.
 * Il controllo è eseguito in Simulazione::aggiungiInputEsterni().
 */
struct Input {
    int id;
    std::vector<double> valori;
};

/*
 * InputCorrente — snapshot dell'input esterno al passo temporale corrente.
 *
 * Usato internamente da Simulazione::avviaSimulazione() per costruire
 * il vettore di input istantanei da passare a Rete::step().
 *
 * Attributi:
 *   id             ID del neurone destinatario
 *   valoreCorrente corrente istantanea [A] nello step corrente
 */
struct InputCorrente {
    int id;
    double valoreCorrente;
};

#endif // INPUT_HPP