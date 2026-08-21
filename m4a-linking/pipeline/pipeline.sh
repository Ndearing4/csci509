#!/bin/sh
# pipeline.sh -- take demo.c through all four stages, one at a time.
# Nathan Dearing, 2026-08-21.
#
# `gcc demo.c counter.c -o demo` runs four separate programs. This script
# runs them one at a time and leaves every intermediate file behind so you
# can read them.
#
# Answer the questions in WALKTHROUGH.md as you go. Running this and not
# reading the artifacts takes four minutes and teaches nothing.

set -e

CFLAGS="-std=c11 -Wall -Wextra -Werror -g -O0"
out=artifacts

rm -rf "$out"
mkdir "$out"

echo "############################################################"
echo "# stage 1: PREPROCESS   gcc -E    (text in, text out)"
echo "############################################################"
gcc $CFLAGS -E demo.c -o "$out/demo.i"
printf 'demo.c   %6s lines\n' "`wc -l < demo.c`"
printf 'demo.i   %6s lines\n' "`wc -l < "$out/demo.i"`"
echo
echo "Almost all of that came from #include <stdio.h>. The preprocessor does"
echo "not parse C -- it pastes files in, expands macros, and strips comments."
echo
echo "Where your own code ended up:"
grep -n "initialised_global   = 7\|hello\|(5) + (5)" "$out/demo.i" | head -5 || true
echo
echo "  ^ GREETING and TWICE are gone. Not renamed, not compiled -- textually"
echo "    replaced before the compiler ever saw them. That is why a macro"
echo "    cannot be inspected in gdb and why TWICE(i++) increments twice."

echo
echo "############################################################"
echo "# stage 2: COMPILE      gcc -S    (C in, assembly out)"
echo "############################################################"
gcc $CFLAGS -S demo.c -o "$out/demo.s"
printf 'demo.s   %6s lines of assembly\n' "`wc -l < "$out/demo.s"`"
echo
echo "The section directives -- this is where the probe in README.md is answered:"
grep -nE '^\s*\.(text|data|bss|section|globl|local|comm)' "$out/demo.s" | head -20
echo
echo "  ^ .globl means the linker will see it. .local means it will not."
echo "    add is there with a .globl. triple is not in that list AT ALL --"
echo "    a static function needs no directive, because nothing outside this"
echo "    file is ever told about it. Confirm in stage 3: nm calls add T and"
echo "    triple t, and the case of the letter is the whole difference."

echo
echo "############################################################"
echo "# stage 3: ASSEMBLE     gcc -c    (assembly in, object file out)"
echo "############################################################"
gcc $CFLAGS -c demo.c -o "$out/demo.o"
gcc $CFLAGS -c counter.c -o "$out/counter.o"
ls -l "$out"/demo.o "$out"/counter.o | awk '{printf "%-24s %8s bytes\n", $9, $5}'
echo
echo "--- nm demo.o ---"
nm "$out/demo.o"
echo
echo "  ^ the letter is the section, uppercase = global, lowercase = local:"
echo "      T  .text, a function defined here"
echo "      t  .text, but static -- invisible to the linker"
echo "      D  .data, initialised"
echo "      B  .bss, zero-initialised"
echo "      R  .rodata, const"
echo "      U  UNDEFINED -- promised, not provided. Someone else must have it."
echo
echo "--- nm counter.o ---"
nm "$out/counter.o"
echo
echo "--- objdump -r demo.o: the holes the linker must fill ---"
objdump -r "$out/demo.o" | head -25
echo
echo "  ^ each line is 'at this offset, patch in the address of that symbol'."
echo "    The call instruction in the object file has a placeholder in it."
echo "    That is a relocation, and it is the reason separate compilation works."

echo
echo "############################################################"
echo "# stage 4: LINK         gcc *.o   (objects in, executable out)"
echo "############################################################"
gcc "$out/demo.o" "$out/counter.o" -o "$out/demo"
ls -l "$out/demo" | awk '{printf "%-24s %8s bytes\n", $9, $5}'
echo
echo "--- size: where the bytes went ---"
size "$out/demo.o" "$out/counter.o" "$out/demo"
echo
echo "  ^ bss costs no file space. It is a promise to the loader to hand you"
echo "    that many zeroed bytes, so a 4 MB zeroed array adds nothing to the"
echo "    executable. That is the README probe."
echo
echo "--- is shared_counter_bump still undefined? ---"
nm "$out/demo" | grep -i shared_counter_bump || true
echo "  ^ T now, not U. The linker resolved it against counter.o."
echo
echo "--- what is STILL undefined, even after linking ---"
nm -u "$out/demo" | head
echo
echo "  ^ printf and strlen are STILL undefined in a finished executable."
echo "    They get resolved at RUN time, by the dynamic loader, out of"
echo "    libc.so -- which is why a binary can fail to start on a machine"
echo "    that compiled and linked it fine. Linking is not the last step."
echo "    The w entries are weak symbols: undefined is allowed, they just"
echo "    resolve to zero and the program checks before calling."
echo "    ldd shows what the loader will go looking for:"
ldd "$out/demo"

echo
echo "############################################################"
echo "# and it runs"
echo "############################################################"
"$out/demo"

echo
echo "Everything is in $out/. Now answer WALKTHROUGH.md."
