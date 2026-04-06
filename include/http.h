#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#define MAX_HTTP_METHOD 8
#define MAX_HTTP_PATH 256
#define MAX_HTTP_QUERY 256
#define MAX_HTTP_BODY 1024

#define MAX_HTTP_HEADERS 10
#define MAX_HTTP_HEADER_NAME 64
#define MAX_HTTP_HEADER_VALUE 256

typedef struct {
  char method[MAX_HTTP_METHOD];
  char path[MAX_HTTP_PATH];
  char query[MAX_HTTP_QUERY];
  char headers[MAX_HTTP_BODY]; /* Intera sezione headers per semplicità di
                                  parsing */
  char body[MAX_HTTP_BODY];
} HttpRequest;

typedef struct {
  int status_code;
  const char *status_message;
  char body[MAX_HTTP_BODY * 2];
  size_t body_len;
} HttpResponse;

/* Parsa una stringa di richiesta HTTP in una struct HttpRequest.
   Ritorna 0 se successo, -1 altrimenti. */
int http_parse_request(const char *raw_request, HttpRequest *out);

/* Costruisce una stringa di risposta HTTP da una struct HttpResponse.
   Ritorna la lunghezza della stringa scritta. */
size_t http_build_response(const HttpResponse *res, char *buffer, size_t size);

/* Estrae un parametro dalla query string (es: ?id=1) */
int http_get_query_param(const char *query, const char *key, char *value,
                         size_t size);

/* Estrae un header (es: X-Auth: value) */
int http_get_header(const char *headers_section, const char *key, char *value,
                    size_t size);

/* Estrae un parametro dal corpo (es: user=Mario&room_id=2) */
int http_get_body_param(const char *body, const char *key, char *value,
                        size_t size);

#endif
