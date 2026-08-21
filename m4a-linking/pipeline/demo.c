/*
 * demo.c -- a small file with one of everything, to watch it go through.
 * Nathan Dearing, 2026-08-21.
 *
 * Nothing to implement. Every declaration here exists to show up somewhere
 * later: in the preprocessed text, in the assembly, in a section, or in the
 * symbol table. `./pipeline.sh` walks it through all four stages and
 * WALKTHROUGH.md asks what you expect to find at each one.
 *
 * Read the file once now and predict, for each item below, which section it
 * ends up in and whether it appears in `nm`'s output at all.
 */

#include <stdio.h>
#include <string.h>

#define GREETING   "hello"              /* a macro: gone after -E */
#define TWICE(x)   ((x) + (x))          /* a function-like macro, also gone */

int    initialised_global   = 7;        /* has a value: goes in .data */
int    zero_global          = 0;        /* zero: goes in .bss, costs no file space */
int    uninitialised_global;            /* also .bss, for the same reason */
static int private_global   = 3;        /* .data, but not visible to the linker */
const  int constant_global  = 11;       /* read-only: .rodata */

char   message[32]          = GREETING; /* .data, 32 bytes of it */
const  char *literal        = "world";  /* the POINTER is .data, the TEXT is .rodata */

/* Defined here, so the linker will see it as a definition. */
int add(int a, int b)
{
        int sum = a + b;                /* a local: no section, it lives on the stack */
        return sum;
}

/* static, so it is local to this file. Watch what nm calls it. */
static int triple(int x)
{
        return TWICE(x) + x;
}

/* Declared but NOT defined anywhere in this file. This is the one that
 * creates a relocation, and the one the linker has to go find. */
extern int shared_counter_bump(void);

int main(void)
{
        static int call_count = 0;      /* static local: .bss, but nameless outside */

        call_count++;
        printf("%s, %s\n", message, literal);
        printf("add(2,3)              = %d\n", add(2, 3));
        printf("triple(5)             = %d\n", triple(5));
        printf("initialised_global    = %d\n", initialised_global);
        printf("private_global        = %d\n", private_global);
        printf("constant_global       = %d\n", constant_global);
        printf("strlen(message)       = %zu\n", strlen(message));
        printf("call_count            = %d\n", call_count);

        /* Defined in counter.c. In demo.o this is a hole plus a relocation;
         * it only becomes an address at link time. */
        printf("shared_counter_bump() = %d\n", shared_counter_bump());

        return 0;
}
