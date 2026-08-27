/*
 * tour.c -- the C that M2 assumes you already have.
 * Nathan Dearing, 2026-08-21.
 *
 * The sequel to m1-data/c-syntax/tour.c, and the same deal: every construct
 * `list/` needs is demonstrated here on a DIFFERENT problem, so that
 * assembling them into a linked list is still your job. There is not one
 * linked list in this file until section 6, and that one is built by hand on
 * the stack with no malloc anywhere near it.
 *
 * Read a section, run it, then CHANGE something and run it again. The
 * "Try it:" notes say what is worth breaking.
 *
 *   ./build.sh
 *
 * WHERE JAVA HELPS AND WHERE IT LIES
 *
 * You have written linked lists before, in a language with references and a
 * garbage collector. Three things carry over and three do not:
 *
 *   carries over:  a node holds a value and a link; an empty list is a null
 *                  link; you traverse by following links until null
 *
 *   does not:      assignment copies the whole struct rather than aliasing
 *                  it (section 1); nobody frees anything for you (section 4);
 *                  a struct has no methods, so every "method" is a function
 *                  whose first argument is the thing it acts on (section 3)
 *
 * Section 5 is the one to slow down on. It looks like a C quirk and it is
 * not -- Java does exactly the same thing, and you have almost certainly
 * been bitten by it there without having the vocabulary for it.
 *
 * `reference/c-for-java-programmers.md` has the full translation table.
 */

#include <stdio.h>
#include <stdlib.h>     /* malloc, free */
#include <string.h>     /* strlen, strcpy */
#include <stddef.h>     /* size_t, offsetof */
#include "counters.h"   /* extern int bump_count; void bump(void); */

/* ------------------------------------------------------------------ *
 * 1. struct: several variables, bundled and given one name.
 * ------------------------------------------------------------------ */
struct Point {
        int x;          /* horizontal */
        int y;          /* vertical */
};

static void s1_struct(void)
{
        struct Point a;                         /* note: "struct Point", not "Point" */
        struct Point b = { 3, 4 };              /* initialise in declaration order */
        struct Point c = { .x = 7, .y = 8 };    /* or by name -- clearer, C99 and up */

        printf("--- 1. struct ---\n");

        a.x = 1;                                /* . reaches a field */
        a.y = 2;
        printf("  a = (%d, %d)\n", a.x, a.y);
        printf("  b = (%d, %d)\n", b.x, b.y);
        printf("  c = (%d, %d)\n", c.x, c.y);

        /* HERE IS THE FIRST PLACE JAVA LIES TO YOU.
         *
         * In Java, `Point d = b;` makes d and b the same object -- two
         * references, one object, and changing d.x changes b.x.
         *
         * In C, a struct is a VALUE, like an int. This copies all 8 bytes. */
        {
                struct Point d = b;

                d.x = 999;
                printf("  after d = b; d.x = 999:\n");
                printf("    d = (%d, %d)   <- changed\n", d.x, d.y);
                printf("    b = (%d, %d)   <- untouched. It is a copy.\n", b.x, b.y);
        }

        /* A struct with no name is possible but pointless here. A struct
         * cannot be compared with == either -- there is no memberwise
         * comparison in C, and padding bytes would make a byte comparison
         * unreliable. Compare field by field.
         *
         * Try it: add `if (a == b)` and read the error. It is one of the
         * clearer messages gcc produces. */
}

/* ------------------------------------------------------------------ *
 * 2. typedef: so you can stop writing "struct" everywhere.
 * ------------------------------------------------------------------ */

/* Two ways to say the same thing. This one names the struct AND the type: */
typedef struct Item {
        int   id;
        int   quantity;
} Item;

/* ... and from here on, `Item` and `struct Item` both work. The tag after
 * `struct` and the name after the closing brace are in different namespaces,
 * which is why they are allowed to be the same word and why seeing
 * `typedef struct Node { ... } Node;` in list.h is not a typo. */

