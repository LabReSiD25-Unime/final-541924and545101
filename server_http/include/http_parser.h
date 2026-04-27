#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

// Definiamo dei limiti massimi per evitare che richieste troppo grandi blocchino il server
#define MAX_HTTP_SIZE 8192  // Dimensione massima di una richiesta intera (8KB)
#define MAX_METHOD_LEN 8    // Lunghezza massima del metodo (es. GET, POST)
#define MAX_URI_LEN 256     // Lunghezza massima dell'indirizzo richiesto
#define MAX_BODY_LEN 4096   // Spazio massimo per i dati inviati nel corpo della richiesta

// Struttura che contiene i pezzi "smontati" della richiesta
typedef struct {
    char method[MAX_METHOD_LEN]; // Qui salviamo se è GET, POST, ecc.
    char uri[MAX_URI_LEN];       // Qui salviamo la risorsa (es. /reservations)
    char body[MAX_BODY_LEN];     // Qui finiscono i dati veri e propri (i parametri)
    int body_len;                // Quanto è lungo effettivamente il body ricevuto
} http_request_t;

// Funzione che analizza la stringa grezza e riempie la struttura sopra
int parse_http_request(const char* raw_request, http_request_t* request);

#endif