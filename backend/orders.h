#ifndef ORDERS_H
#define ORDERS_H

#include <stddef.h>

struct Reorder{
    int priority;
    int sku;
    int qty;
    int eta;
    char date[25];
};
struct Order{
    int priority;
    int sku;
    int qty;
    char destination[100];
    char date[25];
};
struct orderQueueNode{
    struct Order order;
    struct orderQueueNode* next;
};
struct reorderQueueNode{
    struct Reorder reorder;
    struct reorderQueueNode* next;
};

struct orderQueue{
    struct orderQueueNode* front;
    struct orderQueueNode* rear;
};
struct reorderQueue{
    struct reorderQueueNode* front;
    struct reorderQueueNode* rear;
};

struct orderQueueNode* newOrderNode(struct Order order);
struct orderQueue* createOrderQueue();
void enqueueOrder(struct orderQueue* q, struct Order order);
void dequeueOrder(struct orderQueue* q);
char* getOrderQueueJSON(struct orderQueue* q);


struct reorderQueueNode* newReorderNode(struct Reorder reorder);
struct reorderQueue* createReorderQueue();
void enqueueReorder(struct reorderQueue* q, struct Reorder reorder);
void dequeueReorder(struct reorderQueue* q);
char* getReorderQueueJSON(struct reorderQueue* q);

#endif // ORDERS_H
