#include "../include/http_parser.h"
#include "../include/resource_manager.h"
#include "../include/http_handler.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define AUTH_TOKEN "Basic YWRtaW46cGFzc3dvcmQ=" // admin:password

static void send_response(int socket, const char* status, const char* content_type, const char* body) {
    char response[MAX_HTTP_SIZE];
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status, content_type, body ? strlen(body) : 0, body ? body : "");
    send(socket, response, len, MSG_NOSIGNAL);
}

void handle_client(int client_socket, resource_manager_t* rm) {
    char buffer[MAX_HTTP_SIZE];
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    if (bytes_read == sizeof(buffer) - 1) {
        send_response(client_socket, "413 Payload Too Large", "text/plain", "Payload exceeds maximum allowed size");
        close(client_socket);
        return;
    }
    
    buffer[bytes_read] = '\0';

    http_request_t request;
    if (parse_http_request(buffer, &request) != 0) {
        send_response(client_socket, "400 Bad Request", "text/plain", "Invalid HTTP Request");
        close(client_socket);
        return;
    }

    // Basic Auth Check
    if (strstr(buffer, "Authorization: ") == NULL || strstr(buffer, AUTH_TOKEN) == NULL) {
        send_response(client_socket, "401 Unauthorized", "text/plain", "Authentication Required\r\nWWW-Authenticate: Basic realm=\"Restricted\"");
        close(client_socket);
        return;
    }

    if (strcmp(request.method, "GET") == 0) {
        char response_body[MAX_HTTP_SIZE];
        rm_read_all(rm, response_body, sizeof(response_body));
        send_response(client_socket, "200 OK", "application/json", response_body);
    } else if (strcmp(request.method, "POST") == 0) {
        char room[MAX_STR_LEN], student[MAX_STR_LEN], time[MAX_STR_LEN];
        // Expected body: room=AulaA&student=Massimo&time=14-16
        if (sscanf(request.body, "room=%[^&]&student=%[^&]&time=%s", room, student, time) == 3) {
            int id = rm_create(rm, room, student, time);
            char res[64];
            snprintf(res, sizeof(res), "{\"id\": %d}", id);
            send_response(client_socket, "201 Created", "application/json", res);
        } else {
            send_response(client_socket, "400 Bad Request", "text/plain", "Missing reservation parameters (room, student, time)");
        }
    } else if (strcmp(request.method, "PUT") == 0) {
        int id;
        char room[MAX_STR_LEN], student[MAX_STR_LEN], time[MAX_STR_LEN];
        if (sscanf(request.uri, "/reservations/%d", &id) == 1 && 
            sscanf(request.body, "room=%[^&]&student=%[^&]&time=%s", room, student, time) == 3) {
            if (rm_update(rm, id, room, student, time) == 0) {
                send_response(client_socket, "200 OK", "application/json", "{\"status\": \"updated\"}");
            } else {
                send_response(client_socket, "404 Not Found", "text/plain", "Reservation not found");
            }
        } else {
            send_response(client_socket, "400 Bad Request", "text/plain", "Invalid URI or body parameters");
        }
    } else if (strcmp(request.method, "DELETE") == 0) {
        int id;
        if (sscanf(request.uri, "/reservations/%d", &id) == 1) {
            if (rm_delete(rm, id) == 0) {
                send_response(client_socket, "204 No Content", "text/plain", "");
            } else {
                send_response(client_socket, "404 Not Found", "text/plain", "Reservation not found");
            }
        } else {
            send_response(client_socket, "400 Bad Request", "text/plain", "Invalid URI");
        }
    } else {
        send_response(client_socket, "405 Method Not Allowed", "text/plain", "Method not supported");
    }

    close(client_socket);
}
