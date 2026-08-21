#!/bin/sh
# test-spawn.sh -- check spawn's exit codes, signal handling, and reaping.
# Nathan Dearing, 2026-08-21.
#
# Every case here is one your shell will have to get right in stage 1, so a
# failure here is a bug you would otherwise chase inside the shell later.

[ -x ./spawn ] || { echo "build it first: ./build.sh" >&2; exit 2; }

failures=0
fail() { echo "  FAIL  $1"; [ -n "$2" ] && echo "        $2"; failures=`expr $failures + 1`; }
pass() { echo "  ok    $1"; }

# Run ./spawn with the given args and compare its exit status.
expect_rc() {
        want=$1; desc=$2; shift 2
        if ./spawn "$@" >/dev/null 2>&1; then
                got=0
        else
                got=$?
        fi
        if [ "$got" = "$want" ]; then
                pass "$desc (exit $want)"
        else
                fail "$desc: exit $got, expected $want"
        fi
}

echo "=== spawn ==="
echo

echo "--- exit codes"
expect_rc 0   "true"                  true
expect_rc 1   "false"                 false
expect_rc 42  "an explicit exit 42"   sh -c 'exit 42'
expect_rc 255 "exit 255"              sh -c 'exit 255'

echo
echo "--- exec failures, using the shell's own conventions"
expect_rc 127 "a command that does not exist"   ./definitely-not-here
expect_rc 127 "a name not on PATH"              nosuchcommand-xyzzy
if [ -d /etc ]; then
        expect_rc 126 "a path that is not executable" /etc/hostname
fi

echo
echo "--- killed by a signal: 128 + signo, not the exit code"
expect_rc 143 "SIGTERM is 15, so 143"  sh -c 'kill -TERM $$'
expect_rc 137 "SIGKILL is 9, so 137"   sh -c 'kill -KILL $$'

echo
echo "--- and it says so in words"
out=`./spawn sh -c 'kill -TERM $$' 2>&1`
if printf '%s' "$out" | grep -qi 'signal\|terminat'; then
        pass "reports the signal in its message"
else
        fail "a signal death should not be described as a normal exit" \
             "got: $out"
fi

echo
echo "--- arguments actually reach the command"
out=`./spawn echo one two three 2>/dev/null | head -1`
if [ "$out" = "one two three" ]; then
        pass "argv is passed through intact"
else
        fail "argv did not arrive: got '$out'"
fi

echo
echo "--- no zombies left behind"
# Run a hundred children and count how many defunct processes we own.
i=0
while [ "$i" -lt 100 ]; do
        ./spawn true >/dev/null 2>&1
        i=`expr $i + 1`
done
zombies=`ps -o stat= -u "$(id -u)" 2>/dev/null | grep -c '^Z'` || zombies=0
if [ "$zombies" = "0" ]; then
        pass "100 spawns left 0 zombies"
else
        fail "$zombies zombie process(es) after 100 spawns" \
             "every fork needs a matching waitpid"
fi

echo
if [ "$failures" -eq 0 ]; then
        echo "Clean. On to shell/ -- stage 1 is this, in a loop, with a prompt."
        exit 0
fi
echo "$failures check(s) failed."
exit 1
