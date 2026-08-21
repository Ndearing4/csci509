/*
 * sections.c -- twelve declarations. Predict the section for each.
 * Nathan Dearing, 2026-08-21.
 *
 * Nothing to implement. Fill in the table in SECTIONS.md on paper FIRST,
 * then run ./reveal.sh. Looking first turns a five-minute exercise into a
 * five-minute reading of somebody else's answer.
 *
 * The four sections that matter, and the question each one answers:
 *
 *   .text     executable instructions.            Is it code?
 *   .rodata   initialised, read-only.             Is it const?
 *   .data     initialised, writable.              Does it have a nonzero value?
 *   .bss      zero or uninitialised, writable.    Would storing it be storing zeros?
 *
 * .bss is the interesting one. It occupies no space in the file -- just a
 * size in the header -- because "four thousand zero bytes" compresses to the
 * number 4000. The loader zeroes the pages on the way in.
 *
 * Two more that show up here and are worth recognising rather than learning:
 *   .data.rel.*    holds an address that the loader has to fix up before the
 *                  program starts. A pointer's VALUE is not known at compile
 *                  time, so anything initialised to one lands in a section
 *                  the loader will come back and patch. `india` is here.
 *   .rodata.str1.1 string literals, pooled and deduplicated
 */

#include <stdio.h>

/*  1 */ int          alpha = 42;
/*  2 */ int          bravo;
/*  3 */ int          charlie = 0;
/*  4 */ const int    delta = 7;
/*  5 */ static int   echo_ = 9;
/*  6 */ static int   foxtrot;
/*  7 */ char         golf[1024];
/*  8 */ char         hotel[1024] = "text";
/*  9 */ const char  *india = "a string literal";
/* 10 */ const char   juliet[] = "an array of char";
/* 11 */ int         *kilo = NULL;
/* 12 */ int (*lima)(int, int) = NULL;

/* And three that are not variables at all, for contrast. */
int mike(int a, int b)          /* 13 */
{
        static int novemb = 0;  /* 14 -- a static local */
        int oscar = a + b;      /* 15 -- an ordinary local */

        novemb++;
        return oscar + novemb;
}

int main(void)
{
        /* Touch everything so nothing is optimised away and so the linker
         * has a reason to keep each symbol. */
        printf("%d %d %d %d %d %d %zu %s %s %s %p %p %d\n",
               alpha, bravo, charlie, delta, echo_, foxtrot,
               sizeof golf, hotel, india, juliet,
               (void *)kilo, (void *)lima, mike(1, 2));
        return 0;
}
