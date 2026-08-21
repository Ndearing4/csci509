#!/bin/sh
# show-buffering.sh -- run buffering twice, to a terminal and through a pipe.
# Nathan Dearing, 2026-08-21.
#
# The program is the same both times. Everything that differs between these
# two columns is libc deciding, from isatty(), when to call write().

set -e
[ -x ./buffering ] || { echo "build it first: ./build.sh" >&2; exit 2; }

echo "############################################################"
echo "# 1. stdout is this terminal"
echo "############################################################"
./buffering

echo
echo "############################################################"
echo "# 2. stdout is a pipe (into cat, which just passes it through)"
echo "############################################################"
./buffering | cat

echo
echo "############################################################"
echo "# 3. how many write() calls each way"
echo "############################################################"
if command -v strace >/dev/null 2>&1; then
        echo "Run these two yourself -- they cannot be counted from inside this"
        echo "script, because a command substitution makes stdout a pipe and so"
        echo "the terminal case stops being a terminal case:"
        echo
        echo "    strace -e trace=write ./buffering 2>&1 >/dev/null | grep -c write"
        echo "    strace -e trace=write ./buffering > /tmp/b.out 2>/tmp/b.err"
        echo "    grep -c write /tmp/b.err"
        echo
        echo "First one: stdout is your terminal. Second: stdout is a file."
        echo "The counts are very different, and that difference is the entire"
        echo "reason the stdio layer exists."
else
        echo "strace is not installed:  sudo apt install strace"
        echo "It is worth having. It is the tool that answers 'what did this"
        echo "program actually ask the kernel for', which is the whole subject."
fi
echo
echo "Now: diff the two outputs above line by line, and for each difference"
echo "say which buffering mode caused it. Then read the comment block at the"
echo "top of buffering.c and check yourself."
