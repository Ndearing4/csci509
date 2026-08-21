#!/bin/sh
# build.sh -- compile the shell.
# Nathan Dearing, 2026-08-21.
#
# Two objects and a link, spelled out. If you would rather drive this with a
# Makefile, you already wrote one in m2-memory/list -- this is a good place
# to write the second one, and it is four files again.

set -e

CFLAGS="-std=c11 -Wall -Wextra -Werror -g -O0"

echo "+ gcc $CFLAGS -c parse.c -o parse.o"
gcc $CFLAGS -c parse.c -o parse.o

echo "+ gcc $CFLAGS -c shell.c -o shell.o"
gcc $CFLAGS -c shell.c -o shell.o

echo "+ gcc parse.o shell.o -o shell"
gcc parse.o shell.o -o shell

echo
echo "Built ./shell. Read STAGES.md, then ./test-shell.sh"
