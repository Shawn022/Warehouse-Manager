#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "orders.h"

// ORDER QUEUE FUNCTIONS
struct orderQueueNode *newOrderNode(struct Order order)
{
    struct orderQueueNode *temp = (struct orderQueueNode *)malloc(sizeof(struct orderQueueNode));
    if (temp)
    {
        temp->order = order;
        temp->next = NULL;
    }
    return temp;
}
struct orderQueue *createOrderQueue()
{
    struct orderQueue *q = (struct orderQueue *)malloc(sizeof(struct orderQueue));
    q->front = NULL;
    q->rear = NULL;
    return q;
}
void enqueueOrder(struct orderQueue *q, struct Order order)
{
    if (order.priority <= 0)
        return;
    struct orderQueueNode *temp = newOrderNode(order);
    if (!q->front && !q->rear)
    {
        q->front = temp;
        q->rear = temp;
        return;
    }
    struct orderQueueNode *curr = q->front;
    if ((curr->order.priority < order.priority))
    {
        temp->next=curr;
        q->front=temp;
        return;
    }
    while (curr->next && curr->next->order.priority >= order.priority)
    {
        curr = curr->next;
    }
    struct orderQueueNode *nextNode = curr->next;
    curr->next = temp;
    temp->next = nextNode;
    if (temp->next == NULL)
    {
        q->rear = temp;
    }
}
void dequeueOrder(struct orderQueue *q)
{
    if (!q->front)
    {
        printf("Order Queue is empty\n");
        return;
    }
    struct orderQueueNode *temp = q->front;
    q->front = q->front->next;
    if (!q->front || !q->front->next)
    {
        q->rear = q->front;
    }
    free(temp);
}
struct Order peekOrder(struct orderQueue *q)
{
    if (!q->front)
    {
        struct Order emptyOrder = {0};
        return emptyOrder;
    }
    return q->front->order;
}
struct orderQueue* loadOrderQueue(){
    struct orderQueue *q=createOrderQueue();
    FILE* f1=fopen("saves/orders.bin","rb");
    if(!f1)return q;
    int size=100,count=0;
    struct Order *arr=(struct Order*)malloc(sizeof(struct Order)*size);
    if(!arr){
        fclose(f1);
        return q;
    }
    while(1){
        if(count>=size){
            struct Order *tmp = (struct Order *)realloc(arr, sizeof(struct Order) * size*2);
            if (!tmp)
            {
                // realloc failed, stop reading further
                break;
            }
            arr = tmp;
            size = size *2;
        }
        size_t read = fread(&arr[count], sizeof(struct Order), 1, f1);
        if (read != 1)
            break;
        count++;
    }
    for(int i=0;i<count;i++){
        enqueueOrder(q,arr[i]);
    }
    free(arr);
    fclose(f1);
    return q;
}
void saveOrders(struct orderQueue* q){
    if(!q->front)return;
    struct orderQueueNode* curr=q->front;
    FILE* f1=fopen("saves/orders.bin","wb");
    while(curr){
        fwrite(&(curr->order),sizeof(struct Order),1,f1);
        curr=curr->next;
    }
    fclose(f1);
}
char *getOrderQueueJSON(struct orderQueue *q)
{
    if (!q->front)
    {
        char *emptyJson = (char *)malloc(20 * sizeof(char));
        strcpy(emptyJson, "[]");
        return emptyJson;
    }
    char *jsonStr = (char *)malloc(1024 * sizeof(char));
    strcpy(jsonStr, "[");
    struct orderQueueNode *curr = q->front;
    while (curr)
    {
        char orderEntry[256];
        snprintf(orderEntry, sizeof(orderEntry),
                 "{\"priority\":%d,\"sku\":\"%s\",\"qty\":%d,\"destination\":\"%s\",\"date\":\"%s\"}",
                 curr->order.priority,
                 curr->order.sku,
                 curr->order.qty,
                 curr->order.destination,
                 curr->order.date);
        strcat(jsonStr, orderEntry);
        if (curr->next)
        {
            strcat(jsonStr, ",");
        }
        curr = curr->next;
    }
    strcat(jsonStr, "]");
    return jsonStr;
}

/*------------------------------------------------------------------------------------------------------------------------------------------------*/

