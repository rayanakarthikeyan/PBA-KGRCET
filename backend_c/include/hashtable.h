#ifndef HASHTABLE_H
#define HASHTABLE_H

#define TABLE_SIZE 1009
#define MAX_KEY 10000

typedef struct Node {
    int key;
    struct Node* next;
} Node;

typedef struct {
    int* table;
    int count;
} HashTable;

typedef struct {
    Node** table;
    int count;
} ChainTable;

// Function Declarations
int hash_func(int key);
HashTable* create_table();
ChainTable* create_chain_table();

int insert_linear(HashTable* ht, int key);
int insert_quadratic(HashTable* ht, int key);
int insert_chaining(ChainTable* ht, int key);

void free_table(HashTable* ht);
void free_chain_table(ChainTable* ht);

#endif
