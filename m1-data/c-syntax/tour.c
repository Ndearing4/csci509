/*
 * tour.c -- the C syntax M1 assumes you already have.
 *
 * Deliberately NOT about show_bytes. Every construct show_bytes needs is
 * demonstrated here on a different problem, so that assembling them is
 * still your job.
 *
 * Read a section, run it, then CHANGE something and run it again. Breaking
 * it on purpose teaches more than reading it. The compiler is a patient
 * tutor -- every error message here is a lesson, not a failure.
 *
 *   ./build.sh
 */

#include <stdio.h>
#include <stddef.h>     /* size_t, ptrdiff_t */

/* ------------------------------------------------------------------ *
 * 1. printf: the first argument is ALWAYS a format string.
 * ------------------------------------------------------------------ */
static void s1_printf(void)
{
        int    n = 42;
        double d = 3.5;
        char   c = 'A';

        printf("--- 1. printf ---\n");

        /* printf("something") does NOT mean "print something's value".
         * It means "treat this string as instructions". The values you
         * want printed come AFTER, one per % placeholder. */
        printf("  n = %d, d = %f, c = %c\n", n, d, c);

        /* Common conversions:
         *   %d   int (decimal)          %u   unsigned int
         *   %x   unsigned, lowercase hex   %X   uppercase hex
         *   %c   one character           %s   a string
         *   %zu  size_t                  %p   a pointer
         *
         * Between the % and the letter you can put flags and a width.
         * "%5d" = at least 5 columns, padded with spaces.
         * "%05d" = at least 5 columns, padded with ZEROS. */
        printf("  42 in hex: %x  |  width 4: %4x  |  zero-pad 4: %04x\n",
               n, n, n);

        /* Try it: change %04x to %X above and rerun. What happened to the
         * padding, and what happened to the letters? */
}

/* ------------------------------------------------------------------ *
 * 2. & is "the address of".
 * ------------------------------------------------------------------ */
static void s2_address_of(void)
{
        int n = 42;

        printf("--- 2. address-of ---\n");

        /* n is a value. &n is WHERE that value lives in memory. */
        printf("  n  = %d\n", n);
        printf("  &n = %p   <- an address; it changes every run\n", (void *)&n);

        /* %p officially wants a void *, hence the cast. Delete the cast
         * and rebuild to see the compiler complain. */
}

/* ------------------------------------------------------------------ *
 * 3. Declaring a pointer, and * as "the thing pointed at".
 * ------------------------------------------------------------------ */
static void s3_pointers(void)
{
        int  n = 42;
        int *p;         /* read as: the expression *p has type int,
                         * therefore p is a pointer to int. */

        printf("--- 3. pointers ---\n");

        p = &n;         /* p now holds n's address */

        /* In a DECLARATION, * builds a pointer type.
         * In an EXPRESSION, * unwraps one. Same symbol, opposite jobs. */
        printf("  p  = %p\n", (void *)p);
        printf("  *p = %d    <- follow the pointer, get the value\n", *p);

        *p = 99;        /* writing THROUGH the pointer changes n itself */
        printf("  after *p = 99, n is now %d\n", n);

        /* You can declare and initialise in one line, which is what you
         * will usually want: */
        int *q = &n;
        printf("  *q = %d\n", *q);

        /* Gotcha: the * binds to the NAME, not the type.
         *     int *a, b;    <- a is int*, b is a plain int!
         * Declare one pointer per line and this never bites you. */
}

/* ------------------------------------------------------------------ *
 * 4. Pointer arithmetic scales by the pointed-to type.
 * ------------------------------------------------------------------ */
static void s4_arithmetic(void)
{
        int    i = 0;
        short  h = 0;
        int   *pi = &i;
        short *ph = &h;

        printf("--- 4. pointer arithmetic ---\n");

        /* p + 1 does NOT mean "one byte further". It means "one ELEMENT
         * further" -- and how big an element is depends on the pointer's
         * type. This is the single most important fact about pointers. */

        ptrdiff_t int_step   = (char *)(pi + 1) - (char *)pi;
        ptrdiff_t short_step = (char *)(ph + 1) - (char *)ph;

        printf("  int   *p:  p+1 is %td bytes further\n", int_step);
        printf("  short *p:  p+1 is %td bytes further\n", short_step);

        /* So the pointer's type is not decoration. It decides how far a
         * step is, and how many bytes a dereference reads. Ask yourself
         * what step size you would want if you were walking one byte at
         * a time -- and therefore what type the pointer must have. */
}

