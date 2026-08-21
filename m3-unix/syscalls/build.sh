#!/bin/sh
# build.sh -- compile the M3 syscall exercises.
# Nathan Dearing, 2026-08-21.
#
# Three separate programs, so three separate links. Nothing here shares code.

set -e

CFLAGS="-std=c11 -Wall -Wextra -Werror -g -O0"

for src in mycat.c buffering.c walk.c; do
        out=`basename "$src" .c`
        echo "+ gcc $CFLAGS $src -o $out"
        gcc $CFLAGS "$src" -o "$out"
done

echo
echo "Built mycat, buffering, walk."
echo
echo "  ./show-buffering.sh   the one to run first -- it is already finished"
echo "  ./test-mycat.sh       diffs your mycat against the real cat"
echo "  ./walk /etc           compare with: find /etc"
