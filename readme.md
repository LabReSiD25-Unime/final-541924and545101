# Concurrent HTTP Server - Study Room Reservation

Progetto n.1 per il corso di **Laboratorio di Reti e Sistemi Distribuiti**.
Il server implementa un sistema concorrente in C per la gestione delle prenotazioni di aule studio, conforme alle specifiche **RFC 2616 (HTTP/1.1)**.

## Caratteristiche Principali
- **Multi-threading**: Gestione parallela dei client tramite `pthreads`.
- **Sincronizzazione**: Utilizzo di `mutex` per l'integrità dei dati e `semafori` per il controllo delle risorse (max 10 client).
- **Prototipo HTTP/1.1**: Supporto per i metodi `GET, POST, PUT, DELETE`.
- **Status Code**: Gestione di `200, 201, 204, 400, 401, 404, 405`.
- **Autenticazione**: Tutte le richieste richiedono l'header `X-Auth: labresid2025`.

## Struttura del Progetto
- `bin/`: Contiene l'eseguibile compilato (`server`).
- `src/`: File sorgente (`.c`).
- `include/`: File header (`.h`).
- `obj/`: File oggetto compilati.
- `scripts/`: Script Bash per test e verifica.
- `data/`: Database testuale (`bookings.txt`).
- `tests/`: Test unitari secondari.

## Come Iniziare

### Prerequisiti
- GCC (GNU Compiler Collection)
- Make
- Terminale Linux

### Compilazione
Per compilare il progetto, eseguire il comando nella root:
```bash
make
```

### Avvio del Server
```bash
./server 8080
```

### Verifica e Test
Per eseguire una verifica automatica di tutti i requisiti (metodi e status code):
```bash
./scripts/verify_all.sh
```

Per eseguire un test di carico concorrente (20 richieste parallele):
```bash
./scripts/load_test.sh
```

## API Endpoints

| Metodo | Endpoint | Descrizione | Status Code |
| :--- | :--- | :--- | :--- |
| `GET` | `/rooms` | Elenco stato aule (posti liberi/occupati) | 200, 401 |
| `GET` | `/bookings` | Elenco di tutte le prenotazioni attive | 200, 401 |
| `POST` | `/book` | Effettua una nuova prenotazione | 201, 400, 401 |
| `PUT` | `/book` | Aggiorna l'utente di una prenotazione tramite ID | 200, 404, 401 |
| `DELETE`| `/book` | Cancella una prenotazione tramite ID | 204, 404, 401 |

---
**Nota**: Per utilizzare i comandi `curl` manualmente, ricordarsi di includere l'header di autenticazione:
`curl -H "X-Auth: labresid2025" http://localhost:8080/rooms`