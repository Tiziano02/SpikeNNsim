# Developer Notes

Coming soon. È un file privato per gli sviluppatori. Inserisco come funziona la simulazione, sia temporalmente, quali sono gli output e le connessioni tra uno step e l'altro. Deve essere il diario di bordo della simulazione. Deve avere questa struttura : 

1) Spiegazione dei vari metodi update, step e simulazione (le parti del codice rilevanti)
2) Problemi di ottimizzazione del codice
3) Problemi di output dei dati
4) Problemi di input della simulazione
5) Dove e come implementare le nuove features

---

## Note Architetturali e Design delle API

### Pattern "Patch" per la modifica dei parametri
- **Scelta di design:** I parametri fisici di neuroni e sinapsi vengono aggiornati tramite strutture dati `patch` (`patchLIF`, `patchExp`, `patchCurrent`, `patchConductance`) basate su `std::optional<double>`.
- **Vantaggi tecnici:**
  1. *Zero allocazioni sullo heap:* La patch è un POD/aggregato allocato sullo stack, preservando la località di cache sui vettori contigui di `Rete`.
  2. *Compile-time Type Safety:* L'IDE e il compilatore validano i nomi dei parametri (a differenza delle string-map usate in altri simulatori come NEST/Brian).
  3. *C++20 Designated Initializers:* Sintassi compatta inline a riga singola:
     ```cpp
     rete.modificaParametriPopolazione(0, patchLIF{.C = 30 * n * F, .R = 1.2 * Mohm});
     ```

### Dispersione eterogenea per parametro (Zero-Cost Heterogeneity)
Poiché ogni `patch` modifica esclusivamente i campi che contengono un valore (`has_value()`), è possibile applicare **dispersioni relative differenti a parametri differenti** eseguendo chiamate mirate in sequenza, senza bisogno di strutture dati complesse:

```cpp
// Esempio 1: Dispersione del 10% sulla resistenza di membrana R
rete.modificaParametriPopolazione(popId, patchLIF{.R = 1.0 * Mohm}, 0.10);

// Esempio 2: Dispersione del 2% sul potenziale di soglia Vth
rete.modificaParametriPopolazione(popId, patchLIF{.Vth = -50.0 * mV}, 0.02);
```

### Idee Future: Syntactic Sugar su `Popolazione`
Per rendere l'interfaccia ancora più amichevole per l'utente finale, si possono aggiungere metodi helper (fluent interface) sulla classe `Popolazione` che internamente costruiscono e applicano la patch:
```cpp
// Idea futura: helper methods per la popolazione
pop.setResistenza(1.2 * Mohm);
pop.setCapacita(100.0 * p * F);
```
*(Da valutare in una fase di rifinitura delle API utente, lasciando intatto il motore a Patch sottostante).*

---

### Inizializzazione Potenziale di Membrana $V(0)$
In neuroscienze computazionali non si lasciano quasi mai tutti i neuroni identici al valore fisso di riposo ($V_{rest} = -65\text{ mV}$) per evitare artefatti di sincronizzazione iniziale (*sync transient*). 
L'inizializzazione dei potenziali è gestita da tre metodi espliciti dedicati, in cui la diversa tipologia di distribuzione è determinata dal parametro `DistType`:

1. **`setCondizioneInizialeRete(double V, double dispersione = 0.0, DistType tipoDist = DistType::Gaussian)`**: per tutta la rete.
2. **`setCondizioneInizialePopolazione(size_t popId, double V, double dispersione = 0.0, DistType tipoDist = DistType::Gaussian)`**: per una specifica popolazione.
3. **`setCondizioneInizialeIntervallo(size_t start, size_t end, double V, double dispersione = 0.0, DistType tipoDist = DistType::Gaussian)`**: per un intervallo [start, end).

Esempi di utilizzo:
- **Gaussiana attorno al riposo:**
  ```cpp
  rete.setCondizioneInizialePopolazione(popId, -65.0 * mV, 0.05, DistType::Gaussian); // N(-65 mV, 5%)
  ```
