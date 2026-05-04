#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/thread_pool.h"
#include "../include/resource_manager.h"
#include "../include/http_handler.h"

#define _GNU_SOURCE
#define PORT 8080
#define THREAD_COUNT 10

// Struttura per passare i dati necessari al worker thread
typedef struct {
    int client_socket;       // Il socket del singolo cliente appena accettato
    resource_manager_t* rm;  // Il database condiviso delle prenotazioni
} client_task_t;

// Funzione eseguita dai thread del pool per gestire l'elaborazione di una singola connessione client in modo indipendente rispetto al thread principale.
void job_handler(void* arg) {
    client_task_t* task = (client_task_t*)arg; // Eseguiamo il cast del puntatore generico alla nostra struttura
    if (task) { // Controlliamo che il puntatore non sia nullo per sicurezza
        handle_client(task->client_socket, task->rm); // Gestisce la richiesta HTTP delegando al livello applicativo
        free(task); // libera la memoria allocata dinamicamente nel main per evitare memory leak, in quanto limitata e condivisa
    }
}

int main() {
    int server_fd, client_socket; // Descrittori per il socket in ascolto e per i client connessi
    struct sockaddr_in address;   // Struttura standard per definire indirizzo IP e porta 
    int opt = 1;                  // Variabile di opzione usata per configurare il comportamento del socket
    int addrlen = sizeof(address);// Calcoliamo la dimensione della struttura, richiesta dalla system call accept()

    // 1) Inizializziamo il database e il pool di thread (10 worker)
    resource_manager_t* rm = rm_init(); // Alloca e prepara il database in memoria e il suo Mutex
    thread_pool_t* pool = thread_pool_init(THREAD_COUNT); // Avvia i 10 thread che si mettono subito in attesa 

    // 2) Creiamo il socket di ascolto 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); // Stampa l'errore specifico del sistema operativo in caso di fallimento
        exit(EXIT_FAILURE);      // Termina il programma con codice di errore
    }

    // 3) Opzione per riutilizzare la porta subito dopo il riavvio
    // Previene l'errore "Address already in use" se il server viene riavviato rapidamente
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configurazione dell'indirizzo del server
    address.sin_family = AF_INET; // Specifichiamo che useremo la famiglia di protocolli IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Accetta connessioni da qualsiasi IP 
    address.sin_port = htons(PORT);       // Porta 8080 convertita in formato di rete (Big Endian) per compatibilità

    // 4) Colleghiamo il socket all'indirizzo e alla porta
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { 
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 5) Mettiamo il server in ascolto di nuove connessioni
    // Il parametro '3' è il backlog: la lunghezza massima della coda delle connessioni in attesa a livello di kernel
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);

    // 6) Ciclo infinito: accetta  e delega il lavoro al pool
    while (1) {
        // accept() è bloccante: il main thread si ferma qui finché non arriva una richiesta TCP
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Se un'accettazione fallisce, passa direttamente alla prossima iterazione senza crashare
        }

        // Creiamo dinamicamente il task per il thread worker
        client_task_t* task = (client_task_t*)malloc(sizeof(client_task_t)); // Alloca memoria nell'heap per il task
        if (!task) { // Controllo di sicurezza: verifica che ci sia memoria RAM disponibile
            perror("Errore allocazione task");
            close(client_socket); // Se non c'è memoria, chiudiamo il client per non lasciarlo appeso
            continue;
        }
        
        // Popoliamo la struttura con il nuovo socket e il riferimento al database
        task->client_socket = client_socket;
        task->rm = rm;

        // Aggiungiamo il lavoro alla coda del thread pool
        thread_pool_add_job(pool, job_handler, task); // Questo farà scattare un sem_post svegliando un worker
    }

    // Pulizia finale (teorica, perché il server gira all'infinito nel while(1))
    thread_pool_destroy(pool);
    rm_destroy(rm);
    return 0; // Termina il processo principale
}