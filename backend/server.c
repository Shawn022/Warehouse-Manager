#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "inventory.h"
#include "orders.h"

#pragma comment(lib, "ws2_32.lib")

// Portable case-insensitive strstr
char *strcasestr_local(const char *haystack, const char *needle)
{
    if (!haystack || !needle)
        return NULL;
    size_t needle_len = strlen(needle);
    if (needle_len == 0)
        return (char *)haystack;
    for (; *haystack; haystack++)
    {
        size_t i;
        for (i = 0; i < needle_len; i++)
        {
            char a = haystack[i];
            char b = needle[i];
            if (!a)
                return NULL;
            if (tolower((unsigned char)a) != tolower((unsigned char)b))
                break;
        }
        if (i == needle_len)
            return (char *)haystack;
    }
    return NULL;
}

void send_response(SOCKET client, const char *json)
{
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
    if (len < 0)
        return;
    send(client, response, (int)len, 0);
}

void send_options_response(SOCKET client)
{
    char response[512];
    int len = snprintf(response, sizeof(response),
                       "HTTP/1.1 204 No Content\r\n"
                       "Access-Control-Allow-Origin: *\r\n"
                       "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                       "Access-Control-Allow-Headers: Content-Type\r\n"
                       "Access-Control-Max-Age: 86400\r\n"
                       "Connection: close\r\n"
                       "\r\n");
    if (len > 0)
        send(client, response, (int)len, 0);
}

void handle_get(SOCKET client, const char *path, struct hashTable *table, struct orderQueue *orderQ, struct reorderQueue *reorderQ)
{
    if (strcmp(path, "/") == 0)
    {
        send_response(client, "{\"message\":\"Hello from root!\"}");
    }
    else if (strcmp(path, "/inventory") == 0)
    {

        char *jsonStr = getInventoryJSON(table);
        send_response(client, jsonStr);
        free(jsonStr);
    }
    else if (strcmp(path, "/orders") == 0)
    {
        char *jsonStr = getOrderQueueJSON(orderQ);
        send_response(client, jsonStr);
        free(jsonStr);
    }
    else if (strcmp(path, "/reorders") == 0)
    {
        char *jsonStr = getReorderQueueJSON(reorderQ);
        send_response(client, jsonStr);
        free(jsonStr);
    }
    else
    {
        send_response(client, "{\"error\":\"Not Found\"}");
    }
}