- **Uniforme:**
  ```cpp
  rete.setCondizioneInizialePopolazione(popId, -57.5 * mV, 0.10, DistType::Uniform);  // Uniforme attorno alla media
  ```
- **Deterministica (nessuna dispersione, default 0.0):**
  ```cpp
  rete.setCondizioneInizialePopolazione(popId, -65.0 * mV);                            // Esattamente -65 mV
  ```
- **Su tutta la rete:**
  ```cpp
  rete.setCondizioneInizialeRete(-65.0 * mV, 0.05, DistType::Gaussian);
  ```

Sotto il cofano delega al motore a Patch minimale (`patchLIF{.V = V}` o `patchExp{.V = V}`) e sincronizza `statoNeuroni_` e `statoFiring_`.

---

### Rete Benchmark di Validazione Analitica (`script/verifica_validazione.py`)
Per verificare la correttezza numerica end-to-end del simulatore è stata implementata una rete ibrida ad anello (in `src/main.cpp` - Esempio 7):
- **Popolazione:** 4 neuroni LIF (`popRing`, ID 0..3) con delay di conduzione inter-nodo $\Delta t_{syn} = 2.0\text{ ms}$.
- **Neurone Singolo Trigger:** ID 4 (LIF, Eulero) eccitato da uno stimolo DC calibrato per generare 1 solo spike.
- **Neurone Singolo Readout:** ID 5 (Exp, Runge-Kutta 4) connesso a N2 tramite sinapsi Conductance-based AMPA ($E_{rev} = 0\text{ mV}$).
- **Comportamento analitico atteso:**
  1. $t_{hop} = \Delta t_{syn} + \text{tempo salita} \approx 2.10\text{ ms}$.
  2. Periodo anello $T_{ring} = 4 \times 2.10\text{ ms} = 8.40\text{ ms}$.
  3. Propagazione fedele e sincrona sul neurone Readout con ritardo costante $\approx 2.15\text{ ms}$.
Lo script Python `script/verifica_validazione.py` legge i file binari generati e verifica automaticamente tutte le asserzioni analitiche senza dipendenze esterne.

---

### Meccanismo di Congelamento della Rete (`bloccaRete()` / `isBloccata()`)
Prima dell'avvio della simulazione, la topologia della rete e i parametri statici devono essere congelati:
```cpp
rete.bloccaRete(); // oppure: rete.blocca();
```

#### Regole e Comportamento:
1. **Obbligatorio per la Simulazione:** Il costruttore `Simulazione::Simulazione(rete, dt, T)` e il metodo `avviaSimulazione()` verificano `rete.isBloccata()`. Se la rete non è stata bloccata, viene sollevata un'eccezione `std::logic_error`.
2. **Protezione dei Parametri Statici:** Qualsiasi chiamata successiva a metodi che modificano la topologia o i parametri (`modificaParametri*`, `aggiungiNeurone`, `connetti*`, `setCondizioneIniziale*`, ecc.) solleva un'eccezione `std::logic_error` che interrompe immediatamente l'operazione.
3. **Evoluzione Dinamica Preservata:** Durante il loop di simulazione, l'avanzamento temporale `step(dt)` e il solutore numerico ODE aggiornano i potenziali di membrana $V(t)$, le correnti sinaptiche $I_{syn}(t)$ e i treni di spike come previsto.
4. **Compile-time (Typestate Pattern) vs Runtime Guard:**
   - *A tempo di compilazione:* In C++ è possibile impedire le chiamate a tempo di compilazione tramite il **Typestate Pattern**, in cui `rete.blocca()` consuma `Rete` restituendo una classe distinta `ReteBloccata` priva dei metodi di modifica.
   - *A runtime (implementazione attuale):* Una **Runtime State Guard** con `bool isBloccata_` e `std::logic_error` mantiene un'unica classe `Rete` coerente ed ergonomica, con messaggi di diagnostica dettagliati ed espliciti.

---

