/* driver: runs experiments, uses hashtable API and visualizer for live demo */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "../include/hashtable.h"

/* forward declarations for visualizer */
void vis_init();
void vis_draw_header(const char *title);
void vis_draw_status(size_t inserted, size_t probes, double load);
void vis_draw_bars(size_t *buckets, size_t m);
void vis_refresh();
void vis_finish();

/* produce keys per distribution */
static void gen_uniform(uint32_t *out, size_t n, uint32_t range) {
    for (size_t i=0;i<n;i++) out[i] = (uint32_t)(rand() % range);
}
static void gen_sequential(uint32_t *out, size_t n, uint32_t start) {
    for (size_t i=0;i<n;i++) out[i] = start + (uint32_t)i;
}
static void gen_cluster(uint32_t *out, size_t n, uint32_t cluster_start, uint32_t cluster_size) {
    for (size_t i=0;i<n;i++) out[i] = cluster_start + (uint32_t)(rand() % cluster_size);
}

/* run_experiment: runs one strategy and writes a CSV summary */
static void run_experiment(ht_strategy_t strat, size_t m, uint32_t *keys, size_t key_count, const char *out_csv, int use_vis) {
    hashtable_t *ht = ht_create(m, strat);
    if (!ht) { fprintf(stderr,"alloc failed\n"); return; }

    size_t *buckets = calloc(m, sizeof(size_t));
    size_t total_probes = 0;
    clock_t t0 = clock();

    if (use_vis) vis_init();
    vis_draw_header("Dynamic Hash Table Analyzer - demo");
    size_t inserted = 0;
    for (size_t i=0;i<key_count;i++) {
        size_t probes = 0;
        ht_insert(ht, keys[i], &probes);
        total_probes += probes;
        inserted++;
        /* update bucket stats approximate: compute bucket index for visualization */
        size_t bucket_index = (size_t)(keys[i] % m);
        buckets[bucket_index]++;
        if (use_vis) {
            vis_draw_status(inserted, total_probes, ht_load_factor(ht));
            vis_draw_bars(buckets, (size_t)m);
            vis_refresh();
            usleep(40000);
            int ch = getch();
            if (ch == 'q') break;
        }
    }
    clock_t t1 = clock();
    double elapsed = (double)(t1 - t0) / CLOCKS_PER_SEC;
    ht_stats_t st; ht_get_stats(ht, &st); st.elapsed_seconds = elapsed;

    /* write CSV summary */
    FILE *f = fopen(out_csv, "w");
    if (f) {
        fprintf(f, "strategy,table_size,inserts,successful_inserts,total_insert_probes,avg_probes_per_insert,elapsed_seconds,load_factor\n");
        fprintf(f, "%d,%zu,%zu,%zu,%zu,%.4f,%.4f,%.4f\n",
            (int)strat, m, st.inserts_attempted, st.inserts_successful,
            st.total_insert_probes,
            (st.inserts_successful ? (double)st.total_insert_probes / st.inserts_successful : 0.0),
            st.elapsed_seconds, ht_load_factor(ht));
        fclose(f);
    } else { fprintf(stderr,"failed to open %s for writing\n", out_csv); }

    if (use_vis) vis_finish();
    ht_destroy(ht);
    free(buckets);
}

int main(int argc, char **argv) {
    srand((unsigned)time(NULL));
    size_t table_size = 200; /* default */
    size_t nkeys = 500;
    int use_vis = 1;
    int dist = 0; /* 0=uniform, 1=sequential, 2=cluster */

    for (int i=1;i<argc;i++){
        if (strcmp(argv[i], "--no-vis")==0) use_vis = 0;
        else if (strcmp(argv[i], "--size")==0 && i+1 < argc) table_size = (size_t)atoi(argv[++i]);
        else if (strcmp(argv[i], "--nkeys")==0 && i+1 < argc) nkeys = (size_t)atoi(argv[++i]);
        else if (strcmp(argv[i], "--dist")==0 && i+1 < argc) dist = atoi(argv[++i]);
    }

    uint32_t *keys = malloc(sizeof(uint32_t) * nkeys);
    if (!keys) { fprintf(stderr, "malloc failed\n"); return 1; }

    if (dist == 0) gen_uniform(keys, nkeys, (uint32_t)(table_size * 4));
    else if (dist == 1) gen_sequential(keys, nkeys, 1);
    else gen_cluster(keys, nkeys, 1, (uint32_t)(table_size/4 + 1));

    /* ensure results directory exists */
    system("mkdir -p results");

    run_experiment(HT_CHAINING, table_size, keys, nkeys, "results/chaining.csv", use_vis);
    run_experiment(HT_LINEAR_PROBING, table_size, keys, nkeys, "results/linear.csv", use_vis);
    run_experiment(HT_QUADRATIC_PROBING, table_size, keys, nkeys, "results/quadratic.csv", use_vis);
    run_experiment(HT_DOUBLE_HASHING, table_size, keys, nkeys, "results/doublehash.csv", use_vis);

    free(keys);
    printf("Done. CSV results in results/ directory.\n");
    return 0;
}

--- FILE: Makefile ---

CC = gcc
CFLAGS = -O2 -Wall -std=c11 -Iinclude
LDFLAGS = -lncurses
SRCDIR = src
BINDIR = bin

all: $(BINDIR)/dynamic-hashtable

$(BINDIR):
	mkdir -p $(BINDIR)
	mkdir -p results

$(BINDIR)/dynamic-hashtable: $(BINDIR) $(SRCDIR)/main.c $(SRCDIR)/hashtable.c $(SRCDIR)/visualizer.c include/hashtable.h
	$(CC) $(CFLAGS) -o $@ $(SRCDIR)/main.c $(SRCDIR)/hashtable.c $(SRCDIR)/visualizer.c $(LDFLAGS)

clean:
	rm -rf $(BINDIR) results/*.csv

.PHONY: all clean
