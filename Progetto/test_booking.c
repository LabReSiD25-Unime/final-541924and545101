#include <stdio.h>
#include "booking.h"
#include "storage.h"
#include "app.h"

#define BUFFER_SIZE 2048
#define DATA_FILE "data/bookings.txt"

int main(void) {
    char buffer[BUFFER_SIZE];
    int result;

    printf("=== TEST MODULO PRENOTAZIONI ===\n\n");

    /* Inizializzazione archivio */
    init_bookings();
    printf("[OK] Archivio inizializzato.\n\n");

    /* Stato iniziale stanze */
    printf("Stato iniziale stanze:\n");
    app_get_rooms(buffer, sizeof(buffer));
    printf("%s\n", buffer);

    /* Aggiunta prenotazioni */
    result = app_post_booking("Mario", 1, buffer, sizeof(buffer));
    printf("Prenotazione Mario stanza 1 -> codice: %d\n%s\n", result, buffer);

    result = app_post_booking("Luca", 1, buffer, sizeof(buffer));
    printf("Prenotazione Luca stanza 1 -> codice: %d\n%s\n", result, buffer);

    result = app_post_booking("Anna", 2, buffer, sizeof(buffer));
    printf("Prenotazione Anna stanza 2 -> codice: %d\n%s\n", result, buffer);

    result = app_post_booking("Sara", 6, buffer, sizeof(buffer));
    printf("Prenotazione Sara stanza 6 -> codice: %d\n%s\n", result, buffer);

    /* Elenco prenotazioni */
    printf("Elenco prenotazioni:\n");
    app_get_bookings(buffer, sizeof(buffer));
    printf("%s\n", buffer);

    /* Stato stanze dopo le prenotazioni */
    printf("Stato stanze aggiornato:\n");
    app_get_rooms(buffer, sizeof(buffer));
    printf("%s\n", buffer);

    /* Salvataggio su file */
    result = save_bookings_to_file(DATA_FILE);
    if (result == 0) {
        printf("[OK] Prenotazioni salvate su file.\n\n");
    } else {
        printf("[ERRORE] Salvataggio su file fallito.\n\n");
    }

    /* Cancellazione prenotazione con id 2 */
    result = app_delete_booking(2, buffer, sizeof(buffer));
    printf("Cancellazione prenotazione ID 2 -> codice: %d\n%s\n", result, buffer);

    /* Elenco prenotazioni dopo cancellazione */
    printf("Elenco prenotazioni dopo cancellazione:\n");
    app_get_bookings(buffer, sizeof(buffer));
    printf("%s\n", buffer);

    /* Ricaricamento da file */
    init_bookings();
    printf("[OK] Archivio svuotato.\n");

    result = load_bookings_from_file(DATA_FILE);
    printf("Caricamento da file -> prenotazioni caricate: %d\n\n", result);

    printf("Elenco prenotazioni ricaricate da file:\n");
    app_get_bookings(buffer, sizeof(buffer));
    printf("%s\n", buffer);

    printf("Stato finale stanze:\n");
    app_get_rooms(buffer, sizeof(buffer));
    printf("%s\n", buffer);

    printf("=== FINE TEST ===\n");
    return 0;
}