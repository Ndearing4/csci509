/*
 * test_bits.c -- brute-force checker for the M1 bit puzzles.
 * Nathan Dearing, 2026-08-15.
 *
 * Each puzzle is compared against a plain-C reference implementation over a
 * table of nasty edge values plus a large deterministic random sample. The
 * seed is fixed, so a failure you see once you can reproduce exactly.
 *
 * The reference implementations below use loops and conditionals on purpose.
 * They are the definition of correct, not a model answer -- do not copy their
 * shape into bits.c.
 */

#include <stdio.h>
#include <limits.h>
#include "bits.h"

#define N_RANDOM     200000     /* random cases per puzzle */
#define MAX_REPORTS  4          /* failures printed per puzzle before summarizing */

/* ---------- reference implementations (unrestricted C) ---------- */

static int ref_bitAnd(int x, int y)       { return x & y; }
static int ref_bitXor(int x, int y)       { return x ^ y; }
static int ref_isEqual(int x, int y)      { return x == y; }
static int ref_logicalNeg(int x)          { return !x; }
static int ref_getByte(int x, int n)      { return (x >> (n * 8)) & 0xFF; }
static int ref_isNegative(int x)          { return x < 0; }
static int ref_sign(int x)                { return (x > 0) - (x < 0); }
static int ref_conditional(int x, int y, int z) { return x ? y : z; }
static int ref_absVal(int x)              { return x < 0 ? -x : x; }

static int ref_replaceByte(int x, int n, int c)
{
        int shift = n * 8;
        return (x & ~(0xFF << shift)) | (c << shift);
}

static int ref_logicalShift(int x, int n)
{
        unsigned int ux = (unsigned int)x;
        return (int)(ux >> n);
}

static int ref_bitCount(int x)
{
        unsigned int ux = (unsigned int)x;
        int count = 0;
        int i;
        for (i = 0; i < 32; i++) {
                count += (int)((ux >> i) & 1u);
        }
        return count;
}

/* ---------- test scaffolding ---------- */

/* Values that break naive solutions: zero, the sign boundaries, alternating
 * bit patterns, single set bits, and byte-aligned patterns. */
static const int edges[] = {
        0, 1, -1, 2, -2, 3, -3, 7, 8, 16, 31, 32, 127, 128, 255, 256,
        0x7F, 0x80, 0xFF, 0x100, 0xFFFF, 0x10000, 0xFFFFFF, 0x1000000,
        (int)0x55555555, (int)0xAAAAAAAA, (int)0x0F0F0F0F, (int)0xF0F0F0F0,
        (int)0x12345678, (int)0x80000000, (int)0x7FFFFFFF, (int)0xFFFFFFFE,
        INT_MAX, INT_MIN, INT_MAX - 1, INT_MIN + 1
};
static const int n_edges = (int)(sizeof(edges) / sizeof(edges[0]));

static unsigned int rng_state = 0x509C5C1u;

/* xorshift32 -- deterministic so failures reproduce across runs */
static int next_rand(void)
{
        rng_state ^= rng_state << 13;
        rng_state ^= rng_state >> 17;
        rng_state ^= rng_state << 5;
        return (int)rng_state;
}

static int total_failures;      /* across every puzzle */
static int cur_failures;        /* within the puzzle being tested */
static const char *cur_name;

static void begin(const char *name)
{
        cur_name = name;
        cur_failures = 0;
        rng_state = 0x509C5C1u;         /* same sample for every puzzle */
}

static void report1(int a, int got, int want)
{
        if (cur_failures <= MAX_REPORTS) {
                printf("  %s(0x%08X) = %d (0x%08X), want %d (0x%08X)\n",
                       cur_name, (unsigned)a, got, (unsigned)got,
                       want, (unsigned)want);
        }
}

static void report2(int a, int b, int got, int want)
{
        if (cur_failures <= MAX_REPORTS) {
                printf("  %s(0x%08X, 0x%08X) = 0x%08X, want 0x%08X\n",
                       cur_name, (unsigned)a, (unsigned)b,
                       (unsigned)got, (unsigned)want);
        }
}

static void report3(int a, int b, int c, int got, int want)
{
        if (cur_failures <= MAX_REPORTS) {
                printf("  %s(0x%08X, 0x%08X, 0x%08X) = 0x%08X, want 0x%08X\n",
                       cur_name, (unsigned)a, (unsigned)b, (unsigned)c,
                       (unsigned)got, (unsigned)want);
        }
}

static void check1(int a, int got, int want)
{
        if (got != want) { cur_failures++; report1(a, got, want); }
}

static void check2(int a, int b, int got, int want)
{
        if (got != want) { cur_failures++; report2(a, b, got, want); }
}

static void check3(int a, int b, int c, int got, int want)
{
        if (got != want) { cur_failures++; report3(a, b, c, got, want); }
}

static void finish(void)
{
        if (cur_failures == 0) {
                printf("PASS  %s\n", cur_name);
        } else {
                if (cur_failures > MAX_REPORTS) {
                        printf("  ... and %d more\n", cur_failures - MAX_REPORTS);
                }
                printf("FAIL  %s -- %d failing cases\n", cur_name, cur_failures);
                total_failures += cur_failures;
        }
}

