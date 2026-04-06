#ifndef BOOKING_H
#define BOOKING_H

#include <stddef.h>

#define MAX_BOOKINGS 100
#define MAX_NAME_LEN 64
#define MAX_ROOMS 5
#define SLOTS_PER_ROOM 4

typedef struct {
  int id;
  char user[MAX_NAME_LEN];
  int room_id;
  int active;
} Booking;

/* Inizializza l'archivio delle prenotazioni */
void init_bookings(void);

/* Aggiunge una prenotazione.
   Ritorna:
   - id della prenotazione se tutto va bene
   - -1 se non c'è spazio nell'array
   - -2 se la stanza non è valida
   - -3 se la stanza è piena
*/
int add_booking(const char *user, int room_id);

/* Rimuove una prenotazione dato l'id.
   Ritorna:
   - 0 se rimossa correttamente
   - -1 se non trovata
*/
int remove_booking(int booking_id);

/* Aggiorna il nome utente di una prenotazione.
   Ritorna:
   - 0 se aggiornata correttamente
   - -1 se non trovata
*/
int update_booking_user(int booking_id, const char *new_user);

/* Conta quanti posti liberi ci sono in totale */
int count_available_slots(void);

/* Conta quanti posti liberi ci sono in una stanza */
int count_available_slots_in_room(int room_id);

/* Controlla se una prenotazione esiste ancora ed è attiva */
int booking_exists(int booking_id);

/* Scrive nel buffer la lista delle prenotazioni */
void list_bookings(char *buffer, size_t size);

/* Scrive nel buffer lo stato delle stanze */
void list_rooms(char *buffer, size_t size);

/* Restituisce il prossimo id disponibile */
int get_next_booking_id(void);

/* Restituisce la dimensione massima dell'archivio */
int get_booking_capacity(void);

/* Copia la prenotazione presente a un certo indice dentro la struct out.
   Ritorna:
   - 0 se va tutto bene
   - -1 se l'indice non è valido o out è NULL
*/
int get_booking_at_index(int index, Booking *out);

/* Scrive una prenotazione in una certa posizione dell'archivio.
   Ritorna:
   - 0 se va tutto bene
   - -1 se l'indice non è valido o in è NULL
*/
int set_booking_at_index(int index, const Booking *in);

/* Aggiorna il prossimo id disponibile */
void set_next_booking_id(int next_id);

#endif