static void s2_typedef(void)
{
        struct Item explicit_form = { 1, 10 };  /* still legal */
        Item        short_form    = { 2, 20 };  /* what the typedef bought */

        printf("--- 2. typedef ---\n");
        printf("  explicit: id=%d qty=%d\n", explicit_form.id, explicit_form.quantity);
        printf("  short:    id=%d qty=%d\n", short_form.id, short_form.quantity);
        printf("  sizeof(Item) = %zu\n", sizeof(Item));

        /* Java has no equivalent because Java has no `struct` keyword to get
         * rid of. This is pure syntax convenience, and it is worth knowing
         * that it is ONLY that -- a typedef creates no new type, just a
         * second name for one. */
}

/* ------------------------------------------------------------------ *
 * 3. Pointer to a struct, and -> ; and what a "method" becomes.
 * ------------------------------------------------------------------ */

/* Java:  item.restock(5)          -- the object is implicit, called `this`
 * C:     restock(&item, 5)        -- the object is the first argument
 *
 * That is the entire translation. A C "method" is a function that takes a
 * pointer to the thing it operates on. `this` is not magic in Java either;
 * it is exactly this parameter, passed for you. */
static void restock(Item *it, int amount)
{
        it->quantity += amount;         /* -> because `it` is a POINTER */
}

/* Taking the struct BY VALUE instead. This one cannot work, and the point
 * is to see it not work rather than be told. */
static void restock_broken(Item it, int amount)
{
        it.quantity += amount;          /* modifies the local copy */
}

static void s3_arrow(void)
{
        Item  stock = { 7, 100 };
        Item *p     = &stock;

        printf("--- 3. -> and pass-by-pointer ---\n");

        /* p->quantity is EXACTLY (*p).quantity. The arrow exists because
         * that second form needs parentheses -- *p.quantity would parse as
         * *(p.quantity) -- and people got tired of writing them. */
        printf("  stock.quantity      = %d\n", stock.quantity);
        printf("  p->quantity         = %d\n", p->quantity);
        printf("  (*p).quantity       = %d   <- identical\n", (*p).quantity);

        restock(p, 5);
        printf("  after restock(p, 5):        quantity = %d   <- changed\n",
               stock.quantity);

        restock_broken(stock, 5);
        printf("  after restock_broken(...):  quantity = %d   <- did NOT change\n",
               stock.quantity);

        /* restock_broken got a COPY of the whole struct, edited the copy,
         * and returned. Section 1's lesson, now with consequences.
         *
         * This is also why C code passes pointers to structs almost
         * everywhere: not only so the function can modify it, but because
         * copying a 200-byte struct on every call is real work.
         *
         * Try it: make restock_broken take `const Item it` and rebuild. The
         * compiler will now reject the line that modifies it -- that is how
         * you say "I promise not to change this" in C. */
}

/* ------------------------------------------------------------------ *
 * 4. malloc and free: the heap, and who owns what.
 * ------------------------------------------------------------------ */
static void s4_heap(void)
{
        Item *it;
        int  *numbers;
        int   i;

        printf("--- 4. malloc and free ---\n");

        /* Java:  Item it = new Item();     <- typed, zeroed, freed by the GC
         * C:     three separate things you have to do yourself. */

        it = malloc(sizeof *it);        /* 1. ask for bytes */
        if (it == NULL) {               /* 2. malloc can fail. Check it. */
                perror("malloc");
                return;
        }

        /* `sizeof *it`, not `sizeof(Item)`. Both are correct today; only the
         * first stays correct when someone changes `it` to a different type,
         * because it says "however big the thing this points at is". Note
         * that *it is never evaluated -- sizeof only looks at the type. */

        it->id = 42;                    /* 3. malloc does NOT initialise. */
        it->quantity = 0;               /*    Every field is your job. */
        printf("  heap Item: id=%d qty=%d\n", it->id, it->quantity);

        free(it);                       /* 4. and you give it back */

        /* free() does not change your variable. `it` still holds the same
         * address; that address is just no longer yours to use. A pointer in
         * that state is called DANGLING, and reading it is bugs/bug1. */
        printf("  after free, it is still %s\n",
               it == NULL ? "NULL" : "a non-NULL address -- and unusable");
        it = NULL;      /* the habit that makes the mistake impossible */

        /* malloc gives you BYTES, not objects. Ask for room for many: */
        numbers = malloc(10 * sizeof *numbers);
        if (numbers == NULL) {
                perror("malloc");
                return;
        }
        for (i = 0; i < 10; i++)
                numbers[i] = i * i;
        printf("  10 heap ints: %d %d %d ... %d\n",
               numbers[0], numbers[1], numbers[2], numbers[9]);
        printf("  and numbers[i] still means *(numbers + i)\n");

        /* There is no numbers.length. The allocation does not record how big
         * it was in any way you can read -- the allocator knows, and does not
         * tell you. If you need the count, you keep it yourself. Every
         * function in this repo that takes a pointer also takes a size, and
         * that is why.
         *
         * Try it: print numbers[10] and run it. Then run it under valgrind:
         *     valgrind ./tour
         * Nothing crashes and valgrind names the exact line. */

        free(numbers);

        /* One malloc, one free, and free ONLY what malloc returned. Freeing
         * `numbers + 1` or a stack address corrupts the allocator. */
}

