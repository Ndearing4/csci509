/*
 * bug3_leak.c -- normalise a list of names, skipping the ones that are junk.
 * Nathan Dearing, 2026-08-21.
 *
 * Planted bug: one. It is a leak.
 *
 * This one never crashes, never prints anything wrong, and would run in
 * production for years. gdb will not find it for you -- there is no moment
 * where something goes visibly wrong. That is what valgrind is for, and it is
 * the reason valgrind exists as a separate tool rather than a gdb command.
 *
 * So invert the usual order here: run
 *     valgrind --leak-check=full ./bug3_leak
 * read the allocation stack it prints, and only then go find the path in the
 * source that reaches the allocation but not the free.
 *
 * "definitely lost" means nothing points at it any more. "still reachable"
 * means something does -- a different problem, and not this one.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Return a malloc'd upper-cased copy of `s`. Caller frees. */
static char *upcased(const char *s)
{
        size_t n = strlen(s);
        char *out = malloc(n + 1);
        size_t i;

        if (out == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
        }
        for (i = 0; i < n; i++)
                out[i] = (char)toupper((unsigned char)s[i]);
        out[n] = '\0';
        return out;
}

/* Print `name` normalised, unless it is empty or starts with a digit, in
 * which case it is skipped. Returns 1 if it printed. */
static int emit(const char *name)
{
        char *up = upcased(name);

        if (up[0] == '\0')
                return 0;
        if (isdigit((unsigned char)up[0]))
                return 0;

        printf("  %s\n", up);
        free(up);
        return 1;
}

int main(void)
{
        static const char *names[] = {
                "ada", "", "9lives", "grace", "linus", "42", "ken"
        };
        size_t i;
        int printed = 0;

        printf("bug3: normalising %zu names\n", sizeof names / sizeof names[0]);
        for (i = 0; i < sizeof names / sizeof names[0]; i++)
                printed += emit(names[i]);

        printf("bug3: printed %d of %zu\n", printed, sizeof names / sizeof names[0]);
        return EXIT_SUCCESS;
}
