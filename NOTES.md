# Developer Notes

Coming soon. È un file privato per gli sviluppatori. Inserisco come funziona la simulazione, sia temporalmente, quali sono gli output e le connessioni tra uno step e l'altro. Deve essere il diario di bordo della simulazione. Deve avere questa struttura : 


1) Spiegazione dei vari metodi update, step e simulazione (le parti del codice rilevanti)
2) Problemi di ottimizzazione del codice
3) Problemi di output dei dati
4) Problemi di input della simulazione
5) Dove e come implementare le nuove featurs

Featurs : 

1) più simualazioni per un singolo oggetto simulazione 

Esatto, hai descritto il pattern reset + reinizializza. In pratica:
cppvoid avviaSimulazione(const std::string &filenameV, 
                      const std::string &filenameF, 
                      const std::string &filenameS) {
    reset();                                        // azzera tutto
    filenameV_ = filenameV;                         // setta i filename
    filenameF_ = filenameF;
    filenameS_ = filenameS;
    inizializzaOutput();                            // usa i filename dall'attributo
    preparaInput();
    eseguiLoop();
    finalizzaOutput();
}
E reset() azzera stepCorrente_, posizioneBuffer_, chiude i file se aperti, svuota i buffer.

Questo risolve il dubbio in modo elegante: i filename sono attributi, avviaSimulazione li riceve come parametri solo per settarli, e ogni chiamata riparte da uno stato pulito. Puoi fare parameter sweep sullo stesso oggetto senza creare nuovi oggetti.
L'unica cosa a cui fare attenzione è lo stato di inputEsterni_ — dopo un reset() vuoi mantenerli o azzerarli? Probabilmente mantenerli, così non devi richiamare aggiungiInputEsterni ad ogni run se gli input non cambiano. 

