#!/bin/sh
# build.sh -- compile and run the M2 struct and memory tour.
# Nathan Dearing, 2026-08-21.
#
# Same flags as the rest of the repo, on purpose: the warnings are the lesson.

set -e

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 tour.c counters.c -o tour"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 tour.c counters.c -o tour

echo
./tour
