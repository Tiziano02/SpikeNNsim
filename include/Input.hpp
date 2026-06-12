#ifndef INPUT_HPP
#define INPUT_HPP

#include <cmath>
#include <vector>
/*
 * inputEsterno - struttura e comportamenti di un qualsiasi Input esterno
 *
 *
 * Attributi:
 *  tStart_ e tEnd_  - intervallo temporale dell'input esterno
 * 
 * Metodi operativi: 
 *  calcola(double t) - calcola il valore dell'input generico al tempo t  (calcola non mi piace)
 * 
 * Metodi getter:  
 *   getValore(double t) - restituisce il valore dell'input calcolato al tempo t
 */




/*
 * InputEsterno - Struttura base per qualsiasi input di corrente esogeno.
 * Utilizza il pattern NVI (Non-Virtual Interface): l'interfaccia pubblica
 * gestisce l'intervallo temporale, delegando la matematica alle classi figlie.
 
class InputEsterno {
  protected:
    double tStart_, tEnd_;
    virtual double valuta(double t) const = 0;

  public:
    InputEsterno(double start, double end) : tStart_(start), tEnd_(end) {}
    virtual ~InputEsterno() = default;

    // Metodo pubblico (non virtuale) chiamato dalla Simulazione
    double getValore(double t) const {
        if (t >= tStart_ && t <= tEnd_) {
            return valuta(t); // Chiama l'implementazione specifica della figlia
        }
        return 0.0;
    }
  
};
*/

/*
 * InputCostante - Inietta una corrente fissa I_ per tutta la durata dell'intervallo
 
class InputCostante : public InputEsterno {
  private:
    double I_;

  public:
    // Il costruttore DEVE ricevere start e end per passarli al costruttore padre (InputEsterno)
    InputCostante(double I, double start, double end) 
        : InputEsterno(start, end), I_(I) {}

  protected:
    double valuta(double t) const override { 
        return I_; 
    }
};
*/

/*
 * InputSinusoidale - Inietta una corrente oscillante

class InputSinusoidale : public InputEsterno {
  private:
    double ampiezza_, freq_;

  public:
    // Anche qui passiamo start e end alla classe madre
    InputSinusoidale(double ampiezza, double freq, double start, double end) 
        : InputEsterno(start, end), ampiezza_(ampiezza), freq_(freq) {}

  protected:
    double valuta(double t) const override { 
        return ampiezza_ * std::sin(freq_ * t); 
    }
};
 */

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
