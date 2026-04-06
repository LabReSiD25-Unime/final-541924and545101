#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>
#include <semaphore.h>

/* Inizializza un mutex */
void sync_mutex_init(pthread_mutex_t *mutex);

/* Distrugge un mutex */
void sync_mutex_destroy(pthread_mutex_t *mutex);

/* Blocca un mutex */
void sync_mutex_lock(pthread_mutex_t *mutex);

/* Sblocca un mutex */
void sync_mutex_unlock(pthread_mutex_t *mutex);

/* Inizializza un semaforo */
void sync_sem_init(sem_t *sem, int value);

/* Distrugge un semaforo */
void sync_sem_destroy(sem_t *sem);

/* Wait su un semaforo */
void sync_sem_wait(sem_t *sem);

/* Post su un semaforo */
void sync_sem_post(sem_t *sem);

#endif
