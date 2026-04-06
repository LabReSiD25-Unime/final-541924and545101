#ifndef ROUTES_H
#define ROUTES_H

#include "http.h"

/* Gestisce una richiesta HTTP e popola la risposta.
   Ritorna 0 se successo, -1 altrimenti. */
void routes_handle_request(const HttpRequest *req, HttpResponse *res);

#endif