/* ------------------------------------------------------------------ *
 * 5. Why Node ** -- derived, not memorised. THE section.
 * ------------------------------------------------------------------ */

static Item catalogue[3] = { { 10, 1 }, { 20, 2 }, { 30, 3 } };

/* Attempt one: "point the caller's pointer at the first catalogue entry." */
static void select_first_broken(Item *sel)
{
        sel = &catalogue[0];    /* assigns to the PARAMETER */

        /* It really did work -- in here. Watch this line print a real id
         * and the caller's pointer still be NULL a moment later. */
        printf("    inside select_first_broken: sel->id = %d\n", sel->id);
}

/* Attempt two: take the address OF the caller's pointer. */
static void select_first(Item **sel)
{
        *sel = &catalogue[0];   /* assigns to what the parameter points AT */
}

static void s5_double_pointer(void)
{
        Item *selected = NULL;

        printf("--- 5. the double pointer ---\n");

        select_first_broken(selected);
        printf("  after select_first_broken(selected):  selected = %s\n",
               selected == NULL ? "NULL -- nothing happened" : "set");

        select_first(&selected);
        printf("  after select_first(&selected):        selected = %s",
               selected == NULL ? "NULL" : "set");
        if (selected != NULL)
                printf(", id = %d", selected->id);
        printf("\n");

        /* WHY.
         *
         * C passes every argument by copying it. Always, no exceptions.
         *
         * `Item *sel` is a copy of the caller's pointer. It holds the same
         * address, so *sel reaches the same Item -- which is why section 3's
         * restock() worked. But assigning to `sel` itself only overwrites
         * the copy, and the copy dies when the function returns.
         *
         * To change the caller's POINTER you need the caller's pointer's
         * ADDRESS. The address of an `Item *` is an `Item **`. That is the
         * whole derivation, and there is nothing else to it.
         *
         * Read `Item **sel` right to left: sel is a pointer, to a pointer,
         * to an Item.
         *
         *
         * AND JAVA DOES THE SAME THING.
         *
         *     void selectFirst(Item sel) { sel = catalogue[0]; }
         *
         * That method is just as useless, for exactly the same reason: Java
         * passes references by value too, so `sel` is a copy of the caller's
         * reference. Java's escape hatches are to return the new value, or
         * to set a field on some object both sides can see. C has one more
         * option -- take the address of the variable itself -- and that is
         * the only real difference.
         *
         * You have hit this in Java. You just did not have a name for it.
         *
         * Try it: add a third function taking `Item *sel` that RETURNS the
         * new pointer, and have the caller write `selected = select_r();`.
         * That works too, and it is the Java shape. Now look at list.h and
         * ask why the list uses ** instead -- the answer is that a function
         * like list_remove_first already has a return value doing another
         * job, and that walking a `Node **` down the list lets you delete
         * the first node without special-casing it. */
}

/* ------------------------------------------------------------------ *
 * 6. A struct that points at its own type.
 * ------------------------------------------------------------------ */

