#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "inventory.h"

#pragma comment(lib, "ws2_32.lib")

// Portable case-insensitive strstr
char *strcasestr_local(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return (char *)haystack;
    for (; *haystack; haystack++) {
        size_t i;
        for (i = 0; i < needle_len; i++) {
            char a = haystack[i];
            char b = needle[i];
            if (!a) return NULL;
            if (tolower((unsigned char)a) != tolower((unsigned char)b)) break;
        }
        if (i == needle_len) return (char *)haystack;
    }
    return NULL;
}

// Helper: extract a quoted string value for key from a JSON-ish body.
// Returns 1 on success (out filled), 0 on failure.
int extract_json_string(const char *body, const char *key, char *out, size_t out_size) {
    if (!body || !key || !out) return 0;
    char keypat[64];
    snprintf(keypat, sizeof(keypat), "\"%s\"", key);
    char *k = strcasestr_local(body, keypat);
    if (!k) return 0;
    // find ':' after key
    char *colon = strchr(k, ':');
    if (!colon) return 0;
    char *val = colon + 1;
    // skip spaces
    while (*val && isspace((unsigned char)*val)) val++;
    // if starts with quote, copy until next quote
    if (*val == '"') {
        val++;
        size_t i = 0;
        while (*val && *val != '"' && i + 1 < out_size) {
            out[i++] = *val++;
        }
        out[i] = '\0';
        return 1;
    } else {
        // unquoted value: copy until comma or brace
        size_t i = 0;
        while (*val && *val != ',' && *val != '}' && !isspace((unsigned char)*val) && i + 1 < out_size) {
            out[i++] = *val++;
        }
        out[i] = '\0';
        return i > 0;
    }
}

// Helper: extract integer value for key. Returns 1 on success and stores in *out, 0 otherwise.
int extract_json_int(const char *body, const char *key, int *out) {
    if (!body || !key || !out) return 0;
    char keypat[64];
    snprintf(keypat, sizeof(keypat), "\"%s\"", key);
    char *k = strcasestr_local(body, keypat);
    if (!k) return 0;
    char *colon = strchr(k, ':');
    if (!colon) return 0;
    char *val = colon + 1;
    while (*val && isspace((unsigned char)*val)) val++;
    char *endptr = NULL;
    long v = strtol(val, &endptr, 10);
    if (val == endptr) return 0;
    *out = (int)v;
    return 1;
}

// Helper: extract float/double value for key. Returns 1 on success.
int extract_json_float(const char *body, const char *key, float *out) {
    if (!body || !key || !out) return 0;
    char keypat[64];
    snprintf(keypat, sizeof(keypat), "\"%s\"", key);
    char *k = strcasestr_local(body, keypat);
    if (!k) return 0;
    char *colon = strchr(k, ':');
    if (!colon) return 0;
    char *val = colon + 1;
    while (*val && isspace((unsigned char)*val)) val++;
    char *endptr = NULL;
    double v = strtod(val, &endptr);
    if (val == endptr) return 0;
    *out = (float)v;
    return 1;
}

void send_response(SOCKET client, const char *json) {
    char response[8192];
    int content_len = (int)strlen(json);
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        content_len, json);
    if (len < 0) return;
    send(client, response, (int)len, 0);
}

void send_options_response(SOCKET client) {
    char response[512];
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 86400\r\n"
        "Connection: close\r\n"
        "\r\n");
    if (len > 0) send(client, response, (int)len, 0);
}

void handle_get(SOCKET client, const char *path, struct hashTable* table) {
    if (strcmp(path, "/") == 0) {
        send_response(client, "{\"message\":\"Hello from root!\"}");
    } 
    else if(strcmp(path, "/inventory") == 0) {
        
        char* jsonStr=getInventoryJSON(table);
        send_response(client, jsonStr);
        free(jsonStr);
    }
    else if (strcmp(path, "/data") == 0) {
        send_response(client, "{\"data\":[1,2,3,4]}");
    } 
    else {
        send_response(client, "{\"error\":\"Not Found\"}");
    }
}

