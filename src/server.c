#include "server.h"
#include "http.h"
#include "logger.h"
#include "routes.h"
#include "sync.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAX_CONCURRENT_CLIENTS 10

static sem_t client_slots_sem;

/* Struttura passata al thread */
typedef struct {
  int client_socket;
} ClientArgs;

/* Funzione del thread per gestire un client */
static void *handle_client(void *arg) {
  ClientArgs *args = (ClientArgs *)arg;
  int client_socket = args->client_socket;
  free(args);

  char buffer[BUFFER_SIZE];
  ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

  if (bytes_received > 0) {
    buffer[bytes_received] = '\0';

    HttpRequest req;
    HttpResponse res;
    char response_raw[BUFFER_SIZE * 2];

    if (http_parse_request(buffer, &req) == 0) {
      logger_log("Richiesta: %s %s", req.method, req.path);
      routes_handle_request(&req, &res);
    } else {
      logger_error("Errore nel parsing della richiesta");
      res.status_code = 400;
      res.status_message = "Bad Request";
      strcpy(res.body, "Errore 400: Richiesta non valida.\n");
      res.body_len = strlen(res.body);
    }

    size_t response_len =
        http_build_response(&res, response_raw, sizeof(response_raw));
    send(client_socket, response_raw, response_len, 0);
  }

  close(client_socket);
  sync_sem_post(&client_slots_sem);
  return NULL;
}

void server_start(int port) {
  int server_socket;
  struct sockaddr_in server_addr;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    logger_error("Errore creazione socket");
    exit(EXIT_FAILURE);
  }

  /* Permettiamo il riutilizzo dell'indirizzo */
  int opt = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == -1) {
    logger_error("Errore bind sulla porta %d", port);
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, 10) == -1) {
    logger_error("Errore listen");
    exit(EXIT_FAILURE);
  }

  /* Inizializzazione semaforo per limite thread concorrenti */
  sync_sem_init(&client_slots_sem, MAX_CONCURRENT_CLIENTS);

  logger_log("Server avviato sulla porta %d (max %d client concorrenti)...",
             port, MAX_CONCURRENT_CLIENTS);

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    /* Aspettiamo uno slot libero per gestire il client */
    sync_sem_wait(&client_slots_sem);

    int client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

    if (client_socket == -1) {
      logger_error("Errore accept");
      sync_sem_post(&client_slots_sem);
      continue;
    }

    /* Gestiamo ogni client in un nuovo thread */
    pthread_t tid;
    ClientArgs *args = malloc(sizeof(ClientArgs));
    args->client_socket = client_socket;

    if (pthread_create(&tid, NULL, handle_client, args) != 0) {
      logger_error("Errore creazione thread");
      close(client_socket);
      free(args);
    } else {
      pthread_detach(tid);
    }
  }

  close(server_socket);
}
