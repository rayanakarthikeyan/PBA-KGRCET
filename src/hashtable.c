/* Merged implementation for chaining + open addressing in one file.
   This makes compilation/linking simpler (single implementation chosen
   at runtime by ht_strategy_t).
*/

#include "../include/hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* Forward declarations for internal helpers */
static size_t hash32(ht_key_t k, size_t m);
static size_t h1(ht_key_t k, size_t m);
static size_t h2(ht_key_t k, size_t m);

/* Node for chaining */
typedef struct node { ht_key_t key; struct node *next; } node_t;

/* Slot state for open addressing */
typedef enum { EMPTY=0, OCCUPIED=1, DELETED=2 } slot_state_t;

typedef struct slot { ht_key_t key; slot_state_t state; } slot_t;

struct hashtable {
    size_t m; /* capacity - number of buckets or slots */
    size_t n; /* number of elements stored */
    ht_strategy_t strat;
    /* polymorphic storage: either buckets (for chaining) or slots */
    node_t *buckets; / used when HT_CHAINING */
    slot_t slots;    / used when open addressing */
    ht_stats_t stats;
};

/* simple multiplicative hash (32-bit key) */
static size_t hash32(ht_key_t k, size_t m) {
    uint64_t z = (uint64_t)k * 11400714819323198485ULL;
    uint32_t x = (uint32_t)((z >> 32) ^ z);
    return (size_t)(x % (uint32_t)m);
}
static size_t h1(ht_key_t k, size_t m) { return hash32(k, m); }
static size_t h2(ht_key_t k, size_t m) {
    uint64_t z = (uint64_t)k * 14029467366897019727ULL + 1ULL;
    uint32_t x = (uint32_t)((z >> 32) ^ z);
    /* map into [1 .. m-1] */
    if (m <= 2) return 1;
    return (size_t)( (x % (m-1)) + 1 );
}

hashtable_t *ht_create(size_t table_size, ht_strategy_t strat) {
    hashtable_t *ht = malloc(sizeof(*ht));
    if (!ht) return NULL;
    ht->m = table_size;
    ht->n = 0;
    ht->strat = strat;
    ht->buckets = NULL;
    ht->slots = NULL;
    memset(&ht->stats, 0, sizeof(ht->stats));

    if (strat == HT_CHAINING) {
        ht->buckets = calloc(table_size, sizeof(node_t*));
        if (!ht->buckets) { free(ht); return NULL; }
    } else {
        ht->slots = calloc(table_size, sizeof(slot_t));
        if (!ht->slots) { free(ht); return NULL; }
        for (size_t i=0;i<table_size;i++) ht->slots[i].state = EMPTY;
    }
    return ht;
}

void ht_destroy(hashtable_t *ht) {
    if (!ht) return;
    if (ht->strat == HT_CHAINING) {
        for (size_t i=0;i<ht->m;i++) {
            node_t *cur = ht->buckets[i];
            while (cur) { node_t *nx = cur->next; free(cur); cur = nx; }
        }
        free(ht->buckets);
    } else {
        free(ht->slots);
    }
    free(ht);
}

/* --- Chaining operations --- */
static int chaining_insert(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    size_t probes = 0;
    size_t idx = hash32(key, ht->m);
    probes++;
    node_t *cur = ht->buckets[idx];
    while (cur) {
        probes++;
        if (cur->key == key) {
            if (probes_out) *probes_out = probes;
            ht->stats.inserts_attempted++;
            return 0; /* duplicate */
        }
        cur = cur->next;
    }
    node_t *n = malloc(sizeof(*n));
    if (!n) return -1;
    n->key = key; n->next = ht->buckets[idx]; ht->buckets[idx] = n;
    ht->n++;
    ht->stats.inserts_attempted++;
    ht->stats.inserts_successful++;
    ht->stats.total_insert_probes += probes;
    if (probes_out) *probes_out = probes;
    return 1;
}
static int chaining_search(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    size_t probes = 0; size_t idx = hash32(key, ht->m);
    node_t *cur = ht->buckets[idx];
    while (cur) {
        probes++;
        if (cur->key == key) { ht->stats.total_search_probes += probes; if (probes_out) *probes_out = probes; return 1; }
        cur = cur->next;
    }
    probes++; ht->stats.total_search_probes += probes; if (probes_out) *probes_out = probes; return 0;
}
static int chaining_delete(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    size_t probes = 0; size_t idx = hash32(key, ht->m);
    node_t *cur = ht->buckets[idx], *prev = NULL;
    while (cur) {
        probes++;
        if (cur->key == key) {
            if (prev) prev->next = cur->next; else ht->buckets[idx] = cur->next;
            free(cur); ht->n--; if (probes_out) *probes_out = probes; return 1;
        }
        prev = cur; cur = cur->next;
    }
    if (probes_out) *probes_out = probes + 1; return 0;
}