void handle_post(SOCKET client, const char *path, const char *body, struct hashTable* table) {
    if (strcmp(path, "/submit") == 0) {
        printf("POST body: %s\n", body);
        send_response(client, "{\"status\":\"Data received\"}");
    } 
    else if(strcmp(path, "/deleteitem") == 0) {
        printf("POST body: %s\n", body);
        int key = 0; 
       
        const char *p = body;
        while (*p && (*p < '0' || *p > '9') && *p != '-') p++;
        if (*p) key = (int)strtol(p, NULL, 10);
        int removed = removeItem(table, key);
        if (removed) send_response(client, "{\"status\":\"Item deleted\"}");
        else send_response(client, "{\"status\":\"Item not found\"}");
    }
    else if(strcmp(path,"/additem")==0){
        printf("POST body: %s\n", body);
        struct Item newItem;
        memset(&newItem, 0, sizeof(newItem));
        newItem.id = 0;
        newItem.quantity = 0;
        newItem.price = 0.0f;
        newItem.reorder = 0;

        extract_json_string(body, "sku", newItem.sku, sizeof(newItem.sku));
        extract_json_string(body, "name", newItem.name, sizeof(newItem.name));

        extract_json_int(body, "quantity", &newItem.quantity);
        extract_json_int(body, "reorder", &newItem.reorder);
        extract_json_int(body, "id", &newItem.id);
        extract_json_float(body, "price", &newItem.price);

        if (newItem.id <= 0) {
            int maxid = 0;
            for (int i = 0; i < table->size; ++i) {
                struct node *n = table->arr[i];
                while (n) {
                    if (n->value.id > maxid) maxid = n->value.id;
                    n = n->next;
                }
            }
            newItem.id = maxid + 1;
        }

        // Insert the item
        insertItem(table, newItem);
        
        int idx = hashFunction(table, newItem.id);
        printf("Inserted item id=%d sku=%s at index=%d\n", newItem.id, newItem.sku, idx);
        struct node *n = table->arr[idx];
        while (n) {
            printf(" - node id=%d sku=%s\n", n->value.id, n->value.sku);
            n = n->next;
        }
        send_response(client, "{\"status\":\"Item Added\"}");
    }
    else if(strcmp(path,"/updateitem")==0){
        printf("POST body: %s\n",body);
        struct Item updatedItem;
        memset(&updatedItem, 0, sizeof(updatedItem));
        updatedItem.id = 0;
        updatedItem.quantity = 0;
        updatedItem.price = 0.0f;
        updatedItem.reorder = 0;

        sscanf(body, "{\"id\":%d,\"sku\":\"%19[^\"]\",\"name\":\"%49[^\"]\",\"quantity\":%d,\"price\":%f,\"reorder\":%d}",
               &updatedItem.id, updatedItem.sku, updatedItem.name, &updatedItem.quantity, &updatedItem.price, &updatedItem.reorder);

        if (updatedItem.id <= 0) {
            send_response(client, "{\"error\":\"Invalid or missing id for update\"}");
            return;
        }

        updateItem(table, updatedItem);
        send_response(client, "{\"status\":\"Item Updated\"}");
    }
    else {
        send_response(client, "{\"error\":\"Invalid endpoint\"}");
    }
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;
    char buffer[4096];

    WSAStartup(MAKEWORD(2,2), &wsa);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    bind(server_socket, (struct sockaddr *)&server, sizeof(server));
    listen(server_socket, 5);

    printf("Server running on http://localhost:8080\n");

    struct hashTable* table=createTable(10);
        // Sample items
        struct Item item1={1,"A-100","ToothBrushes",120,9.99,50};
        struct Item item2={2,"B-200","ToothPaste",8,19.99,20};
        struct Item item3={3,"C-300","MouthWash",45,14.99,40};
        insertItem(table,item1);
        insertItem(table,item2);
        insertItem(table,item3);


    while (1) {
        c = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        memset(buffer, 0, sizeof(buffer));
        int received = 0;
        // Read until we have headers
        while (1) {
            int r = recv(client_socket, buffer + received, sizeof(buffer) - 1 - received, 0);
            if (r <= 0) break;
            received += r;
            buffer[received] = '\0';
            if (strstr(buffer, "\r\n\r\n") != NULL) break;
            // if buffer full, stop
            if (received >= (int)sizeof(buffer) - 1) break;
        }

        // Parse method and path
        char method[16] = {0}, path[256] = {0};
        sscanf(buffer, "%15s %255s", method, path);

        // Find headers end
        char *headers_end = strstr(buffer, "\r\n\r\n");
        int content_len = 0;
        if (headers_end) {
            // parse Content-Length header if present
            // case-insensitive search for Content-Length
            char *cl = NULL;
            // We'll implement a portable case-insensitive strstr below and use it via function pointer
            extern char *strcasestr_local(const char *haystack, const char *needle);
            cl = strcasestr_local(buffer, "Content-Length:");
            if (cl) {
                cl += strlen("Content-Length:");
                while (*cl == ' ') cl++;
                content_len = atoi(cl);
            }
            // If client expects 100-continue, send interim response so client sends body
            char *expect = strcasestr_local(buffer, "Expect: 100-continue");
            if (expect) {
                const char *cont = "HTTP/1.1 100 Continue\r\n\r\n";
                send(client_socket, cont, (int)strlen(cont), 0);
            }
        }

        // Read remaining body if any
        int body_offset = headers_end ? (int)(headers_end - buffer) + 4 : received;
        int body_bytes = received - body_offset;
        while (content_len > body_bytes) {
            int r = recv(client_socket, buffer + received, sizeof(buffer) - 1 - received, 0);
            if (r <= 0) break;
            received += r;
            body_bytes += r;
            buffer[received] = '\0';
        }

        char *body = "";
        if (body_offset < received) body = buffer + body_offset; else body = "";

    // Normalize path to lowercase for routing
    char path_lower[256] = {0};
    for (int i = 0; i < (int)strlen(path) && i < 255; ++i) path_lower[i] = (char)tolower((unsigned char)path[i]);

    // Log request for debugging
    printf("Request: %s %s\n", method, path);
        if (body && body[0]) printf("Body (%d bytes): %.*s\n", body_bytes, body_bytes, body);

        if (strcmp(method, "GET") == 0) {
            handle_get(client_socket, path_lower , table);
        } else if (strcmp(method, "POST") == 0) {
            handle_post(client_socket, path_lower, body, table);
        } else if (strcmp(method, "OPTIONS") == 0) {
            send_options_response(client_socket);
        } else {
            send_response(client_socket, "{\"error\":\"Unsupported method\"}");
        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
