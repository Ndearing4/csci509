/*
 * bug1_uaf.c -- a task list that runs each task and frees it.
 * Nathan Dearing, 2026-08-21.
 *
 * Planted bug: one. It is a use-after-free.
 *
 * Builds clean under -Wall -Wextra -Werror, prints one line, and segfaults.
 *
 * The interesting question is not that it crashed -- it is *why* the second
 * iteration reads garbage. `free` does not wipe the chunk and hand it back to
 * the kernel. It hands it to the allocator's free list, and glibc stores that
 * list's own bookkeeping inside the chunk you just gave up: a forward pointer
 * at offset 0 and a guard value at offset 8. `next` in this struct lives at
 * offset 8. So the allocator overwrites it, and the next hop lands nowhere.
 *
 * Freed memory is not yours and is not preserved. On a different allocator,
 * or with `next` at a different offset, this would silently "work" for years.
 * Do not read this crash as C protecting you -- it is a coincidence of layout.
 *
 * Find it in gdb first. Suggested route:
 *     break run_all
 *     next   (repeatedly, watching t)
 *     print t          -- before the free and after it
 *     print *t         -- and watch `next` change without you touching it
 *     x/4xg t          -- the raw chunk, with glibc's bookkeeping in it
 * Then confirm with valgrind and see which line number it names.
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct Task {
        int id;                 /* what the task would do, if it did anything */
        struct Task *next;      /* next task in the queue */
} Task;

/* Push a task on the front and return the new head. */
static Task *push(Task *head, int id)
{
        Task *t = malloc(sizeof *t);

        if (t == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
        }
        t->id = id;
        t->next = head;
        return t;
}

/* Run every task, freeing each one as it finishes. Returns how many ran. */
static int run_all(Task *head)
{
        Task *t = head;
        int ran = 0;

        while (t != NULL) {
                printf("  running task %d\n", t->id);
                ran++;
                free(t);
                t = t->next;
        }
        return ran;
}

int main(void)
{
        Task *queue = NULL;
        int i;

        for (i = 1; i <= 5; i++)
                queue = push(queue, i);

        printf("bug1: running the queue\n");
        printf("bug1: %d tasks ran\n", run_all(queue));
        return EXIT_SUCCESS;
}