### Integrazione Analitica Esatta (`'A'`) vs Integratori Numerici (`'E'`, `'R'`)
Per modelli fisici con equazioni differenziali lineari a tratti, SpikeNNsim supporta la soluzione analitica esatta in forma chiusa (stile NEST / Brian 2 *Exact Integration*):

1. **Neurone `LIF` (`tipoIntegratore_ = 'A'`):**
   - L'equazione di membrana $\tau \frac{dV}{dt} = -(V - V_{rest}) + R \cdot I(t)$ tra $t$ e $t + dt$ con $I$ costante nello step ammette soluzione esatta:
     $$V(t + dt) = V(t) \cdot P + C_1 + C_2 \cdot I(t)$$
     dove i coefficienti dipendono solo dalla struttura fissa del neurone e dal passo $dt$:
     - $P = e^{-dt / \tau}$
     - $C_1 = V_{rest} \cdot (1 - P)$
     - $C_2 = R \cdot (1 - P)$
     - $P_{th} = e^{-dt / \tau_{rel}}$ (decadimento esatto della soglia adattiva $V_{th}$)
   - In `LIF::prepare(dt)`, $P, C_1, C_2, P_{th}$ vengono calcolati una volta sola. Durante la simulazione, l'aggiornamento richiede appena **2 moltiplicazioni e 2 addizioni** per step temporale, con **zero errore di troncamento** e stabilità incondizionata.

2. **Sinapsi (`CurrentSyn` e `ConductanceSyn` con `'A'`, `'E'`, `'R'`):**
   - **Simmetria completa con i Neuroni:** Le sinapsi supportano la selezione dell'integratore (`tipoIntegratore_ = 'A' | 'E' | 'R'`) sia alla creazione (`connettiNeuroni`, `connettiPopolazione`, `connettiPopolazioni`), sia a runtime tramite `modificaIntegratoreSinapsi(idx, typeIntegratore)`.
   - Con `'A'` (Analitico esatto), il decadimento naturale continuo $\frac{dI}{dt} = -\frac{I}{\tau_{syn}}$ (o $\frac{dg}{dt} = -\frac{g}{\tau_{syn}}$) adotta la soluzione esatta $e^{-dt / \tau_{syn}}$, precalcolata in `setDelayRing(dt)`:
     `Isyn_ *= decayFactor_;`
   - Con `'E'` adotta Eulero in avanti (`Isyn_ += dt * (-Isyn_ / tau_)`), e con `'R'` adotta Runge-Kutta 4.
   - La compatibilità con la **plasticità sinaptica (STDP)** è preservata al 100%: l'apprendimento modifica l'efficacia $w(t)$ negli spike discreti, mentre il decadimento temporale tra spike segue l'integratore selezionato.

3. **Neurone `Exp`:**
   - Data la presenza del termine non-lineare $\Delta_T \exp\left(\frac{V - V_T}{\Delta_T}\right)$, non esiste una soluzione in forma chiusa; `Exp` adotta solutori numerici (Runge-Kutta 4 `'R'` o Eulero `'E'`). Se viene richiesto `'A'`, fa fallback trasparente su `'R'`.

4. **Equivalenza Fisica Analitico vs Eulero per $dt \ll \tau$ (`main.cpp` - Esempio 11):**
   - Espandendo in serie di Taylor il fattore di decadimento esponenziale:
     $$e^{-dt / \tau} = 1 - \frac{dt}{\tau} + \frac{1}{2}\left(\frac{dt}{\tau}\right)^2 - \mathcal{O}\left(\left(\frac{dt}{\tau}\right)^3\right)$$
   - Quando $\frac{dt}{\tau} \ll 1$ (es. $dt = 0.05\text{ ms}$ con $\tau = 10\text{ ms} \implies \frac{dt}{\tau} = 0.005$), il termine del primo ordine $(1 - dt/\tau)$ coincide con l'equazione di Eulero in avanti.
   - Di conseguenza:
     - I treni di spike emessi da un neurone LIF con `'A'` e con `'E'` sono **100% identici al microsecondo** (stesso numero di spike, jitter temporale $\Delta t_{spike} = 0.0\text{ ms}$).
     - La differenza di potenziale di membrana $\max |V_A(t) - V_E(t)| < 0.09\text{ mV}$ (errore relativo $< 0.15\%$).
     - La differenza nella corrente sinaptica $\max |I_{A}(t) - I_{E}(t)| < 0.03\text{ nA}$.
   - **Perché preferire l'Analitico `'A'`?**
     - **Precisione esatta:** Zero accumulo di errore di troncamento sui lunghi orizzonti temporali.
     - **Stabilità incondizionata:** Non risente del limite di Nyquist/Courant ($dt < \tau$), non esplode mai numericamente.
     - **Costo computazionale inferiore:** Nel loop di simulazione richiede solo 2 moltiplicazioni e 2 addizioni (nessuna divisione o ricalcolo di derivate).

