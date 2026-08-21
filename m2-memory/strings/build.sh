#!/bin/sh
# build.sh -- compile and run the M2 string tests.
# Nathan Dearing, 2026-08-21.
#
# Still not a Makefile. The Makefile you write by hand lives next door in
# list/, which is the exercise; this directory stays out of its way.

set -e

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c mystr.c -o mystr.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c mystr.c -o mystr.o

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c test_mystr.c -o test_mystr.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c test_mystr.c -o test_mystr.o

echo "+ gcc mystr.o test_mystr.o -o test_mystr"
gcc mystr.o test_mystr.o -o test_mystr

echo
./test_mystr