/* A struct cannot CONTAIN itself -- that would need infinite space, and the
 * compiler says so: "field has incomplete type". It can hold a POINTER to
 * itself, because a pointer is always 8 bytes no matter what it points at,
 * including something not fully defined yet.
 *
 * Note this one uses `struct Link *` inside its own definition. At that
 * point the typedef name `Link` does not exist yet, so the tag is required.
 * That is why list.h says `struct Node *next;` inside `typedef struct Node`. */
typedef struct Link {
        int          value;
        struct Link *next;
} Link;

static void s6_self_reference(void)
{
        /* Three links, on the STACK, chained by hand. No malloc, no loop,
         * no functions -- the shape only, so that building one properly is
         * still the exercise in list/. */
        Link third  = { 30, NULL };
        Link second = { 20, &third };
        Link first  = { 10, &second };

        const Link *walk;               /* const: this loop only reads */

        printf("--- 6. a self-referential struct ---\n");

        printf("  sizeof(Link) = %zu  (an int, padding, and an 8-byte pointer)\n",
               sizeof(Link));

        /* The traversal. This much you already know from Java; what is new
         * is that `next` is an address you can print. */
        for (walk = &first; walk != NULL; walk = walk->next)
                printf("  value %2d  at %p  -> next %p\n",
                       walk->value, (const void *)walk, (const void *)walk->next);

        printf("  the last next prints as (nil) -- that is glibc showing you\n");
        printf("  a NULL pointer, and it is how the loop knows to stop.\n");
        printf("  An empty list is just NULL by itself -- no node at all.\n");

        /* Two things that are different from Java and both matter:
         *
         *   - these three Links vanish when this function returns. They are
         *     locals. Returning &first from here would be bugs/bug6.
         *     A list that outlives the function that built it has to be on
         *     the heap, which is what section 4 was for.
         *
         *   - nothing here counts references. If you overwrite first.next
         *     you have not "deleted" second -- on the heap you would simply
         *     have lost the only pointer to it, and that is a leak.
         *
         * Try it: swap the declaration order of `first` and `third` and
         * rebuild. Read the error, and work out why the compiler insists
         * they are declared in that order and Java would not care. */
}

/* ------------------------------------------------------------------ *
 * 7. There is no String. There is a char array with a 0 at the end.
 * ------------------------------------------------------------------ */
static void s7_strings(void)
{
        char        buf[16] = "hello";  /* 16 bytes; 6 of them used */
        const char *lit     = "world";  /* a pointer to unwritable memory */
        size_t      i;

        printf("--- 7. C strings ---\n");

        printf("  buf = \"%s\"\n", buf);
        printf("  sizeof buf  = %zu   <- the ARRAY: how much room there is\n",
               sizeof buf);
        printf("  strlen(buf) = %zu   <- the STRING: how much is used\n",
               strlen(buf));

        /* Those are two different questions and people conflate them
         * constantly. sizeof is answered at compile time by the type;
         * strlen walks the bytes at run time looking for the 0. */

        printf("  the bytes:  ");
        for (i = 0; i < 8; i++)
                printf("%02x ", (unsigned char)buf[i]);
        printf("\n              h  e  l  l  o  \\0\n");
        printf("  the three bytes after it are 00 as well, because an\n");
        printf("  initialiser shorter than the array zero-fills the rest.\n");
        printf("  Only the FIRST 00 is the terminator; the others are\n");
        printf("  unused room. An uninitialised char buf[16] would hold\n");
        printf("  whatever was on the stack, which is bugs/bug5.\n");

        /* That 00 is the whole convention. Nothing records the length; every
         * function that takes a `char *` walks until it finds a zero byte.
         * Lose the zero and strlen walks off the end of the array, which is
         * why m2-memory/strings/ spends a whole exercise on strncpy. */

        strcpy(buf, "hi");
        printf("  after strcpy(buf, \"hi\"): \"%s\", strlen %zu, sizeof still %zu\n",
               buf, strlen(buf), sizeof buf);

        printf("  lit = \"%s\"\n", lit);

        /* `lit` points into read-only memory -- the string literal lives in
         * .rodata, which M4a's symbols/ exercise looks at directly. Writing
         * through it segfaults. It is `const char *` for that reason, and
         * the const is a warning to you, not a property of the pointer.
         *
         * Try it: drop the const, add `lit[0] = 'W';`, and run it. It
         * compiles with a warning and dies at run time. That is the single
         * most common way a C beginner gets a segfault with no obvious bug.
         *
         * Try it: change `char buf[16]` to `char buf[4]` and rebuild. gcc
         * catches THIS one at compile time, because the initialiser is a
         * literal it can measure. It will not catch the strcpy on the next
         * line if you make the string longer -- that one is bugs/bug4. */
}

