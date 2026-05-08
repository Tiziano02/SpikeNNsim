/*
Nome  del progetto : Framework per la simulazioni di reti di neuroni

Autore : Tiziano Costantini

Data : 07/06/2026

Descrizione del progetto : 

permetter di creare un grafo di neuroni, 
connessi tra loro da sinapsi, 
e di simulare il comportamento dinamico di questo grafo nel tempo.


Classe Neurone : 
1)I membri della classe neurone dovranno essere: 
    - un identificatore univoco
    - il suo stato dinamico (il potenziale di membrana)
    - parametri biologici (ad esempio, la soglia di attivazione, il tempo refrattario, ecc.)
2)I metodi della classe neurone dovranno essere:
    - un metodo di update del singolo neurone che aggiorna il suo stato in base all'input totale ricevuto 

Classe ReteNeurale :
1)I membri della classe ReteNeurale dovranno essere:
    - una lista di neuroni
    - una matrice di connessioni (o una lista di adiacenza) 
    che rappresenta le sinapsi tra i neuroni, con i loro pesi e segni associati
    - lista di input associati a ciascun neurone 
2)I metodi della classe ReteNeurale dovranno essere:
    - un metodo per aggiungere neuroni alla rete
    - un metodo per connettere i neuroni tra loro, specificando il peso e il segno della connessione
    - un metodo per simulare l'andamento della rete nel tempo, ad esempio step() : 
        - calcola l'input totale per ogni neurone basato sullo step precedente 
        - aggiorna lo stato di ogni neurone (update() per ogni neurone)in base all'input totale e alle regole di attivazione
        - catturo l'output di ogni neurone (ad esempio, se ha superato la soglia di attivazione) e lo uso come input per i neuroni connessi nel passo successivo
 
In futuro si potra fare la classe sinapsi per rappresentare le connessioni tra i neuroni, con parametri come la forza sinaptica, il ritardo di trasmissione, facilitazione o depressione sinaptica 
Per ora sarà rappresentata dal grafo stesso, cioè dalla lista di adiacenza dei neuroni : 
- peso e segno associato a ogni connessione dei neuroni.

Quindi idealmente : 

ReteNeurale rete; // creazione della rete neurale

vector<Neurone> neuroni; // creazione dei neuroni

vector<vector<double>> connessioni; // creazione delle connessioni tra i neuroni ? --> fare una classe Connessioni con il suo metodo per aggiungere connesioni --> non mi piace preferisco gestire la connessione tramite la classe rete 

vector<double> input; // creazione degli input 

//inizializzazione dei neuroni tramite costruttore

//inizializzazione delle connessioni tra i neuroni tramite il metodo di connessione della rete

// inizializzazione degli input associati a ciascun neurone a tempo t = 0

// inizializzazione della rete 

// simulazione del comportamento dinamico dei neuroni nel tempo

- metodo step() della variabile rete per t che va da 0 a T (tempo totale della simulazione) 

*/

#include "ReteNeurale.hpp"
#include <random>

int main(){

    int N = 100;
    std::vector<Neurone> neuroni;

    for(int i = 0; i < N; ++i){
        Neurone n(i);
        neuroni.push_back(n); // crea N neuroni con ID da 0 a N-1
    }

    ReteNeurale rete; // creazione della rete neurale

    for(const auto& neurone : neuroni)
        rete.aggiungiNeurone(neurone); // aggiunge i neuroni alla rete

    // connetti i neuroni in modo casuale con probabilità 0.1
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_real_distribution<> weight_inh(4.0, 6.0);

    for(int i = 0; i < N; ++i){
        for(int j = 0; j < N; ++j){
            if(i != j && prob_dist(gen) < 0.1){
                double peso;
                if(i < 80){ // eccitatore (primi 80)
                    peso = 30.0;
                } else { // inibitore (ultimi 20)
                    peso = -30.0 *  weight_inh(gen);
                }
                rete.connettiNeuroni(i, j, peso);
            }
        }
    }

    // imposta input esterno random tra 18 e 22 per tutti i neuroni
    std::uniform_real_distribution<> input_dist(18.0, 22.0);
    for(int i = 0; i < N; ++i){
        rete.setInput(i, input_dist(gen));
    }

rete.simulazione(
    0.1,
    1000.0,
    "potenziali.txt",
    "firing.txt"
);
}