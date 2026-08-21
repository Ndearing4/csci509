/*
 * test_mystr.c -- harness for the M2 string exercises.
 * Nathan Dearing, 2026-08-21.
 *
 * Six groups. Where the semantics are supposed to match libc exactly, the
 * harness calls the real function and compares -- that is the definition of
 * "same as strlen". Where the interesting behavior is what happens to the
 * bytes *around* the destination, it uses a canary and shows you the buffer.
 *
 * The canary: every destination is the first 8 bytes of a 24-byte buffer
 * pre-filled with 0xAA. Anything that is still 0xAA afterwards was not
 * written; anything past byte 8 that is no longer 0xAA is an overrun.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mystr.h"

#define DST_SIZE  8             /* the pretend buffer the caller declared */
#define PAD_SIZE  24            /* the real allocation, so overruns are visible */
#define CANARY    0xAA

static int checks_run;
static int checks_failed;
static int group_failed;

static void group(const char *name)
{
        if (checks_run > 0)
                printf("%s\n", group_failed ? "  ^ group FAILED" : "  ok");
        group_failed = 0;
        printf("--- %s\n", name);
}

static void check(int ok, const char *what, const char *detail)
{
        checks_run++;
        if (!ok) {
                checks_failed++;
                group_failed++;
                printf("  FAIL  %s\n", what);
                if (detail != NULL)
                        printf("        %s\n", detail);
        }
}

/* Fill `buf` with the canary byte. */
static void arm(unsigned char *buf)
{
        memset(buf, CANARY, PAD_SIZE);
}

/* Print `n` bytes as hex, with . for canary and letters shown as themselves,
 * so a padded/truncated buffer is readable at a glance. */
static void dump(const char *label, const unsigned char *buf, size_t n)
{
        size_t i;

        printf("        %-14s", label);
        for (i = 0; i < n; i++) {
                if (buf[i] == CANARY)
                        printf(" --");
                else if (buf[i] == 0)
                        printf(" 00");
                else if (buf[i] >= 0x20 && buf[i] < 0x7F)
                        printf("  %c", buf[i]);
                else
                        printf(" %02X", buf[i]);
        }
        printf("\n");
}

/* Every byte from `from` to PAD_SIZE still canary? */
static int untouched_from(const unsigned char *buf, size_t from)
{
        size_t i;

        for (i = from; i < PAD_SIZE; i++)
                if (buf[i] != CANARY)
                        return 0;
        return 1;
}

/* strcmp only promises a sign, so compare signs, not values. */
static int same_sign(int a, int b)
{
        if (a < 0)
                return b < 0;
        if (a > 0)
                return b > 0;
        return b == 0;
}

