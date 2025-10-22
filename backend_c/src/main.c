#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"

void analyze(int nkeys, const char* outfile);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <nkeys> <outfile>\n", argv[0]);
        return 1;
    }

    int nkeys = atoi(argv[1]);
    const char* outfile = argv[2];

    analyze(nkeys, outfile);
    printf("Results saved to %s\n", outfile);
    return 0;
}
