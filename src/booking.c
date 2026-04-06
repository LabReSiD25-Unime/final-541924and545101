#include "booking.h"
#include "sync.h"
#include <stdio.h>
#include <string.h>

/* Mutex per l'accesso ai dati delle prenotazioni */
static pthread_mutex_t bookings_mutex;

/* Archivio prenotazioni in memoria */
static Booking bookings[MAX_BOOKINGS];

/* Contatore usato per generare id univoci */
static int next_booking_id = 1;

/* Controlla se l'id stanza è valido */
static int is_valid_room(int room_id) {
  return room_id >= 1 && room_id <= MAX_ROOMS;
}

/* Conta quante prenotazioni attive ci sono in una stanza */
static int count_bookings_in_room(int room_id) {
  int count = 0;
  int i;

  for (i = 0; i < MAX_BOOKINGS; i++) {
    if (bookings[i].active && bookings[i].room_id == room_id) {
      count++;
    }
  }

  return count;
}

/* Cerca il primo slot libero nell'array */
static int find_free_index(void) {
  int i;

  for (i = 0; i < MAX_BOOKINGS; i++) {
    if (!bookings[i].active) {
      return i;
    }
  }

  return -1;
}

/* Pulisce una singola prenotazione */
static void clear_booking(Booking *b) {
  if (b == NULL) {
    return;
  }

  b->id = 0;
  b->user[0] = '\0';
  b->room_id = 0;
  b->active = 0;
}

/* Inizializza tutto l'archivio e il mutex */
void init_bookings(void) {
  int i;

  sync_mutex_init(&bookings_mutex);

  for (i = 0; i < MAX_BOOKINGS; i++) {
    clear_booking(&bookings[i]);
  }

  next_booking_id = 1;
}

/* Restituisce il prossimo id disponibile */
int get_next_booking_id(void) {
  int id;
  sync_mutex_lock(&bookings_mutex);
  id = next_booking_id;
  sync_mutex_unlock(&bookings_mutex);
  return id;
}

/* Aggiunge una prenotazione */
int add_booking(const char *user, int room_id) {
  int index;
  int ret_id;

  if (!is_valid_room(room_id)) {
    return -2;
  }

  sync_mutex_lock(&bookings_mutex);

  if (count_bookings_in_room(room_id) >= SLOTS_PER_ROOM) {
    sync_mutex_unlock(&bookings_mutex);
    return -3;
  }

  index = find_free_index();
  if (index == -1) {
    sync_mutex_unlock(&bookings_mutex);
    return -1;
  }

  bookings[index].id = next_booking_id;
  strncpy(bookings[index].user, user, MAX_NAME_LEN - 1);
  bookings[index].user[MAX_NAME_LEN - 1] = '\0';
  bookings[index].room_id = room_id;
  bookings[index].active = 1;

  ret_id = bookings[index].id;
  next_booking_id++;

  sync_mutex_unlock(&bookings_mutex);

  return ret_id;
}

/* Rimuove una prenotazione tramite id */
int remove_booking(int booking_id) {
  int i;
  int found = -1;

  sync_mutex_lock(&bookings_mutex);

  for (i = 0; i < MAX_BOOKINGS; i++) {
    if (bookings[i].active && bookings[i].id == booking_id) {
      clear_booking(&bookings[i]);
      found = 0;
      break;
    }
  }

  sync_mutex_unlock(&bookings_mutex);

  return found;
}

/* Controlla se una prenotazione esiste */
int booking_exists(int booking_id) {
  int i;
  int exists = 0;

  sync_mutex_lock(&bookings_mutex);

  for (i = 0; i < MAX_BOOKINGS; i++) {
    if (bookings[i].active && bookings[i].id == booking_id) {
      exists = 1;
      break;
    }
  }

  sync_mutex_unlock(&bookings_mutex);

  return exists;
}

/* Aggiorna il nome utente di una prenotazione */
int update_booking_user(int booking_id, const char *new_user) {
  int i;
  int found = -1;

  if (new_user == NULL || new_user[0] == '\0') {
    return -1;
  }

  sync_mutex_lock(&bookings_mutex);

  for (i = 0; i < MAX_BOOKINGS; i++) {
    if (bookings[i].active && bookings[i].id == booking_id) {
      strncpy(bookings[i].user, new_user, MAX_NAME_LEN - 1);
      bookings[i].user[MAX_NAME_LEN - 1] = '\0';
      found = 0;
      break;
    }
  }

  sync_mutex_unlock(&bookings_mutex);

  return found;
}

