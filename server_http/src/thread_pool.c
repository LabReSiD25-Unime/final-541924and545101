#include "../include/thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

// Questa è la funzione che ogni thread esegue all'infinito
static void* thread_worker(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;

    while (1) {
        // Il thread si mette in pausa qui se la coda è vuota (semaforo a 0)
        sem_wait(&pool->queue_sem);

        // Sezione critica: dobbiamo prendere un lavoro dalla coda
        pthread_mutex_lock(&pool->queue_mutex);
        
        // Se il server si sta spegnendo, il thread esce dal ciclo
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }

        // Estrazione del primo lavoro disponibile (logica FIFO)
        job_t* job = pool->queue_head;
        if (job) {
            pool->queue_head = job->next;
            if (pool->queue_head == NULL) {
                pool->queue_tail = NULL;
            }
        }
        pthread_mutex_unlock(&pool->queue_mutex); // Rilasciamo il mutex subito dopo l'estrazione

        // Se abbiamo trovato un lavoro, lo eseguiamo fuori dal mutex per non bloccare gli altri
        if (job) {
            (*(job->function))(job->arg); // Chiamata alla funzione (es. handle_client)
            free(job); // Liberiamo la memoria del task eseguito
        }
    }

    return NULL;
}

// Funzione che crea il pool e fa nascere i thread
thread_pool_t* thread_pool_init(int num_threads) {
    thread_pool_t* pool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    pool->num_threads = num_threads;
    pool->queue_head = NULL;
    pool->queue_tail = NULL;
    pool->shutdown = 0;

    // Inizializziamo mutex e semaforo (parte da 0 lavori)
    pthread_mutex_init(&pool->queue_mutex, NULL);
    sem_init(&pool->queue_sem, 0, 0);

    // Creiamo i thread worker
    pool->threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, thread_worker, (void*)pool);// void pool=passo ai thread l'indirizzo della struttura pool
    }

    return pool;
}

// Il dispatcher usa questa funzione per aggiungere nuovi client alla coda
void thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg) {
    job_t* job = (job_t*)malloc(sizeof(job_t));
    job->function = function;
    job->arg = arg;
    job->next = NULL;

    // Inserimento sicuro nella coda
    pthread_mutex_lock(&pool->queue_mutex);
    if (pool->queue_tail == NULL) {
        pool->queue_head = job;
        pool->queue_tail = job;
    } else {
        pool->queue_tail->next = job;
        pool->queue_tail = job;
    }
    pthread_mutex_unlock(&pool->queue_mutex);

    // Incrementiamo il semaforo: se un thread dormiva, ora si sveglia per lavorare
    sem_post(&pool->queue_sem);
}

// Funzione per chiudere tutto senza lasciare thread "appesi"
void thread_pool_destroy(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1; // Diciamo ai thread che devono chiudersi
    pthread_mutex_unlock(&pool->queue_mutex);

    // Mandiamo tanti segnali quanti sono i thread per essere sicuri di svegliarli tutti
    for (int i = 0; i < pool->num_threads; i++) {
        sem_post(&pool->queue_sem);
    }

    // Aspettiamo che ogni thread finisca il suo ultimo lavoro e si chiuda
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // Pulizia finale della memoria
    free(pool->threads);
    pthread_mutex_destroy(&pool->queue_mutex);
    sem_destroy(&pool->queue_sem);
    free(pool);
}