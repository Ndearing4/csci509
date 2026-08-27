/*
 * counters.h -- the ONE declaration of a global shared across files.
 * Nathan Dearing, 2026-08-23.
 *
 * HEADER GUARDS, EXPLAINED ONCE.
 *
 * #ifndef / #define / #endif is a header guard. Without it, a header
 * #included into two different .c files that both get compiled into the
 * same program -- or #included twice via two different paths in one file --
 * would paste its contents in twice. That is why counters.c and tour.c can
 * both #include this file (see build.sh) and land in the same build without
 * a "redefined" error: the guard makes the second #include a no-op, since
 * the macro is already defined and the preprocessor skips straight to
 * #endif.
 *
 * (list.h and mystr.h carry the same guard for the same reason -- see the
 * one-line note at their #ifndef rather than this explanation again.)
 */

#ifndef COUNTERS_H
#define COUNTERS_H

extern int bump_count;   /* DECLARATION: "this name exists, defined elsewhere" */

void bump(void);          /* defined in counters.c */

#endif /* COUNTERS_H */
