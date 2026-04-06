#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int http_parse_request(const char *raw_request, HttpRequest *out) {
  char first_line[512];
  char *line_end;
  char *method, *full_path, *version;
  char *query_start;

  if (raw_request == NULL || out == NULL)
    return -1;

  memset(out, 0, sizeof(HttpRequest));

  /* Estraiamo la prima riga */
  line_end = strstr(raw_request, "\r\n");
  if (line_end == NULL)
    return -1;

  size_t line_len = line_end - raw_request;
  if (line_len >= sizeof(first_line))
    return -1;
  strncpy(first_line, raw_request, line_len);
  first_line[line_len] = '\0';

  /* Parsing Method Path Version */
  method = strtok(first_line, " ");
  full_path = strtok(NULL, " ");
  version = strtok(NULL, " ");

  if (method == NULL || full_path == NULL)
    return -1;

  strncpy(out->method, method, MAX_HTTP_METHOD - 1);

  /* Separiamo path e query string */
  query_start = strchr(full_path, '?');
  if (query_start != NULL) {
    *query_start = '\0';
    strncpy(out->path, full_path, MAX_HTTP_PATH - 1);
    strncpy(out->query, query_start + 1, MAX_HTTP_QUERY - 1);
  } else {
    strncpy(out->path, full_path, MAX_HTTP_PATH - 1);
  }

  /* Salviamo la sezione headers */
  char *headers_end = strstr(raw_request, "\r\n\r\n");
  if (headers_end != NULL) {
    size_t headers_len = (size_t)(headers_end - line_end - 2);
    if (headers_len >= MAX_HTTP_BODY)
      headers_len = MAX_HTTP_BODY - 1;
    strncpy(out->headers, line_end + 2, headers_len);
    out->headers[headers_len] = '\0';

    char *body_start = headers_end + 4;
    strncpy(out->body, body_start, MAX_HTTP_BODY - 1);
  }

  return 0;
}

size_t http_build_response(const HttpResponse *res, char *buffer, size_t size) {
  if (res == NULL || buffer == NULL || size == 0)
    return 0;

  /* Per il 204 No Content non restituiamo il corpo */
  if (res->status_code == 204) {
    return snprintf(buffer, size,
                    "HTTP/1.1 204 No Content\r\n"
                    "Connection: close\r\n"
                    "\r\n");
  }

  return snprintf(buffer, size,
                  "HTTP/1.1 %d %s\r\n"
                  "Content-Type: text/plain; charset=utf-8\r\n"
                  "Content-Length: %zu\r\n"
                  "Connection: close\r\n"
                  "\r\n"
                  "%s",
                  res->status_code, res->status_message, res->body_len,
                  res->body);
}

static char *find_param(const char *source, const char *key) {
  if (source == NULL || key == NULL)
    return NULL;

  char *p = strstr(source, key);
  while (p != NULL) {
    /* Controlliamo che sia l'inizio di un parametro (inizio stringa o dopo &)
     */
    if (p == source || *(p - 1) == '&') {
      char *eq = strchr(p, '=');
      if (eq != NULL)
        return eq + 1;
    }
    p = strstr(p + 1, key);
  }
  return NULL;
}

int http_get_query_param(const char *query, const char *key, char *value,
                         size_t size) {
  char key_eq[MAX_HTTP_HEADER_NAME];
  snprintf(key_eq, sizeof(key_eq), "%s=", key);

  char *val_start = find_param(query, key_eq);
  if (val_start == NULL)
    return -1;

  char *val_end = strchr(val_start, '&');
  size_t len =
      (val_end != NULL) ? (size_t)(val_end - val_start) : strlen(val_start);

  if (len >= size)
    len = size - 1;
  strncpy(value, val_start, len);
  value[len] = '\0';

  return 0;
}

int http_get_header(const char *headers_section, const char *key, char *value,
                    size_t size) {
  if (headers_section == NULL || key == NULL || value == NULL)
    return -1;

  char key_colon[MAX_HTTP_HEADER_NAME];
  snprintf(key_colon, sizeof(key_colon), "%s: ", key);

  char *line = strstr(headers_section, key_colon);
  if (line == NULL)
    return -1;

  char *val_start = line + strlen(key_colon);
  char *val_end = strstr(val_start, "\r\n");
  size_t len =
      (val_end != NULL) ? (size_t)(val_end - val_start) : strlen(val_start);

  if (len >= size)
    len = size - 1;
  strncpy(value, val_start, len);
  value[len] = '\0';

  return 0;
}

int http_get_body_param(const char *body, const char *key, char *value,
                        size_t size) {
  return http_get_query_param(body, key, value, size);
}