int main(void)
{
        static const char *samples[] = {
                "", "a", "abc", "hello, world", "1234567", "12345678", "\t\n "
        };
        size_t s;
        unsigned char buf[PAD_SIZE];
        char detail[128];

        printf("=== M2 strings ===\n\n");

        group("1. my_strlen");
        for (s = 0; s < sizeof samples / sizeof samples[0]; s++) {
                size_t got = my_strlen(samples[s]);
                size_t want = strlen(samples[s]);

                snprintf(detail, sizeof detail,
                         "my_strlen(\"%s\") = %zu, strlen = %zu",
                         samples[s], got, want);
                check(got == want, "matches strlen", detail);
        }

        group("2. my_strcpy");
        {
                arm(buf);
                check(my_strcpy((char *)buf, "abc") == (char *)buf,
                      "returns dst", NULL);
                check(buf[0] == 'a' && buf[1] == 'b' && buf[2] == 'c',
                      "copied the characters", NULL);
                check(buf[3] == 0, "wrote the terminator", NULL);
                check(untouched_from(buf, 4),
                      "wrote exactly strlen(src)+1 bytes and no more", NULL);
                dump("after \"abc\":", buf, 12);

                arm(buf);
                my_strcpy((char *)buf, "");
                check(buf[0] == 0 && untouched_from(buf, 1),
                      "copying \"\" writes exactly one byte", NULL);
        }

        group("3. my_strcmp");
        {
                struct { const char *a, *b; } pairs[] = {
                        { "", "" }, { "a", "a" }, { "a", "b" }, { "b", "a" },
                        { "abc", "abd" }, { "abc", "ab" }, { "ab", "abc" },
                        { "", "a" }, { "a", "" }, { "Z", "a" }
                };
                size_t i;

                for (i = 0; i < sizeof pairs / sizeof pairs[0]; i++) {
                        int got = my_strcmp(pairs[i].a, pairs[i].b);
                        int want = strcmp(pairs[i].a, pairs[i].b);

                        snprintf(detail, sizeof detail,
                                 "(\"%s\", \"%s\"): got %d, strcmp gives %d",
                                 pairs[i].a, pairs[i].b, got, want);
                        check(same_sign(got, want), "sign matches strcmp", detail);
                }

                /* The signedness trap. 0x80 is above 0x7F as an unsigned
                 * char and below it as a signed one, and this machine's
                 * plain char is signed. */
                {
                        const char high[] = { (char)0x80, 0 };
                        const char low[]  = { (char)0x7F, 0 };
                        int got = my_strcmp(high, low);

                        snprintf(detail, sizeof detail,
                                 "got %d; negative means you compared as signed char", got);
                        check(got > 0, "0x80 sorts after 0x7F", detail);
                }
        }

        group("4. my_strncpy -- the two surprises");
        {
                /* (a) source longer than n: truncated, and NOT terminated. */
                arm(buf);
                my_strncpy((char *)buf, "abcdefghij", DST_SIZE);
                check(memcmp(buf, "abcdefgh", DST_SIZE) == 0,
                      "copies exactly n bytes when src is longer", NULL);
                check(buf[DST_SIZE - 1] != 0,
                      "does NOT terminate when src does not fit",
                      "if you wrote a '\0' here you wrote strlcpy, not strncpy");
                check(untouched_from(buf, DST_SIZE),
                      "writes nothing past n bytes", NULL);
                dump("src too long:", buf, 12);
                printf("        ^ no 00 anywhere. buf is not a string. printf(\"%%s\") on it\n");
                printf("          runs off the end -- that is the bug this teaches.\n");

                /* (b) source exactly n: still no terminator. */
                arm(buf);
                my_strncpy((char *)buf, "abcdefgh", DST_SIZE);
                check(buf[DST_SIZE - 1] == 'h' && untouched_from(buf, DST_SIZE),
                      "an exactly-n-length source is also left unterminated", NULL);

                /* (c) source shorter than n: the ENTIRE remainder is zeroed. */
                arm(buf);
                my_strncpy((char *)buf, "ab", DST_SIZE);
                check(buf[0] == 'a' && buf[1] == 'b', "copied the short source", NULL);
                check(buf[2] == 0 && buf[3] == 0 && buf[4] == 0 &&
                      buf[5] == 0 && buf[6] == 0 && buf[7] == 0,
                      "zero-fills all n bytes, not just one terminator",
                      "strncpy(dst, \"ab\", 4096) writes 4096 bytes");
                check(untouched_from(buf, DST_SIZE), "and stops at n", NULL);
                dump("src short:", buf, 12);

                /* (d) n of 0 writes nothing at all. */
                arm(buf);
                my_strncpy((char *)buf, "abc", 0);
                check(untouched_from(buf, 0), "n of 0 writes nothing", NULL);

                check(my_strncpy((char *)buf, "x", 4) == (char *)buf,
                      "returns dst", NULL);
        }

        group("5. safe_copy -- what people think strncpy does");
        {
                size_t want;

                arm(buf);
                want = safe_copy((char *)buf, "abcdefghij", DST_SIZE);
                check(want == 10, "returns the length of src, not what it copied",
                      "returning the source length is what makes truncation detectable");
                check(want >= DST_SIZE, "so want >= size is the truncation test", NULL);
                check(buf[DST_SIZE - 1] == 0, "ALWAYS terminates within size", NULL);
                check(memcmp(buf, "abcdefg", 7) == 0, "kept the first size-1 chars", NULL);
                check(untouched_from(buf, DST_SIZE), "never writes past size", NULL);
                dump("truncated:", buf, 12);

                arm(buf);
                want = safe_copy((char *)buf, "abc", DST_SIZE);
                check(want == 3, "returns 3 for a source that fits", NULL);
                check(strcmp((char *)buf, "abc") == 0, "copied it whole", NULL);
                check(buf[4] == CANARY,
                      "does NOT zero-fill the rest -- unlike strncpy", NULL);
                dump("fits:", buf, 12);

                arm(buf);
                want = safe_copy((char *)buf, "abc", 0);
                check(want == 3, "still reports the source length when size is 0", NULL);
                check(untouched_from(buf, 0),
                      "a size of 0 writes nothing at all -- not even a terminator",
                      "there is no byte to put it in");
        }

        group("6. my_strdup");
        {
                const char *src = "hello, world";
                char *copy = my_strdup(src);
                char *again = my_strdup(src);

                check(copy != NULL && again != NULL, "returned a pointer", NULL);
                if (copy != NULL && again != NULL) {
                        check(strcmp(copy, src) == 0,
                              "the copy holds the same string", NULL);
                        check(copy != src,
                              "it is a copy, not the original pointer", NULL);
                        check(copy != again,
                              "each call is a separate allocation the caller frees", NULL);
                }
                free(copy);
                free(again);

                copy = my_strdup("");
                check(copy != NULL && copy[0] == 0,
                      "duplicating \"\" gives a one-byte terminated string",
                      "one byte, not zero -- asking malloc for strlen(s) is the classic off-by-one");
                free(copy);
        }

        printf("%s\n", group_failed ? "  ^ group FAILED" : "  ok");

        printf("\n=== %d checks, %d failed ===\n", checks_run, checks_failed);
        if (checks_failed == 0) {
                printf("All groups pass. my_strdup touches the heap, so:\n");
                printf("    valgrind --leak-check=full ./test_mystr\n");
                printf("An off-by-one in the malloc size passes every check above.\n");
                return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
}