5. **Modifica a Runtime del Risolutore Numerico su 4 Livelli di Granularità:**
   - SpikeNNsim offre la stessa simmetria e granularità sia per la modifica dei parametri fisici sia per la selezione dell'integratore (`'A'`, `'E'`, `'R'`):
     1. **Singolo elemento:**
        - `rete.modificaIntegratoreNeurone(idx, type)`
        - `rete.modificaIntegratoreSinapsi(idx, type)`
     2. **Lista batch di indici:**
        - `rete.modificaIntegratoreNeuroni(idxs, type)`
        - `rete.modificaIntegratoreSinapsi(idxs, type)`
     3. **Popolazione intera:**
        - `rete.modificaIntegratorePopolazione(idxPop, type)`
        - `rete.modificaIntegratoreSinapsiPopolazione(idxPop, type)`
     4. **Sub-popolazione:**
        - `rete.modificaIntegratoreSubPopolazione(idxPop, idxSubPop, type)`
        - `rete.modificaIntegratoreSinapsiSubPopolazione(idxPop, idxSubPop, type)`
   - Ciascun metodo verifica lo stato di congelamento (`verificaNonBloccata`) e valida rigorosamente i limiti dimensionali.

---

### Sinapsi a Delta Istantanee (senza decadimento esponenziale)
SpikeNNsim supporta la modalità **Sinapsi a Delta** tramite la flag booleana `isDelta` (riservata alle sole sinapsi **Current-based**):
1. **Modello Fisico & Matematico:**
   - *Sinapsi classica con decadimento continuo:* Quando arriva uno spike dopo il `delay`, la corrente riceve un incremento e decade secondo $\frac{dI}{dt} = -\frac{I}{\tau}$.
   - *Sinapsi a delta (`isDelta = true`):* L'impulso viene trasmesso istantaneamente ed è attivo **per un singolo time step $dt$** (all'arrivo dal delay ring). Al time step successivo, la corrente si azzera istantaneamente ($I_{syn} = 0$) senza code o decadimenti esponenziali nel tempo.
2. **Esclusività per Current-based:**
   - La modalità a delta è supportata e applicabile **esclusivamente alle sinapsi Current-based** (`CurrentSyn`), dove rappresenta una scarica impulsiva istantanea di carica.
   - Le sinapsi Conductance-based (`ConductanceSyn`) mantengono invece la loro naturale cinetica biofisica continua di conduttanza di canale ($g_{syn}(t)$).
3. **Livelli di Configurazione:**
   - **Tramite Patch:** `patchCurrent{.isDelta = true}` (usabile sia in `connettiNeuroni` / `connettiPopolazione`, sia in `modificaSinapsi`).
   - **Tramite API Dedicate:**
     - Singola sinapsi: `rete.modificaDeltaSinapsi(idx, bool isDelta)`
     - Batch di indici: `rete.modificaDeltaSinapsi(idxs, bool isDelta)`
     - Popolazione: `rete.modificaDeltaSinapsiPopolazione(idxPop, bool isDelta)`
     - Sub-popolazione: `rete.modificaDeltaSinapsiSubPopolazione(idxPop, idxSubPop, bool isDelta)`
   - **Metodo di ispezione:** `rete.isSinapsiDelta(idx)`






