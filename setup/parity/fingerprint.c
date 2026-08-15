/*
 * fingerprint.c -- implementation of the parity checks.
 *
 * Nothing here is clever. The point is that every line prints something the
 * C standard deliberately leaves up to the platform, so two machines that
 * disagree will say so out loud instead of silently miscompiling later.
 *
 * CSCI 509 prep, module M0. Nathan Dearing, 2026-08-11.
 */

#include <limits.h>   /* CHAR_BIT, CHAR_MIN */
#include <stddef.h>   /* size_t, ptrdiff_t */
#include <stdint.h>   /* uint32_t */
#include <stdio.h>
#include <stdlib.h>   /* malloc, free, exit */

#include "fingerprint.h"

/* Static storage. Lives for the whole run in the data segment rather than in
   any stack frame -- used as the "static" sample address below. */
static int static_sample = 1;

void print_toolchain(void)
{
    puts("toolchain");

#ifdef __GNUC__
    printf("  compiler          gcc-compatible, __GNUC__ = %d\n", __GNUC__);
#else
    puts("  compiler          not gcc-compatible");
#endif

#ifdef __VERSION__
    printf("  version           %s\n", __VERSION__);
#endif

#ifdef __STDC_VERSION__
    /* 201112L is C11, 201710L is C17. The value is a long, hence %ld. */
    printf("  C standard        __STDC_VERSION__ = %ldL\n", __STDC_VERSION__);
#else
    puts("  C standard        pre-C95 (__STDC_VERSION__ undefined)");
#endif

    putchar('\n');
}

void print_type_sizes(void)
{
    puts("type sizes in bytes -- C fixes almost none of these, the platform does");
    printf("  char              %zu   (CHAR_BIT = %d bits)\n",
           sizeof(char), CHAR_BIT);
    printf("  short             %zu\n", sizeof(short));
    printf("  int               %zu\n", sizeof(int));
    printf("  long              %zu\n", sizeof(long));
    printf("  long long         %zu\n", sizeof(long long));
    printf("  float             %zu\n", sizeof(float));
    printf("  double            %zu\n", sizeof(double));
    printf("  long double       %zu\n", sizeof(long double));
    printf("  void *            %zu\n", sizeof(void *));
    printf("  size_t            %zu\n", sizeof(size_t));
    printf("  ptrdiff_t         %zu\n", sizeof(ptrdiff_t));

    /* Plain char is signed on x86 Linux and unsigned on ARM Linux. It is one
       of the few differences that genuinely shows up between two Linux boxes,
       so it is worth stating rather than inferring. */
    printf("  plain char is     %s\n", (CHAR_MIN < 0) ? "signed" : "unsigned");

    putchar('\n');
}

void print_byte_order(void)
{
    uint32_t word = 0x01020304u;
    /* Reading an object's representation one byte at a time is exactly what
       unsigned char is for -- it is the one type allowed to alias anything. */
    const unsigned char *bytes = (const unsigned char *) &word;
    size_t i;

    puts("byte order");
    fputs("  0x01020304 is stored as", stdout);
    for (i = 0; i < sizeof word; i++) {
        printf(" %02x", bytes[i]);
    }
    putchar('\n');

    if (bytes[0] == 0x04) {
        puts("  -> little endian: least significant byte at the lowest address");
    } else if (bytes[0] == 0x01) {
        puts("  -> big endian: most significant byte at the lowest address");
    } else {
        puts("  -> neither; investigate before trusting anything else here");
    }

    putchar('\n');
}

void print_memory_layout(void)
{
    int stack_sample = 0;
    const char *literal = "some read-only text";
    void *heap_sample = malloc(1);

    if (heap_sample == NULL) {
        fputs("malloc failed\n", stderr);
        exit(EXIT_FAILURE);
    }

    puts("where things live -- addresses change per run, the ordering is the point");
    printf("  string literal    %p\n", (const void *) literal);
    printf("  static variable   %p\n", (void *) &static_sample);
    printf("  heap block        %p\n", heap_sample);
    printf("  stack variable    %p\n", (void *) &stack_sample);
    puts("  the heap grows upward from low addresses, the stack downward from high");

    free(heap_sample);

    putchar('\n');
}
