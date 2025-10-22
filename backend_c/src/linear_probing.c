#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"

int hash_func(int key) {
    return key % TABLE_SIZE;
}

HashTable* create_table() {
    HashTable* ht = malloc(sizeof(HashTable));
    ht->table = calloc(TABLE_SIZE, sizeof(int));
    ht->count = 0;
    return ht;
}

int insert_linear(HashTable* ht, int key) {
    int idx = hash_func(key);
    int probes = 0;

    while (ht->table[idx] != 0) {
        idx = (idx + 1) % TABLE_SIZE;
        probes++;
    }

    ht->table[idx] = key;
    ht->count++;
    return probes;
}

void free_table(HashTable* ht) {
    free(ht->table);
    free(ht);
}
