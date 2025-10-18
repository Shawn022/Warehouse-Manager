#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "inventory.h"
#include "orders.h"

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

    struct orderQueue* orderQ=createOrderQueue();
    struct reorderQueue* reorderQ=createReorderQueue();

        // ---- sample orders - enqueued at server start for testing (5 each) ----
        struct Order o1 = { .priority = 3, .sku = 1001, .qty = 5 };
        strncpy(o1.destination, "Dock A", sizeof(o1.destination)-1);
        strncpy(o1.date, "2025-10-18", sizeof(o1.date)-1);
        enqueueOrder(orderQ, o1);

        struct Order o2 = { .priority = 2, .sku = 1002, .qty = 3 };
        strncpy(o2.destination, "Dock B", sizeof(o2.destination)-1);
        strncpy(o2.date, "2025-10-18", sizeof(o2.date)-1);
        enqueueOrder(orderQ, o2);

        struct Order o3 = { .priority = 1, .sku = 2001, .qty = 10 };
        strncpy(o3.destination, "Outbound 1", sizeof(o3.destination)-1);
        strncpy(o3.date, "2025-10-19", sizeof(o3.date)-1);
        enqueueOrder(orderQ, o3);

        struct Order o4 = { .priority = 3, .sku = 2002, .qty = 2 };
        strncpy(o4.destination, "Outbound 2", sizeof(o4.destination)-1);
        strncpy(o4.date, "2025-10-20", sizeof(o4.date)-1);
        enqueueOrder(orderQ, o4);

        struct Order o5 = { .priority = 2, .sku = 3001, .qty = 1 };
        strncpy(o5.destination, "Retail", sizeof(o5.destination)-1);
        strncpy(o5.date, "2025-10-21", sizeof(o5.date)-1);
        enqueueOrder(orderQ, o5);

        // sample reorders (priority-ordered) - 5 entries
        struct Reorder r1 = { .priority = 3, .sku = 4001, .qty = 50, .eta = 7 };
        strncpy(r1.date, "2025-10-25", sizeof(r1.date)-1);
        enqueueReorder(reorderQ, r1);

        struct Reorder r2 = { .priority = 3, .sku = 4002, .qty = 30, .eta = 10 };
        strncpy(r2.date, "2025-10-26", sizeof(r2.date)-1);
        enqueueReorder(reorderQ, r2);

        struct Reorder r3 = { .priority = 2, .sku = 5001, .qty = 40, .eta = 14 };
        strncpy(r3.date, "2025-11-01", sizeof(r3.date)-1);
        enqueueReorder(reorderQ, r3);

        struct Reorder r4 = { .priority = 2, .sku = 5002, .qty = 25, .eta = 21 };
        strncpy(r4.date, "2025-11-08", sizeof(r4.date)-1);
        enqueueReorder(reorderQ, r4);

        struct Reorder r5 = { .priority = 1, .sku = 6001, .qty = 15, .eta = 30 };
        strncpy(r5.date, "2025-11-20", sizeof(r5.date)-1);
        enqueueReorder(reorderQ, r5);


    


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
