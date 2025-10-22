#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>
#include <stddef.h>

typedef uint32_t ht_key_t;

/* stats collected per run */
typedef struct {
    size_t inserts_attempted;
    size_t inserts_successful;
    size_t total_insert_probes; /* sum of probes during all inserts */
    size_t total_search_probes; /* sum of probes during search tests */
    double elapsed_seconds;
} ht_stats_t;

/* collision strategies enum */
typedef enum {
    HT_CHAINING = 0,
    HT_LINEAR_PROBING,
    HT_QUADRATIC_PROBING,
    HT_DOUBLE_HASHING
} ht_strategy_t;

/* opaque table type */
typedef struct hashtable hashtable_t;

/* create/destroy */
hashtable_t *ht_create(size_t table_size, ht_strategy_t strat);
void ht_destroy(hashtable_t *ht);

/* operations */
int ht_insert(hashtable_t *ht, ht_key_t key, size_t *probes_out);
int ht_search(hashtable_t *ht, ht_key_t key, size_t *probes_out);
int ht_delete(hashtable_t *ht, ht_key_t key, size_t *probes_out);

/* utilities */
size_t ht_capacity(hashtable_t *ht);
double ht_load_factor(hashtable_t *ht);
void ht_get_stats(hashtable_t *ht, ht_stats_t *out);

#endif
