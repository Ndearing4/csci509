/*
 * bug5_uninit.c -- average the readings from a sensor, dropping bad samples.
 * Nathan Dearing, 2026-08-21.
 *
 * Planted bug: one. It reads memory that was never written.
 *
 * malloc does not zero. calloc does. That is the whole bug, and the reason it
 * is worth a program of its own is that the wrong answer it produces is
 * *plausible* -- usually the freshly-mapped pages really are zero, so this
 * prints something sensible until the heap has been used a bit.
 *
 * Run it a few times. Then run it after bug4, in the same shell. Then run it
 * under valgrind:
 *     valgrind --track-origins=yes ./bug5_uninit
 * Without --track-origins valgrind tells you where the value was *used*.
 * With it, valgrind tells you where the memory came from, which is the line
 * you actually need. Read both, once each, so you know why the flag exists.
 *
 * In gdb the tell is: print the array right after the malloc and look at what
 * is in it. Nothing about that memory is guaranteed.
 */

#include <stdio.h>
#include <stdlib.h>

#define BAD_SAMPLE  (-1000)     /* the sentinel the sensor sends on a misread */

/* Copy the good samples out of `raw` into a fresh array of `n` slots.
 * Returns the array; *ngood gets how many were good. Caller frees. */
static int *filter(const int *raw, size_t n, size_t *ngood)
{
        int *clean = malloc(n * sizeof *clean);
        size_t i;
        size_t k = 0;

        if (clean == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
        }

        for (i = 0; i < n; i++) {
                if (raw[i] == BAD_SAMPLE)
                        continue;
                clean[k] = raw[i];
                k++;
        }

        *ngood = k;
        return clean;
}

int main(void)
{
        static const int raw[] = {
                21, 22, BAD_SAMPLE, 23, 24, BAD_SAMPLE, 22, 21
        };
        size_t n = sizeof raw / sizeof raw[0];
        size_t ngood = 0;
        int *clean = filter(raw, n, &ngood);
        long sum = 0;
        size_t i;

        /* average every slot in the array */
        for (i = 0; i < n; i++)
                sum += clean[i];

        printf("bug5: %zu raw samples, %zu good\n", n, ngood);
        printf("bug5: average = %ld\n", sum / (long)n);

        if (sum > 1000)
                printf("bug5: that reading looks too high\n");

        free(clean);
        return EXIT_SUCCESS;
}
