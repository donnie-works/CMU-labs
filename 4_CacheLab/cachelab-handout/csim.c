#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>


struct CacheNode {
    int valid;
    int tag;
    int timestamp;
};
typedef struct CacheNode CN;

int main(int argc, char** argv) {
    // parse command line args using getopt
    int opt;
    int s;      
    int E;      // lines
    int b;      // block offset bits
    char* t;
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                t = optarg;
                break;
        }
    }

    int S = (1<<s); // number of sets (2^s)
    CN* CacheLines = calloc((S*E), sizeof(CN));

    // open file for reading
    FILE* TraceFile = fopen(t, "r");
    if (!TraceFile){
        perror("Error opening trace file");
        return EXIT_FAILURE;
    }
    // initialize variables
    char opchar;
    long long opaddr;
    int opsize;
    int hit_found = 0;          // acts as a flag
    int empty_line_found = 0;   // acts as a flag
    int hits = 0;
    int misses = 0;
    int evicts = 0;
    int ts_counter = 0;         // lowest number = LRU 
    // parse contents
    while (fscanf(TraceFile, " %c %llx,%d", &opchar, &opaddr, &opsize) != EOF) {
        hit_found = 0;
        empty_line_found = 0;
        //process each line
        if (opchar == 'I')
            continue; 
        ts_counter++;
        int setIndex = (opaddr >> b) & ((1 << s) - 1);                         
        int tag = opaddr >> (b + s);
        // HIT case
        for (int i = 0; i < E; i++) {
            CN* Current = &CacheLines[setIndex * E + i];
            if (Current->valid == 1 && Current->tag == tag){
                hit_found = 1;
                Current->timestamp = ts_counter; 
                break;
            }
        }
        if (hit_found == 1)
            hits+=1;
        // MISS case with empty lines available
        else{
            misses+=1;
            for (int i = 0; i < E; i++) {
                CN* Current = &CacheLines[setIndex * E + i];
                if (Current->valid == 0) {
                    Current->valid = 1;
                    Current->tag = tag;
                    Current->timestamp = ts_counter;
                    empty_line_found = 1;
                    break;
                }
            }
            // MISS case and no empty lines (full) 
            if (empty_line_found == 0) {
                // evict LRU
                CN* lru_line = &CacheLines[setIndex * E + 0];
                int smallest = lru_line->timestamp;
                for (int i = 0; i < E; i++) {
                    CN* Current = &CacheLines[setIndex * E + i];
                    if (Current->timestamp < smallest){
                        smallest = Current->timestamp;
                        lru_line = Current;
                    }
                }
                lru_line->tag = tag;
                lru_line->timestamp = ts_counter;
                evicts += 1;
            }
        }
        // Load can be hit or miss, but store is always a hit
        if (opchar == 'M')
            hits += 1;
    }
    // close file
    fclose(TraceFile);

    printSummary(hits, misses, evicts);
    free(CacheLines);
    return 0;
}
