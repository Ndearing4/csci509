/*
 * frames.c -- small functions, chosen so their disassembly is worth reading.
 * Nathan Dearing, 2026-08-21.
 *
 * Nothing to implement, and nothing here is clever. Every function exists to
 * make one thing visible in `objdump -d`:
 *
 *   add2        the two-argument function from the exit test. The smallest
 *               complete frame: prologue, two arguments, one local, epilogue.
 *   sum_array   a loop, a pointer walked, and a local that survives across
 *               iterations.
 *   seven_args  more arguments than there are argument registers, so the
 *               last one arrives on the stack.
 *   swap        a function that writes through pointers, so you can see the
 *               difference between an address and a value in the assembly.
 *   deep        three frames stacked, for reading a backtrace against the
 *               actual saved return addresses.
 *
 * ./disasm.sh builds it at -O0 and at -O2 and shows you both. Read -O0
 * first: it is not what the machine would ideally run, it is a literal
 * transcription of your C with every variable given a stack slot, and that
 * literalness is exactly what makes it readable.
 */

#include <stdio.h>
#include <stddef.h>

/* Two arguments in, one out. The exit test function. */
int add2(int a, int b)
{
        int sum = a + b;

        return sum;
}

/* A loop, a pointer, and an accumulator. */
long sum_array(const int *values, size_t n)
{
        long total = 0;
        size_t i;

        for (i = 0; i < n; i++)
                total += values[i];

        return total;
}

/* Seven integer arguments. The System V AMD64 ABI passes the first six in
 * registers and the rest on the stack, so g arrives somewhere different from
 * the other six -- find it in the disassembly. */
int seven_args(int a, int b, int c, int d, int e, int f, int g)
{
        return a + b + c + d + e + f + g;
}

/* Writes through pointers. In the assembly, note that the arguments are
 * addresses and that reaching the values takes an extra memory access each
 * way -- that indirection is the entire cost of a pointer. */
void swap(int *x, int *y)
{
        int tmp = *x;

        *x = *y;
        *y = tmp;
}

/* Three frames, so a backtrace has something to walk. Break on deep_c in gdb
 * and compare `bt` against what you can find by hand with x/ on the stack. */
static int deep_c(int n)
{
        return n * 3;
}

static int deep_b(int n)
{
        return deep_c(n) + 1;
}

int deep(int n)
{
        return deep_b(n) + 1;
}

int main(void)
{
        int values[] = { 1, 2, 3, 4, 5 };
        int x = 10;
        int y = 20;

        printf("add2(2, 3)        = %d\n", add2(2, 3));
        printf("sum_array(1..5)   = %ld\n",
               sum_array(values, sizeof values / sizeof values[0]));
        printf("seven_args(1..7)  = %d\n", seven_args(1, 2, 3, 4, 5, 6, 7));
        swap(&x, &y);
        printf("after swap        = %d %d\n", x, y);
        printf("deep(5)           = %d\n", deep(5));
        return 0;
}
