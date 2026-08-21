#!/bin/sh
# build.sh -- compile the M3 process exercises.
# Nathan Dearing, 2026-08-21.

set -e

CFLAGS="-std=c11 -Wall -Wextra -Werror -g -O0"

for src in spawn.c zombies.c; do
        out=`basename "$src" .c`
        echo "+ gcc $CFLAGS $src -o $out"
        gcc $CFLAGS "$src" -o "$out"
done

echo
echo "Built spawn, zombies."
echo
echo "  ./zombies         already finished -- run it first, follow its prompts"
echo "  ./test-spawn.sh   the exercise"
