#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

/* Inizializza il logger */
void logger_init(void);

/* Distrugge il logger */
void logger_destroy(void);

/* Logga un messaggio con formato printf */
void logger_log(const char *format, ...);

/* Logga un errore con formato printf */
void logger_error(const char *format, ...);

#endif
