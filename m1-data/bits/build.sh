#!/bin/sh
# build.sh -- compile and run the M1 puzzle tests.
# Nathan Dearing, 2026-08-15.
#
# Deliberately not a Makefile. Writing make by hand is M2's exit test, and
# doing the steps long-hand first means the Makefile you write later is
# describing something you have already watched happen.

set -e

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c bits.c -o bits.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c bits.c -o bits.o

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c test_bits.c -o test_bits.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c test_bits.c -o test_bits.o

echo "+ gcc bits.o test_bits.o -o test_bits"
gcc bits.o test_bits.o -o test_bits

echo
./test_bits
