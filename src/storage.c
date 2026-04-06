#include <stdio.h>
#include "storage.h"
#include "booking.h"

/* Salva tutte le prenotazioni attive su file */
int save_bookings_to_file(const char *filename) {
    FILE *fp;
    Booking temp;
    int i;
    int capacity;

    if (filename == NULL) {
        return -1;
    }

    fp = fopen(filename, "w");
    if (fp == NULL) {
        return -1;
    }

    capacity = get_booking_capacity();

    for (i = 0; i < capacity; i++) {
        if (get_booking_at_index(i, &temp) == 0 && temp.active) {
            fprintf(fp, "%d;%s;%d;%d\n",
                    temp.id,
                    temp.user,
                    temp.room_id,
                    temp.active);
        }
    }

    fclose(fp);
    return 0;
}

/* Carica le prenotazioni da file */
int load_bookings_from_file(const char *filename) {
    FILE *fp;
    Booking temp;
    int loaded_count = 0;
    int max_id = 0;
    int index = 0;

    if (filename == NULL) {
        return -1;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    /* Prima puliamo l'archivio */
    init_bookings();

    while (index < get_booking_capacity() &&
           fscanf(fp, "%d;%63[^;];%d;%d\n",
                  &temp.id,
                  temp.user,
                  &temp.room_id,
                  &temp.active) == 4) {

        if (set_booking_at_index(index, &temp) == 0) {
            loaded_count++;

            if (temp.id > max_id) {
                max_id = temp.id;
            }

            index++;
        } else {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);

    /* Il prossimo id deve essere il massimo trovato + 1 */
    set_next_booking_id(max_id + 1);

    return loaded_count;
}