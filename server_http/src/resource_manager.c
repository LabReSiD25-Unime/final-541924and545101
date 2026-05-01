#include "../include/resource_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Funzione per allocare il manager in heap e inizializzare i valori di partenza
resource_manager_t* rm_init() {
    resource_manager_t* rm = (resource_manager_t*)malloc(sizeof(resource_manager_t)); // Alloca la memoria per l'intera struttura
    rm->count = 0;   // All'avvio, il database è vuoto
    rm->next_id = 1; // Il primo ID disponibile per la prima POST sarà 1
    pthread_mutex_init(&rm->mutex, NULL); // Inizializziamo il lock per l'accesso esclusivo al database
    return rm; // Restituisce il puntatore al db pronto
}

// Crea una prenotazione proteggendo la sezione critica con il mutex
int rm_create(resource_manager_t* rm, const char* room, const char* student, const char* time) {
    pthread_mutex_lock(&rm->mutex); // Blocchiamo l'accesso in scrittur
    
    // Se abbiamo raggiunto il limite massimo (100), sblocchiamo il mutex e usciamo restituendo errore
    if (rm->count >= MAX_RESERVATIONS) {
        pthread_mutex_unlock(&rm->mutex); // Mai dimenticare di sbloccare prima di un return anticipato!
        return -1;
    }
    
    int id = rm->next_id++; // Salva il valore attuale e poi incrementa il contatore per il prossimo utilizzo 
    rm->reservations[rm->count].id = id; // Assegna l'ID univoco al record nell'attuale posizione libera (count)
    
    // Usiamo strncpy per sicurezza: evitiamo di scrivere fuori dai buffer se i parametri passati dal client sono troppo lunghi
    // Usiamo MAX_STR_LEN - 1 per lasciare sempre lo spazio per il terminatore di stringa '\0'
    strncpy(rm->reservations[rm->count].room_name, room, MAX_STR_LEN - 1);
    strncpy(rm->reservations[rm->count].student_name, student, MAX_STR_LEN - 1);
    strncpy(rm->reservations[rm->count].time_slot, time, MAX_STR_LEN - 1);
    
    rm->count++; // Incrementiamo il numero totale di record presenti
    
    pthread_mutex_unlock(&rm->mutex); // Fine sezione critica: rilasciamo il lock per gli altri thread
    return id; // Restituiamo l'ID al chiamante per poterlo inserire nel JSON 201 Created
}

// Legge tutto l'array e lo formatta in una grande stringa JSON per la risposta HTTP
int rm_read_all(resource_manager_t* rm, char* buffer, int buffer_size) {
    pthread_mutex_lock(&rm->mutex); // Blocchiamo anche in lettura per evitare che qualcuno modifichi i dati mentre li formattiamo
    int offset = 0; // Variabile per tracciare quanti byte abbiamo scritto nel buffer
    
    // Costruiamo manualmente la stringa JSON concatenando i vari pezzi 
    offset += snprintf(buffer + offset, buffer_size - offset, "[\n"); // Apre l'array JSON
    for (int i = 0; i < rm->count; i++) { // Itera solo per gli elementi effettivamente inseriti (rm->count)
        // snprintf accoda i dati al buffer calcolando l'offset corretto, in modo da non sovrascrivere i cicli precedenti
        offset += snprintf(buffer + offset, buffer_size - offset, 
            "  {\"id\": %d, \"room\": \"%s\", \"student\": \"%s\", \"time\": \"%s\"}%s\n", 
            rm->reservations[i].id, rm->reservations[i].room_name, 
            rm->reservations[i].student_name, rm->reservations[i].time_slot,
            (i == rm->count - 1) ? "" : ","); // Operatore ternario: omette la virgola sull'ultimo elemento JSON 
    }
    offset += snprintf(buffer + offset, buffer_size - offset, "]\n"); // Chiude l'array JSON
    
    pthread_mutex_unlock(&rm->mutex); // Rilascia il lock
    return offset; // Ritorna il numero totale di byte scritti
}

// Cerca una prenotazione tramite il suo ID e la aggiorna (Metodo PUT)
int rm_update(resource_manager_t* rm, int id, const char* room, const char* student, const char* time) {
    pthread_mutex_lock(&rm->mutex); // Sezione critica
    for (int i = 0; i < rm->count; i++) { // Ricerca lineare del record
        if (rm->reservations[i].id == id) { // Trovato!
            // Aggiorniamo solo i campi passati dal client. I controlli if prevengono null 
            if (room) strncpy(rm->reservations[i].room_name, room, MAX_STR_LEN - 1);
            if (student) strncpy(rm->reservations[i].student_name, student, MAX_STR_LEN - 1);
            if (time) strncpy(rm->reservations[i].time_slot, time, MAX_STR_LEN - 1);
            
            pthread_mutex_unlock(&rm->mutex); // Trovato e aggiornato, sblocco ed esco
            return 0; // Successo
        }
    }
    pthread_mutex_unlock(&rm->mutex); // Se arrivo qui, l'ID non esiste: sblocco comunque!
    return -1; // Fallimento (restituirà 404 Not Found)
}

// Elimina una prenotazione mantenendo compatta la memoria (De-frammentazione)
int rm_delete(resource_manager_t* rm, int id) {
    pthread_mutex_lock(&rm->mutex); // Sezione critica
    for (int i = 0; i < rm->count; i++) { // Ricerca il target da eliminare
        if (rm->reservations[i].id == id) { // Target trovato all'indice 'i'
            // Shift a sinistra: sovrascrive l'elemento eliminato spostando tutti i successivi indietro di una posizione
            // Questo previene la frammentazione logica 
            for (int j = i; j < rm->count - 1; j++) {
                rm->reservations[j] = rm->reservations[j + 1];
            }
            rm->count--; // Riduce il numero totale di elementi validi
            pthread_mutex_unlock(&rm->mutex); // Rilascia il lock
            return 0; // Successo (restituirà 204 No Content)
        }
    }
    pthread_mutex_unlock(&rm->mutex);
    return -1; // Target non trovato
}

// Pulisce il mutex e libera tutta la memoria del manager prima della chiusura totale del server
void rm_destroy(resource_manager_t* rm) {
    pthread_mutex_destroy(&rm->mutex); // Distrugge l'oggetto Mutex per liberare le risorse del kernel
    free(rm); // Libera la struttura principale allocata con malloc in rm_init()
}