/* Conta i posti liberi in una stanza (unlocked) */
static int count_available_slots_in_room_unlocked(int room_id) {
  if (!is_valid_room(room_id)) {
    return -1;
  }
  int occupied = count_bookings_in_room(room_id);
  return SLOTS_PER_ROOM - occupied;
}

/* Conta i posti liberi in una stanza */
int count_available_slots_in_room(int room_id) {
  int available;
  sync_mutex_lock(&bookings_mutex);
  available = count_available_slots_in_room_unlocked(room_id);
  sync_mutex_unlock(&bookings_mutex);
  return available;
}

/* Conta tutti i posti liberi complessivi */
int count_available_slots(void) {
  int total_slots = MAX_ROOMS * SLOTS_PER_ROOM;
  int occupied = 0;
  int i;

  sync_mutex_lock(&bookings_mutex);

  for (i = 0; i < MAX_BOOKINGS; i++) {
    if (bookings[i].active) {
      occupied++;
    }
  }

  sync_mutex_unlock(&bookings_mutex);

  return total_slots - occupied;
}

/* Scrive l'elenco prenotazioni in un buffer */
void list_bookings(char *buffer, size_t size) {
  int i;
  int found = 0;
  int written;

  if (buffer == NULL || size == 0) {
    return;
  }

  sync_mutex_lock(&bookings_mutex);

  buffer[0] = '\0';

  for (i = 0; i < MAX_BOOKINGS; i++) {
    if (bookings[i].active) {
      size_t used = strlen(buffer);

      if (used >= size - 1) {
        sync_mutex_unlock(&bookings_mutex);
        return;
      }

      written = snprintf(buffer + used, size - used,
                         "ID: %d | Utente: %s | Stanza: %d\n", bookings[i].id,
                         bookings[i].user, bookings[i].room_id);

      if (written < 0 || (size_t)written >= (size - used)) {
        sync_mutex_unlock(&bookings_mutex);
        return;
      }

      found = 1;
    }
  }

  if (!found) {
    snprintf(buffer, size, "Nessuna prenotazione presente.\n");
  }

  sync_mutex_unlock(&bookings_mutex);
}

/* Scrive lo stato delle stanze in un buffer */
void list_rooms(char *buffer, size_t size) {
  int room_id;
  int written;

  if (buffer == NULL || size == 0) {
    return;
  }

  sync_mutex_lock(&bookings_mutex);

  buffer[0] = '\0';

  for (room_id = 1; room_id <= MAX_ROOMS; room_id++) {
    int available = count_available_slots_in_room_unlocked(room_id);
    int occupied = SLOTS_PER_ROOM - available;
    size_t used = strlen(buffer);

    if (used >= size - 1) {
      sync_mutex_unlock(&bookings_mutex);
      return;
    }

    written = snprintf(buffer + used, size - used,
                       "Stanza %d -> Occupati: %d | Liberi: %d\n", room_id,
                       occupied, available);

    if (written < 0 || (size_t)written >= (size - used)) {
      sync_mutex_unlock(&bookings_mutex);
      return;
    }
  }

  sync_mutex_unlock(&bookings_mutex);
}

/* Restituisce la capacità massima dell'archivio */
int get_booking_capacity(void) { return MAX_BOOKINGS; }

/* Copia una prenotazione dall'archivio verso l'esterno */
int get_booking_at_index(int index, Booking *out) {
  if (index < 0 || index >= MAX_BOOKINGS || out == NULL) {
    return -1;
  }

  sync_mutex_lock(&bookings_mutex);
  *out = bookings[index];
  sync_mutex_unlock(&bookings_mutex);

  return 0;
}

/* Scrive una prenotazione in una posizione dell'archivio */
int set_booking_at_index(int index, const Booking *in) {
  if (index < 0 || index >= MAX_BOOKINGS || in == NULL) {
    return -1;
  }

  sync_mutex_lock(&bookings_mutex);
  bookings[index] = *in;
  sync_mutex_unlock(&bookings_mutex);

  return 0;
}

/* Aggiorna il prossimo id disponibile */
void set_next_booking_id(int next_id) {
  if (next_id > 0) {
    sync_mutex_lock(&bookings_mutex);
    next_booking_id = next_id;
    sync_mutex_unlock(&bookings_mutex);
  }
}