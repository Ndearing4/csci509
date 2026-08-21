#!/bin/sh
# break-it.sh -- five real link errors, with the evidence to diagnose them.
# Nathan Dearing, 2026-08-21.
#
#     ./break-it.sh              the errors and the symbol tables
#     ./break-it.sh --answers    ... and the diagnoses
#
# Read the error, read the nm output, and say what is wrong BEFORE looking at
# the answer. All five are errors you will meet again, and four of them say
# almost nothing useful on their own -- the symbol table is where the answer
# actually is.
#
# Everything is generated into ./broken/ and can be deleted at any time.

answers=no
[ "$1" = "--answers" ] && answers=yes

d=broken
rm -rf "$d"
mkdir "$d"

CFLAGS="-std=c11 -Wall -Wextra -g -O0"

banner() {
        echo
        echo "############################################################"
        echo "# CASE $1: $2"
        echo "############################################################"
}

answer() {
        if [ "$answers" = yes ]; then
                echo
                echo "--- answer ---"
                printf '%s\n' "$1"
        else
                echo
                echo "(diagnose it, then re-run with --answers)"
        fi
}

# ---------------------------------------------------------------- case 1
banner 1 "undefined reference"

cat > "$d/c1_main.c" <<'EOF_C'
int compute_total(int n);

int main(void)
{
        return compute_total(3);
}
EOF_C

cat > "$d/c1_util.c" <<'EOF_C'
int computeTotal(int n)
{
        return n * 2;
}
EOF_C

gcc $CFLAGS -c "$d/c1_main.c" -o "$d/c1_main.o"
gcc $CFLAGS -c "$d/c1_util.c" -o "$d/c1_util.o"
echo "+ gcc c1_main.o c1_util.o -o c1"
gcc "$d/c1_main.o" "$d/c1_util.o" -o "$d/c1" 2>&1 | sed 's/^/    /' || true
echo
echo "--- nm c1_main.o | grep -i total ---"
nm "$d/c1_main.o" | grep -i total | sed 's/^/    /'
echo "--- nm c1_util.o | grep -i total ---"
nm "$d/c1_util.o" | grep -i total | sed 's/^/    /'
answer "The U symbol and the T symbol are not the same name.
compute_total vs computeTotal. C is case sensitive and the linker
matches on the exact string; it does not know the two are related and
will not suggest anything.

This is why the error names a symbol and not a file: the linker has no
idea which file was SUPPOSED to define it. Put the declaration in a
header both files include, and the compiler catches the typo instead."

# ---------------------------------------------------------------- case 2
banner 2 "multiple definition"

cat > "$d/c2_a.c" <<'EOF_C'
#include "c2.h"

int shared_config = 1;

int a_value(void)
{
        return shared_config;
}
EOF_C

cat > "$d/c2_b.c" <<'EOF_C'
#include "c2.h"

int shared_config = 2;

int b_value(void)
{
        return shared_config;
}
EOF_C

cat > "$d/c2.h" <<'EOF_C'
#ifndef C2_H
#define C2_H
int a_value(void);
int b_value(void);
#endif
EOF_C

cat > "$d/c2_main.c" <<'EOF_C'
#include "c2.h"

int main(void)
{
        return a_value() + b_value();
}
EOF_C

gcc $CFLAGS -I"$d" -c "$d/c2_a.c" -o "$d/c2_a.o"
gcc $CFLAGS -I"$d" -c "$d/c2_b.c" -o "$d/c2_b.o"
gcc $CFLAGS -I"$d" -c "$d/c2_main.c" -o "$d/c2_main.o"
echo "+ gcc c2_main.o c2_a.o c2_b.o -o c2"
gcc "$d/c2_main.o" "$d/c2_a.o" "$d/c2_b.o" -o "$d/c2" 2>&1 | sed 's/^/    /' || true
echo
echo "--- nm c2_a.o | grep shared_config ---"
nm "$d/c2_a.o" | grep shared_config | sed 's/^/    /'
echo "--- nm c2_b.o | grep shared_config ---"
nm "$d/c2_b.o" | grep shared_config | sed 's/^/    /'
answer "Two D symbols with the same name. The linker's other failure mode:
it needs exactly one definition per symbol, and it has two.

Note both are D, not U -- a definition, not a reference. A DECLARATION
in a header is fine in as many files as you like. A DEFINITION belongs
in exactly one .c file, and the header should say

        extern int shared_config;

which is a promise, not a definition.

Try changing one of them to 'int shared_config;' with no initialiser
and relink. It still fails, and it did NOT before gcc 10, which
defaulted to -fcommon and silently merged them. That silent merge is
why programs used to work by accident. Build with -fcommon to see the
old behaviour, once, then never use it."

# ---------------------------------------------------------------- case 3
banner 3 "the definition is right there, and the linker cannot see it"

cat > "$d/c3_main.c" <<'EOF_C'
int helper(int n);

int main(void)
{
        return helper(3);
}
EOF_C

cat > "$d/c3_util.c" <<'EOF_C'
static int helper(int n)
{
        return n + 1;
}

int wrapper(int n)
{
        return helper(n);
}
EOF_C

