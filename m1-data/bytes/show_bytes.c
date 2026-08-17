/*
 * show_bytes.c -- print any object's representation in memory order.
 * Nathan Dearing, 2026-08-15.
 *
 * The second half of M1's exit test. Unlike bits.c there are no operator
 * restrictions here -- write ordinary C.
 *
 * What you need to work out:
 *
 *   - Why the parameter is `const void *` and not `const char *`. What would
 *     go wrong if it were `int *`?

        Int would restrict it to 32 bit

 *   - You cannot dereference a void pointer. What do you have to convert it
 *     to first, and why is `unsigned char` the right choice rather than
 *     plain `char`? (Hint: this machine's plain char is signed, and you are
 *     about to print byte values above 0x7F.)
 *   - Why `%02X` and not `%X` when printing each byte.
 *   - Why passing `sizeof x` matters and the function cannot just figure the
 *     size out for itself.
 */

#include <stdio.h>
#include "show_bytes.h"

void show_bytes(const char *label, const void *object, size_t size)
{
        

        // unsigned char *p = &object;

        printf("%s ", label);
        
        unsigned char *new_object = (unsigned char *)object;

        size_t i;
        for (i = 0; i<size; i++) {
                printf("%02X ", new_object[i]);
        }

        

        printf("\n");


}


