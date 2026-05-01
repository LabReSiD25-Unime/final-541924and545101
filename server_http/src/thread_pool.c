#include "../include/thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

// Questa è la funzione che ogni thread esegue all'infinito
static void* thread_worker(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg; // Cast dell'argomento per accedere alla struttura del pool

    while (1) { // Ciclo di vita continuo del worker
        // Il thread si mette in pausa qui se la coda è vuota (semaforo a 0)
        // Non consuma CPU finché non arriva una sem_post
        sem_wait(&pool->queue_sem);

        // Sezione critica: dobbiamo prendere un lavoro dalla coda, blocchiamo gli altri thread
        pthread_mutex_lock(&pool->queue_mutex);
        
        // Se il server si sta spegnendo (flag a 1), il thread esce dal ciclo per morire dolcemente
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex); // Sblocca il mutex prima di uscire per evitare deadlock
            break;
        }

        // Estrazione del primo lavoro disponibile 
        job_t* job = pool->queue_head; 
        if (job) { // Se c'è effettivamente un lavoro
            pool->queue_head = job->next; // La testa diventa l'elemento successivo
            if (pool->queue_head == NULL) { // Se la lista ora è vuota...
                pool->queue_tail = NULL;    // ...resettiamo anche la coda
            }
        }
        pthread_mutex_unlock(&pool->queue_mutex); // Rilasciamo il mutex subito dopo l'estrazione per far accedere gli altri

        // Se abbiamo trovato un lavoro, lo eseguiamo fuori dal mutex per non bloccare gli altri thread
        if (job) {
            (*(job->function))(job->arg); // Chiamata indiretta alla funzione passata 
            free(job); // Liberiamo la memoria del nodo della lista ormai processato
        }
    }

    return NULL; // Il thread termina
}

// Funzione che crea il pool e fa nascere i thread
thread_pool_t* thread_pool_init(int num_threads) {
    thread_pool_t* pool = (thread_pool_t*)malloc(sizeof(thread_pool_t)); // Alloca la struttura di controllo principale
    pool->num_threads = num_threads; // Salva la grandezza del pool
    pool->queue_head = NULL;         // Inizializza la lista concatenata come vuota
    pool->queue_tail = NULL;
    pool->shutdown = 0;              // Flag di spegnimento inizializzato a falso

    // Inizializziamo mutex e semaforo (parte da 0 lavori, quindi i thread si bloccheranno subito)
    pthread_mutex_init(&pool->queue_mutex, NULL);
    sem_init(&pool->queue_sem, 0, 0); // 0 indica che è condiviso tra thread dello stesso processo, il secondo 0 è il valore iniziale

    // Creiamo l'array che conterrà gli ID dei thread worker
    pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        // Facciamo nascere i thread, passandogli la funzione worker e l'intero pool come argomento
        pthread_create(&pool->threads[i], NULL, thread_worker, (void*)pool);
    }

    return pool; // Ritorna il puntatore al pool pronto all'uso
}

// Il dispatcher (main) usa questa funzione per aggiungere nuovi client alla coda
void thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg) {
    job_t* job = (job_t*)malloc(sizeof(job_t)); // Crea un nuovo nodo per la lista
    job->function = function; // Assegna la funzione da eseguire
    job->arg = arg;           // Assegna i parametri per la funzione
    job->next = NULL;         // Essendo inserito in coda, non ha successori per ora

    // Inserimento sicuro nella coda protetto da Race Conditions
    pthread_mutex_lock(&pool->queue_mutex);
    if (pool->queue_tail == NULL) { // Caso 1: La coda era completamente vuota
        pool->queue_head = job;
        pool->queue_tail = job;
    } else {                        // Caso 2: Ci sono già elementi, appendiamo alla fine
        pool->queue_tail->next = job;
        pool->queue_tail = job;
    }
    pthread_mutex_unlock(&pool->queue_mutex); // Fine sezione critica

    // Incrementiamo il semaforo: il contatore sale di 1. 
    // Se un thread dormiva in sem_wait, ora si sveglia per prendere questo lavoro
    sem_post(&pool->queue_sem);
}

// Funzione per chiudere tutto senza lasciare thread "appesi"
void thread_pool_destroy(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1; // Diciamo ai thread che devono chiudersi attivando il flag
    pthread_mutex_unlock(&pool->queue_mutex);

    // Mandiamo tanti segnali quanti sono i thread per essere sicuri di svegliarli tutti
    // Altrimenti, se la coda è vuota, resterebbero bloccati in sem_wait all'infinito
    for (int i = 0; i < pool->num_threads; i++) {
        sem_post(&pool->queue_sem);
    }

    // Aspettiamo che ogni thread finisca il suo ultimo lavoro e si chiuda 
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // Pulizia finale della memoria per evitare leak
    free(pool->threads);
    pthread_mutex_destroy(&pool->queue_mutex);
    sem_destroy(&pool->queue_sem);
    free(pool);
}