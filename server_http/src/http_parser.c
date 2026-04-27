#include "../include/http_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int parse_http_request(const char* raw_request, http_request_t* request) {
    // Puliamo la struttura per non avere residui di richieste precedenti
    memset(request, 0, sizeof(http_request_t));

    char first_line[MAX_URI_LEN + MAX_METHOD_LEN + 16];
    
    // Cerchiamo la fine della prima riga (finisce sempre con \r\n in HTTP)
    const char* line_end = strstr(raw_request, "\r\n");
    if (!line_end) return -1; // Se non c'è il ritorno a capo, la richiesta è malformata

    size_t line_len = line_end - raw_request;
    if (line_len >= sizeof(first_line)) return -1; // Protezione contro nomi troppo lunghi
    
    // Copiamo solo la prima riga per analizzarla
    strncpy(first_line, raw_request, line_len);
    first_line[line_len] = '\0';

    // Usiamo sscanf per dividere la riga: (prendiamo i primi due)
    if (sscanf(first_line, "%s %s", request->method, request->uri) != 2) {//se non si trovano almeno due stringhe la richiesta è malformata
        return -1;
    }

    // Il "body" (i dati) inizia dopo una riga vuota, quindi dopo due \r\n consecutivi
    const char* body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // Saltiamo i 4 caratteri della riga vuota (\r\n\r\n)
        request->body_len = strlen(body_start);
        
        // Copiamo i dati nel body rispettando il limite di sicurezza
        if (request->body_len < MAX_BODY_LEN) {
            strncpy(request->body, body_start, MAX_BODY_LEN - 1);
        } else {
            strncpy(request->body, body_start, MAX_BODY_LEN - 1);
            request->body_len = MAX_BODY_LEN - 1;
        }
    }

    return 0;
}