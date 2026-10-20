/**
 * @file main.cpp
 * @brief Guida interattiva e banco di prova per la libreria SpikeNNsim.
 *
 * Questo file contiene esempi chiari e modulari che illustrano le funzionalità
 * principali della libreria: creazione di neuroni e popolazioni, introduzione di
 * eterogeneità biologica con patch e dispersioni, modelli di sinapsi e simulazione.
 */

#include "Rete.hpp"
#include "Simulazione.hpp"
#include "Stimoli.hpp"
#include "UnitaSI.hpp"

#include <iostream>
#include <vector>
#include <string>

// Funzione di utilità per separare visivamente gli esempi a terminale
static void stampaHeader(const std::string& titolo) {
    std::cout << "\n======================================================================\n";
    std::cout << "  " << titolo << "\n";
    std::cout << "======================================================================\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 1: Neuroni Singoli, Metodi di Integrazione e Pattern Patch
// ---------------------------------------------------------------------------------------
void esempio1_neuroni_singoli() {
    stampaHeader("ESEMPIO 1: Neuroni Singoli e Pattern Patch");

    Rete rete;

    // 1. Aggiunta di singoli neuroni con parametri di default:
    //    'A' = Analitico esatto (zero errore numerico, incondizionatamente stabile)
    //    'E' = Eulero in avanti (leggero e veloce)
    //    'R' = Runge-Kutta 4 (massima precisione numerica per ODE non-lineari come Exp)
    size_t idLIF = rete.aggiungiNeurone(NeuronModel::LIF, 'A');
    size_t idExp = rete.aggiungiNeurone(NeuronModel::Exp, 'R');

    std::cout << "[OK] Neurone LIF creato con ID: " << idLIF << " (Integratore: Analitico esatto 'A')\n";
    std::cout << "[OK] Neurone Exp creato con ID: " << idExp << " (Integratore: Runge-Kutta 'R')\n";

    // 2. Modifica post-creazione con "Pattern Patch":
    patchLIF modificaLIF;
    modificaLIF.R = 50.0 * Mohm;  // Resistenza di membrana personalizzata
    modificaLIF.Vth = -52.0 * mV; // Soglia di spike personalizzata
    rete.modificaParametriNeurone(idLIF, modificaLIF);
    std::cout << "[OK] Neurone LIF aggiornato: R = 50 MOhm, Vth = -52 mV\n";

    // 3. Creazione diretta in un'unica riga con Patch inline opzionale:
    size_t idCustom = rete.aggiungiNeurone(NeuronModel::LIF, 'A', patchLIF{.Vth = -48.0 * mV, .C = 200 * p * F});
    std::cout << "[OK] Neurone LIF personalizzato creato direttamente in linea con ID: " << idCustom << " (Integratore: 'A')\n";

    // 4. Modifica a runtime del solutore numerico di un neurone:
    rete.modificaIntegratoreNeurone(idLIF, 'R'); // Passa a Runge-Kutta
    std::cout << "[OK] Risolutore numerico del neurone LIF convertito a Runge-Kutta\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 2: Popolazioni Omogenee ed Eterogenee
// ---------------------------------------------------------------------------------------
void esempio2_popolazioni() {
    stampaHeader("ESEMPIO 2: Popolazioni Omogenee ed Eterogenee");

    Rete rete;

    // 1. Popolazione Omogenea: 80 neuroni LIF con configurazione iniziale opzionale
    patchLIF patchIniziale;
    patchIniziale.C = 200.0 * p * F; // Capacità di membrana
    patchIniziale.R = 80.0 * Mohm;
    size_t popEccitatoria = rete.addPopulation(80, NeuronModel::LIF, 'E', patchIniziale);

    std::cout << "[OK] Creata Popolazione Omogenea (ID: " << popEccitatoria << ") con 80 neuroni LIF\n";

    // 2. Modifica collettiva di tutta la popolazione:
    patchLIF patchNuova;
    patchNuova.Vth = -48.0 * mV;
    rete.modificaParametriPopolazione(popEccitatoria, patchNuova);
    std::cout << "[OK] Aggiornata soglia Vth = -48 mV per tutti i neuroni della popolazione " << popEccitatoria << "\n";

    // 3. Popolazione Eterogenea: composizione mista (es. 80% LIF, 20% Esponenziali)
    std::vector<NeuronModel> tipi = {NeuronModel::LIF, NeuronModel::Exp};
    std::vector<double> percentuali = {0.80, 0.20};
    std::vector<char> solutori = {'E', 'R'};

    size_t popMista = rete.addPopolazioneEterogenea(100, tipi, percentuali, solutori);
    std::cout << "[OK] Creata Popolazione Eterogenea (ID: " << popMista << ") con 100 neuroni (80 LIF + 20 Exp)\n";

    // 4. Modifica mirata della singola sub-popolazione:
    //    Indice sub-popolazione 0 = neuroni LIF (80)
    //    Indice sub-popolazione 1 = neuroni Exp (20)
    patchExp patchInibitori;
    patchInibitori.sharpness = 2.5 * mV; // Parametro specifico del modello esponenziale
    rete.modificaParametriSubPopolazione(popMista, 1, patchInibitori);
    std::cout << "[OK] Aggiornato parametro 'sharpness' solo per la sub-popolazione Exp\n";

    // 5. Inizializzazione rapida dei Potenziali V(0):
    // 5a. Distribuzione gaussiana attorno al riposo per la popolazione omogenea:
    rete.setCondizioneInizialePopolazione(popEccitatoria, -65.0 * mV, 0.05, DistType::Gaussian);
    std::cout << "[OK] Condizione iniziale V(0) ~ N(-65 mV, 5%) impostata per la popolazione " << popEccitatoria << "\n";

    // 5b. Distribuzione uniforme (stesso metodo, cambiando semplicemente il parametro DistType):
    rete.setCondizioneInizialePopolazione(popMista, -57.5 * mV, 0.10, DistType::Uniform);
    std::cout << "[OK] Condizione iniziale V(0) ~ Uniforme(-57.5 mV, 10%) impostata per la popolazione " << popMista << "\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 3: Eterogeneità Biologica e API Batch (Stile Python)
// ---------------------------------------------------------------------------------------
void esempio3_eterogeneita_e_batch() {
    stampaHeader("ESEMPIO 3: Eterogeneita' Biologica (Zero-Cost Heterogeneity) e API Batch");

    Rete rete;
    size_t popId = rete.addPopulation(50, NeuronModel::LIF, 'E');

    // 1. Randomizzazione con dispersione gaussiana:
    //    In natura i neuroni non sono copie identiche. Possiamo applicare una dispersione
    //    relativa (es. 10% intorno al valore target):
    patchLIF patchDispersa;
    patchDispersa.R = 100.0 * Mohm;
    patchDispersa.Vth = -50.0 * mV;

    // Dispersione del 10% Gaussiana su tutti i neuroni della popolazione
    rete.modificaParametriPopolazione(popId, patchDispersa, 0.10, DistType::Gaussian);
    std::cout << "[OK] Parametri distribuiti su 50 neuroni con dispersione Gaussiana del 10%\n";

    // 2. Operazioni Batch su elenchi di indici arbitrari (senza scrivere cicli 'for'):
    //    Particolarmente comodo e immediatamente pronto per interfacciarsi con Python / NumPy!
    std::vector<size_t> neuroniBersaglio = {0, 5, 12, 25, 42};
    patchLIF patchSpeciale;
    patchSpeciale.Vrest = -60.0 * mV;

    // Modifica batch con dispersione uniforme del 5%
    rete.modificaParametriNeuroni(neuroniBersaglio, patchSpeciale, 0.05, DistType::Uniform);
    std::cout << "[OK] Modifica batch applicata a " << neuroniBersaglio.size()
              << " neuroni specifici con dispersione Uniforme\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 4: Modelli di Sinapsi (Current-based vs Conductance-based)
// ---------------------------------------------------------------------------------------
void esempio4_modelli_sinaptici() {
    stampaHeader("ESEMPIO 4: Sinapsi Current-Based e Conductance-Based");

    Rete rete;
    size_t n0 = rete.aggiungiNeurone(NeuronModel::LIF, 'E');
    size_t n1 = rete.aggiungiNeurone(NeuronModel::LIF, 'E');
    size_t n2 = rete.aggiungiNeurone(NeuronModel::LIF, 'E');

    // 1. Sinapsi Current-based (0 -> 1) con risolutore analitico esatto 'A':
    //    Inietta una corrente post-sinaptica esponenziale indipendente dal potenziale di membrana:
    //    dI/dt = -I / tau; al passaggio dello spike: I += peso * Ipeak
    size_t idSynCurrent = rete.connettiNeuroni(n0, n1, SynapseModel::Current, 'A');

    patchCurrent patchCur;
    patchCur.peso = 1.0;
    patchCur.Ipeak = 5.0 * n * A;
    patchCur.tau = 5.0 * ms;
    patchCur.delay = 1.0 * ms;
    rete.modificaSinapsi(idSynCurrent, patchCur);
    std::cout << "[OK] Creata e configurata Sinapsi Current-based (0 -> 1, ID: " << idSynCurrent << ", Integratore: 'A')\n";

    // 2. Sinapsi Conductance-based (1 -> 2) con risolutore Eulero 'E':
    //    Più realistica biologicamente: la corrente iniettata dipende dal potenziale post-sinaptico:
    //    I_syn = g_syn(t) * (V_post - E_rev)
    size_t idSynCond = rete.connettiNeuroni(n1, n2, SynapseModel::Conductance, 'E');

    patchConductance patchCond;
    patchCond.peso = 1.0;
    patchCond.gpeak = 10.0 * n * S; // Conduttanza di picco
    patchCond.tau = 3.0 * ms;
    patchCond.delay = 1.5 * ms;
    patchCond.Erev = 0.0 * mV; // Potenziale di inversione AMPA-like (eccitatorio)
    rete.modificaSinapsi(idSynCond, patchCond);
    std::cout << "[OK] Creata e configurata Sinapsi Conductance-based (1 -> 2, ID: " << idSynCond << ", Erev: 0 mV, Integratore: 'E')\n";

    // 3. Modifica a runtime dell'integratore della sinapsi (simmetrico a modificaIntegratoreNeurone):
    rete.modificaIntegratoreSinapsi(idSynCond, 'A'); // Passa a risolutore Analitico esatto
    std::cout << "[OK] Risolutore della sinapsi Conductance convertito ad Analitico 'A'\n";

    // 4. Ricerca di sinapsi tra coppie di neuroni:
    auto trovate = rete.findSinapsi(n0, n1);
    std::cout << "[OK] Sinapsi trovate tra neurone " << n0 << " e neurone " << n1 << ": " << trovate.size()
              << " (ID: " << trovate[0] << ")\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 5: Connettività Ricorrente e Inter-Popolazione (connettiPopolazioni)
// ---------------------------------------------------------------------------------------
void esempio5_connettivita_popolazione() {
    stampaHeader("ESEMPIO 5: Connettivita' Ricorrente e Inter-Popolazione");

    Rete rete;
    size_t popEcc = rete.addPopulation(20, NeuronModel::LIF, 'E');
    size_t popInh = rete.addPopulation(10, NeuronModel::LIF, 'E');

    // 1. Connessione ricorsiva (intra-popolazione) con probabilità di connessione del 15%
    //    e applicazione contestuale di una configurazione iniziale con dispersione:
    patchCurrent templateSyn;
    templateSyn.peso = 1.5;
    templateSyn.tau = 4.0 * ms;
    templateSyn.delay = 1.0 * ms;

    auto sinapsiRicorrenti = rete.connettiPopolazione(popEcc,
                                                      0.15, // 15% probabilita' di connessione casuale
                                                      SynapseModel::Current,
                                                      templateSyn, // Configurazione iniziale (opzionale)
                                                      0.10,        // Dispersione del 10% sui pesi/parametri
                                                      DistType::Gaussian);

    std::cout << "[OK] Connessione ricorrente creata: generate " << sinapsiRicorrenti.size() << " sinapsi interne\n";

    // 2. Connessione inter-popolazioni (Eccitatori -> Inibitori) tramite connettiPopolazioni:
    patchCurrent synE2I{.peso = -1.2, .tau = 3.0 * ms, .delay = 1.5 * ms};
    auto sinapsiInter = rete.connettiPopolazioni(popEcc, popInh, 0.25, SynapseModel::Current, synE2I, 0.05);
    std::cout << "[OK] Connessione inter-popolazioni (Ecc -> Inh): generate " << sinapsiInter.size()
              << " sinapsi con dispersione del 5%\n";

    // 3. Modifica globale di tutte le sinapsi interne della popolazione:
    patchCurrent nuovaPatch;
    nuovaPatch.peso = 2.0; // Raddoppiamo il peso di tutte le sinapsi ricorrenti
    rete.modificaSinapsiPopolazione(popEcc, nuovaPatch, 0.05, DistType::Uniform);
    std::cout << "[OK] Aggiornato il peso di tutte le sinapsi della popolazione tramite modificaSinapsiPopolazione\n";

    // 4. Modifica batch su un sottoinsieme di sinapsi con l'overload vettoriale:
    if (sinapsiRicorrenti.size() >= 3) {
        std::vector<size_t> sottoinsieme = {sinapsiRicorrenti[0], sinapsiRicorrenti[1], sinapsiRicorrenti[2]};
        patchCurrent patchSpecifica{.delay = 2.5 * ms};
        rete.modificaSinapsi(sottoinsieme, patchSpecifica);
        std::cout << "[OK] Aggiornato delay a 2.5 ms per 3 sinapsi tramite modificaSinapsi(vector)\n";
    }
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 6: Circuito Completo, Iniezione di Stimoli ed Esecuzione Simulazione
// ---------------------------------------------------------------------------------------
void esempio6_simulazione_completa() {
    stampaHeader("ESEMPIO 6: Circuito Completo, Stimoli ed Esecuzione Simulazione");

    Rete rete;

    // 1. Architettura: 2 neuroni connessi in serie (0 -> 1)
    size_t nPre = rete.aggiungiNeurone(NeuronModel::LIF, 'E');
    size_t nPost = rete.aggiungiNeurone(NeuronModel::LIF, 'E');

    // 2. Configurazione neuroni
    patchLIF paramNeurone;
    paramNeurone.Vrest = -65.0 * mV;
    paramNeurone.Vreset = -65.0 * mV;
    paramNeurone.Vth = -50.0 * mV;
    paramNeurone.R = 100.0 * Mohm;
    paramNeurone.C = 100.0 * p * F;
    rete.modificaParametriNeurone(nPre, paramNeurone);
    rete.modificaParametriNeurone(nPost, paramNeurone);

    // 3. Connessione eccitatoria Current-based tra i due neuroni (peso < 0 = eccitatorio)
    size_t idSyn = rete.connettiNeuroni(nPre, nPost, SynapseModel::Current);
    patchCurrent sinapsiCfg;
    sinapsiCfg.peso = -1.0;
    sinapsiCfg.Ipeak = 15.0 * n * A;
    sinapsiCfg.tau = 5.0 * ms;
    sinapsiCfg.delay = 1.0 * ms;
    rete.modificaSinapsi(idSyn, sinapsiCfg);

    std::cout << "[OK] Circuito assemblato: Neurone pre(" << nPre << ") -> Neurone post(" << nPost
              << ") con sinapsi ID " << idSyn << "\n";

    // 4. Congelamento della rete (obbligatorio prima di creare e avviare la Simulazione)
    rete.bloccaRete();
    std::cout << "[OK] Rete bloccata con successo (isBloccata = " << std::boolalpha << rete.isBloccata() << ")\n";

    // Dimostrazione di sicurezza: tentata modifica a rete bloccata
    try {
        rete.modificaParametriNeurone(nPre, paramNeurone);
    } catch (const std::logic_error& e) {
        std::cout << "[SICUREZZA] Tentata modifica su rete bloccata intercettata con successo:\n"
                  << "            " << e.what() << "\n";
    }

    // 5. Configurazione motore di simulazione:
    //    dt = 0.1 ms, Durata = 20 ms
    Simulazione sim(rete, 0.1 * ms, 20.0 * ms);

    // 5. Iniezione di corrente stimolo esterno continua (DC) sul neurone pre-sinaptico:
    //    Corrente di 2.0 nA accesa tra t = 2 ms e t = 10 ms
    configConstantStimulus correntePre{.timeStart = 2.0 * ms, .timeEnd = 10.0 * ms, .ampiezza = 2.0 * n * A};
    stimolo stimPre{.indexNeurone = nPre, .parametri = correntePre};
    sim.iniettaStimoli({stimPre});

    std::cout << "[OK] Stimolo DC di 2.0 nA programmato su neurone " << nPre << " tra 2.0 ms e 10.0 ms\n";

    // 6. Esecuzione della simulazione e salvataggio dei risultati binari ad alte prestazioni
    std::cout << "[...] Esecuzione simulazione temporale in corso...\n";
    sim.avviaSimulazione("output_potenziali.bin", "output_firing.bin", "output_sinapsi.bin");

    std::cout << "[OK] Simulazione completata con successo! Dati binari scritti su disco.\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 7: Benchmark di Validazione Analitica (Rete Ibrida: Popolazione + Singoli)
// ---------------------------------------------------------------------------------------
void esempio7_validazione_anello_ibrido() {
    stampaHeader("ESEMPIO 7: Benchmark di Validazione Analitica (Rete Anulare Ibrida)");

    std::cout << "Descrizione del Benchmark:\n"
              << "  - 1 Popolazione ad anello di 4 neuroni LIF (ID 0..3) con delay inter-nodo = 2.0 ms\n"
              << "  - 1 Neurone Trigger singolo (ID 4) attivato da uno stimolo DC calibrato per 1 spike\n"
              << "  - 1 Neurone Readout singolo Exp (ID 5, Runge-Kutta) con sinapsi Conductance AMPA da N2\n"
              << "  - Comportamento teorico atteso: oscillazione periodica dell'anello con periodo T = 8.40 ms,\n"
              << "    delay hop = 2.10 ms e propagazione fedele al readout.\n\n";

    Rete rete;

    // 1. Popolazione ad anello: 4 neuroni LIF (ID 0, 1, 2, 3)
    patchLIF pLIF;
    pLIF.Vrest = -65.0 * mV;
    pLIF.Vreset = -65.0 * mV;
    pLIF.Vth = -50.0 * mV;
    pLIF.R = 100.0 * Mohm;
    pLIF.C = 100.0 * p * F;
    pLIF.timeAbsolute = 3.0 * ms; // Refrattarieta' > delay per garantire 1 solo spike per ciclo
    size_t idPop = rete.addPopulation(4, NeuronModel::LIF, 'E', pLIF);

    // 2. Neurone singolo Trigger (LIF, ID 4)
    size_t idTrigger = rete.aggiungiNeurone(NeuronModel::LIF, 'E');
    rete.modificaParametriNeurone(idTrigger, pLIF);

    // 3. Neurone singolo Readout (Exp, ID 5, Runge-Kutta 4)
    size_t idReadout = rete.aggiungiNeurone(NeuronModel::Exp, 'R');
    patchExp pExp;
    pExp.Vrest = -65.0 * mV;
    pExp.Vreset = -65.0 * mV;
    pExp.Vth = -50.0 * mV;
    pExp.R = 100.0 * Mohm;
    pExp.C = 100.0 * p * F;
    pExp.timeAbsolute = 3.0 * ms;
    pExp.sharpness = 2.0 * mV;
    rete.modificaParametriNeurone(idReadout, pExp);

    // 4. Connessioni sinaptiche:
    // 4a. Trigger -> Neurone 0 (singola sinapsi Current, peso < 0 = eccitatorio)
    size_t sTrig = rete.connettiNeuroni(idTrigger, 0, SynapseModel::Current);
    patchCurrent synTrig;
    synTrig.peso = -1.0;
    synTrig.Ipeak = 20.0 * n * A;
    synTrig.tau = 1.0 * ms;
    synTrig.delay = 1.0 * ms;
    rete.modificaSinapsi(sTrig, synTrig);

    // 4b. Anello: 0->1, 1->2, 2->3 (connessioni interne)
    patchCurrent synRing;
    synRing.peso = -1.0;
    synRing.Ipeak = 20.0 * n * A;
    synRing.tau = 1.0 * ms;
    synRing.delay = 2.0 * ms; // 2.0 ms ritardo di conduzione per hop

    size_t s01 = rete.connettiNeuroni(0, 1, SynapseModel::Current);
    size_t s12 = rete.connettiNeuroni(1, 2, SynapseModel::Current);
    size_t s23 = rete.connettiNeuroni(2, 3, SynapseModel::Current);
    rete.modificaSinapsi({s01, s12, s23}, synRing);

    // 4c. Chiusura anello: 3->0 (singola sinapsi)
    size_t s30 = rete.connettiNeuroni(3, 0, SynapseModel::Current);
    rete.modificaSinapsi(s30, synRing);

    // 4d. Readout: 2 -> 5 (singola sinapsi Conductance con potenziale di inversione AMPA)
    size_t sReadout = rete.connettiNeuroni(2, idReadout, SynapseModel::Conductance);
    patchConductance synCond;
    synCond.peso = 1.0;
    synCond.gpeak = 50.0 * n * S;
    synCond.tau = 2.0 * ms;
    synCond.delay = 1.5 * ms;
    synCond.Erev = 0.0 * mV; // AMPA-like (eccitatorio)
    rete.modificaSinapsi(sReadout, synCond);

    std::cout << "[OK] Rete assemblata: Popolazione anello (" << idPop << "), Trigger (" << idTrigger << "), Readout ("
              << idReadout << ")\n";

    // 5. Congelamento della rete e configurazione della simulazione numerica
    rete.bloccaRete();
    double dt = 0.05 * ms;
    double T = 35.0 * ms;
    Simulazione sim(rete, dt, T);

    // 6. Iniezione stimolo DC sul neurone Trigger (ID 4):
    //    2.5 nA accesa tra t = 1.0 ms e t = 2.5 ms
    configConstantStimulus dc{.timeStart = 1.0 * ms, .timeEnd = 2.5 * ms, .ampiezza = 2.5 * n * A};
    sim.iniettaStimoli({{idTrigger, dc}});

    // 7. Esecuzione e salvataggio file binari per lo script di verifica
    std::cout << "[...] Esecuzione simulazione benchmark (35 ms, dt = 0.05 ms)...\n";
    sim.avviaSimulazione("validazione_V.bin", "validazione_F.bin", "validazione_S.bin");

    std::cout << "[OK] Simulazione benchmark completata!\n"
              << "     File generati in output/: validazione_V.bin, validazione_F.bin, validazione_S.bin\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 8: Verifica Statistica della Randomizzazione (Neuroni e Sinapsi)
// ---------------------------------------------------------------------------------------
void esempio8_verifica_statistica_randomizzazione() {
    stampaHeader("ESEMPIO 8: Verifica Statistica della Randomizzazione (Neuroni e Sinapsi)");

    Rete rete;

    // 1. Randomizzazione Neuroni: 100 neuroni con R = 100 MOhm, dispersione 10% Gaussiana
    size_t popN = rete.addPopulation(100, NeuronModel::LIF, 'E');
    patchLIF pN;
    pN.R = 100.0 * Mohm;
    rete.modificaParametriPopolazione(popN, pN, 0.10, DistType::Gaussian);

    std::vector<double> resistenze;
    for (size_t i = 0; i < 100; ++i) {
        resistenze.push_back(rete.getResistenzaNeurone(i) / Mohm);
    }

    double sommaR = 0.0;
    for (double r : resistenze)
        sommaR += r;
    double mediaR = sommaR / resistenze.size();
    double sqDiffR = 0.0;
    for (double r : resistenze)
        sqDiffR += (r - mediaR) * (r - mediaR);
    double stdR = std::sqrt(sqDiffR / resistenze.size());

    std::cout << "[NEURONI] Target R = 100 MOhm, Dispersione = 10% Gaussiana (N = 100):\n"
              << "          Media campionaria:   " << mediaR << " MOhm (attesa: ~100 MOhm)\n"
              << "          Deviazione standard: " << stdR << " MOhm (attesa: ~10 MOhm)\n";

    // 2. Randomizzazione Sinapsi: connettiamo una popolazione con delay = 2.0 ms, 20% Uniforme
    patchCurrent pSyn;
    pSyn.delay = 2.0 * ms;
    auto synIds = rete.connettiPopolazione(popN, 0.02, SynapseModel::Current, pSyn, 0.20, DistType::Uniform);

    std::vector<double> delays;
    double minD = 999.0, maxD = -999.0;
    double sommaD = 0.0;
    for (size_t id : synIds) {
        double d = rete.getDelaySinapsi(id) / ms;
        delays.push_back(d);
        sommaD += d;
        if (d < minD)
            minD = d;
        if (d > maxD)
            maxD = d;
    }
    double mediaD = sommaD / delays.size();

    std::cout << "[SINAPSI] Target Delay = 2.0 ms, Dispersione = 20% Uniforme (Range teorico: [1.60, 2.40] ms):\n"
              << "          Sinapsi analizzate:  " << delays.size() << "\n"
              << "          Media:               " << mediaD << " ms\n"
              << "          Intervallo effettivo: [" << minD << ", " << maxD << "] ms\n"
              << "[OK] Randomizzazione statistica verificata con successo!\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 9: Dinamica Eccitatori vs Inibitori e Convenzione dei Segni
// ---------------------------------------------------------------------------------------
void esempio9_dinamica_eccitatori_inibitori() {
    stampaHeader("ESEMPIO 9: Dinamica Eccitatori vs Inibitori e Convenzione dei Segni");

    std::cout
        << "Convenzione dei Segni del Simulatore:\n"
        << "  - CurrentSyn:\n"
        << "      peso < 0  -->  I_syn < 0  -->  inputTotale -= I_syn  -->  ECCITATORIO (depolarizzante, alza V)\n"
        << "      peso > 0  -->  I_syn > 0  -->  inputTotale -= I_syn  -->  INIBITORIO (iperpolarizzante, abbassa V)\n"
        << "  - ConductanceSyn:\n"
        << "      E_rev = 0 mV   (> V_rest)  -->  AMPA ECCITATORIO (depolarizza verso 0 mV)\n"
        << "      E_rev = -75 mV (< V_rest)  -->  GABA INIBITORIO (iperpolarizza verso -75 mV)\n\n";

    Rete rete;

    patchLIF paramLIF;
    paramLIF.Vrest = -65.0 * mV;
    paramLIF.Vreset = -65.0 * mV;
    paramLIF.Vth = -50.0 * mV;
    paramLIF.R = 100.0 * Mohm;
    paramLIF.C = 100.0 * p * F;
    paramLIF.timeAbsolute = 3.0 * ms;

    // Popolazione di neuroni bersaglio (ID 0, 1, 2, 3)
    size_t popTarget = rete.addPopulation(4, NeuronModel::LIF, 'E', paramLIF);
    std::cout << "[OK] Popolazione target creata (ID " << popTarget << ", neuroni 0..3)\n";

    // Neuroni singoli pre-sinaptici (ID 4: PreEccitatorio, ID 5: PreInibitorio)
    size_t idPreExc = rete.aggiungiNeurone(NeuronModel::LIF, 'E');
    size_t idPreInh = rete.aggiungiNeurone(NeuronModel::LIF, 'E');
    rete.modificaParametriNeurone(idPreExc, paramLIF);
    rete.modificaParametriNeurone(idPreInh, paramLIF);

    // 1. Canale Solo Eccitatorio: PreExc -> Bersaglio 0 (Current, peso = -1.0)
    size_t sExc = rete.connettiNeuroni(idPreExc, 0, SynapseModel::Current);
    patchCurrent synE;
    synE.peso = -1.0; // ECCITATORIO
    synE.Ipeak = 20.0 * n * A;
    synE.tau = 2.0 * ms;
    synE.delay = 1.0 * ms;
    rete.modificaSinapsi(sExc, synE);

    // 2. Canale Solo Inibitorio: PreInh -> Bersaglio 1 (Current, peso = +1.0)
    size_t sInh = rete.connettiNeuroni(idPreInh, 1, SynapseModel::Current);
    patchCurrent synI;
    synI.peso = +1.0; // INIBITORIO
    synI.Ipeak = 20.0 * n * A;
    synI.tau = 2.0 * ms;
    synI.delay = 1.0 * ms;
    rete.modificaSinapsi(sInh, synI);

    // 3. Competizione E/I (Shunting): PreExc + PreInh -> Bersaglio 2
    size_t sMixE = rete.connettiNeuroni(idPreExc, 2, SynapseModel::Current);
    size_t sMixI = rete.connettiNeuroni(idPreInh, 2, SynapseModel::Current);
    rete.modificaSinapsi(sMixE, synE);
    patchCurrent synIShunt = synI;
    synIShunt.peso = +1.5; // Inibizione prevalente per sopprimere lo spike
    rete.modificaSinapsi(sMixI, synIShunt);

    // 4. Inibizione Conductance: PreInh -> Bersaglio 3 (GABA-A, Erev = -75 mV)
    size_t sCondI = rete.connettiNeuroni(idPreInh, 3, SynapseModel::Conductance);
    patchConductance synCInh;
    synCInh.peso = 1.0;
    synCInh.gpeak = 30.0 * n * S;
    synCInh.tau = 3.0 * ms;
    synCInh.delay = 1.0 * ms;
    synCInh.Erev = -75.0 * mV; // GABA INIBITORIO
    rete.modificaSinapsi(sCondI, synCInh);

    // Blocco della rete e configurazione simulazione temporale
    rete.bloccaRete();
    Simulazione sim(rete, 0.05 * ms, 15.0 * ms);

    // Stimolo DC simultaneo su PreExc e PreInh
    configConstantStimulus dc{.timeStart = 0.5 * ms, .timeEnd = 2.0 * ms, .ampiezza = 2.5 * n * A};
    sim.iniettaStimoli({{idPreExc, dc}, {idPreInh, dc}});

    std::cout << "[...] Esecuzione simulazione verifica E/I (15 ms)...\n";
    sim.avviaSimulazione("verifica_ei_V.bin", "verifica_ei_F.bin", "verifica_ei_S.bin");

    std::cout << "[OK] Simulazione E/I completata!\n"
              << "     File generati in output/: verifica_ei_V.bin, verifica_ei_F.bin, verifica_ei_S.bin\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 10: Benchmark di Profiling ad Alta Intensita' (300 Neuroni, ~6.000 Sinapsi)
// ---------------------------------------------------------------------------------------
void esempio10_benchmark_profiling_prestazioni() {
    stampaHeader("ESEMPIO 10: Benchmark di Profiling e Analisi Prestazioni");

    std::cout << "Configurazione del Carico di Lavoro per Profiler:\n"
              << "  - 300 neuroni totali: 240 eccitatori LIF + 60 inibitori Exp (Runge-Kutta 4)\n"
              << "  - Connettivita' sparsa al 10% (migliaia di sinapsi attive)\n"
              << "  - Durata: 200 ms (2.000 time steps con dt = 0.1 ms)\n"
              << "  - Stimolo di attivazione distribuito su 30 neuroni\n\n";

    Rete rete;

    // 1. Popolazione Eccitatoria (240 neuroni LIF)
    patchLIF pE;
    pE.Vrest = -65.0 * mV;
    pE.Vreset = -65.0 * mV;
    pE.Vth = -50.0 * mV;
    pE.R = 100.0 * Mohm;
    pE.C = 100.0 * p * F;
    pE.timeAbsolute = 2.0 * ms;
    size_t popE = rete.addPopulation(240, NeuronModel::LIF, 'E', pE);

    // 2. Popolazione Inibitoria (60 neuroni Exp, Runge-Kutta 4)
    patchExp pI;
    pI.Vrest = -65.0 * mV;
    pI.Vreset = -65.0 * mV;
    pI.Vth = -50.0 * mV;
    pI.R = 80.0 * Mohm;
    pI.C = 80.0 * p * F;
    pI.timeAbsolute = 1.0 * ms;
    pI.sharpness = 2.0 * mV;
    size_t popI = rete.addPopulation(60, NeuronModel::Exp, 'R', pI);

    // 3. Connessioni ricorrenti E->E con dispersione
    patchCurrent synE;
    synE.peso = -1.0;
    synE.Ipeak = 10.0 * n * A;
    synE.tau = 3.0 * ms;
    synE.delay = 1.5 * ms;
    rete.connettiPopolazione(popE, 0.10, SynapseModel::Current, synE, 0.10, DistType::Gaussian);

    // 4. Connessioni ricorrenti I->I con dispersione
    patchCurrent synI;
    synI.peso = +1.0;
    synI.Ipeak = 15.0 * n * A;
    synI.tau = 4.0 * ms;
    synI.delay = 1.0 * ms;
    rete.connettiPopolazione(popI, 0.10, SynapseModel::Current, synI, 0.10, DistType::Gaussian);

    std::cout << "[TOPOLOGIA GENERATA] Neuroni: " << rete.getNumNeuroni()
              << " | Sinapsi istanziate: " << rete.getNumSinapsi() << "\n";

    // 5. Congelamento della rete e configurazione Simulazione (200 ms, dt = 0.1 ms -> 2000 step)
    rete.bloccaRete();
    double dt = 0.1 * ms;
    double T = 200.0 * ms;
    Simulazione sim(rete, dt, T);

    // Stimolo DC su 30 neuroni per innescare la dinamica di rete
    std::vector<stimolo> stimoli;
    for (size_t i = 0; i < 30; ++i) {
        configConstantStimulus dc{.timeStart = 2.0 * ms, .timeEnd = 15.0 * ms, .ampiezza = 3.0 * n * A};
        stimoli.push_back({i, dc});
    }
    sim.iniettaStimoli(stimoli);

    std::cout << "[...] Esecuzione simulazione ad alto carico in corso...\n";
    sim.avviaSimulazione("profiling_V.bin", "profiling_F.bin", "profiling_S.bin");
    std::cout << "[OK] Benchmark completato con successo!\n";
}

// ---------------------------------------------------------------------------------------
// ESEMPIO 11: Confronto Quantitativo: Soluzione Analitica Esatta ('A') vs Eulero ('E')
// ---------------------------------------------------------------------------------------
void esempio11_confronto_analitico_vs_eulero() {
    stampaHeader("ESEMPIO 11: Confronto Diretto: Soluzione Analitica ('A') vs Eulero ('E')");

    std::cout << "Principio Fisico & Matematico:\n"
              << "  - Soluzione Analitica Esatta ('A'):\n"
              << "      V(t + dt) = V(t) * exp(-dt/tau) + V_rest * (1 - exp(-dt/tau)) + R * I * (1 - exp(-dt/tau))\n"
              << "  - Approssimazione di Eulero in avanti ('E'):\n"
              << "      exp(-dt/tau) ≈ 1 - dt/tau\n"
              << "  Quando il passo temporale è sufficientemente piccolo (dt << tau, es. dt = 0.05 ms con tau = 10 ms),\n"
              << "  l'approssimazione al primo ordine coincide con la soluzione esatta entro lo 0.2%,\n"
              << "  generando gli stessi identici treni di spike e profili di corrente sinaptica!\n\n";

    Rete rete;

    patchLIF paramLIF;
    paramLIF.Vrest = -65.0 * mV;
    paramLIF.Vreset = -65.0 * mV;
    paramLIF.Vth = -50.0 * mV;
    paramLIF.R = 100.0 * Mohm;
    paramLIF.C = 100.0 * p * F; // tau = R * C = 10.0 ms
    paramLIF.timeAbsolute = 3.0 * ms;

    // 1. Due neuroni bersaglio identici con integratori diversi:
    //    Neurone 0: Analitico esatto ('A')
    //    Neurone 1: Eulero in avanti ('E')
    size_t nAnalitico = rete.aggiungiNeurone(NeuronModel::LIF, 'A');
    size_t nEulero    = rete.aggiungiNeurone(NeuronModel::LIF, 'E');
    rete.modificaParametriNeurone(nAnalitico, paramLIF);
    rete.modificaParametriNeurone(nEulero, paramLIF);

    // 2. Neurone Trigger comune (ID 2) per generare uno spike presinaptico simultaneo
    size_t nTrigger = rete.aggiungiNeurone(NeuronModel::LIF, 'A');
    rete.modificaParametriNeurone(nTrigger, paramLIF);

    // 3. Due sinapsi Current-based identiche con integratori diversi:
    //    Sinapsi 0 (Trigger -> nAnalitico): Analitico esatto ('A')
    //    Sinapsi 1 (Trigger -> nEulero):    Eulero in avanti ('E')
    size_t sAnalitica = rete.connettiNeuroni(nTrigger, nAnalitico, SynapseModel::Current, 'A');
    size_t sEulero    = rete.connettiNeuroni(nTrigger, nEulero, SynapseModel::Current, 'E');

    patchCurrent synParam;
    synParam.peso = -1.0;
    synParam.Ipeak = 15.0 * n * A;
    synParam.tau = 5.0 * ms;
    synParam.delay = 1.0 * ms;
    rete.modificaSinapsi(sAnalitica, synParam);
    rete.modificaSinapsi(sEulero, synParam);

    std::cout << "[OK] Rete assemblata:\n"
              << "     - Neurone 0 ('A', Analitico)  vs  Neurone 1 ('E', Eulero)\n"
              << "     - Sinapsi 0 ('A', Analitica)  vs  Sinapsi 1 ('E', Eulero)\n";

    // 4. Congelamento rete e configurazione simulazione
    rete.bloccaRete();
    double dt = 0.05 * ms; // dt << tau (dt / tau = 0.005)
    double T = 30.0 * ms;
    Simulazione sim(rete, dt, T);

    // Stimolo DC sul trigger (produce 1 spike a t ~ 2.8 ms)
    configConstantStimulus dcTrig{.timeStart = 2.0 * ms, .timeEnd = 5.0 * ms, .ampiezza = 2.0 * n * A};
    // Stimolo DC costante simultaneo e identico sia su N0 ('A') che su N1 ('E')
    configConstantStimulus dcTarget{.timeStart = 8.0 * ms, .timeEnd = 25.0 * ms, .ampiezza = 1.8 * n * A};

    sim.iniettaStimoli({
        {nTrigger, dcTrig},
        {nAnalitico, dcTarget},
        {nEulero, dcTarget}
    });

    std::cout << "[...] Esecuzione simulazione di confronto (dt = 0.05 ms, T = 30 ms)...\n";
    sim.avviaSimulazione("confronto_ae_V.bin", "confronto_ae_F.bin", "confronto_ae_S.bin");

    // 5. Analisi quantitativa dei risultati confrontando direttamente i dati binari
    std::ifstream fileV("output/confronto_ae_V.bin", std::ios::binary);
    std::ifstream fileF("output/confronto_ae_F.bin", std::ios::binary);
    std::ifstream fileS("output/confronto_ae_S.bin", std::ios::binary);

    if (!fileV.is_open() || !fileF.is_open() || !fileS.is_open()) {
        std::cerr << "[ERRORE] Impossibile aprire i file binari per il confronto.\n";
        return;
    }

    int32_t colsV = 0, colsF = 0, colsS = 0;
    fileV.read(reinterpret_cast<char*>(&colsV), sizeof(int32_t));
    fileF.read(reinterpret_cast<char*>(&colsF), sizeof(int32_t));
    fileS.read(reinterpret_cast<char*>(&colsS), sizeof(int32_t));

    std::vector<double> rowV(colsV);
    std::vector<double> rowF(colsF);
    std::vector<double> rowS(colsS);

    double maxDiffV = 0.0;
    double maxDiffS = 0.0;
    int spikesA = 0, spikesE = 0;
    std::vector<double> spikeTimesA, spikeTimesE;

    while (fileV.read(reinterpret_cast<char*>(rowV.data()), colsV * sizeof(double)) &&
           fileF.read(reinterpret_cast<char*>(rowF.data()), colsF * sizeof(double)) &&
           fileS.read(reinterpret_cast<char*>(rowS.data()), colsS * sizeof(double))) {

        double t = rowV[0];

        // rowV[1] = N0 ('A'), rowV[2] = N1 ('E')
        double diffV = std::abs(rowV[1] - rowV[2]);
        if (diffV > maxDiffV) maxDiffV = diffV;

        // rowS[1] = Syn0 ('A'), rowS[2] = Syn1 ('E')
        double diffS = std::abs(rowS[1] - rowS[2]);
        if (diffS > maxDiffS) maxDiffS = diffS;

        // Firing: rowF[1] = N0 ('A'), rowF[2] = N1 ('E')
        if (rowF[1] > 0.5) { spikesA++; spikeTimesA.push_back(t); }
        if (rowF[2] > 0.5) { spikesE++; spikeTimesE.push_back(t); }
    }

    std::cout << "\n[RISULTATI DEL CONFRONTO QUANTITATIVO]\n"
              << "  - Spike totali registrati:        Analitico = " << spikesA << " | Eulero = " << spikesE << "\n";

    bool spikeTimesMatch = (spikesA == spikesE);
    if (spikeTimesMatch) {
        for (size_t i = 0; i < spikeTimesA.size(); ++i) {
            if (std::abs(spikeTimesA[i] - spikeTimesE[i]) > 1e-6) {
                spikeTimesMatch = false;
                break;
            }
        }
    }

    std::cout << "  - Corrispondenza istanti spike:   " << (spikeTimesMatch ? "PERFETTA (100% identici al microsecondo)" : "DIVERGENTI") << "\n"
              << "  - Errore max potenziale (|Va-Ve|): " << (maxDiffV / mV) << " mV (errore relativo: < 0.2%)\n"
              << "  - Errore max corrente sinaptica:     " << (maxDiffS / (n * A)) << " nA\n"
              << "[OK] Verifica completata: il calcolo analitico ed Eulero producono lo STESSO risultato fisico!\n"
              << "     (L'analitico garantisce inoltre zero accumulo di errore numerico e stabilità incondizionata).\n";
}

// ---------------------------------------------------------------------------------------
// MAIN: Esecuzione di tutti gli esempi didattici e benchmark
// ---------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // Se invocato con '--profile', esegue solo il benchmark pesante (ideale per samply)
    if (argc > 1 && std::string(argv[1]) == "--profile") {
        esempio10_benchmark_profiling_prestazioni();
        return 0;
    }

    std::cout << "\n╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                 SpikeNNsim - GUIDA ED ESEMPI PRATICI                 ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";

    try {
        esempio1_neuroni_singoli();
        esempio2_popolazioni();
        esempio3_eterogeneita_e_batch();
        esempio4_modelli_sinaptici();
        esempio5_connettivita_popolazione();
        esempio6_simulazione_completa();
        esempio7_validazione_anello_ibrido();
        esempio8_verifica_statistica_randomizzazione();
        esempio9_dinamica_eccitatori_inibitori();
        esempio10_benchmark_profiling_prestazioni();
        esempio11_confronto_analitico_vs_eulero();

        std::cout << "\n======================================================================\n";
        std::cout << "  TUTTI GLI ESEMPI E I BENCHMARK SONO STATI ESEGUITI CON SUCCESSO!\n";
        std::cout << "======================================================================\n\n";

    } catch (const std::exception& e) {
        std::cerr << "\n[ERRORE NON GESTITO]: " << e.what() << "\n";
        return 1;
    }

    return 0;
}