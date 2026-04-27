#include "../include/resource_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Funzione per allocare il manager e inizializzare il mutex
resource_manager_t* rm_init() {
    resource_manager_t* rm = (resource_manager_t*)malloc(sizeof(resource_manager_t));
    rm->count = 0;
    rm->next_id = 1;
    pthread_mutex_init(&rm->mutex, NULL); // Inizializziamo il lock per l'accesso esclusivo
    return rm;
}

// Crea una prenotazione proteggendo la sezione critica con il mutex
int rm_create(resource_manager_t* rm, const char* room, const char* student, const char* time) {
    pthread_mutex_lock(&rm->mutex); // Blocchiamo l'accesso agli altri thread
    
    // Se abbiamo raggiunto il limite di 100, sblocchiamo e usciamo con errore
    if (rm->count >= MAX_RESERVATIONS) {
        pthread_mutex_unlock(&rm->mutex);
        return -1;
    }
    
    int id = rm->next_id++;//genera un nuovo ID e aggiorna il contatore per il prossimo utilizzo
    rm->reservations[rm->count].id = id;//salva l'ID all'interno dell'elenco delle prenotazioni
    
    // Usiamo strncpy per sicurezza: evitiamo di scrivere fuori dai buffer se i nomi sono troppo lunghi
    strncpy(rm->reservations[rm->count].room_name, room, MAX_STR_LEN - 1);
    strncpy(rm->reservations[rm->count].student_name, student, MAX_STR_LEN - 1);
    strncpy(rm->reservations[rm->count].time_slot, time, MAX_STR_LEN - 1);
    
    rm->count++;
    
    pthread_mutex_unlock(&rm->mutex); // Rilasciamo il lock per gli altri
    return id;
}

// Legge tutto l'array e lo formatta come stringa JSON per la risposta HTTP
int rm_read_all(resource_manager_t* rm, char* buffer, int buffer_size) {
    pthread_mutex_lock(&rm->mutex);
    int offset = 0;
    
    // Costruiamo manualmente la stringa JSON
    offset += snprintf(buffer + offset, buffer_size - offset, "[\n");
    for (int i = 0; i < rm->count; i++) {
        offset += snprintf(buffer + offset, buffer_size - offset, 
            "  {\"id\": %d, \"room\": \"%s\", \"student\": \"%s\", \"time\": \"%s\"}%s\n", 
            rm->reservations[i].id, rm->reservations[i].room_name, 
            rm->reservations[i].student_name, rm->reservations[i].time_slot,
            (i == rm->count - 1) ? "" : ",");
    }
    offset += snprintf(buffer + offset, buffer_size - offset, "]\n");
    
    pthread_mutex_unlock(&rm->mutex);
    return offset;
}

// Cerca una prenotazione per ID e la aggiorna
int rm_update(resource_manager_t* rm, int id, const char* room, const char* student, const char* time) {
    pthread_mutex_lock(&rm->mutex);
    for (int i = 0; i < rm->count; i++) {
        if (rm->reservations[i].id == id) {
            // Aggiorniamo solo i campi passati (non NULL)
            if (room) strncpy(rm->reservations[i].room_name, room, MAX_STR_LEN - 1);
            if (student) strncpy(rm->reservations[i].student_name, student, MAX_STR_LEN - 1);
            if (time) strncpy(rm->reservations[i].time_slot, time, MAX_STR_LEN - 1);
            
            pthread_mutex_unlock(&rm->mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&rm->mutex);
    return -1;
}

// Elimina una prenotazione spostando gli elementi successivi a sinistra
int rm_delete(resource_manager_t* rm, int id) {
    pthread_mutex_lock(&rm->mutex);
    for (int i = 0; i < rm->count; i++) {
        if (rm->reservations[i].id == id) {
            // Shift degli elementi per non lasciare "buchi" nell'array
            for (int j = i; j < rm->count - 1; j++) {
                rm->reservations[j] = rm->reservations[j + 1];
            }
            rm->count--;
            pthread_mutex_unlock(&rm->mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&rm->mutex);
    return -1;
}

// Pulisce il mutex e libera la memoria del manager
void rm_destroy(resource_manager_t* rm) {
    pthread_mutex_destroy(&rm->mutex);
    free(rm);
}