/*
 * bits.h -- M1 exit test: datalab-style bit puzzles.
 * Nathan Dearing, 2026-08-15.
 */

#ifndef BITS_H
#define BITS_H

/* Each puzzle is solved with a restricted operator set and no loops,
 * conditionals, or function calls. Per-puzzle rules are in bits.c. */

int bitAnd(int x, int y);                    /* x & y, using only ~ and | */
int bitXor(int x, int y);                    /* x ^ y, using only ~ and & */
int isEqual(int x, int y);                   /* 1 if x == y, else 0 */
int logicalNeg(int x);                       /* !x, without using ! */
int getByte(int x, int n);                   /* byte n of x, 0 = least significant */
int replaceByte(int x, int n, int c);        /* x with byte n replaced by c */
int logicalShift(int x, int n);              /* x >> n, shifting in zeros */
int isNegative(int x);                       /* 1 if x < 0, else 0 */
int sign(int x);                             /* -1, 0, or 1 */
int conditional(int x, int y, int z);        /* x ? y : z */
int absVal(int x);                           /* |x|, assumes x != INT_MIN */
int bitCount(int x);                         /* number of 1 bits in x */

#endif /* BITS_H */
