#include "app.h"
#include "booking.h"
#include "storage.h"
#include <stdio.h>
#include <string.h>

#define DATA_FILE "data/bookings.txt"

/* Controlla se il nome utente è valido */
static int is_valid_user(const char *user) {
  if (user == NULL) {
    return 0;
  }

  if (user[0] == '\0') {
    return 0;
  }

  return 1;
}

/* Restituisce lo stato delle stanze */
void app_get_rooms(char *response, size_t size) {
  if (response == NULL || size == 0) {
    return;
  }

  list_rooms(response, size);
}

/* Restituisce l'elenco delle prenotazioni */
void app_get_bookings(char *response, size_t size) {
  if (response == NULL || size == 0) {
    return;
  }

  list_bookings(response, size);
}

/* Gestisce una nuova prenotazione */
int app_post_booking(const char *user, int room_id, char *response,
                     size_t size) {
  int result;

  if (response == NULL || size == 0) {
    return -1;
  }

  response[0] = '\0';

  if (!is_valid_user(user)) {
    snprintf(response, size, "Errore: nome utente non valido.\n");
    return -1;
  }

  result = add_booking(user, room_id);

  if (result > 0) {
    save_bookings_to_file(DATA_FILE);
    snprintf(response, size,
             "Prenotazione effettuata con successo.\n"
             "ID prenotazione: %d\n"
             "Utente: %s\n"
             "Stanza: %d\n",
             result, user, room_id);
    return 0;
  }

  if (result == -2) {
    snprintf(response, size, "Errore: stanza non valida.\n");
    return -2;
  }

  if (result == -3) {
    snprintf(response, size, "Errore: stanza piena.\n");
    return -3;
  }

  if (result == -1) {
    snprintf(response, size, "Errore: archivio prenotazioni pieno.\n");
    return -4;
  }

  snprintf(response, size, "Errore generico durante la prenotazione.\n");
  return -4;
}

/* Gestisce la cancellazione di una prenotazione */
int app_delete_booking(int booking_id, char *response, size_t size) {
  int result;

  if (response == NULL || size == 0) {
    return -1;
  }

  response[0] = '\0';

  result = remove_booking(booking_id);

  if (result == 0) {
    save_bookings_to_file(DATA_FILE);
    snprintf(response, size,
             "Prenotazione con ID %d cancellata correttamente.\n", booking_id);
    return 0;
  }

  snprintf(response, size, "Errore: nessuna prenotazione trovata con ID %d.\n",
           booking_id);
  return -1;
}

/* Gestisce l'aggiornamento di una prenotazione */
int app_put_booking(int booking_id, const char *new_user, char *response,
                    size_t size) {
  int result;

  if (response == NULL || size == 0) {
    return -1;
  }

  response[0] = '\0';

  if (!is_valid_user(new_user)) {
    snprintf(response, size, "Errore: nuovo nome utente non valido.\n");
    return -1;
  }

  result = update_booking_user(booking_id, new_user);

  if (result == 0) {
    save_bookings_to_file(DATA_FILE);
    snprintf(
        response, size,
        "Prenotazione con ID %d aggiornata correttamente. Nuovo utente: %s\n",
        booking_id, new_user);
    return 0;
  }

  snprintf(response, size, "Errore: nessuna prenotazione trovata con ID %d.\n",
           booking_id);
  return -1;
}