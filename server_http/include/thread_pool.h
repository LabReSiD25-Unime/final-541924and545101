#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <semaphore.h>

// Definiamo cosa è un "lavoro" (job) da fare
typedef struct job {
    void (*function)(void*); // Puntatore alla funzione da eseguire (es. gestire un client)
    void* arg;               // Argomento da passare alla funzione (es. il socket del client)
    struct job* next;        // Puntatore al prossimo lavoro nella coda (lista concatenata)
} job_t;

// Struttura principale del Pool di Thread
typedef struct {
    pthread_t* threads;       // Array che contiene i thread worker
    int num_threads;          // Numero totale di thread nel pool (nel nostro caso 10)
    job_t* queue_head;        // Testa della coda dei lavori
    job_t* queue_tail;        // Coda della lista dei lavori
    pthread_mutex_t queue_mutex; // Mutex per proteggere l'accesso alla coda
    sem_t queue_sem;          // Semaforo per contare i lavori pronti e svegliare i thread
    int shutdown;             // Dice ai thread di terminare l'esecuzione perchè non ci sono più lavori
} thread_pool_t;

// Funzioni per gestire il pool
thread_pool_t* thread_pool_init(int num_threads);
void thread_pool_add_job(thread_pool_t* pool, void (*function)(void*), void* arg);
void thread_pool_destroy(thread_pool_t* pool);

#endif