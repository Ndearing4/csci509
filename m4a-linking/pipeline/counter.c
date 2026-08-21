/*
 * counter.c -- the other translation unit, so there is something to link to.
 * Nathan Dearing, 2026-08-21.
 *
 * Exists so demo.o has one genuinely undefined symbol to go looking for.
 * `nm demo.o` marks it U; `nm counter.o` marks it T; the linker's whole job
 * in this program is noticing that those two are the same name.
 */

static int count;               /* .bss, private to this file */

/* Bump the counter and return its new value. */
int shared_counter_bump(void)
{
        count++;
        return count;
}
