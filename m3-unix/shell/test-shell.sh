#!/bin/sh
# test-shell.sh -- drive the shell through the M3 exit test.
# Nathan Dearing, 2026-08-21.
#
# Feeds commands on stdin and checks what comes out. Grouped by stage, and
# each group reports how many it got -- so this is useful from the end of
# stage 1 onwards, not only when everything is finished.
#
# It CANNOT test Ctrl-C: that needs a controlling terminal, and stdin here is
# a pipe. STAGES.md stage 4 has the manual checklist for that half.

[ -x ./shell ] || { echo "build it first: ./build.sh" >&2; exit 2; }

tmp=`mktemp -d`
trap 'rm -rf "$tmp"' EXIT

failures=0
fail() { echo "  FAIL  $1"; [ -n "$2" ] && echo "        $2"; failures=`expr $failures + 1`; }
pass() { echo "  ok    $1"; }

# Feed $2 to the shell, compare its stdout against $3.
expect_out() {
        desc=$1; input=$2; want=$3
        got=`printf '%s\n' "$input" | ./shell 2>/dev/null`
        if [ "$got" = "$want" ]; then
                pass "$desc"
        else
                fail "$desc" "got [$got], wanted [$want]"
        fi
}

# Feed $2 to the shell, expect exit status $3.
expect_rc() {
        desc=$1; input=$2; want=$3
        if printf '%s\n' "$input" | ./shell >/dev/null 2>&1; then
                got=0
        else
                got=$?
        fi
        if [ "$got" = "$want" ]; then
                pass "$desc"
        else
                fail "$desc" "exit $got, wanted $want"
        fi
}

# Feed $2 to the shell as the SECOND line, after a line that must work.
# Requiring the good line's output as well as a message on stderr keeps this
# from passing on a shell that just prints an error and gives up -- which an
# unimplemented stub does, and which would otherwise look like five passing
# checks. It also tests the right behaviour: a syntax error skips that line,
# it does not end the shell.
expect_error() {
        desc=$1; input=$2
        out=`printf 'echo SENTINEL\n%s\n' "$input" | ./shell 2>/dev/null`
        err=`printf 'echo SENTINEL\n%s\n' "$input" | ./shell 2>&1 >/dev/null`
        if [ "$out" != "SENTINEL" ]; then
                fail "$desc" "the shell stopped running commands; stdout was [$out]"
        elif [ -z "$err" ]; then
                fail "$desc" "nothing on stderr -- a syntax error must say so"
        else
                pass "$desc"
        fi
}

echo "=== shell ==="
echo

echo "--- stage 0: the parser rejects what it should"
expect_error "a trailing pipe"        'ls |'
expect_error "a leading pipe"         '| ls'
expect_error "> with no filename"     'ls >'
expect_error "< with no filename"     'ls <'
expect_error "a redirect with no command" '> out.txt'

echo
echo "--- stage 1: run a command"
expect_out "echo"                  'echo hello'            'hello'
expect_out "multiple arguments"    'echo a b c'            'a b c'
expect_out "several lines"         'echo one
echo two'                                                  'one
two'
expect_out "a blank line is not an error" '
echo after'                                                'after'
expect_rc  "exit status comes from the command" 'false'  1
expect_rc  "and from an explicit exit code"     'sh -c "exit 7"'  7
expect_rc  "exit builtin"                       'exit'   0
expect_error "a command that does not exist"    'nosuchcommand-xyzzy'

echo
echo "--- stage 1: builtins"
got=`printf 'cd /tmp\npwd\n' | ./shell 2>/dev/null`
if [ "$got" = "/tmp" ]; then
        pass "cd changes the shell's own directory"
else
        fail "cd did not take effect" "got [$got] -- if it is not /tmp, cd ran in a child"
fi
expect_error "cd to somewhere that is not there" 'cd /no/such/dir/here'

echo
echo "--- stage 2: redirection"
rm -f "$tmp/out.txt"
printf 'echo redirected > %s/out.txt\n' "$tmp" | ./shell >/dev/null 2>&1
if [ "`cat "$tmp/out.txt" 2>/dev/null`" = "redirected" ]; then
        pass "> writes to a file"
else
        fail "> did not produce the file"
fi

printf 'echo second >> %s/out.txt\n' "$tmp" | ./shell >/dev/null 2>&1
if [ "`wc -l < "$tmp/out.txt" 2>/dev/null`" = "2" ]; then
        pass ">> appends instead of truncating"
