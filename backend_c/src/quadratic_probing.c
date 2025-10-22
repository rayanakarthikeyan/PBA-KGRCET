#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"

int insert_quadratic(HashTable* ht, int key) {
    int idx = hash_func(key);
    int probes = 0;
    int i = 1;

    while (ht->table[idx] != 0) {
        idx = (idx + i * i) % TABLE_SIZE;
        probes++;
        i++;
    }

    ht->table[idx] = key;
    ht->count++;
    return probes;
}
