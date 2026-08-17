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
        
        return ~(~x|~y);
}

/* 2. bitXor -- x ^ y using only ~ and &.
 *    Allowed: ~ &          Max ops: 14
 *    Hint: x ^ y is "in one but not both". Build "in x but not y" and
 *    "in y but not x" separately, then combine -- but you have no |,
 *    so De Morgan again to get the or. */
int bitXor(int x, int y)
{
        
        return ~(x&y)&~(~x&~y);
}

/* 3. isEqual -- 1 if x == y, else 0.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 5
 *    Hint: what single operator gives exactly zero when two values match? */
int isEqual(int x, int y)
{
        
        
        return !(x^y);
}

/* 4. logicalNeg -- !x without using !.
 *    Allowed: ~ & ^ | + << >>          Max ops: 12
 *    Hint: 0 is the only value where both x and its two's complement
 *    negation have a 0 sign bit. Or-ing x with -x and looking at the
 *    sign bit separates zero from everything else. */
int logicalNeg(int x)
{
        int ux = x;             

        int signX = ux >> 31;
        int twosCompXSign = (~ux + 1) >> 31;

        return (signX | twosCompXSign) + 1; //if 0, it's compliment should also be 0. 
}

/* 5. getByte -- byte n of x, with n = 0 the least significant byte.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 6
 *    Examples: getByte(0x12345678, 0) == 0x78
 *              getByte(0x12345678, 3) == 0x12
 *    Hint: n bytes is how many bits? Shift first, mask second -- masking
 *    first throws away the byte you wanted. */
int getByte(int x, int n)
{
        

        int s = x >> (n << 3);
        return s & 0x000000FF;
}

/* 6. replaceByte -- x with byte n replaced by c. 0 <= n <= 3, 0 <= c <= 255.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 10
 *    Example: replaceByte(0x12345678, 1, 0xAB) == 0x1234AB78
 *    Hint: clear the target byte with an inverted mask, then or the new
 *    one in. Note c is guaranteed to fit in a byte, so you do not have to
 *    defend against a c that is too big. */
int replaceByte(int x, int n, int c)
{
        

        int xEmpty = x & (~(0xFF<<(n<<3)));
        int cMask = c << (n<<3);

        return xEmpty + cMask;
}

/* 7. logicalShift -- x >> n but shifting in zeros, not sign bits. 0 <= n <= 31.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 20
 *    Example: logicalShift(0x80000000, 4) == 0x08000000
 *    Hint: do the arithmetic shift, then clear the top n bits that got
 *    filled with ones. Building that mask is the puzzle. Careful: the
 *    obvious mask needs a shift by 32 when n is 0, which is undefined.
 *    Shift by 31 and then once more. 
 logicalShift(0x00000001, 0x00000000) = 0x00000000, want 0x00000001
  logicalShift(0xFFFFFFFF, 0x00000000) = 0x00000000, want 0xFFFFFFFF
  logicalShift(0xFFFFFFFF, 0x00000001) = 0x00000000, want 0x7FFFFFFF
  logicalShift(0xFFFFFFFF, 0x00000002) = 0x00000000, want 0x3FFFFFFF
 
 
 */
int logicalShift(int x, int n)
{
        
        // int sign = x >> 31;
        int shiftedX = x >> n;

        int mask = (1 << 31) >> n << 1;

        return shiftedX & ~mask;
}

/* 8. isNegative -- 1 if x < 0, else 0.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 6
 *    Hint: the sign bit already is the answer. Move it. */
int isNegative(int x)
{
        
        return (x >> 31) &1;
}

/* 9. sign -- return -1 if x < 0, 0 if x == 0, 1 if x > 0.
 *    Allowed: ! ~ & ^ | + << >>        Max ops: 10
 *    Hint: x >> 31 is 0 or -1 -- that is the negative case for free.
 *    Or it with "is x nonzero", which puzzle 4 taught you. */
int sign(int x)
{
        
        int retSign = ~0;

        // int xc = ~x + 1;

        int isNotNeg = ((x >> 31 ) & 1) ^1;  //1 if notNeg, 0 otherwize

        int isNotZero = (!x^1) & isNotNeg;

        return retSign + isNotNeg + isNotZero;
}

/* 10. conditional -- x ? y : z.
 *     Allowed: ! ~ & ^ | + << >>       Max ops: 16
 *     Hint: turn x into an all-ones-or-all-zeros mask, then select.
 *     "Turn 0/1 into 0x00000000/0xFFFFFFFF" is the single most reusable
 *     trick in this whole file -- negation does it. */
int conditional(int x, int y, int z)
{
        

        int mask = !x; //1 if x=0, 0 otherwise

        int maskFull = ~mask + 1; //if 0, then 000... if 1, then 1111....

        //Ones mean false, so Z. 

        int retVal = (z & maskFull) | (y & ~maskFull);

        return retVal;
}

/* 11. absVal -- |x|. Assumes x != INT_MIN (whose absolute value does not fit).
 *     Allowed: ! ~ & ^ | + << >>       Max ops: 10
 *     Hint: the branchless idiom is (x + mask) ^ mask where mask is x >> 31.
 *     Do not just use it -- work out on paper why that is exactly two's
 *     complement negation when mask is all ones, and identity when it is zero. */
int absVal(int x)
{
        
        // int mask = 0xffffffff >> 1;

        int sign = (x >> 31) &1;


        int mask = !sign; //1 if x=0, 0 otherwise

        int maskFull = ~mask + 1; //if 0, then 000... if 1, then 1111....

        //Ones mean false, so Z. 

        int retVal = (x & maskFull) | ((~x + 1) & ~maskFull);

        return retVal;
}

/* 12. bitCount -- how many bits of x are 1.
 *     Allowed: ! ~ & ^ | + << >>       Max ops: 40
 *     Hint: divide and conquer, and remember constants are capped at 0xFF.
 *     Sum bits in pairs, then nibbles, then bytes, then halves. You will
 *     need masks like 0x55555555 -- build them by shifting and or-ing 0x55.
 *     This is the hardest one here. Leave it for last. */
int bitCount(int x)
{

        // into bins of 2
        // int fullMask = ~0;
        // fullMask << 0;


        int mask = 0x55;
        mask = mask << 8 | mask;
        mask = mask << 16 | mask;
        int R1Counts = (x & mask) + ((x >> 1) & mask);

        // into bins of 4
        mask = 0x33;
        mask = mask << 8 | mask;
        mask = mask << 16 | mask;
        int R2Counts = (R1Counts & mask) + ((R1Counts >> 2) & mask);

        // into bins of 8
        mask = 0x0f;
        mask = mask << 8 | mask;
        mask = mask << 16 | mask;
        int R3Counts = (R2Counts & mask) + ((R2Counts >> 4) & mask);

        //into bins of 16
        mask = 0xff;
        mask = mask << 16 | mask;
        int R4Counts = (R3Counts & mask) + ((R3Counts >> 8) & mask);

        //into bins of 32
        mask = 0xff;
        mask = mask | mask << 8;
        int R5Counts = (R4Counts & mask) + ((R4Counts >> 16) & mask);

        return R5Counts;
}
