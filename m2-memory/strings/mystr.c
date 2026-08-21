/*
 * mystr.c -- M2 string exercises: reimplement six of <string.h>.
 * Nathan Dearing, 2026-08-21.
 *
 * Contracts are in mystr.h. Six stubs.
 *
 * Do NOT include <string.h> here and do not call the real ones. The whole
 * exercise is writing the loop that walks to the '\0' yourself, enough times
 * that the shape of it stops being something you look up.
 *
 * Order: strlen, strcpy, strcmp, then strncpy, then safe_copy, then strdup.
 * strncpy after strcpy so the difference is fresh; strdup last because it is
 * the only one that touches the heap.
 *
 * The stubs ignore their arguments via (void) casts so the file builds clean
 * under -Wall -Wextra -Werror. Delete each cast as you go.
 */

#include <stdlib.h>
#include "mystr.h"

size_t my_strlen(const char *s)
{
        (void)s;
        return 0;
}

char *my_strcpy(char *dst, const char *src)
{
        (void)src;
        return dst;
}

char *my_strncpy(char *dst, const char *src, size_t n)
{
        (void)src;
        (void)n;
        return dst;
}

size_t safe_copy(char *dst, const char *src, size_t size)
{
        (void)dst;
        (void)src;
        (void)size;
        return 0;
}

char *my_strdup(const char *s)
{
        (void)s;
        return NULL;
}

int my_strcmp(const char *a, const char *b)
{
        (void)a;
        (void)b;
        return 0;
}
