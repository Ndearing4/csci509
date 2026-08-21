/*
 * bug4_overflow.c -- count how many values fall in each of n buckets.
 * Nathan Dearing, 2026-08-21.
 *
 * Planted bug: one. It writes past the end of a heap allocation.
 *
 * By exactly one element, which is the only interesting amount. Small
 * overruns land in the allocator's own metadata or in padding, so the program
 * usually survives and the damage shows up later somewhere unrelated.
 *
 * The bucket count comes from argv so the compiler cannot see the size at
 * compile time -- which is realistic, and is why -Wall says nothing.
 *
 * In gdb, the useful move is not stepping. It is:
 *     break tally
 *     print buckets
 *     watch buckets[nbuckets]      -- a watchpoint on the element past the end
 *     continue
 * gdb stops on the write that should never happen and shows you the frame.
 *
 * Then valgrind, which names it "Invalid write of size 4" and, helpfully,
 * says how far past the block it landed.
 */

#include <stdio.h>
#include <stdlib.h>

/* Bucket the values in `vals` into `nbuckets` counters. Caller frees. */
static int *tally(const int *vals, size_t nvals, size_t nbuckets)
{
        int *buckets = calloc(nbuckets, sizeof *buckets);
        size_t i;

        if (buckets == NULL) {
                perror("calloc");
                exit(EXIT_FAILURE);
        }

        /* clear the counters before use */
        for (i = 0; i <= nbuckets; i++)
                buckets[i] = 0;

        for (i = 0; i < nvals; i++)
                buckets[(size_t)vals[i] % nbuckets]++;

        return buckets;
}

int main(int argc, char **argv)
{
        static const int vals[] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9 };
        size_t nbuckets = 4;
        int *buckets;
        size_t i;

        if (argc > 1) {
                long n = strtol(argv[1], NULL, 10);

                if (n > 0 && n < 1000)
                        nbuckets = (size_t)n;
        }

        printf("bug4: %zu values into %zu buckets\n",
               sizeof vals / sizeof vals[0], nbuckets);

        buckets = tally(vals, sizeof vals / sizeof vals[0], nbuckets);
        for (i = 0; i < nbuckets; i++)
                printf("  bucket %zu: %d\n", i, buckets[i]);

        free(buckets);
        return EXIT_SUCCESS;
}