void handle_post(SOCKET client, const char *path, const char *body, struct hashTable *table, struct orderQueue *orderQ, struct reorderQueue *reorderQ)
{
    if (strcmp(path, "/deleteitem") == 0)
    {
        printf("POST body: %s\n", body);
        int key = 0;

        const char *p = body;
        while (*p && (*p < '0' || *p > '9') && *p != '-')
            p++;
        if (*p)
            key = (int)strtol(p, NULL, 10);
        int removed = removeItem(table, key);
        if (removed)
            send_response(client, "{\"status\":\"Item deleted\"}");
        else
            send_response(client, "{\"status\":\"Item not found\"}");
    }
    else if (strcmp(path, "/additem") == 0)
    {
        printf("POST body: %s\n", body);
        struct Item newItem;
        memset(&newItem, 0, sizeof(newItem));
        newItem.id = 0;
        newItem.quantity = 0;
        newItem.price = 0.0f;
        newItem.reorder = 0;

        sscanf(body, "{\"sku\":\"%19[^\"]\",\"name\":\"%49[^\"]\",\"quantity\":%d,\"price\":%f,\"reorder\":%d,\"id\":%d}",
               newItem.sku, newItem.name, &newItem.quantity, &newItem.price, &newItem.reorder, &newItem.id);

        if (newItem.id <= 0)
        {
            int maxid = 0;
            for (int i = 0; i < table->size; ++i)
            {
                struct node *n = table->arr[i];
                while (n)
                {
                    if (n->value.id > maxid)
                        maxid = n->value.id;
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
        while (n)
        {
            printf(" - node id=%d sku=%s\n", n->value.id, n->value.sku);
            n = n->next;
        }
        send_response(client, "{\"status\":\"Item Added\"}");
    }
    else if (strcmp(path, "/updateitem") == 0)
    {
        printf("POST body: %s\n", body);
        struct Item updatedItem;
        memset(&updatedItem, 0, sizeof(updatedItem));
        updatedItem.id = 0;
        updatedItem.quantity = 0;
        updatedItem.price = 0.0f;
        updatedItem.reorder = 0;

        sscanf(body, "{\"id\":%d,\"sku\":\"%19[^\"]\",\"name\":\"%49[^\"]\",\"quantity\":%d,\"price\":%f,\"reorder\":%d}",
               &updatedItem.id, updatedItem.sku, updatedItem.name, &updatedItem.quantity, &updatedItem.price, &updatedItem.reorder);

        if (updatedItem.id <= 0)
        {
            send_response(client, "{\"error\":\"Invalid or missing id for update\"}");
            return;
        }

        updateItem(table, updatedItem);
        send_response(client, "{\"status\":\"Item Updated\"}");
    }

    //=====================REORDERS================================
    else if (strcmp(path, "/reorders") == 0)
    {
        struct Reorder newOrder = {0, 0, "", 0, 0, ""};

        sscanf(body, "{\"sku\":\"%19[^\"]\",\"qty\":\"%d\",\"priority\":\"%d\",\"etaDays\":%d,\"date\":\"%24[^\"]\"}",
               newOrder.sku, &newOrder.qty, &newOrder.priority, &newOrder.eta, newOrder.date);

        enqueueReorder(reorderQ, newOrder);
        printf("\n%d", newOrder.priority);
        send_response(client, "{\"status\":\"Order Added in Queue\"}");
    }
    else if (strcmp(path, "/reorders/approve") == 0)
    {
        struct Reorder reorder = peekReorder(reorderQ);
        if (reorder.sku[0] != '\0')
        {
            // Find item by SKU and add qty
            for (int i = 0; i < table->size; ++i)
            {
                struct node *curr = table->arr[i];
                while (curr)
                {
                    if (strcmp(curr->value.sku, reorder.sku) == 0)
                    {
                        curr->value.quantity += reorder.qty;
                        printf("Reorder approved: sku=%s new qty=%d\n", curr->value.sku, curr->value.quantity);
                    }
                    curr = curr->next;
                }
            }
            dequeueReorder(reorderQ);
            send_response(client, "{\"status\":\"Reorder Approved and Inventory Updated\"}");
        }
        else
        {
            send_response(client, "{\"status\":\"No Reorders to Approve\"}");
        }
    }
    else if (strcmp(path, "/reorders/cancel") == 0)
    {
        dequeueReorder(reorderQ);
        send_response(client, "{\"status\":\"Reorder Canceled\"}");
    }

    //===========================ORDERS===========================================
    else if (strcmp(path, "/orders") == 0)
    {
        struct Order newOrder = {0, 0, "", 0, "", ""};

        sscanf(body, "{\"sku\":\"%19[^\"]\",\"qty\":\"%d\",\"priority\":\"%d\",\"destination\":\"%49[^\"]\",\"date\":\"%24[^\"]\"}",
               newOrder.sku, &newOrder.qty, &newOrder.priority, newOrder.destination, newOrder.date);

        enqueueOrder(orderQ, newOrder);
        printf("\n%d ", newOrder.priority);
        send_response(client, "{\"status\":\"Order Added in Queue\"}");
    }
    else if (strcmp(path, "/orders/approve") == 0)
    {
        struct Order order = peekOrder(orderQ);
        if (order.sku[0] != '\0')
        {
            // Find item by SKU and subtract qty
            for (int i = 0; i < table->size; ++i)
            {
                struct node *curr = table->arr[i];
                while (curr)
                {
                    if (strcmp(curr->value.sku, order.sku) == 0)
                    {
                        curr->value.quantity -= order.qty;
                        if (curr->value.quantity < 0)
                            curr->value.quantity = 0;
                        printf("Order approved: sku=%s new qty=%d\n", curr->value.sku, curr->value.quantity);
                    }
                    curr = curr->next;
                }
            }
            dequeueOrder(orderQ);
            send_response(client, "{\"status\":\"Order Approved and Inventory Updated\"}");
        }
        else
        {
            send_response(client, "{\"status\":\"No Orders to Approve\"}");
        }
    }
    else if (strcmp(path, "/orders/cancel") == 0)
    {
        dequeueOrder(orderQ);
        send_response(client, "{\"status\":\"Order Canceled\"}");
    }
    else
    {
        send_response(client, "{\"error\":\"Invalid endpoint\"}");
    }
}

int main()
{
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;
    char buffer[4096];

    WSAStartup(MAKEWORD(2, 2), &wsa);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    bind(server_socket, (struct sockaddr *)&server, sizeof(server));
    listen(server_socket, 5);

    printf("Server running on http://localhost:8080\n");

    struct hashTable *table = loadTableData();

    struct orderQueue *orderQ = createOrderQueue();
    struct reorderQueue *reorderQ = createReorderQueue();

    // SAMPLE ORDERS FOR TESTING
    struct Order o1 = {3, 100, "A-100", 5, "Dock A", "2025-10-18"};
    enqueueOrder(orderQ, o1);
    struct Order o2 = {2, 200, "B-200", 3, "Dock B", "2025-10-18"};
    enqueueOrder(orderQ, o2);
    struct Order o3 = {1, 300, "C-300", 10, "Outbound 1", "2025-10-19"};
    enqueueOrder(orderQ, o3);
    struct Order o4 = {3, 300, "C-300", 2, "Outbound 2", "2025-10-20"};
    enqueueOrder(orderQ, o4);
    struct Order o5 = {2, 300, "C-300", 1, "Retail", "2025-10-21"};
    enqueueOrder(orderQ, o5);

    // sample reorders - FOR TESTING==============
    struct Reorder r1 = {3, 100, "A-100", 50, 7, "2025-10-25"};
    enqueueReorder(reorderQ, r1);
    struct Reorder r2 = {3, 100, "A-100", 30, 10, "2025-10-26"};
    enqueueReorder(reorderQ, r2);
    struct Reorder r3 = {2, 200, "B-200", 40, 14, "2025-11-01"};
    enqueueReorder(reorderQ, r3);
    struct Reorder r4 = {2, 200, "B-200", 25, 21, "2025-11-08"};
    enqueueReorder(reorderQ, r4);
    struct Reorder r5 = {1, 300, "C-300", 15, 30, "2025-11-20"};
    enqueueReorder(reorderQ, r5);

    while (1)
    {
        c = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        memset(buffer, 0, sizeof(buffer));
        int received = 0;
        // Read until we have headers
        while (1)
        {
            int r = recv(client_socket, buffer + received, sizeof(buffer) - 1 - received, 0);
            if (r <= 0)
                break;
            received += r;
            buffer[received] = '\0';
            if (strstr(buffer, "\r\n\r\n") != NULL)
                break;
            // if buffer full, stop
            if (received >= (int)sizeof(buffer) - 1)
                break;
        }

        // Parse method and path
        char method[16] = {0}, path[256] = {0};
        sscanf(buffer, "%15s %255s", method, path);

        // Find headers end
        char *headers_end = strstr(buffer, "\r\n\r\n");
        int content_len = 0;
        if (headers_end)
        {
            // parse Content-Length header if present
            // case-insensitive search for Content-Length
            char *cl = NULL;
            // We'll implement a portable case-insensitive strstr below and use it via function pointer
            extern char *strcasestr_local(const char *haystack, const char *needle);
            cl = strcasestr_local(buffer, "Content-Length:");
            if (cl)
            {
                cl += strlen("Content-Length:");
                while (*cl == ' ')
                    cl++;
                content_len = atoi(cl);
            }
            // If client expects 100-continue, send interim response so client sends body
            char *expect = strcasestr_local(buffer, "Expect: 100-continue");
            if (expect)
            {
                const char *cont = "HTTP/1.1 100 Continue\r\n\r\n";
                send(client_socket, cont, (int)strlen(cont), 0);
            }
        }

        // Read remaining body if any
        int body_offset = headers_end ? (int)(headers_end - buffer) + 4 : received;
        int body_bytes = received - body_offset;
        while (content_len > body_bytes)
        {
            int r = recv(client_socket, buffer + received, sizeof(buffer) - 1 - received, 0);
            if (r <= 0)
                break;
            received += r;
            body_bytes += r;
            buffer[received] = '\0';
        }

        char *body = "";
        if (body_offset < received)
            body = buffer + body_offset;
        else
            body = "";

        // Normalize path to lowercase for routing
        char path_lower[256] = {0};
        for (int i = 0; i < (int)strlen(path) && i < 255; ++i)
            path_lower[i] = (char)tolower((unsigned char)path[i]);

        // Log request for debugging
        printf("Request: %s %s\n", method, path);
        if (body && body[0])
            printf("Body (%d bytes): %.*s\n", body_bytes, body_bytes, body);

        if (strcmp(method, "GET") == 0)
        {
            handle_get(client_socket, path_lower, table, orderQ, reorderQ);
        }
        else if (strcmp(method, "POST") == 0)
        {
            handle_post(client_socket, path_lower, body, table, orderQ, reorderQ);
        }
        else if (strcmp(method, "OPTIONS") == 0)
        {
            send_options_response(client_socket);
        }
        else
        {
            send_response(client_socket, "{\"error\":\"Unsupported method\"}");
        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