/* ---------- the tests ---------- */

static void test_unary(const char *name, int (*f)(int), int (*ref)(int),
                       int skip_int_min)
{
        int i, k;

        begin(name);
        for (i = 0; i < n_edges; i++) {
                if (skip_int_min && edges[i] == INT_MIN) continue;
                check1(edges[i], f(edges[i]), ref(edges[i]));
        }
        for (k = 0; k < N_RANDOM; k++) {
                int x = next_rand();
                if (skip_int_min && x == INT_MIN) continue;
                check1(x, f(x), ref(x));
        }
        finish();
}

static void test_binary(const char *name, int (*f)(int, int),
                        int (*ref)(int, int))
{
        int i, j, k;

        begin(name);
        for (i = 0; i < n_edges; i++) {
                for (j = 0; j < n_edges; j++) {
                        check2(edges[i], edges[j],
                               f(edges[i], edges[j]), ref(edges[i], edges[j]));
                }
        }
        for (k = 0; k < N_RANDOM; k++) {
                int x = next_rand();
                int y = next_rand();
                check2(x, y, f(x, y), ref(x, y));
        }
        finish();
}

static void test_getByte(void)
{
        int i, n, k;

        begin("getByte");
        for (i = 0; i < n_edges; i++) {
                for (n = 0; n < 4; n++) {
                        check2(edges[i], n, getByte(edges[i], n),
                               ref_getByte(edges[i], n));
                }
        }
        for (k = 0; k < N_RANDOM; k++) {
                int x = next_rand();
                int n2 = next_rand() & 3;
                check2(x, n2, getByte(x, n2), ref_getByte(x, n2));
        }
        finish();
}

static void test_replaceByte(void)
{
        int i, n, k;

        begin("replaceByte");
        for (i = 0; i < n_edges; i++) {
                for (n = 0; n < 4; n++) {
                        check3(edges[i], n, 0xAB,
                               replaceByte(edges[i], n, 0xAB),
                               ref_replaceByte(edges[i], n, 0xAB));
                        check3(edges[i], n, 0x00,
                               replaceByte(edges[i], n, 0x00),
                               ref_replaceByte(edges[i], n, 0x00));
                        check3(edges[i], n, 0xFF,
                               replaceByte(edges[i], n, 0xFF),
                               ref_replaceByte(edges[i], n, 0xFF));
                }
        }
        for (k = 0; k < N_RANDOM; k++) {
                int x = next_rand();
                int n2 = next_rand() & 3;
                int c = next_rand() & 0xFF;
                check3(x, n2, c, replaceByte(x, n2, c),
                       ref_replaceByte(x, n2, c));
        }
        finish();
}

static void test_logicalShift(void)
{
        int i, n, k;

        begin("logicalShift");
        for (i = 0; i < n_edges; i++) {
                for (n = 0; n < 32; n++) {
                        check2(edges[i], n, logicalShift(edges[i], n),
                               ref_logicalShift(edges[i], n));
                }
        }
        for (k = 0; k < N_RANDOM; k++) {
                int x = next_rand();
                int n2 = next_rand() & 31;
                check2(x, n2, logicalShift(x, n2), ref_logicalShift(x, n2));
        }
        finish();
}

static void test_conditional(void)
{
        int i, j, k;

        begin("conditional");
        for (i = 0; i < n_edges; i++) {
                for (j = 0; j < n_edges; j++) {
                        check3(edges[i], edges[j], 0x1234,
                               conditional(edges[i], edges[j], 0x1234),
                               ref_conditional(edges[i], edges[j], 0x1234));
                }
        }
        for (k = 0; k < N_RANDOM; k++) {
                int x = next_rand();
                int y = next_rand();
                int z = next_rand();
                check3(x, y, z, conditional(x, y, z), ref_conditional(x, y, z));
        }
        finish();
}

int main(void)
{
        printf("=== M1 bit puzzles ===\n");
        printf("(edge table: %d values, plus %d random cases per puzzle)\n\n",
               n_edges, N_RANDOM);

        test_binary("bitAnd", bitAnd, ref_bitAnd);
        test_binary("bitXor", bitXor, ref_bitXor);
        test_binary("isEqual", isEqual, ref_isEqual);
        test_unary("logicalNeg", logicalNeg, ref_logicalNeg, 0);
        test_getByte();
        test_replaceByte();
        test_logicalShift();
        test_unary("isNegative", isNegative, ref_isNegative, 0);
        test_unary("sign", sign, ref_sign, 0);
        test_conditional();
        test_unary("absVal", absVal, ref_absVal, 1);    /* INT_MIN excluded */
        test_unary("bitCount", bitCount, ref_bitCount, 0);

        printf("\n");
        if (total_failures == 0) {
                printf("All puzzles pass. Now run ./check-rules.sh -- correct is\n");
                printf("only half of it, the operator restrictions are the point.\n");
                return 0;
        }
        printf("%d failing cases total.\n", total_failures);
        return 1;
}
