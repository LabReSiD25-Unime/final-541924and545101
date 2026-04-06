#include "sync.h"
#include <stdio.h>
#include <stdlib.h>

void sync_mutex_init(pthread_mutex_t *mutex) {
  if (pthread_mutex_init(mutex, NULL) != 0) {
    perror("Errore inizializzazione mutex");
    exit(EXIT_FAILURE);
  }
}

void sync_mutex_destroy(pthread_mutex_t *mutex) {
  pthread_mutex_destroy(mutex);
}

void sync_mutex_lock(pthread_mutex_t *mutex) { pthread_mutex_lock(mutex); }

void sync_mutex_unlock(pthread_mutex_t *mutex) { pthread_mutex_unlock(mutex); }

void sync_sem_init(sem_t *sem, int value) {
  if (sem_init(sem, 0, value) != 0) {
    perror("Errore inizializzazione semaforo");
    exit(EXIT_FAILURE);
  }
}

void sync_sem_destroy(sem_t *sem) { sem_destroy(sem); }

void sync_sem_wait(sem_t *sem) { sem_wait(sem); }

void sync_sem_post(sem_t *sem) { sem_post(sem); }