/* ------------------------------------------------------------------ *
 * 8. The three places a variable can live.
 * ------------------------------------------------------------------ */
static int  static_var = 1;             /* .data, exists for the whole run */

static void s8_storage(void)
{
        int         stack_var = 2;      /* born here, dies at the closing brace */
        static int  static_local = 3;   /* like static_var, but named privately */
        int        *heap_var = malloc(sizeof *heap_var);
        const char *literal  = "in rodata";

        printf("--- 8. stack, heap, static ---\n");

        if (heap_var == NULL) {
                perror("malloc");
                return;
        }
        *heap_var = 4;

        printf("  literal      %p   read-only, in the executable\n", (const void *)literal);
        printf("  static_var   %p   fixed address, whole program run\n", (void *)&static_var);
        printf("  static_local %p   same idea, private name\n", (void *)&static_local);
        printf("  heap_var     %p   from malloc, yours until you free it\n", (void *)heap_var);
        printf("  stack_var    %p   this frame only\n", (void *)&stack_var);

        printf("\n  Sort those five addresses. Low to high they come out\n");
        printf("  literal < static < heap < ... < stack, with an enormous gap\n");
        printf("  before the stack -- it grows DOWNWARD from the top of the\n");
        printf("  address space toward the heap growing up. That layout is in\n");
        printf("  the top-level README, measured on this machine in M0.\n");

        free(heap_var);

        /* Java has two of these and hides the difference: locals of primitive
         * type on the stack, everything made with `new` on the heap, and the
         * GC deciding when heap things end. C makes you say which, and the
         * lifetime rule is the thing to hold on to:
         *
         *   stack   until the function returns          -- automatic
         *   heap    until you call free                 -- yours
         *   static  until the program exits             -- automatic
         *
         * Every bug in bugs/ is a pointer used outside one of those three
         * windows. That is not a coincidence; it is the definition. */
}

/* ------------------------------------------------------------------ *
 * 9. extern: one global's definition, shared across files.
 * ------------------------------------------------------------------ */
static void s9_extern(void)
{
        printf("--- 9. extern: sharing one global across files ---\n");

        printf("  bump_count starts at %d (defined once, in counters.c)\n",
               bump_count);
        bump();
        bump();
        bump();
        printf("  after three calls to bump(): bump_count = %d\n", bump_count);

        /* counters.h has this DECLARED with `extern` -- every file that
         * #includes it can read and write bump_count. counters.c has it
         * DEFINED, with no extern, no static: that is the one file whose
         * storage this name refers to. tour.c never defines bump_count; it
         * only borrows the definition counters.c provides.
         *
         * Contrast with section 8's static_var: that one is `static` at
         * file scope, which is the opposite of extern -- invisible outside
         * tour.c. No other .c file could write `extern int static_var;`
         * and reach it; the linker would refuse. static hides a global,
         * extern shares one, and a name is never both at once.
         *
         * Try it: delete counters.c from build.sh's gcc command line and
         * rebuild. You get "undefined reference to bump_count" from the
         * LINKER, not the compiler -- gcc was satisfied with the
         * declaration alone; ld is the one that needed the definition.
         * That error is worth seeing on purpose once; M4a's symbols/
         * exercise is built entirely out of errors shaped like it. */
}

int main(void)
{
        s1_struct();         putchar('\n');
        s2_typedef();        putchar('\n');
        s3_arrow();          putchar('\n');
        s4_heap();           putchar('\n');
        s5_double_pointer(); putchar('\n');
        s6_self_reference(); putchar('\n');
        s7_strings();        putchar('\n');
        s8_storage();        putchar('\n');
        s9_extern();
        return 0;
}
