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

        size_t len = 0;

        for (size_t i = 0; s[i] != '\0'; i++) {
                len++;
        }

        return len;
}

char *my_strcpy(char *dst, const char *src)
{
        (void)src;

        //Using <= to get \0 at end of src
        for (size_t i = 0; i <= my_strlen(src); i++) {
                dst[i] = src[i];
        }

        return dst;
}

char *my_strncpy(char *dst, const char *src, size_t n)
{
        (void)src;
        (void)n;

        size_t src_len = my_strlen(src);
        for (size_t i = 0; i < n; i ++ ) {
                if (i >= src_len) {
                        dst[i] = '\0';
                } else {
                        dst[i] = src[i];
                }

        }


        return dst;
}

size_t safe_copy(char *dst, const char *src, size_t size)
{
        (void)dst;
        (void)src;
        (void)size;

        if (size) {
                for (size_t i = 0; i < size - 1; i++) {
                        if (i < my_strlen(src)) {
                                dst[i] = src[i];
                        }
                }

                size_t term_idx =  my_strlen(src) < size - 1 ? my_strlen(src) : size - 1;

                dst[term_idx] = '\0';
        }

        return my_strlen(src);
}

char *my_strdup(const char *s)
{
        (void)s;

        char *s_dup = malloc(my_strlen(s) + 1);
        
        for(size_t i = 0; i <= my_strlen(s); i ++) {
                s_dup[i] = s[i];
        }

        return s_dup;
}

int my_strcmp(const char *a, const char *b)
{
        (void)a;
        (void)b;

        for (size_t i = 0; i<my_strlen(a) && i<my_strlen(b); i++) {
                if ((unsigned char)a[i] < (unsigned char)b[i]) {
                        return -1;
                } else if ((unsigned char)a[i] > (unsigned char)b[i]) {
                        return 1;
                }
        }

        if (my_strlen(a) < my_strlen(b)) {
                return -1;
        } else if (my_strlen(a) > my_strlen(b)) {
                return 1;
        }

        return 0;
}