gcc $CFLAGS -c "$d/c3_main.c" -o "$d/c3_main.o"
gcc $CFLAGS -c "$d/c3_util.c" -o "$d/c3_util.o" 2>/dev/null
echo "+ gcc c3_main.o c3_util.o -o c3"
gcc "$d/c3_main.o" "$d/c3_util.o" -o "$d/c3" 2>&1 | sed 's/^/    /' || true
echo
echo "--- grep -n helper c3_util.c ---"
grep -n helper "$d/c3_util.c" | sed 's/^/    /'
echo "--- nm c3_util.o | grep helper ---"
nm "$d/c3_util.o" | grep helper | sed 's/^/    /'
answer "It is a LOWERCASE t. The function is defined, it is in .text, and
the linker still says undefined reference -- because static gives it
internal linkage, and internal linkage means the symbol exists for this
translation unit only.

This is the one that wastes the most time, because the definition is
sitting right there in a file you are looking at and the error insists
it does not exist. The case of a single letter in nm's output is the
whole diagnosis.

Uppercase = the linker can use it from elsewhere. Lowercase = it cannot.
Learn to read that column and this error takes ten seconds instead of
twenty minutes."

# ---------------------------------------------------------------- case 4
banner 4 "the same error, for a completely different reason"

cat > "$d/c4_main.c" <<'EOF_C'
int helper(int n);

int main(void)
{
        return helper(3);
}
EOF_C

cat > "$d/c4_util.c" <<'EOF_C'
int helper(int n)
{
        return n + 1;
}
EOF_C

gcc $CFLAGS -c "$d/c4_main.c" -o "$d/c4_main.o"
gcc $CFLAGS -c "$d/c4_util.c" -o "$d/c4_util.o"
echo "+ gcc c4_main.o -o c4          <- note what is missing"
gcc "$d/c4_main.o" -o "$d/c4" 2>&1 | sed 's/^/    /' || true
echo
echo "--- nm c4_util.o | grep helper ---"
nm "$d/c4_util.o" | grep helper | sed 's/^/    /'
answer "Uppercase T this time, so the definition is fine and exported. The
object file was simply never passed to the linker.

Cases 3 and 4 produce the SAME message and have nothing in common. That
is the point of running them next to each other: 'undefined reference'
means 'nobody I was given defines this', and there are at least four
reasons for that -- misspelled, static, not linked, or in a library you
did not name. The message cannot tell them apart. nm can.

A build system is what stops case 4 from happening. That is what the
Makefile in m2-memory/list was for."

# ---------------------------------------------------------------- case 5
banner 5 "the order of the arguments is part of the meaning"

cat > "$d/c5_lib.c" <<'EOF_C'
int lib_double(int n)
{
        return n * 2;
}
EOF_C

cat > "$d/c5_main.c" <<'EOF_C'
int lib_double(int n);

int main(void)
{
        return lib_double(21);
}
EOF_C

gcc $CFLAGS -c "$d/c5_lib.c" -o "$d/c5_lib.o"
gcc $CFLAGS -c "$d/c5_main.c" -o "$d/c5_main.o"
ar rcs "$d/libc5.a" "$d/c5_lib.o"
echo "made libc5.a:"
ar t "$d/libc5.a" | sed 's/^/    /'
echo
echo "+ gcc -L$d -lc5 $d/c5_main.o -o $d/c5      <- library FIRST"
gcc -L"$d" -lc5 "$d/c5_main.o" -o "$d/c5" 2>&1 | sed 's/^/    /' || true
echo
echo "+ gcc $d/c5_main.o -L$d -lc5 -o $d/c5      <- library LAST"
# The status has to come from gcc, not from the sed at the end of a pipe.
if gcc "$d/c5_main.o" -L"$d" -lc5 -o "$d/c5" 2>"$d/c5.err"; then
        echo "    (linked cleanly)"
        "$d/c5" || echo "    it runs, and returns $? -- lib_double(21)"
else
        sed 's/^/    /' "$d/c5.err"
fi
answer "Same files, same library, different order, and only one of them
works.

The linker walks its arguments strictly left to right, carrying a set of
symbols that are still undefined. When it reaches an ARCHIVE (.a) it
pulls in only those members that resolve something currently undefined,
and then moves on and never looks back.

Library first: nothing is undefined yet, so libc5.a resolves nothing and
is discarded. Then c5_main.o arrives needing lib_double, and the archive
is already behind it.

Library last: c5_main.o registers lib_double as undefined, then the
archive is searched and supplies it.

Hence the rule: object files first, libraries after, and a library
after everything that uses it. Circular dependencies between two
archives are why -( ... -) and repeating a library on the line exist.

This is archive-specific. A shared library (.so) does not behave this
way, which is why the rule seems to come and go."

# ---------------------------------------------------------------------
echo
echo "############################################################"
echo "# summary"
echo "############################################################"
cat <<'EOF_SUM'

Two linker errors, five causes:

  undefined reference to 'x'
      the name is misspelled                        (case 1)
      it is static, so it has internal linkage      (case 3)
      the object file was not on the command line   (case 4)
      the library was, but in the wrong position    (case 5)

  multiple definition of 'x'
      a definition instead of a declaration in a header, or just
      twice in two .c files                         (case 2)

nm tells them apart and the message does not. Three letters cover it:
  U  undefined here    T  defined and exported    t  defined but hidden

EOF_SUM
[ "$answers" = yes ] || echo "Re-run with --answers when you have diagnosed all five."
