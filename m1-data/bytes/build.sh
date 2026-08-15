#!/bin/sh
# build.sh -- compile and run the show_bytes driver.
# Nathan Dearing, 2026-08-15.
#
# Not a Makefile, for the same reason as bits/build.sh: writing make by hand
# is M2's exit test.

set -e

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c show_bytes.c -o show_bytes.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c show_bytes.c -o show_bytes.o

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c main.c -o main.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c main.c -o main.o

echo "+ gcc show_bytes.o main.o -o bytes"
gcc show_bytes.o main.o -o bytes

echo
./bytes
