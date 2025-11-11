#ifndef INVENTORY_H
#define INVENTORY_H

#include <stddef.h>

struct Item{
    int id;
    char sku[20];
    char name[50];
    int quantity;
    float price;
    int reorder;
};

struct node{
    int key;
    struct Item value;
    struct node *next;
};

struct hashTable{
    int size;
    struct node **arr;
};

struct node* newnode(int key,struct Item value);
struct hashTable* createTable(int size);
int hashFunction(struct hashTable* table,int key);
void insertItem(struct hashTable* table,struct Item value);
void updateItem(struct hashTable* table,struct Item value);
int removeItem(struct hashTable* table,int key);
struct hashTable* loadTableData();
void writeWarehouseItems(struct hashTable* table);
char* getInventoryJSON(struct hashTable* table);

#endif // INVENTORY_H
