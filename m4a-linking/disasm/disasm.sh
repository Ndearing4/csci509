#!/bin/sh
# disasm.sh -- disassemble frames.c at -O0 and at -O2.
# Nathan Dearing, 2026-08-21.
#
#     ./disasm.sh              add2 at both optimisation levels
#     ./disasm.sh sum_array    any other function by name
#     ./disasm.sh --all        everything, into artifacts/
#
# -Mintel because Intel syntax puts the destination on the left, like C
# assignment does. AT&T syntax is the GNU default and puts it on the right;
# gdb defaults to AT&T too. You will meet both. Pick one to think in and
# know that the other exists -- `set disassembly-flavor intel` in gdb, and
# -Matt here, if you want to swap.

set -e

out=artifacts
mkdir -p "$out"

echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c frames.c -o $out/frames_O0.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c frames.c -o "$out/frames_O0.o"
echo "+ gcc -std=c11 -Wall -Wextra -Werror -g -O2 -c frames.c -o $out/frames_O2.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O2 -c frames.c -o "$out/frames_O2.o"
gcc -std=c11 -Wall -Wextra -Werror -g -O0 frames.c -o "$out/frames"

if [ "$1" = "--all" ]; then
        objdump -d -Mintel --no-show-raw-insn "$out/frames_O0.o" > "$out/frames_O0.asm"
        objdump -d -Mintel --no-show-raw-insn "$out/frames_O2.o" > "$out/frames_O2.asm"
        objdump -dS -Mintel "$out/frames_O0.o" > "$out/frames_O0_source.asm"
        echo
        echo "wrote:"
        echo "  $out/frames_O0.asm           -O0, Intel syntax"
        echo "  $out/frames_O2.asm           -O2, for comparison"
        echo "  $out/frames_O0_source.asm    -O0 interleaved with the C source"
        echo
        echo "That last one is the most useful file in this directory. objdump -S"
        echo "puts each C line above the instructions it turned into, which is"
        echo "the fastest way to learn to read this at all."
        exit 0
fi

fn=${1:-add2}

# One function's worth of disassembly, by name.
extract() {
        awk -v fn="$1" '
                $0 ~ ("<" fn ">:") { printing = 1 }
                printing { print }
                printing && NF == 0 { exit }
        ' "$2"
}

echo
echo "############################################################"
echo "# $fn at -O0 -- a literal transcription of your C"
echo "############################################################"
objdump -d -Mintel --no-show-raw-insn "$out/frames_O0.o" > "$out/.o0"
extract "$fn" "$out/.o0"

echo
echo "############################################################"
echo "# $fn at -O2 -- what the compiler would rather run"
echo "############################################################"
objdump -d -Mintel --no-show-raw-insn "$out/frames_O2.o" > "$out/.o2"
extract "$fn" "$out/.o2" || echo "  (not present -- at -O2 it may have been inlined away entirely)"

rm -f "$out/.o0" "$out/.o2"

echo
echo "############################################################"
echo "# the calling convention, System V AMD64"
echo "############################################################"
cat <<'EOF_ABI'
  integer/pointer arguments, in order:  rdi rsi rdx rcx r8 r9
  everything after the sixth:           on the stack, pushed right to left
  return value:                         rax
  callee must preserve:                 rbx rbp r12 r13 r14 r15
  caller must assume trashed:           rax rcx rdx rsi rdi r8-r11

  32-bit operations name the low half: edi is the low 32 bits of rdi. So
  add2's `int a` arrives in edi, not rdi -- same register, narrower view.

  rsp  stack pointer, always the current top
  rbp  frame pointer: a fixed anchor for this frame, so locals can be named
       relative to it while rsp moves around. -O2 often drops it and
       addresses everything off rsp instead, which is why an optimised
       backtrace needs the .eh_frame unwind tables rather than a chain of
       saved rbp values.
EOF_ABI

echo
echo "Now work through ANNOTATE.md."
