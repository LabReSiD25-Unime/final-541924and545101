#include "../include/http_parser.h"     
#include "../include/resource_manager.h" 
#include "../include/http_handler.h"    
#include <unistd.h>                    
#include <string.h>                    
#include <stdio.h>                     
#include <stdlib.h>                    
#include <sys/socket.h>                 

// Token pre-calcolato per l'autenticazione Basic (La stringa "admin:password" codificata in Base64)
#define AUTH_TOKEN "Basic YWRtaW46cGFzc3dvcmQ=" 

/**
 * Funzione di supporto (resa 'static' per essere visibile solo all'interno di questo file)
 */
static void send_response(int socket, const char* status, const char* content_type, const char* body) {
    char response[MAX_HTTP_SIZE]; // Buffer temporaneo che conterrà l'intera risposta HTTP formattata
    
    // snprintf formatta la stringa e garantisce di non superare mai MAX_HTTP_SIZE, prevenendo Buffer Overflow in uscita
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"               // Inserisce la riga di stato 
        "Content-Type: %s\r\n"          // Inserisce il tipo 
        "Content-Length: %ld\r\n"       // Inserisce la lunghezza esatta del payload in byte
        "Connection: close\r\n"         // Chiusura
        "\r\n"                          // Sequenza obbligatoria che separa fisicamente gli Header dal Body
        "%s",                           // Inserisce il contenuto effettivo 
        status, 
        content_type, 
        // Uso dell'operatore ternario (condizione ? vero : falso) per evitare crash se il body è NULL
        body ? strlen(body) : 0,        // Se body esiste ne calcolo la lunghezza, altrimenti stampo 0
        body ? body : "");              // Se body esiste lo stampo, altrimenti stampo una stringa vuota
    
    // Invia i dati sul socket. MSG_NOSIGNAL evita un SIGPIPE  se il client chiude di colpo
    send(socket, response, len, MSG_NOSIGNAL); 
}

/**
 * Funzione principale eseguita dai Worker Thread per gestire una singola richiesta HTTP.
 */
