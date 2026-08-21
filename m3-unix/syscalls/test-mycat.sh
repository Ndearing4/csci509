#!/bin/sh
# test-mycat.sh -- diff your mycat against the real cat.
# Nathan Dearing, 2026-08-21.
#
# Compares stdout, stderr behaviour, and exit status. The exit status and the
# stderr/stdout split are half the exercise -- a mycat that prints the right
# bytes and exits 0 on a missing file has not matched cat.

set -e
[ -x ./mycat ] || { echo "build it first: ./build.sh" >&2; exit 2; }

tmp=`mktemp -d`
trap 'rm -rf "$tmp"' EXIT
failures=0

fail() { echo "  FAIL  $1"; failures=`expr $failures + 1`; }
pass() { echo "  ok    $1"; }

# Three files worth caring about: ordinary text, empty, and binary with NULs
# and high bytes, which is where a str* function in your copy loop shows up.
printf 'one\ntwo\nthree\n'            > "$tmp/a.txt"
: > "$tmp/empty.txt"
printf 'x\000y\377z\n'                > "$tmp/bin.dat"
head -c 100000 /dev/urandom           > "$tmp/big.dat"

# Run both, capture stdout, stderr and status separately.
compare() {
        desc=$1; shift
        # `set -e` is on, so the status has to be captured through an if.
        # `cmd || true; rc=$?` would record the status of `true`, which is a
        # classic and silent way to make a test suite always pass.
        if cat "$@" > "$tmp/want.out" 2> "$tmp/want.err"; then
                want_rc=0
        else
                want_rc=$?
        fi
        if ./mycat "$@" > "$tmp/got.out" 2> "$tmp/got.err"; then
                got_rc=0
        else
                got_rc=$?
        fi

        if ! cmp -s "$tmp/want.out" "$tmp/got.out"; then
                fail "$desc: stdout differs from cat"
                return
        fi
        if [ "$want_rc" != "$got_rc" ]; then
                fail "$desc: exit status $got_rc, cat gives $want_rc"
                return
        fi
        if [ -s "$tmp/want.err" ] && [ ! -s "$tmp/got.err" ]; then
                fail "$desc: cat wrote to stderr and you did not"
                return
        fi
        if [ ! -s "$tmp/want.err" ] && [ -s "$tmp/got.err" ]; then
                fail "$desc: you wrote to stderr and cat did not"
                return
        fi
        pass "$desc"
}

echo "=== mycat vs cat ==="
echo

compare "one text file"        "$tmp/a.txt"
compare "an empty file"        "$tmp/empty.txt"
compare "NULs and high bytes"  "$tmp/bin.dat"
compare "100 KB of random"     "$tmp/big.dat"
compare "three files at once"  "$tmp/a.txt" "$tmp/bin.dat" "$tmp/a.txt"
compare "a file that is not there" "$tmp/nope.txt"
compare "a directory"          "$tmp"
compare "good file then bad"   "$tmp/a.txt" "$tmp/nope.txt"

# stdin, both ways
if printf 'piped\n' | ./mycat > "$tmp/got.out" 2>/dev/null &&
   [ "`cat "$tmp/got.out"`" = "piped" ]; then
        pass "no arguments reads stdin"
else
        fail "no arguments should copy stdin to stdout"
fi

if printf 'dashed\n' | ./mycat - > "$tmp/got.out" 2>/dev/null &&
   [ "`cat "$tmp/got.out"`" = "dashed" ]; then
        pass "a lone - means stdin"
else
        fail "a lone - should mean stdin"
fi

# Short writes only show up when the output is a pipe with a small reader.
if ./mycat "$tmp/big.dat" 2>/dev/null | wc -c | grep -q '^ *100000$'; then
        pass "100 KB survives being written into a pipe"
else
        fail "bytes lost writing to a pipe -- your write() is not looping"
fi

echo
if [ "$failures" -eq 0 ]; then
        echo "Clean. mycat matches cat."
        exit 0
fi
echo "$failures check(s) failed."
exit 1
