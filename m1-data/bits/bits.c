/*
 * bits.c -- M1 exit test: datalab-style bit puzzles.
 * Nathan Dearing, 2026-08-15.
 *
 * THE RULES (these are the whole point -- the puzzles are trivial without them)
 *
 *   - No loops:        no for, while, do.
 *   - No conditionals: no if, switch, and no ?: ternary.
 *   - No comparisons:  no == != < > <= >=, and no && or ||.
 *   - No function calls, no casts, no other data types, no macros.
 *   - Integer constants must be 0x00 to 0xFF. Bigger masks get built
 *     from small ones with shifts and ors.
 *   - Assume 32-bit int, two's complement, arithmetic right shift.
 *   - Shifting by less than 0 or more than 31 is undefined behavior.
 *     Never do it, even in a branch you think can't be reached.
 *
 * Allowed operators unless a puzzle says otherwise:  ! ~ & ^ | + << >>
 *
 * Each stub currently ignores its arguments via (void) casts so the file
 * builds clean under -Wall -Wextra -Werror before you start. Delete those
 * casts as you implement each one -- a (void) cast is technically a cast,
 * so leaving them in breaks the rules above.
 *
 * Work top to bottom. They are ordered so each one teaches the trick the
 * next one assumes. Run ./build.sh after each puzzle.
 */

#include "bits.h"

/* 1. bitAnd -- x & y using only ~ and |.
 *    Allowed: ~ |          Max ops: 8
 *    Hint: De Morgan. Write out what ~(~x | ~y) is on paper, one bit at a time. */
int bitAnd(int x, int y)
{
        (void)x; (void)y;       /* TODO: delete when you implement this */
        return 0;
}

/* 2. bitXor -- x ^ y using only ~ and &.
 *    Allowed: ~ &          Max ops: 14
 *    Hint: x ^ y is "in one but not both". Build "in x but not y" and
 *    "in y but not x" separately, then combine -- but you have no |,
 *    so De Morgan again to get the or. */
int bitXor(int x, int y)
{
        (void)x; (void)y;       /* TODO */
        return 0;
}

/* 3. isEqual -- 1 if x == y, else 0.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 5
 *    Hint: what single operator gives exactly zero when two values match? */
int isEqual(int x, int y)
{
        (void)x; (void)y;       /* TODO */
        return 0;
}

/* 4. logicalNeg -- !x without using !.
 *    Allowed: ~ & ^ | + << >>          Max ops: 12
 *    Hint: 0 is the only value where both x and its two's complement
 *    negation have a 0 sign bit. Or-ing x with -x and looking at the
 *    sign bit separates zero from everything else. */
int logicalNeg(int x)
{
        (void)x;                /* TODO */
        return 0;
}

/* 5. getByte -- byte n of x, with n = 0 the least significant byte.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 6
 *    Examples: getByte(0x12345678, 0) == 0x78
 *              getByte(0x12345678, 3) == 0x12
 *    Hint: n bytes is how many bits? Shift first, mask second -- masking
 *    first throws away the byte you wanted. */
int getByte(int x, int n)
{
        (void)x; (void)n;       /* TODO */
        return 0;
}

/* 6. replaceByte -- x with byte n replaced by c. 0 <= n <= 3, 0 <= c <= 255.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 10
 *    Example: replaceByte(0x12345678, 1, 0xAB) == 0x1234AB78
 *    Hint: clear the target byte with an inverted mask, then or the new
 *    one in. Note c is guaranteed to fit in a byte, so you do not have to
 *    defend against a c that is too big. */
int replaceByte(int x, int n, int c)
{
        (void)x; (void)n; (void)c;      /* TODO */
        return 0;
}

/* 7. logicalShift -- x >> n but shifting in zeros, not sign bits. 0 <= n <= 31.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 20
 *    Example: logicalShift(0x80000000, 4) == 0x08000000
 *    Hint: do the arithmetic shift, then clear the top n bits that got
 *    filled with ones. Building that mask is the puzzle. Careful: the
 *    obvious mask needs a shift by 32 when n is 0, which is undefined.
 *    Shift by 31 and then once more. */
int logicalShift(int x, int n)
{
        (void)x; (void)n;       /* TODO */
        return 0;
}

/* 8. isNegative -- 1 if x < 0, else 0.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 6
 *    Hint: the sign bit already is the answer. Move it. */
int isNegative(int x)
{
        (void)x;                /* TODO */
        return 0;
}

/* 9. sign -- return -1 if x < 0, 0 if x == 0, 1 if x > 0.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 10
 *    Hint: x >> 31 is 0 or -1 -- that is the negative case for free.
 *    Or it with "is x nonzero", which puzzle 4 taught you. */
int sign(int x)
{
        (void)x;                /* TODO */
        return 0;
}

/* 10. conditional -- x ? y : z.
 *     Allowed: ! ~ & ^ | + << >>       Max ops: 16
 *     Hint: turn x into an all-ones-or-all-zeros mask, then select.
 *     "Turn 0/1 into 0x00000000/0xFFFFFFFF" is the single most reusable
 *     trick in this whole file -- negation does it. */
int conditional(int x, int y, int z)
{
        (void)x; (void)y; (void)z;      /* TODO */
        return 0;
}

/* 11. absVal -- |x|. Assumes x != INT_MIN (whose absolute value does not fit).
 *     Allowed: ! ~ & ^ | + << >>       Max ops: 10
 *     Hint: the branchless idiom is (x + mask) ^ mask where mask is x >> 31.
 *     Do not just use it -- work out on paper why that is exactly two's
 *     complement negation when mask is all ones, and identity when it is zero. */
int absVal(int x)
{
        (void)x;                /* TODO */
        return 0;
}

/* 12. bitCount -- how many bits of x are 1.
 *     Allowed: ! ~ & ^ | + << >>       Max ops: 40
 *     Hint: divide and conquer, and remember constants are capped at 0xFF.
 *     Sum bits in pairs, then nibbles, then bytes, then halves. You will
 *     need masks like 0x55555555 -- build them by shifting and or-ing 0x55.
 *     This is the hardest one here. Leave it for last. */
int bitCount(int x)
{
        (void)x;                /* TODO */
        return 0;
}
