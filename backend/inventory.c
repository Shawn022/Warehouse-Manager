#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inventory.h"

struct node *newnode(int key, struct Item value)
{
    struct node *temp = (struct node *)malloc(sizeof(struct node));
    temp->key = key;
    temp->value = value;
    temp->next = NULL;
    return temp;
}
struct hashTable *createTable(int size)
{
    struct hashTable *temp = (struct hashTable *)malloc(sizeof(struct hashTable));
    temp->size = size;
    temp->arr = (struct node **)malloc(size * sizeof(struct node *));
    for (int i = 0; i < size; i++)
    {
        temp->arr[i] = NULL;
    }
    return temp;
}
int hashFunction(struct hashTable *table, int key)
{
    if (!table || table->size <= 0)
        return 0;

    while(key>9){
        key/=10;
    }
    return key;
}
void insertItem(struct hashTable *table, struct Item value)
{
    int index = hashFunction(table, value.id);
    struct node *new = newnode(value.id, value);
    if (table->arr[index] == NULL)
    {
        table->arr[index] = new;
    }
    else
    {
        struct node *curr = table->arr[index];
        while (curr->next)
        {
            curr = curr->next;
        }
        curr->next = new;
    }
    
}
void updateItem(struct hashTable *table, struct Item value)
{
    int index = hashFunction(table, value.id);
    struct node *curr = table->arr[index];
    while (curr)
    {
        if (curr->key == value.id)
        {
            curr->value = value;
            return;
        }
        curr = curr->next;
    }
    printf("Item with key %d not found for update\n", value.id);
    
}
int removeItem(struct hashTable *table, int key)
{
    int index = hashFunction(table, key);
    struct node *curr = table->arr[index];
    if (!curr)
    {
        printf("Item not found\n");
        return 0;
    }
    if (curr->key == key)
    {
        table->arr[index] = curr->next;
        free(curr);
        return 1;
    }
    while (curr->next && curr->next->key != key)
    {
        curr = curr->next;
    }
    if (!curr->next)
    {
        printf("Item not found\n");
        return 0;
    }
    struct node *temp = curr->next;
    curr->next = curr->next->next;
    free(temp);
    printf("Item with key %d removed\n", key);
    return 1;
}

struct hashTable *loadTableData()
{
    struct hashTable *table = createTable(10);

    FILE *f1 = fopen("saves/inventory.bin", "rb");
    if (!f1)
    {
        // No existing data, return empty table
        return table;
    }

    int capacity = 100;
    struct Item *warehouseItems = (struct Item *)malloc(sizeof(struct Item) * capacity);
    if (!warehouseItems)
    {
        fclose(f1);
        return table;
    }

    int count = 0;
    while (1)
    {
        if (count >= capacity)
        {
            int newCap = capacity * 2;
            struct Item *tmp = (struct Item *)realloc(warehouseItems, sizeof(struct Item) * newCap);
            if (!tmp)
            {
                // realloc failed, stop reading further
                break;
            }
            warehouseItems = tmp;
            capacity = newCap;
        }

        size_t read = fread(&warehouseItems[count], sizeof(struct Item), 1, f1);
        if (read != 1)
            break;
        count++;
    }

    fclose(f1);

    for (int i = 0; i < count; i++)
    {
        insertItem(table, warehouseItems[i]);
    }

    free(warehouseItems);
    return table;
}

char* getInventoryJSON(struct hashTable* table){
    char* jsonStr=(char*)malloc(10000*sizeof(char));
    strcpy(jsonStr,"[");
    for(int i=0;i<table->size;i++){
        struct node* curr=table->arr[i];
        while(curr){
            char itemStr[200];
            sprintf(itemStr,"{\"id\":%d,\"sku\":\"%s\",\"name\":\"%s\",\"quantity\":%d,\"price\":%.2f,\"reorder\":%d},",curr->value.id,curr->value.sku,curr->value.name,curr->value.quantity,curr->value.price,curr->value.reorder);
            strcat(jsonStr,itemStr);
            curr=curr->next;
        }
    }
    if(strlen(jsonStr)>1){
        jsonStr[strlen(jsonStr)-1]='\0'; // Remove last comma
    }
    strcat(jsonStr,"]");
    return jsonStr;
}
void writeWarehouseItems(struct hashTable* table){
    FILE* f1=fopen("saves/inventory.bin","wb");
    for(int i=0;i<table->size;i++){
        struct node* curr=table->arr[i];
        while(curr){
            fwrite(&(curr->value),sizeof(struct Item),1,f1);
            curr=curr->next;
        }
    }
    fclose(f1);
}