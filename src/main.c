#include "booking.h"
#include "logger.h"
#include "server.h"
#include "storage.h"
#include <signal.h>
#include <stdlib.h>

#define DATA_FILE "data/bookings.txt"

void handle_sigint(int sig) {
  (void)sig;
  logger_log("Spegnimento server...");
  logger_destroy();
  exit(0);
}

int main(int argc, char *argv[]) {
  int port = DEFAULT_PORT;

  if (argc > 1) {
    port = atoi(argv[1]);
  }

  /* Gestione segnale per chiusura pulita */
  signal(SIGINT, handle_sigint);

  /* Inizializzazione moduli */
  logger_init();
  init_bookings();

  /* Caricamento dati iniziali */
  int loaded = load_bookings_from_file(DATA_FILE);
  if (loaded >= 0) {
    logger_log("Caricate %d prenotazioni dal file %s", loaded, DATA_FILE);
  } else {
    logger_log("Nessun dato precedente trovato o errore nel caricamento. "
               "Inizio con l'archivio vuoto.");
  }

  /* Avvio server */
  server_start(port);

  return 0;
}
