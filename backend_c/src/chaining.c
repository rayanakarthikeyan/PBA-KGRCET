#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"

ChainTable* create_chain_table() {
    ChainTable* ht = malloc(sizeof(ChainTable));
    ht->table = calloc(TABLE_SIZE, sizeof(Node*));
    ht->count = 0;
    return ht;
}

int insert_chaining(ChainTable* ht, int key) {
    int idx = hash_func(key);
    Node* new_node = malloc(sizeof(Node));
    new_node->key = key;
    new_node->next = ht->table[idx];
    ht->table[idx] = new_node;
    ht->count++;
    return 1; // single operation per insert
}

void free_chain_table(ChainTable* ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node* cur = ht->table[i];
        while (cur) {
            Node* temp = cur;
            cur = cur->next;
            free(temp);
        }
    }
    free(ht->table);
    free(ht);
}
