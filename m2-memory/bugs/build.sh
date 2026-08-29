#!/bin/sh
# build.sh -- compile all six planted-bug programs.
# Nathan Dearing, 2026-08-21.
#
# Builds, deliberately does not run. bug2 aborts and bug4 can corrupt the
# heap; you want to meet those inside gdb, not in a build script's output.
#
# -g -O0 is not optional here. At -O2 gcc inlines these functions, reorders
# the lines, and keeps variables in registers, and then gdb's single-stepping
# jumps around and `print` says <optimized out>.

set -e

# GCC 12+ added -Wuse-after-free and GCC 13+ added -Wdangling-pointer to
# -Wall, and both are exactly bug1 and bug6's planted bugs. Left on, -Werror
# would catch two of the six at compile time and the "the compiler found
# none of them" lesson would be false on this toolchain. Suppressed here so
# the exercise still starts from a clean build; gdb/valgrind/ASan are still
# where you're meant to find them.
CFLAGS="-std=c11 -Wall -Wextra -Werror -Wno-use-after-free -Wno-dangling-pointer -g -O0"

for src in bug1_uaf.c bug2_double_free.c bug3_leak.c \
           bug4_overflow.c bug5_uninit.c bug6_dangling.c; do
        out=`basename "$src" .c`
        echo "+ gcc $CFLAGS $src -o $out"
        gcc $CFLAGS "$src" -o "$out"
done

echo
echo "All six built clean under -Wall -Wextra -Werror."
echo "That is the first lesson: the compiler found none of them."
echo
echo "Start with HUNT.md. Predict, then gdb, then valgrind -- in that order."
