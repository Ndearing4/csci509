/*
 * main.c -- driver exercising show_bytes across every M1 topic.
 * Nathan Dearing, 2026-08-15.
 *
 * Until show_bytes is implemented the byte lines come out blank; the
 * commentary and the sizeof output still work. Each section says what you
 * should see on x86-64 once it works, so you can check yourself.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "show_bytes.h"

/* The skip-ahead probe, made runnable. An array parameter is not an array. */
static void takes_an_array(int arr[10])
{
        /* gcc warns here on purpose, and -Werror would stop the build -- so
         * the warning is suppressed for exactly this line. Read the warning
         * text with the pragma removed at least once: it is gcc telling you
         * about the single most common C misunderstanding there is. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-array-argument"
        printf("  inside the function, sizeof(arr) = %zu\n", sizeof(arr));
#pragma GCC diagnostic pop
        printf("  ... because arr decayed to a %s\n", "pointer (int *)");
}

int main(void)
{
        printf("=== 1. integers and endianness ===\n");
        {
                int x = 0x12345678;
                show_bytes("x = 0x12345678      ", &x, sizeof x);
                printf("  expect on x86-64: 78 56 34 12 (little endian)\n\n");
        }

        printf("=== 2. two's complement ===\n");
        {
                int pos = 5;
                int neg = -5;
                show_bytes("pos =  5            ", &pos, sizeof pos);
                show_bytes("neg = -5            ", &neg, sizeof neg);
                printf("  expect: 05 00 00 00  and  FB FF FF FF\n");
                printf("  work out by hand why -5 is FB FF FF FF, not 85 00 00 00\n\n");
        }

        printf("=== 3. signed vs unsigned is only interpretation ===\n");
        {
                int si = -1;
                unsigned int ui = 0xFFFFFFFFu;
                show_bytes("(int) -1            ", &si, sizeof si);
                show_bytes("(unsigned) 4294967295", &ui, sizeof ui);
                printf("  identical bytes. The type is not in the object, it is\n");
                printf("  in how the compiler was told to read the object.\n\n");
        }

        printf("=== 4. the same number as int and as float ===\n");
        {
                int i = 3490593;
                float f = 3490593.0f;
                show_bytes("int   3490593       ", &i, sizeof i);
                show_bytes("float 3490593.0f    ", &f, sizeof f);
                printf("  same value, completely different representation.\n");
                printf("  the float bytes are sign|exponent|mantissa -- pull them\n");
                printf("  apart by hand and confirm you get the value back.\n\n");
        }

        printf("=== 5. why 0.1 + 0.2 != 0.3 ===\n");
        {
                double a = 0.1 + 0.2;
                double b = 0.3;
                show_bytes("0.1 + 0.2           ", &a, sizeof a);
                show_bytes("0.3                 ", &b, sizeof b);
                printf("  0.1 + 0.2 == 0.3 evaluates to %s\n", (a == b) ? "true" : "false");
                printf("  printed with 20 digits: %.20f vs %.20f\n", a, b);
                printf("  neither 0.1 nor 0.2 is exactly representable in binary.\n");
                printf("  one hour on this, then move on -- thin on 509 relevance.\n\n");
        }

        printf("=== 6. struct layout and padding ===\n");
        {
                struct gappy { char c; int i; char d; };
                struct gappy g;

                /* memset first: padding bytes are otherwise indeterminate, and
                 * reading them is exactly the kind of thing valgrind yells
                 * about in M2. Zeroing makes the holes visible as 00. */
                memset(&g, 0, sizeof g);
                g.c = 0x11;
                g.i = 0x22222222;
                g.d = 0x33;

                show_bytes("struct{char,int,char}", &g, sizeof g);
                printf("  sizeof = %zu, but the fields only need %zu bytes\n",
                       sizeof g, sizeof(char) + sizeof(int) + sizeof(char));
                printf("  the gaps are alignment padding. reorder the fields\n");
                printf("  largest-first and watch sizeof shrink.\n\n");
        }

        printf("=== 7. pointers are objects too ===\n");
        {
                int target = 42;
                int *p = &target;
                show_bytes("the pointer itself  ", &p, sizeof p);
                printf("  p points at %p, and the bytes above are that address\n",
                       (void *)p);
                printf("  stored little-endian. sizeof(int *) = %zu\n\n", sizeof p);
        }

        printf("=== 8. arrays decay, and sizeof is where you notice ===\n");
        {
                int arr[10];
                memset(arr, 0, sizeof arr);
                printf("  in the scope where it was declared, sizeof(arr) = %zu\n",
                       sizeof arr);
                takes_an_array(arr);
                printf("  if that difference was obvious to you before running\n");
                printf("  this, M1's probe says you can skip ahead.\n\n");
        }

        printf("=== 9. plain char signedness ===\n");
        {
                char c = (char)0xFF;
                int as_int = c;
                printf("  CHAR_MIN = %d, so plain char is %s here\n",
                       CHAR_MIN, (CHAR_MIN < 0) ? "SIGNED" : "unsigned");
                printf("  (char)0xFF promoted to int = %d\n", as_int);
                printf("  this is why show_bytes must use unsigned char.\n\n");
        }

        return 0;
}