/* ------------------------------------------------------------------ *
 * 5. Arrays, indexing, and what [ ] actually means.
 * ------------------------------------------------------------------ */
static void s5_indexing(void)
{
        int  a[4] = { 10, 20, 30, 40 };
        int *p    = a;          /* an array name decays to a pointer to
                                 * its first element -- no & needed */

        printf("--- 5. indexing ---\n");

        /* a[i] is not a separate feature. The language DEFINES it as:
         *     a[i]  ==  *(a + i)
         * Indexing is pointer arithmetic plus a dereference. */
        printf("  a[2]      = %d\n", a[2]);
        printf("  *(a + 2)  = %d   <- identical by definition\n", *(a + 2));
        printf("  p[2]      = %d   <- works on any pointer, not just arrays\n",
               p[2]);

        /* Which means: once you have a pointer, you can index it. You do
         * not need an array to use [ ]. */
}

/* ------------------------------------------------------------------ *
 * 6. for loops, and picking the right counter type.
 * ------------------------------------------------------------------ */
static void s6_loops(void)
{
        int    a[4] = { 10, 20, 30, 40 };
        size_t n    = 4;
        size_t i;

        printf("--- 6. loops ---\n");

        /* for (initialise ; keep going while this is true ; do after each pass) */
        for (i = 0; i < n; i++)
                printf("  a[%zu] = %d\n", i, a[i]);

        /* Why size_t and not int? size_t is UNSIGNED, and sizeof gives you
         * a size_t. Comparing a signed int against an unsigned size_t is a
         * real bug class, so -Wextra warns and -Werror makes it fatal.
         *
         * Try it: change `size_t i;` to `int i;` and rebuild. Read the
         * error. That is the exact error you will hit if you copy the
         * loop out of CS:APP verbatim. */
}

/* ------------------------------------------------------------------ *
 * 7. Casting a pointer: same address, different interpretation.
 * ------------------------------------------------------------------ */
static void s7_casting(void)
{
        int    x  = 0x11223344;
        int   *pi = &x;
        short *ph = (short *)pi;        /* <- the cast */

        printf("--- 7. casting a pointer ---\n");

        printf("  as one int:      %08x\n", x);
        printf("  as two shorts:   %04x %04x\n", ph[0], ph[1]);
        printf("  same address?    %s\n",
               (void *)pi == (void *)ph ? "yes" : "no");

        /* Nothing in memory changed. The bytes are identical. All the cast
         * did was change the compiler's instructions for READING them:
         * one 4-byte read became two 2-byte reads.
         *
         * Note the order the two shorts came out in. You already know why.
         *
         * Caveat worth knowing: aliasing an int through a short * like this
         * is not strictly legal C -- it happens to work at -O0. There is
         * exactly ONE type you are always allowed to inspect any object's
         * bytes through. Your show_bytes.c header comment names it, and now
         * you know why that rule exists. */
}

/* ------------------------------------------------------------------ *
 * 8. Passing pointers to functions.
 * ------------------------------------------------------------------ */
static void add_one(int *p)     /* takes an ADDRESS */
{
        *p = *p + 1;            /* modifies the caller's variable */
}

static void s8_functions(void)
{
        int n = 5;

        printf("--- 8. pointers as parameters ---\n");

        add_one(&n);            /* pass the address, not the value */
        printf("  after add_one(&n), n = %d\n", n);

        /* C always copies arguments. Passing &n copies the ADDRESS, and a
         * copy of an address still points at the original. That is the
         * only way a function can reach out and change something. */
}

int main(void)
{
        s1_printf();     putchar('\n');
        s2_address_of(); putchar('\n');
        s3_pointers();   putchar('\n');
        s4_arithmetic(); putchar('\n');
        s5_indexing();   putchar('\n');
        s6_loops();      putchar('\n');
        s7_casting();    putchar('\n');
        s8_functions();
        return 0;
}