else
        fail ">> should append" "the file has `wc -l < "$tmp/out.txt" 2>/dev/null` line(s), wanted 2"
fi

printf 'echo replaced > %s/out.txt\n' "$tmp" | ./shell >/dev/null 2>&1
if [ "`wc -l < "$tmp/out.txt" 2>/dev/null`" = "1" ]; then
        pass "> truncates an existing file"
else
        fail "> should truncate"
fi

printf 'three\none\ntwo\n' > "$tmp/in.txt"
got=`printf 'sort < %s/in.txt\n' "$tmp" | ./shell 2>/dev/null`
if [ "$got" = "one
three
two" ]; then
        pass "< feeds a file to stdin"
else
        fail "< did not redirect stdin" "got [$got]"
fi

got=`printf 'echo still-works\n' | ./shell 2>/dev/null`
if [ "$got" = "still-works" ]; then
        pass "the shell's own stdout survives a redirect"
else
        fail "the shell redirected ITSELF" "dup2 belongs in the child, after fork"
fi

echo
echo "--- stage 3: pipes"
expect_out "a two-stage pipeline" 'echo hello | tr a-z A-Z' 'HELLO'

printf 'a\nb\nc\n' > "$tmp/abc.txt"
got=`printf 'cat %s/abc.txt | grep b\n' "$tmp" | ./shell 2>/dev/null`
if [ "$got" = "b" ]; then
        pass "cat file | grep"
else
        fail "cat | grep" "got [$got], wanted [b]"
fi

expect_out "a three-stage pipeline" 'echo one two three | tr " " "\n" | sort' 'one
three
two'

# The exit test itself.
rm -f "$tmp/exit.txt"
printf 'foo\nbar\nfoobar\n' > "$tmp/haystack"
printf 'cat %s/haystack | grep foo > %s/exit.txt\n' "$tmp" "$tmp" | ./shell >/dev/null 2>&1
if [ "`wc -l < "$tmp/exit.txt" 2>/dev/null`" = "2" ]; then
        pass "pipe AND redirect on the same line -- the exit test"
else
        fail "the exit test line did not work" \
             "the redirect belongs to the LAST command only"
fi

expect_rc "exit status is the last command's" 'true | false' 1
expect_rc "and not the first's"               'false | true' 0

# A reader that quits early must not hang the writer forever.
got=`printf 'yes hello | head -2\n' | timeout 10 ./shell 2>/dev/null`
if [ "$got" = "hello
hello" ]; then
        pass "an early-quitting reader does not hang the pipeline"
elif [ -z "$got" ]; then
        fail "yes | head -2 produced nothing or hung" \
             "if it hung: a pipe read waits for EOF until EVERY write end is closed"
else
        fail "yes | head -2" "got [$got]"
fi

# More than a pipe buffer, so the writer blocks unless the reader is running.
got=`printf 'seq 1 200000 | wc -l\n' | timeout 20 ./shell 2>/dev/null | tr -d ' '`
if [ "$got" = "200000" ]; then
        pass "200k lines through the pipe -- all children forked before waiting"
else
        fail "a large pipeline did not complete" \
             "got [$got]; waiting for child 1 before forking child 2 deadlocks at 64 KB"
fi

echo
echo "--- no zombies"
i=0
cmds=""
while [ "$i" -lt 100 ]; do
        cmds="$cmds
true"
        i=`expr $i + 1`
done
printf '%s\n' "$cmds" | ./shell >/dev/null 2>&1
zombies=`ps -o stat= -u "$(id -u)" 2>/dev/null | grep -c '^Z'` || zombies=0
if [ "$zombies" = "0" ]; then
        pass "100 commands left 0 zombies"
else
        fail "$zombies zombie process(es) after 100 commands" \
             "every fork needs a matching waitpid -- including in a pipeline"
fi

echo
if [ "$failures" -eq 0 ]; then
        echo "Clean, as far as a pipe can tell."
        echo
        echo "Ctrl-C is not tested here and cannot be -- stdin is a pipe, so"
        echo "there is no controlling terminal to send SIGINT from. Do the"
        echo "manual checklist at the end of STAGES.md in a real terminal."
        exit 0
fi
echo "$failures check(s) failed."
echo "Expected, until stage 4. Check them off stage by stage."
exit 1
