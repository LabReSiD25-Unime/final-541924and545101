#include "routes.h"
#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void routes_handle_request(const HttpRequest *req, HttpResponse *res) {
  char auth_header[64];
  if (req == NULL || res == NULL)
    return;

  memset(res, 0, sizeof(HttpResponse));

  /* Controllo Autenticazione (per 401 Unauthorized) */
  if (http_get_header(req->headers, "X-Auth", auth_header,
                      sizeof(auth_header)) != 0 ||
      strcmp(auth_header, "labresid2025") != 0) {
    res->status_code = 401;
    res->status_message = "Unauthorized";
    snprintf(res->body, sizeof(res->body),
             "Errore 401: Non autorizzato. Header 'X-Auth: labresid2025' "
             "mancante o errato.\n");
    res->body_len = strlen(res->body);
    return;
  }

  res->status_code = 200;
  res->status_message = "OK";

  if (strcmp(req->method, "GET") == 0) {
    if (strcmp(req->path, "/rooms") == 0) {
      app_get_rooms(res->body, sizeof(res->body));
      res->body_len = strlen(res->body);
    } else if (strcmp(req->path, "/bookings") == 0) {
      app_get_bookings(res->body, sizeof(res->body));
      res->body_len = strlen(res->body);
    } else {
      res->status_code = 404;
      res->status_message = "Not Found";
      snprintf(res->body, sizeof(res->body),
               "Errore 404: Pagina non trovata.\n");
      res->body_len = strlen(res->body);
    }
  } else if (strcmp(req->method, "POST") == 0) {
    if (strcmp(req->path, "/book") == 0) {
      char user[64];
      char room_id_str[16];
      int room_id;

      if (http_get_body_param(req->body, "user", user, sizeof(user)) == 0 &&
          http_get_body_param(req->body, "room_id", room_id_str,
                              sizeof(room_id_str)) == 0) {

        room_id = atoi(room_id_str);
        int result =
            app_post_booking(user, room_id, res->body, sizeof(res->body));

        if (result == 0) {
          res->status_code = 201;
          res->status_message = "Created";
        } else {
          res->status_code = 400;
          res->status_message = "Bad Request";
        }
      } else {
        res->status_code = 400;
        res->status_message = "Bad Request";
        snprintf(res->body, sizeof(res->body),
                 "Errore: parametri mancano (user, room_id).\n");
      }
      res->body_len = strlen(res->body);
    } else {
      res->status_code = 404;
      res->status_message = "Not Found";
      res->body_len = 0;
    }
  } else if (strcmp(req->method, "PUT") == 0) {
    if (strcmp(req->path, "/book") == 0) {
      char id_str[16];
      char user[64];
      int booking_id;

      if (http_get_query_param(req->query, "id", id_str, sizeof(id_str)) == 0 &&
          http_get_query_param(req->query, "user", user, sizeof(user)) == 0) {

        booking_id = atoi(id_str);
        int result =
            app_put_booking(booking_id, user, res->body, sizeof(res->body));

        if (result == 0) {
          res->status_code = 200;
          res->status_message = "OK";
        } else {
          res->status_code = 404;
          res->status_message = "Not Found";
        }
      } else {
        res->status_code = 400;
        res->status_message = "Bad Request";
        snprintf(res->body, sizeof(res->body),
                 "Errore: parametri mancano (id, user).\n");
      }
      res->body_len = strlen(res->body);
    } else {
      res->status_code = 404;
      res->status_message = "Not Found";
      res->body_len = 0;
    }
  } else if (strcmp(req->method, "DELETE") == 0) {
    if (strcmp(req->path, "/book") == 0) {
      char id_str[16];
      int booking_id;

      if (http_get_query_param(req->query, "id", id_str, sizeof(id_str)) == 0) {
        booking_id = atoi(id_str);
        int result =
            app_delete_booking(booking_id, res->body, sizeof(res->body));

        if (result == 0) {
          res->status_code = 204;
          res->status_message = "No Content";
          res->body_len = 0;
          res->body[0] = '\0';
        } else {
          res->status_code = 404;
          res->status_message = "Not Found";
        }
      } else {
        res->status_code = 400;
        res->status_message = "Bad Request";
        snprintf(res->body, sizeof(res->body),
                 "Errore: parametro 'id' mancante.\n");
      }
      res->body_len = strlen(res->body);
    } else {
      res->status_code = 404;
      res->status_message = "Not Found";
      res->body_len = 0;
    }
  } else {
    res->status_code = 405;
    res->status_message = "Method Not Allowed";
    snprintf(res->body, sizeof(res->body), "Metodo %s non supportato.\n",
             req->method);
    res->body_len = strlen(res->body);
  }
}
