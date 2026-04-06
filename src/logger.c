#include "logger.h"
#include "sync.h"
#include <stdio.h>
#include <time.h>

static pthread_mutex_t logger_mutex;

void logger_init(void) { sync_mutex_init(&logger_mutex); }

void logger_destroy(void) { sync_mutex_destroy(&logger_mutex); }

static void get_timestamp(char *buffer, size_t size) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void logger_log(const char *format, ...) {
  char timestamp[20];
  va_list args;

  get_timestamp(timestamp, sizeof(timestamp));

  sync_mutex_lock(&logger_mutex);
  printf("[%s] INFO: ", timestamp);
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\n");
  fflush(stdout);
  sync_mutex_unlock(&logger_mutex);
}

void logger_error(const char *format, ...) {
  char timestamp[20];
  va_list args;

  get_timestamp(timestamp, sizeof(timestamp));

  sync_mutex_lock(&logger_mutex);
  fprintf(stderr, "[%s] ERROR: ", timestamp);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
  fflush(stderr);
  sync_mutex_unlock(&logger_mutex);
}
