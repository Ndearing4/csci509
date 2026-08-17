#!/bin/sh
# build.sh -- compile and run the C syntax tour.
#
# Same flags as the rest of M1, on purpose: the warnings are the lesson.

set -e

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 tour.c -o tour"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 tour.c -o tour

echo
./tour
