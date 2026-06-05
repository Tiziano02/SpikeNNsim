
Documentazione delle classi Neurone e ReteNeurale nel file ReteNeurale.hpp : 
1) ordine delle conessioni nella matrice di connessioni: 
    - connessioni_[i][j] rappresenta la connessione da neurone i a neurone j, con il peso specificato
    - peso positivo indica una connessione eccitatoria, peso negativo indica una connessione inibitoria, peso zero indica nessuna connessione
2) gestione dell'input esterno: 
    - inputEsterno_[i] rappresenta l'input esterno associato al neurone i, che viene aggiunto all'input totale calcolato dalle connessioni sinaptiche
3) dinamica del potenziale di membrana:
    - il potenziale di membrana V_ viene aggiornato in base alla formula: dV/dt = (-(V_ - Vrest_) + inputTotale)/tau_
    - quando V_ supera la soglia Vth_, il neurone spara (fired = true), il potenziale viene resettato a Vreset_ e inizia il periodo refrattario
4) gestione del periodo refrattario:
    - tempoRR_ rappresenta il tempo refrattario rimanente, durante il quale il neurone non può sparare
    - quando un neurone spara, tempoRR_ viene impostato a tauR_, e viene decrementato ad ogni passo di simulazione fino a tornare a zero
5) spike interviene immediatamente nel passo successivo senza ritardo di trasmissione 
6) simulazione:
     - il metodo simulazione() esegue la simulazione della rete per un tempo totale T con passo dt, salvando i potenziali di membrana e gli stati di firing in due file separati
7) estendibilità futura: 
    -
    -


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

