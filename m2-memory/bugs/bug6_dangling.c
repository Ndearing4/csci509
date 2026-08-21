/*
 * bug6_dangling.c -- label a report with a name built on the stack.
 * Nathan Dearing, 2026-08-21.
 *
 * Planted bug: one. A pointer outlives the storage it points into.
 *
 * No heap involved, and that matters. Valgrind does notice something here,
 * but read what it actually says:
 *
 *     Conditional jump or move depends on uninitialised value(s)
 *        at strlen ... by printf ... by main (bug6_dangling.c:65)
 *     Uninitialised value was created by a stack allocation
 *        at 0x1090A0: ??? (in ./bug6_dangling)
 *
 * Three things wrong with that as a diagnosis. It calls a dangling pointer an
 * uninitialised value, which is a different bug with a different fix. The
 * origin frame is `???` with no line number, so it does not tell you which
 * buffer. And it fires on line 65 -- the printf that prints the *correct*
 * string -- because the frame is already dead by then even though the bytes
 * have not been trampled yet.
 *
 * So valgrind is not silent, it is *misleading*, which is worse. A tool that
 * misses a bug leaves you where you started; a tool that names the wrong bug
 * sends you off fixing something that was never broken. Reading tool output
 * critically is the actual skill.
 *
 * The one that names it correctly, eventually:
 *     gcc -std=c11 -Wall -Wextra -Werror -g -O0 -fsanitize=address  *         bug6_dangling.c -o bug6_asan
 *     ./bug6_asan                                    <- says nothing
 *     ASAN_OPTIONS=detect_stack_use_after_return=1 ./bug6_asan
 *
 * The second one gets it exactly right: `stack-use-after-return`, the frame
 * it belonged to (`describe`), and both line numbers. The first one is
 * silent, because that check is off by default -- it costs real runtime, so
 * you only pay for it when you already suspect the stack.
 *
 * Line those three runs up: valgrind confidently wrong, ASan silent, ASan
 * correct once asked properly. That comparison is the whole exercise for this
 * file, and the takeaway is that "I ran the tool and it was clean" is not a
 * statement about your program. It is a statement about the tool's defaults.
 *
 * In gdb the route is:
 *     break describe
 *     next until it returns
 *     print r.label            -- an address
 *     print $sp                -- compare them: label points *below* the
 *                                 stack pointer, into a frame that is gone
 *     finish, then print r.label again and watch the next call overwrite it
 *
 * The symptom in the wild is a string that is correct when printed
 * immediately and garbage after any other function call happens in between.
 * This program prints both, so you can see the difference in one run:
 *
 *     bug6: immediately after: id=42 label=report-042
 *     ========================================
 *     bug6: after one more call: id=42 label=========
 *
 * Nothing wrote to r.label. divider()'s frame landed on the same addresses
 * and filled them with '='. Both printfs are equally wrong; only one of them
 * looks it.
 */

#include <stdio.h>
#include <stdlib.h>

struct Report {
        int id;                 /* which report */
        const char *label;      /* human-readable name for it */
};

/* Fill in `r`'s label for the given id. */
static void describe(struct Report *r, int id)
{
        char name[32];

        snprintf(name, sizeof name, "report-%03d", id);
        r->id = id;
        r->label = name;
}

/* Print a divider. Does nothing interesting -- it is here to use some stack. */
static void divider(int width)
{
        char bar[64];
        int i;

        for (i = 0; i < width && i < 63; i++)
                bar[i] = '=';
        bar[i] = '\0';
        printf("%s\n", bar);
}

int main(void)
{
        struct Report r;

        describe(&r, 42);
        printf("bug6: immediately after: id=%d label=%s\n", r.id, r.label);

        divider(40);

        printf("bug6: after one more call: id=%d label=%s\n", r.id, r.label);
        return EXIT_SUCCESS;
}
