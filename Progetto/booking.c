#include <stdio.h>
#include <string.h>
#include "booking.h"

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

/* Inizializza tutto l'archivio */
void init_bookings(void) {
    int i;

    for (i = 0; i < MAX_BOOKINGS; i++) {
        clear_booking(&bookings[i]);
    }

    next_booking_id = 1;
}

/* Restituisce il prossimo id disponibile */
int get_next_booking_id(void) {
    return next_booking_id;
}

/* Aggiunge una prenotazione */
int add_booking(const char *user, int room_id) {
    int index;

    if (!is_valid_room(room_id)) {
        return -2;
    }

    if (count_bookings_in_room(room_id) >= SLOTS_PER_ROOM) {
        return -3;
    }

    index = find_free_index();
    if (index == -1) {
        return -1;
    }

    bookings[index].id = next_booking_id;
    strncpy(bookings[index].user, user, MAX_NAME_LEN - 1);
    bookings[index].user[MAX_NAME_LEN - 1] = '\0';
    bookings[index].room_id = room_id;
    bookings[index].active = 1;

    next_booking_id++;

    return bookings[index].id;
}

/* Rimuove una prenotazione tramite id */
int remove_booking(int booking_id) {
    int i;

    for (i = 0; i < MAX_BOOKINGS; i++) {
        if (bookings[i].active && bookings[i].id == booking_id) {
            clear_booking(&bookings[i]);
            return 0;
        }
    }

    return -1;
}

/* Controlla se una prenotazione esiste */
int booking_exists(int booking_id) {
    int i;

    for (i = 0; i < MAX_BOOKINGS; i++) {
        if (bookings[i].active && bookings[i].id == booking_id) {
            return 1;
        }
    }

    return 0;
}

/* Conta i posti liberi in una stanza */
int count_available_slots_in_room(int room_id) {
    int occupied;

    if (!is_valid_room(room_id)) {
        return -1;
    }

    occupied = count_bookings_in_room(room_id);
    return SLOTS_PER_ROOM - occupied;
}

/* Conta tutti i posti liberi complessivi */
int count_available_slots(void) {
    int total_slots = MAX_ROOMS * SLOTS_PER_ROOM;
    int occupied = 0;
    int i;

    for (i = 0; i < MAX_BOOKINGS; i++) {
        if (bookings[i].active) {
            occupied++;
        }
    }

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

    buffer[0] = '\0';

    for (i = 0; i < MAX_BOOKINGS; i++) {
        if (bookings[i].active) {
            size_t used = strlen(buffer);

            if (used >= size - 1) {
                return;
            }

            written = snprintf(
                buffer + used,
                size - used,
                "ID: %d | Utente: %s | Stanza: %d\n",
                bookings[i].id,
                bookings[i].user,
                bookings[i].room_id
            );

            if (written < 0 || (size_t)written >= (size - used)) {
                return;
            }

            found = 1;
        }
    }

    if (!found) {
        snprintf(buffer, size, "Nessuna prenotazione presente.\n");
    }
}
/* Scrive lo stato delle stanze in un buffer */
void list_rooms(char *buffer, size_t size) {
    int room_id;
    int written;

    if (buffer == NULL || size == 0) {
        return;
    }

    buffer[0] = '\0';

    for (room_id = 1; room_id <= MAX_ROOMS; room_id++) {
        int available = count_available_slots_in_room(room_id);
        int occupied = SLOTS_PER_ROOM - available;
        size_t used = strlen(buffer);

        if (used >= size - 1) {
            return;
        }

        written = snprintf(
            buffer + used,
            size - used,
            "Stanza %d -> Occupati: %d | Liberi: %d\n",
            room_id,
            occupied,
            available
        );

        if (written < 0 || (size_t)written >= (size - used)) {
            return;
        }
    }
}
/* Restituisce la capacità massima dell'archivio */
int get_booking_capacity(void) {
    return MAX_BOOKINGS;
}

/* Copia una prenotazione dall'archivio verso l'esterno */
int get_booking_at_index(int index, Booking *out) {
    if (index < 0 || index >= MAX_BOOKINGS || out == NULL) {
        return -1;
    }

    *out = bookings[index];
    return 0;
}

/* Scrive una prenotazione in una posizione dell'archivio */
int set_booking_at_index(int index, const Booking *in) {
    if (index < 0 || index >= MAX_BOOKINGS || in == NULL) {
        return -1;
    }

    bookings[index] = *in;
    return 0;
}

/* Aggiorna il prossimo id disponibile */
void set_next_booking_id(int next_id) {
    if (next_id > 0) {
        next_booking_id = next_id;
    }
}