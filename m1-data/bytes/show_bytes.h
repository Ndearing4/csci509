/*
 * show_bytes.h -- print any object's representation in memory order.
 * Nathan Dearing, 2026-08-15.
 */

#ifndef SHOW_BYTES_H
#define SHOW_BYTES_H

#include <stddef.h>

/* Print the `size` bytes starting at `object` in address order, lowest
 * address first, as two-digit hex separated by spaces, prefixed by `label`
 * and followed by a newline.
 *
 * Example, for an int holding 0x12345678 on this machine:
 *     show_bytes("x", &x, sizeof x)  ->  x: 78 56 34 12
 *
 * Works on any object of any type -- that is the whole point. The parameter
 * is a void pointer because the function has no business knowing what it is
 * looking at; it reports the bytes and lets you interpret them. */
void show_bytes(const char *label, const void *object, size_t size);

#endif /* SHOW_BYTES_H */