void handle_client(int client_socket, resource_manager_t* rm) {
    char buffer[MAX_HTTP_SIZE]; // Buffer di appoggio per salvare i byte in ingresso dal client
    
    // Legge i dati grezzi dal socket. sizeof(buffer) - 1 lascia lo spazio per il terminatore '\0'
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    
    // Se la read restituisce 0 (connessione chiusa) o un numero negativo (errore di rete), abortiamo
    if (bytes_read <= 0) {
        close(client_socket); // Chiudiamo il socket per non sprecare descrittori di sistema
        return;               // Il thread ha finito, torna nel pool
    }
    
    // MITIGAZIONE BUFFER OVERFLOW: Se i byte letti riempiono tutto il buffer disponibile...
    if (bytes_read == sizeof(buffer) - 1) {
        // ...rifiutiamo la richiesta con un 413 Payload Too Large per evitare che richieste enormi consumino RAM
        send_response(client_socket, "413 Payload Too Large", "text/plain", "Payload exceeds maximum allowed size");
        close(client_socket); // Chiudiamo la connessione malevola/errata
        return;
    }
    
    buffer[bytes_read] = '\0'; // Inseriamo esplicitamente il terminatore di stringa per permettere alle funzioni C di leggerlo in sicurezza

    // PARSING: Dichiariamo la struttura vuota per ospitare i dati analizzati
    http_request_t request; 
    
    // Passiamo il buffer grezzo al parser. Se restituisce != 0, significa che la sintassi HTTP era scorretta
    if (parse_http_request(buffer, &request) != 0) {
        // Segnaliamo l'errore semantico al client con un 400 Bad Request
        send_response(client_socket, "400 Bad Request", "text/plain", "Invalid HTTP Request");
        close(client_socket);
        return;
    }

    // SICUREZZA (Basic Auth): Cerchiamo nel buffer l'header "Authorization" e il nostro token Base64
    if (strstr(buffer, "Authorization: ") == NULL || strstr(buffer, AUTH_TOKEN) == NULL) {
        // Se manca o è sbagliato, inviamo 401 e forziamo il browser a mostrare il popup di login tramite WWW-Authenticate
        send_response(client_socket, "401 Unauthorized", "text/plain", "Authentication Required\r\nWWW-Authenticate: Basic realm=\"Restricted\"");
        close(client_socket);
        return;
    }

    // --- INIZIO GESTIONE DEI METODI CRUD (Create, Read, Update, Delete) ---

    // 1. READ (Metodo GET): Richiesta di lettura dell'intero database
    if (strcmp(request.method, "GET") == 0) { // strcmp ritorna 0 se le stringhe sono identiche
        char response_body[MAX_HTTP_SIZE]; // Buffer per contenere la stringa JSON generata
        
        // Chiamiamo il database che prenderà il lock (Mutex), formatterà il JSON e lo metterà in response_body
        rm_read_all(rm, response_body, sizeof(response_body));
        
        // Inviamo il JSON al client con stato di successo 200 OK
        send_response(client_socket, "200 OK", "application/json", response_body);

    // 2. CREATE (Metodo POST): Inserimento di una nuova prenotazione da form HTML
    } else if (strcmp(request.method, "POST") == 0) {
        char room[MAX_STR_LEN], student[MAX_STR_LEN], time[MAX_STR_LEN]; // Variabili temporanee per i campi del form
        
        // Estrazione dati dal payload (es: room=Aula_A&student=Mario&time=10). 
        // Il pattern %[^&] significa: "leggi tutto finché non incontri il carattere '&'"
        if (sscanf(request.body, "room=%[^&]&student=%[^&]&time=%s", room, student, time) == 3) { // Se trova esattamente 3 valori
            
            // Tenta di inserire la prenotazione nel Resource Manager 
            int id = rm_create(rm, room, student, time);
            
            char res[64]; // Piccolo buffer per preparare la risposta JSON contenente il nuovo ID
            snprintf(res, sizeof(res), "{\"id\": %d}", id); // Formattiamo la stringa JSON {"id": X}
            
            // Rispondiamo con il codice standard per le risorse appena create: 201 Created
            send_response(client_socket, "201 Created", "application/json", res);
        } else {
            // Se la sscanf non trova 3 valori, i parametri passati dal client erano incompleti o malformati
            send_response(client_socket, "400 Bad Request", "text/plain", "Missing reservation parameters");
        }

    // 3. UPDATE (Metodo PUT): Modifica di una prenotazione esistente tramite ID
    } else if (strcmp(request.method, "PUT") == 0) {
        int id; // Variabile per estrarre l'ID
        char room[MAX_STR_LEN], student[MAX_STR_LEN], time[MAX_STR_LEN];
        
        // Prima sscanf sull'URI: estrae l'ID dal percorso (es: /reservations/5)
        // Seconda sscanf sul Body: estrae i nuovi parametri aggiornati
        if (sscanf(request.uri, "/reservations/%d", &id) == 1 && 
            sscanf(request.body, "room=%[^&]&student=%[^&]&time=%s", room, student, time) == 3) {
            
            // Chiamiamo l'aggiornamento. rm_update restituisce 0 in caso di successo
            if (rm_update(rm, id, room, student, time) == 0) {
                // Modifica effettuata, confermiamo con un generico 200 OK
                send_response(client_socket, "200 OK", "application/json", "{\"status\": \"updated\"}");
            } else {
                // Se rm_update fallisce, significa che l'ID fornito non esiste nell'array
                send_response(client_socket, "404 Not Found", "text/plain", "Reservation not found");
            }
        } else {
            // Se le regex di sscanf falliscono, l'URI o il Body non rispettano il formato atteso
            send_response(client_socket, "400 Bad Request", "text/plain", "Invalid URI or body");
        }

    // 4. DELETE (Metodo DELETE): Cancellazione di una prenotazione tramite ID
    } else if (strcmp(request.method, "DELETE") == 0) {
        int id; // Variabile per contenere l'ID del target da eliminare
        
        // Estraiamo l'ID dall'indirizzo richiesto
        if (sscanf(request.uri, "/reservations/%d", &id) == 1) {
            
            // rm_delete eseguirà la de-frammentazione (shift a sinistra) e restituirà 0 se ha successo
            if (rm_delete(rm, id) == 0) {
                // Lo standard HTTP per una cancellazione a buon fine che non richiede ulteriori dati in risposta è il 204
                send_response(client_socket, "204 No Content", "text/plain", "");
            } else {
                // Se rm_delete fallisce, il record non c'è più o non c'è mai stato
                send_response(client_socket, "404 Not Found", "text/plain", "Reservation not found");
            }
        } else {
            // L'ID non era un numero valido
            send_response(client_socket, "400 Bad Request", "text/plain", "Invalid URI");
        }

    // CASO DI DEFAULT: Il client usa un metodo HTTP che non abbiamo implementato 
    } else {
        // Restituiamo il codice specifico per i metodi rifiutati
        send_response(client_socket, "405 Method Not Allowed", "text/plain", "Method not supported");
    }

    // FASE FINALE: La richiesta è stata gestita, chiudiamo il socket per terminare il protocollo TCP con questo client
    close(client_socket);
}