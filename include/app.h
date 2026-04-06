#ifndef APP_H
#define APP_H

#include <stddef.h>

/* Restituisce lo stato delle stanze nel buffer response */
void app_get_rooms(char *response, size_t size);

/* Restituisce l'elenco delle prenotazioni nel buffer response */
void app_get_bookings(char *response, size_t size);

/* Prova ad aggiungere una prenotazione.
   Ritorna:
   - 0 se va tutto bene
   - -1 se il nome utente non è valido
   - -2 se la stanza non è valida
   - -3 se la stanza è piena
   - -4 se non c'è spazio nell'archivio
*/
int app_post_booking(const char *user, int room_id, char *response,
                     size_t size);

/* Prova a cancellare una prenotazione.
   Ritorna:
   - 0 se va tutto bene
   - -1 se l'id non esiste
*/
int app_delete_booking(int booking_id, char *response, size_t size);

/* Prova ad aggiornare una prenotazione.
   Ritorna:
   - 0 se va tutto bene
   - -1 se l'id non esiste o il nome non è valido
*/
int app_put_booking(int booking_id, const char *new_user, char *response,
                    size_t size);

#endif