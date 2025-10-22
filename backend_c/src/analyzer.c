#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hashtable.h"

void analyze(int nkeys, const char* outfile) {
    FILE* f = fopen(outfile, "w");
    fprintf(f, "LoadFactor,LinearProbing,QuadraticProbing,Chaining\n");

    srand(time(NULL));
    for (int load = 100; load <= nkeys; load += 100) {
        HashTable* linear = create_table();
        HashTable* quadratic = create_table();
        ChainTable* chain = create_chain_table();

        int totalLinear = 0, totalQuad = 0, totalChain = 0;

        for (int i = 0; i < load; i++) {
            int key = rand() % MAX_KEY;
            totalLinear += insert_linear(linear, key);
            totalQuad += insert_quadratic(quadratic, key);
            totalChain += insert_chaining(chain, key);
        }

        fprintf(f, "%d,%.2f,%.2f,%.2f\n",
                load,
                (double)totalLinear / load,
                (double)totalQuad / load,
                (double)totalChain / load);

        free_table(linear);
        free_table(quadratic);
        free_chain_table(chain);
    }

    fclose(f);
}
