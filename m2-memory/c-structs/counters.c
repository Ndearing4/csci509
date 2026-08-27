/*
 * counters.c -- the ONE definition of the global counters.h declares.
 * Nathan Dearing, 2026-08-23.
 *
 * A declaration (counters.h's `extern int bump_count;`) says a name exists
 * and what type it is, without reserving storage. A definition (the line
 * below) is where storage actually gets set aside. Every other .c file that
 * wants bump_count #includes counters.h and gets the declaration; exactly
 * one .c file -- this one -- provides the definition, or the linker has
 * nothing to point every other file's uses at.
 *
 * Contrast with tour.c section 8's static_var: `static` at file scope means
 * "invisible outside this translation unit" -- the opposite problem from
 * extern, which means "let another translation unit see this one's global."
 * A static global can never be extern'd from elsewhere; an extern
 * declaration always needs exactly one non-static definition somewhere.
 */

#include "counters.h"

int bump_count = 0;      /* DEFINITION: the storage lives here, once */

void bump(void)
{
        bump_count++;
}
