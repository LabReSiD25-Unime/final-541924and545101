#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <pthread.h>

// Definiamo i limiti massimi per l'array in memoria e le stringhe
#define MAX_RESERVATIONS 100
#define MAX_STR_LEN 128

// Struttura che rappresenta i dati di una singola prenotazione
typedef struct {
    int id;                                 // ID numerico univoco
    char room_name[MAX_STR_LEN];            // Nome dell'aula prenotata
    char student_name[MAX_STR_LEN];         // Nome dello studente
    char time_slot[MAX_STR_LEN];            // Orario della prenotazione
} reservation_t;

// Struttura del gestore: contiene l'array di prenotazioni e il mutex per la sincronizzazione
typedef struct {
    reservation_t reservations[MAX_RESERVATIONS]; // Il nostro "database" in RAM
    int count;                                    // Quante prenotazioni abbiamo al momento
    int next_id;                                  // Contatore per assegnare l'ID alla prossima prenotazione
    pthread_mutex_t mutex;                        // Lucchetto per evitare che più thread scrivano insieme
} resource_manager_t;

// Firme delle funzioni per la gestione CRUD
resource_manager_t* rm_init();
int rm_create(resource_manager_t* rm, const char* room, const char* student, const char* time);
int rm_read_all(resource_manager_t* rm, char* buffer, int buffer_size);
int rm_update(resource_manager_t* rm, int id, const char* room, const char* student, const char* time);
int rm_delete(resource_manager_t* rm, int id);
void rm_destroy(resource_manager_t* rm);

#endif