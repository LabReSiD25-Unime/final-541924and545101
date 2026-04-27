#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/thread_pool.h"
#include "../include/resource_manager.h"
#include "../include/http_handler.h"

#define PORT 8080
#define THREAD_COUNT 10

// Struttura per passare i dati necessari al worker thread
typedef struct {
    int client_socket;       // Il socket del singolo cliente
    resource_manager_t* rm;  // Il database condiviso delle prenotazioni
} client_task_t;

// Funzione eseguita dai thread del pool per gestire l'elaborazione di una singola connessione client in modo indipendente rispetto al thread principale.
void job_handler(void* arg) {
    client_task_t* task = (client_task_t*)arg;
    if (task) {
        handle_client(task->client_socket, task->rm); // Gestisce la richiesta HTTP
        free(task); // Fondamentale: libera la memoria allocata nel main
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 1) Inizializziamo il database e il pool di thread (10 worker)
    resource_manager_t* rm = rm_init();
    thread_pool_t* pool = thread_pool_init(THREAD_COUNT);

    // 2) Creiamo il socket di ascolto (IPv4, TCP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 3) Opzione per riutilizzare la porta subito dopo il riavvio
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Accetta connessioni da qualsiasi IP
    address.sin_port = htons(PORT);       // Porta 8080 convertita in formato di rete

    // 4) Colleghiamo il socket all'indirizzo e alla porta
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 5) Mettiamo il server in ascolto di nuove connessioni
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);

    // 6) Ciclo infinito: accetta un cliente e delega il lavoro al pool
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        // Creiamo dinamicamente il task per il thread worker
        client_task_t* task = (client_task_t*)malloc(sizeof(client_task_t));
        if (!task) {
            perror("Errore allocazione task");
            close(client_socket);
            continue;
        }
        task->client_socket = client_socket;
        task->rm = rm;

        // Aggiungiamo il lavoro alla coda del thread pool
        thread_pool_add_job(pool, job_handler, task);
    }

    // Pulizia finale (teorica, perché il server gira all'infinito)
    thread_pool_destroy(pool);
    rm_destroy(rm);
    return 0;
}