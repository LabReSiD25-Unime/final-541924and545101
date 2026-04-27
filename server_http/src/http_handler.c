#include "../include/http_parser.h"
#include "../include/resource_manager.h"
#include "../include/http_handler.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

// Token pre-calcolato per l'autenticazione Basic (admin:password in Base64)
#define AUTH_TOKEN "Basic YWRtaW46cGFzc3dvcmQ=" 

/**
 * Funzione di supporto per costruire e inviare la risposta HTTP al client.
 * Rispetta rigorosamente lo standard RFC 2616 (HTTP/1.1).
 */
static void send_response(int socket, const char* status, const char* content_type, const char* body) {
    char response[MAX_HTTP_SIZE];
    
    // snprintf garantisce di non eccedere la dimensione del buffer 'response'
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"               // Riga di stato (es. 200 OK)
        "Content-Type: %s\r\n"          // Tipo di contenuto (es. application/json)
        "Content-Length: %ld\r\n"       // Dimensione del corpo della risposta
        "Connection: close\r\n"         // Comunichiamo al client che chiuderemo la connessione
        "\r\n"                          // Carattere di separazione OBBLIGATORIO tra header e body
        "%s",                           // Corpo della risposta
        status, content_type, body ? strlen(body) : 0, body ? body : "");
    
    // Invia i dati sul socket. MSG_NOSIGNAL evita che il server crashi se il client chiude il socket improvvisamente.
    send(socket, response, len, MSG_NOSIGNAL);
}

/**
 * Funzione principale eseguita dai Worker Thread per gestire una singola richiesta.
 */
void handle_client(int client_socket, resource_manager_t* rm) {
    char buffer[MAX_HTTP_SIZE];
    
    // Legge i dati grezzi dal socket. bytes_read contiene il numero di byte effettivamente ricevuti.
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    
    // Gestione errore di lettura o connessione chiusa prematuramente.
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    // MITTIGAZIONE BUFFER OVERFLO: Se la richiesta satura il buffer, la rifiutiamo.
    if (bytes_read == sizeof(buffer) - 1) {
        send_response(client_socket, "413 Payload Too Large", "text/plain", "Payload exceeds maximum allowed size");
        close(client_socket);
        return;
    }
    
    buffer[bytes_read] = '\0'; // Aggiunge il terminatore di stringa ai dati letti.

    // PARSING: Trasforma la stringa grezza in una struttura organizzata (Metodo, URI, Body).
    http_request_t request;
    if (parse_http_request(buffer, &request) != 0) {
        send_response(client_socket, "400 Bad Request", "text/plain", "Invalid HTTP Request");
        close(client_socket);
        return;
    }

    // Verifica se l'header Authorization contiene il token corretto.
    if (strstr(buffer, "Authorization: ") == NULL || strstr(buffer, AUTH_TOKEN) == NULL) {
        // Se manca l'autenticazione, invia 401 e specifica il "realm" come da standard.
        send_response(client_socket, "401 Unauthorized", "text/plain", "Authentication Required\r\nWWW-Authenticate: Basic realm=\"Restricted\"");
        close(client_socket);
        return;
    }

    //  GESTIONE DEI METODI CRUD

    // 1. READ (Lettura di tutte le prenotazioni)
    if (strcmp(request.method, "GET") == 0) {
        char response_body[MAX_HTTP_SIZE];
        // Accede al database in RAM tramite il resource manager.
        rm_read_all(rm, response_body, sizeof(response_body));
        send_response(client_socket, "200 OK", "application/json", response_body);

    // 2. CREATE (Inserimento nuova prenotazione)
    } else if (strcmp(request.method, "POST") == 0) {
        char room[MAX_STR_LEN], student[MAX_STR_LEN], time[MAX_STR_LEN];
        // Estrae i parametri dal corpo form-urlencoded (es: room=Aula_MIFT&student=Massimo...).
        if (sscanf(request.body, "room=%[^&]&student=%[^&]&time=%s", room, student, time) == 3) {
            // Chiama rm_create che gestisce l'accesso esclusivo tramite Mutex.
            int id = rm_create(rm, room, student, time);
            char res[64];
            snprintf(res, sizeof(res), "{\"id\": %d}", id);
            send_response(client_socket, "201 Created", "application/json", res);
        } else {
            send_response(client_socket, "400 Bad Request", "text/plain", "Missing reservation parameters");
        }

    // 3. UPDATE (Modifica di una prenotazione esistente)
    } else if (strcmp(request.method, "PUT") == 0) {
        int id;
        char room[MAX_STR_LEN], student[MAX_STR_LEN], time[MAX_STR_LEN];
        // Estrae l'ID dall'URI (es: /reservations/1) e i dati dal body.
        if (sscanf(request.uri, "/reservations/%d", &id) == 1 && 
            sscanf(request.body, "room=%[^&]&student=%[^&]&time=%s", room, student, time) == 3) {
            if (rm_update(rm, id, room, student, time) == 0) {
                send_response(client_socket, "200 OK", "application/json", "{\"status\": \"updated\"}");
            } else {
                send_response(client_socket, "404 Not Found", "text/plain", "Reservation not found");
            }
        } else {
            send_response(client_socket, "400 Bad Request", "text/plain", "Invalid URI or body");
        }

    // 4. DELETE (Cancellazione di una prenotazione)
    } else if (strcmp(request.method, "DELETE") == 0) {
        int id;
        if (sscanf(request.uri, "/reservations/%d", &id) == 1) {
            // Qui rm_delete effettuerà lo "shift a sinistra" per ricompattare l'array.
            if (rm_delete(rm, id) == 0) {
                send_response(client_socket, "204 No Content", "text/plain", "");
            } else {
                send_response(client_socket, "404 Not Found", "text/plain", "Reservation not found");
            }
        } else {
            send_response(client_socket, "400 Bad Request", "text/plain", "Invalid URI");
        }

    } else {
        // Se il client usa un metodo non implementato (es. PATCH o HEAD).
        send_response(client_socket, "405 Method Not Allowed", "text/plain", "Method not supported");
    }

    // Chiusura del socket: il thread ha finito il suo compito e torna nel pool.
    close(client_socket);
}