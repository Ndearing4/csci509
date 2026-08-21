/*
 * bug2_double_free.c -- a message and a one-slot cache that both own it.
 * Nathan Dearing, 2026-08-21.
 *
 * Planted bug: one. It is a double free.
 *
 * The interesting part is where it crashes. glibc detects the second free by
 * noticing its own bookkeeping is inconsistent, and reports it from inside
 * free() -- so the top of the backtrace is libc, not your code. The bug is
 * two frames down, and it is not on the line that crashed.
 *
 * In gdb:
 *     run
 *     backtrace        -- ignore the libc frames, find yours
 *     frame N          -- the first frame in this file
 *     print msg
 * The question to answer is not "what crashed" but "who else thought they
 * owned this pointer".
 *
 * The fix is not "remove one of the frees". The fix is deciding, in a comment
 * at the top of cache_put, who owns the buffer. Write that comment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *cached;            /* the one-slot cache */

/* Remember `msg` so a later lookup can return it without rebuilding it. */
static void cache_put(char *msg)
{
        cached = msg;
}

/* Drop whatever the cache is holding. */
static void cache_clear(void)
{
        free(cached);
        cached = NULL;
}

/* Build the message for a given id. Caller frees. */
static char *build_message(int id)
{
        char *msg = malloc(32);

        if (msg == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
        }
        snprintf(msg, 32, "message for id %d", id);
        return msg;
}

int main(void)
{
        char *msg = build_message(7);

        printf("bug2: %s\n", msg);
        cache_put(msg);

        /* done with our copy */
        free(msg);

        printf("bug2: clearing the cache\n");
        cache_clear();

        printf("bug2: done\n");
        return EXIT_SUCCESS;
}
