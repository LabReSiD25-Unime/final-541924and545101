# Server HTTP concorrente per la gestione di prenotazioni di aule studio

## Descrizione del progetto

Questo progetto realizza un server HTTP concorrente in linguaggio C per la gestione di prenotazioni di aule studio o postazioni disponibili.  
L'obiettivo è permettere a più client di inviare richieste al server per visualizzare lo stato delle stanze, consultare le prenotazioni effettuate, inserire nuove prenotazioni e cancellarne di esistenti.

Il progetto è stato organizzato in modo modulare, separando la logica applicativa dalla parte di rete e dalla sincronizzazione.

## Caso d'uso

Il server simula un piccolo sistema di prenotazione di aule studio.

Ogni stanza ha un numero limitato di posti disponibili.  
Un client può:

- visualizzare lo stato delle stanze
- visualizzare l'elenco delle prenotazioni
- effettuare una prenotazione indicando nome utente e stanza
- cancellare una prenotazione tramite id

In presenza di più richieste concorrenti, il sistema dovrà garantire la coerenza dei dati usando meccanismi di sincronizzazione come mutex e semafori.

## Struttura del progetto

Il progetto è suddiviso in moduli.

### Moduli principali

- `booking.c / booking.h`  
  Gestione dell'archivio delle prenotazioni in memoria.

- `storage.c / storage.h`  
  Salvataggio e caricamento delle prenotazioni da file.

- `app.c / app.h`  
  Logica applicativa richiamabile dalle route del server.

- `server.c / server.h`  
  Gestione del server socket, accettazione connessioni e thread client.

- `http.c / http.h`  
  Parsing delle richieste HTTP e costruzione delle risposte.

- `routes.c / routes.h`  
  Smistamento delle richieste verso le funzioni applicative corrette.

- `sync.c / sync.h`  
  Gestione di mutex e semafori per la sincronizzazione.

- `logger.c / logger.h`  
  Logging delle richieste e delle operazioni principali.

## Funzionalità principali

Il sistema supporta le seguenti operazioni:

- visualizzazione delle stanze disponibili
- visualizzazione delle prenotazioni attive
- inserimento di una nuova prenotazione
- cancellazione di una prenotazione esistente
- salvataggio su file delle prenotazioni
- caricamento da file delle prenotazioni

## Configurazione del modello dati

Nel modulo prenotazioni sono state definite le seguenti costanti:

- `MAX_BOOKINGS = 100`
- `MAX_ROOMS = 5`
- `SLOTS_PER_ROOM = 4`

Ogni prenotazione contiene:

- id univoco
- nome dell'utente
- id della stanza
- flag di attivazione

## Endpoint previsti

Il server HTTP è pensato per supportare i seguenti endpoint:

- `GET /rooms`  
  Restituisce lo stato delle stanze con posti occupati e liberi.

- `GET /bookings`  
  Restituisce l'elenco delle prenotazioni attive.

- `POST /book`  
  Inserisce una nuova prenotazione.

- `DELETE /book?id=...`  
  Cancella una prenotazione tramite id.

## Esempi di utilizzo logico

### Visualizzazione stanze
`GET /rooms`

### Visualizzazione prenotazioni
`GET /bookings`

### Nuova prenotazione
`POST /book`

Parametri possibili:
- `user=Mario`
- `room_id=2`

### Cancellazione prenotazione
`DELETE /book?id=3`

## Persistenza dei dati

Le prenotazioni vengono salvate nel file:

`data/bookings.txt`

Il formato usato per ogni riga è il seguente:

`id;utente;room_id;active`

Esempio:

`1;Mario;2;1`

## Compilazione del test locale

Per verificare il corretto funzionamento del modulo prenotazioni senza il server HTTP, è possibile compilare il file di test.

### Se i file si trovano tutti nella stessa cartella

```bash
gcc -Wall -Wextra -o test_booking test_booking.c booking.c storage.c app.c
./test_booking