// REODER QUEUE FUNCTIONS
struct reorderQueueNode *newReorderNode(struct Reorder reorder)
{
    struct reorderQueueNode *temp = (struct reorderQueueNode *)malloc(sizeof(struct reorderQueueNode));
    if (temp)
    {
        temp->reorder = reorder;
        temp->next = NULL;
    }
    return temp;
}

struct reorderQueue *createReorderQueue()
{
    struct reorderQueue *q = (struct reorderQueue *)malloc(sizeof(struct reorderQueue));
    q->front = NULL;
    q->rear = NULL;
    return q;
}
void enqueueReorder(struct reorderQueue *q, struct Reorder reorder)
{
    if (reorder.priority <= 0)
        return;
    struct reorderQueueNode *temp = newReorderNode(reorder);
    if (!q->front && !q->rear)
    {
        q->front = temp;
        q->rear = temp;
        return;
    }
    struct reorderQueueNode *curr = q->front;
    if ((curr->reorder.priority < reorder.priority))
    {
        temp->next=curr;
        q->front=temp;
        return;
    }
    while (curr->next && curr->next->reorder.priority >= reorder.priority)
    {
        curr = curr->next;
    }
    struct reorderQueueNode *nextNode = curr->next;
    curr->next = temp;
    temp->next = nextNode;
    if (temp->next == NULL)
    {
        q->rear = temp;
    }
}
void dequeueReorder(struct reorderQueue *q)
{
    if (!q->front)
    {
        printf("Reorder Queue is empty\n");
        return;
    }
    struct reorderQueueNode *temp = q->front;
    q->front = q->front->next;
    if (!q->front || !q->front->next)
    {
        q->rear = q->front;
    }
    free(temp);
}
struct Reorder peekReorder(struct reorderQueue *q)
{
    if (!q->front)
    {
        struct Reorder temp = {0};
        return temp;
    }
    return q->front->reorder;
}
struct reorderQueue* loadReorderQueue(){
    struct reorderQueue *q=createReorderQueue();
    FILE* f1=fopen("saves/reorders.bin","rb");
    if(!f1)return q;
    int size=100,count=0;
    struct Reorder *arr=(struct Reorder*)malloc(sizeof(struct Reorder)*size);
    if(!arr){
        fclose(f1);
        return q;
    }
    while(1){
        if(count>=size){
            struct Reorder *tmp = (struct Reorder *)realloc(arr, sizeof(struct Reorder) * size*2);
            if (!tmp)
            {
                // realloc failed, stop reading further
                break;
            }
            arr = tmp;
            size = size *2;
        }
        size_t read = fread(&arr[count], sizeof(struct Reorder), 1, f1);
        if (read != 1)
            break;
        count++;
    }
    for(int i=0;i<count;i++){
        enqueueReorder(q,arr[i]);
    }
    free(arr);
    fclose(f1);
    return q;
}
void saveReorders(struct reorderQueue* q){
    if(!q->front)return;
    struct reorderQueueNode* curr=q->front;
    FILE* f1=fopen("saves/reorders.bin","wb");
    while(curr){
        fwrite(&(curr->reorder),sizeof(struct Reorder),1,f1);
        curr=curr->next;
    }
    fclose(f1);
}

char *getReorderQueueJSON(struct reorderQueue *q)
{
    if (!q->front)
    {
        char *emptyJson = (char *)malloc(20 * sizeof(char));
        strcpy(emptyJson, "[]");
        return emptyJson;
    }
    char *jsonStr = (char *)malloc(1024 * sizeof(char));
    strcpy(jsonStr, "[");
    struct reorderQueueNode *curr = q->front;
    while (curr)
    {
        char reorderEntry[256];
        snprintf(reorderEntry, sizeof(reorderEntry),
                 "{\"priority\":%d,\"sku\":\"%s\",\"qty\":%d,\"eta\":%d,\"date\":\"%s\"}",
                 curr->reorder.priority,
                 curr->reorder.sku,
                 curr->reorder.qty,
                 curr->reorder.eta,
                 curr->reorder.date);
        strcat(jsonStr, reorderEntry);
        if (curr->next)
        {
            strcat(jsonStr, ",");
        }
        curr = curr->next;
    }
    strcat(jsonStr, "]");
    return jsonStr;
}