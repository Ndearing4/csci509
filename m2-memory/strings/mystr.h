/*
 * mystr.h -- M2 string exercises: reimplement six of <string.h>.
 * Nathan Dearing, 2026-08-21.
 *
 * A C string is not a type. It is a convention: a `char *` pointing at a run
 * of bytes that ends with a '\0'. Nothing carries the length, nothing carries
 * the capacity, and nothing checks either one. Every bug in this file's
 * subject matter comes from that.
 *
 * The rule that follows from it: a buffer of N bytes holds N-1 characters.
 * The off-by-one is not an edge case here, it is the normal case.
 */

#ifndef MYSTR_H
#define MYSTR_H

#include <stddef.h>

/* 1. Length of `s`, not counting the terminator. Same as strlen.
 *    strlen("abc") is 3, and "abc" occupies 4 bytes. */
size_t my_strlen(const char *s);

/* 2. Copy `src`, terminator included, into `dst`. Returns `dst`. Same as
 *    strcpy: it has no idea how big `dst` is and cannot fail. The caller
 *    guarantees room for my_strlen(src) + 1 bytes, and when the caller is
 *    wrong this writes past the end and nothing says so.
 *
 *    Write it, then look at the signature and say out loud what information
 *    it is missing. That absence is the reason for every other function here. */
char *my_strcpy(char *dst, const char *src);

/* 3. Copy at most `n` bytes of `src` into `dst`. Returns `dst`.
 *
 *    Reproduce strncpy EXACTLY, including both halves people get wrong:
 *
 *      - If my_strlen(src) >= n, exactly n bytes are copied and **no
 *        terminator is written**. dst is not a string afterwards.
 *      - If my_strlen(src) < n, the remainder of the n bytes is padded with
 *        '\0' -- all of it, not just one. Copying a 3-byte string with
 *        n = 4096 writes 4096 bytes.
 *
 *    Neither behavior is a mistake in the standard; strncpy was written for
 *    fixed-width records in the 1970s, not for safety. It is in this exercise
 *    because it is the single most misused function in C, and the misuse is
 *    always "I used strncpy so it must be safe."
 *
 *    The harness plants a canary byte after dst and prints what survives. */
char *my_strncpy(char *dst, const char *src, size_t n);

/* 4. Copy `src` into a buffer of `size` bytes, ALWAYS leaving a terminated
 *    string (unless size is 0, when it writes nothing). Returns
 *    my_strlen(src) -- the length it *wanted* to write.
 *
 *    So the caller detects truncation with:
 *        if (safe_copy(buf, src, sizeof buf) >= sizeof buf) { it was cut }
 *
 *    This is BSD's strlcpy. It is what people think strncpy does. Note what
 *    makes it work: returning the source length rather than the destination,
 *    so a truncation is visible to the caller instead of silent. */
size_t safe_copy(char *dst, const char *src, size_t size);

/* 5. Return a malloc'd copy of `s` that the caller must free, or NULL if the
 *    allocation failed. Same as strdup.
 *
 *    How many bytes do you ask malloc for? Getting that wrong by one is the
 *    canonical heap overflow, and the harness checks for it with valgrind's
 *    help rather than its own -- run this one under valgrind. */
char *my_strdup(const char *s);

/* 6. Compare `a` and `b`. Negative, zero, or positive when a sorts before,
 *    equal to, or after b. Same as strcmp.
 *
 *    Compare as `unsigned char`, not plain `char`. This machine's char is
 *    signed, so a byte above 0x7F compares as negative and "z" would sort
 *    after a UTF-8 continuation byte. The exact magnitude is unspecified --
 *    only the sign is defined -- so the harness only checks the sign. */
int my_strcmp(const char *a, const char *b);

#endif /* MYSTR_H */
