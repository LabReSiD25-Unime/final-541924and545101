#ifndef STORAGE_H
#define STORAGE_H

/* Salva tutte le prenotazioni attive su file.
   Ritorna:
   - 0 se va tutto bene
   - -1 in caso di errore
*/
int save_bookings_to_file(const char *filename);

/* Carica le prenotazioni da file.
   Ritorna:
   - numero di prenotazioni caricate se va tutto bene
   - -1 in caso di errore
*/
int load_bookings_from_file(const char *filename);

#endif