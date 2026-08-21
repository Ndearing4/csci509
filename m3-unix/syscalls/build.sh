#!/bin/sh
# build.sh -- compile the M3 syscall exercises.
# Nathan Dearing, 2026-08-21.
#
# Three separate programs, so three separate links. Nothing here shares code.

set -e

CFLAGS="-std=c11 -Wall -Wextra -Werror -g -O0"

for src in fdtour.c mycat.c buffering.c walk.c; do
        out=`basename "$src" .c`
        echo "+ gcc $CFLAGS $src -o $out"
        gcc $CFLAGS "$src" -o "$out"
done

echo
echo "Built fdtour, mycat, buffering, walk."
echo
echo "  ./fdtour              START HERE -- finished, not a stub"
echo "  ./show-buffering.sh   then this one, also finished"
echo "  ./test-mycat.sh       diffs your mycat against the real cat"
echo "  ./walk /etc           compare with: find /etc"
