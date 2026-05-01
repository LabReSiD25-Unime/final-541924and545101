#include "../include/http_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int parse_http_request(const char* raw_request, http_request_t* request) {
    // Puliamo la struttura settando tutti i byte a 0 per non avere residui sporchi di richieste precedenti in memoria
    memset(request, 0, sizeof(http_request_t));

    char first_line[MAX_URI_LEN + MAX_METHOD_LEN + 16]; // Buffer temporaneo 
    
    // Cerchiamo la fine della prima riga: strstr cerca la sottostringa "\r\n"
    const char* line_end = strstr(raw_request, "\r\n");
    if (!line_end) return -1; // Se non c'è il ritorno a capo, la richiesta è malformata 
    size_t line_len = line_end - raw_request; // Calcoliamo la lunghezza della prima riga tramite aritmetica dei puntatori
    if (line_len >= sizeof(first_line)) return -1; // Protezione contro nomi/URL malevoli troppo lunghi
    
    // Copiamo solo la prima riga nel buffer temporaneo per analizzarla in sicurezza
    strncpy(first_line, raw_request, line_len);
    first_line[line_len] = '\0'; // strncpy non aggiunge sempre il terminatore, lo forziamo manualmente

    // Usiamo sscanf per dividere la riga usando gli spazi come separatore (estraiamo Metodo e URI)
    if (sscanf(first_line, "%s %s", request->method, request->uri) != 2) {
        return -1; // Se non si trovano almeno due stringhe valide, la richiesta è malformata
    }

    // Il "body" (i dati) inizia dopo gli header, segnalato da una riga vuota, quindi due \r\n consecutivi
    const char* body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // Spostiamo il puntatore in avanti di 4 byte per saltare i caratteri "\r\n\r\n"
        request->body_len = strlen(body_start); // Misuriamo i dati rimanenti
        
        // Copiamo i dati nel body rispettando rigorosamente il limite di sicurezza del nostro buffer
        if (request->body_len < MAX_BODY_LEN) {
            strncpy(request->body, body_start, MAX_BODY_LEN - 1);
        } else {
            // Se il payload è più grande del consentito, lo tronchiamo per evitare buffer overflow 
            strncpy(request->body, body_start, MAX_BODY_LEN - 1);
            request->body_len = MAX_BODY_LEN - 1;
        }
    }

    return 0; // Restituisce 0 in caso di parsing avvenuto con successo
}