/* --- Open addressing helpers --- */
static ssize_t oa_probe_find(hashtable_t *ht, ht_key_t key, int for_insert, size_t *probes_out) {
    size_t m = ht->m;
    size_t probes = 0;
    size_t idx0 = h1(key, m);
    size_t idx = idx0;
    size_t step = 0;
    size_t dh_step = h2(key, m);
    ssize_t first_deleted = -1;

    for (size_t i=0;i<m;i++) {
        probes++;
        slot_t *s = &ht->slots[idx];
        if (s->state == EMPTY) {
            if (for_insert) {
                if (first_deleted != -1) { if (probes_out) *probes_out = probes; return first_deleted; }
                if (probes_out) *probes_out = probes; return (ssize_t)idx;
            } else {
                if (probes_out) *probes_out = probes; return -1;
            }
        } else if (s->state == OCCUPIED && s->key == key) {
            if (probes_out) *probes_out = probes; return (ssize_t)idx;
        } else if (s->state == DELETED) {
            if (first_deleted == -1) first_deleted = (ssize_t)idx;
        }
        /* advance */
        if (ht->strat == HT_LINEAR_PROBING) {
            idx = (idx0 + ++step) % m;
        } else if (ht->strat == HT_QUADRATIC_PROBING) {
            step++;
            idx = (idx0 + step * step) % m;
        } else if (ht->strat == HT_DOUBLE_HASHING) {
            idx = (idx + dh_step) % m;
        } else {
            idx = (idx + 1) % m;
        }
    }
    if (probes_out) *probes_out = probes;
    if (first_deleted != -1 && for_insert) return first_deleted;
    return -1;
}

static int oa_insert(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    if (ht->n >= ht->m) { if (probes_out) *probes_out = 0; ht->stats.inserts_attempted++; return 0; }
    size_t probes = 0;
    ssize_t idx = oa_probe_find(ht, key, 1, &probes);
    ht->stats.inserts_attempted++;
    if (idx < 0) { if (probes_out) *probes_out = probes; return 0; }
    if (ht->slots[idx].state == OCCUPIED && ht->slots[idx].key == key) { if (probes_out) *probes_out = probes; return 0; }
    ht->slots[idx].key = key; ht->slots[idx].state = OCCUPIED; ht->n++;
    ht->stats.inserts_successful++;
    ht->stats.total_insert_probes += probes;
    if (probes_out) *probes_out = probes;
    return 1;
}
static int oa_search(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    size_t probes = 0; ssize_t idx = oa_probe_find(ht, key, 0, &probes);
    ht->stats.total_search_probes += probes; if (probes_out) *probes_out = probes; return (idx >= 0) ? 1 : 0;
}
static int oa_delete(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    size_t probes = 0; ssize_t idx = oa_probe_find(ht, key, 0, &probes);
    if (idx < 0) { if (probes_out) *probes_out = probes; return 0; }
    ht->slots[idx].state = DELETED; ht->n--; if (probes_out) *probes_out = probes; return 1;
}

/* Public wrappers */
int ht_insert(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    if (!ht) return -1;
    if (ht->strat == HT_CHAINING) return chaining_insert(ht, key, probes_out);
    return oa_insert(ht, key, probes_out);
}
int ht_search(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    if (!ht) return -1;
    if (ht->strat == HT_CHAINING) return chaining_search(ht, key, probes_out);
    return oa_search(ht, key, probes_out);
}
int ht_delete(hashtable_t *ht, ht_key_t key, size_t *probes_out) {
    if (!ht) return -1;
    if (ht->strat == HT_CHAINING) return chaining_delete(ht, key, probes_out);
    return oa_delete(ht, key, probes_out);
}

size_t ht_capacity(hashtable_t *ht) { return ht ? ht->m : 0; }
double ht_load_factor(hashtable_t *ht) { return ht ? ((double)ht->n / (double)ht->m) : 0.0; }
void ht_get_stats(hashtable_t *ht, ht_stats_t *out) { if (ht && out) *out = ht->stats; }
