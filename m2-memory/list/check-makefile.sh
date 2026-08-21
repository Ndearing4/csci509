#!/bin/sh
# check-makefile.sh -- verify the Makefile you wrote behaves like a Makefile.
# Nathan Dearing, 2026-08-21.
#
# Seven checks, matching the seven numbered requirements in MAKEFILE.md. It
# builds, touches files, and rebuilds, watching which object files actually
# get recompiled. It does not read your Makefile except to look for the
# compile flags -- everything else is judged by what make does.
#
# Leaves the tree built. Run ./check-makefile.sh after every change while you
# are writing it.

failures=0

fail() {
        echo "  FAIL  $1"
        [ -n "$2" ] && echo "        $2"
        failures=`expr $failures + 1`
}

pass() {
        echo "  ok    $1"
}

# Modification time with nanoseconds, or the word "missing".
mtime() {
        if [ -f "$1" ]; then
                stat -c %y "$1"
        else
                echo missing
        fi
}

echo "=== Makefile check ==="
echo

if [ ! -f Makefile ] && [ ! -f makefile ]; then
        echo "No Makefile here. That is the exercise -- see MAKEFILE.md."
        exit 2
fi

if ! command -v make >/dev/null 2>&1; then
        echo "make is not installed. Run setup/setup-wsl.sh." >&2
        exit 2
fi

# --- 1. builds from clean ------------------------------------------------
make clean >/dev/null 2>&1
rm -f test_list list.o test_list.o
if make >/tmp/mk.$$ 2>&1 && [ -x ./test_list ]; then
        pass "1. make builds ./test_list from a clean tree"
else
        fail "1. make did not produce an executable ./test_list" \
             "make output is in /tmp/mk.$$"
        echo
        echo "Nothing else can be checked until it builds. Stopping."
        exit 1
fi
rm -f /tmp/mk.$$

# --- 2. no work when nothing changed -------------------------------------
out=`make 2>&1`
if printf '%s' "$out" | grep -qE '(^|[^-[:alnum:]])(gcc|cc|g\+\+)([^-[:alnum:]]|$)'; then
        fail "2. a second make recompiled with nothing changed" \
             "a target whose name is not the file it creates is always out of date"
else
        pass "2. a second make does nothing"
fi

# Checks 3 and 4 watch the object files by name. MAKEFILE.md asks for
# list.o and test_list.o next to the sources; if they are somewhere else,
# say so rather than silently passing.
if [ ! -f list.o ] || [ ! -f test_list.o ]; then
        fail "3-4. cannot find list.o and test_list.o next to the sources" \
             "either you are linking in one step, or the objects are elsewhere"
        echo
        echo "Separate compilation is the point of checks 3 and 4. See MAKEFILE.md."
        skip_incremental=yes
fi

# --- 3. touching the header rebuilds both objects ------------------------
if [ -z "$skip_incremental" ]; then
        before_l=`mtime list.o`
        before_t=`mtime test_list.o`
        touch list.h
        make >/dev/null 2>&1
        after_l=`mtime list.o`
        after_t=`mtime test_list.o`

        if [ "$before_l" != "$after_l" ] && [ "$before_t" != "$after_t" ]; then
                pass "3. touching list.h rebuilt both objects"
        else
                stale=""
                [ "$before_l" = "$after_l" ] && stale="list.o"
                [ "$before_t" = "$after_t" ] && stale="$stale test_list.o"
                fail "3. touching list.h did not rebuild:$stale" \
                     "make cannot see #include -- list.h has to be in the prerequisites"
        fi

        # --- 4. touching one source rebuilds only that object ------------
        before_l=`mtime list.o`
        before_t=`mtime test_list.o`
        touch list.c
        make >/dev/null 2>&1
        after_l=`mtime list.o`
        after_t=`mtime test_list.o`

        if [ "$before_l" = "$after_l" ]; then
                fail "4. touching list.c did not rebuild list.o" \
                     "list.o must depend on list.c"
        elif [ "$before_t" != "$after_t" ]; then
                fail "4. touching list.c also rebuilt test_list.o" \
                     "test_list.o does not depend on list.c -- only on test_list.c and list.h"
        else
                pass "4. touching list.c rebuilt only list.o"
        fi
fi

# --- 5. clean ------------------------------------------------------------
make clean >/dev/null 2>&1
left=""
for f in test_list list.o test_list.o; do
        [ -e "$f" ] && left="$left $f"
done
missing_src=""
for f in list.c list.h test_list.c; do
        [ -e "$f" ] || missing_src="$missing_src $f"
done

if [ -n "$missing_src" ]; then
        fail "5. make clean deleted source files:$missing_src" \
             "that is not recoverable -- restore them with git checkout"
elif [ -n "$left" ]; then
        fail "5. make clean left build products behind:$left" ""
else
        pass "5. make clean removes the build products and nothing else"
fi

# --- 6. the flags --------------------------------------------------------
mk=Makefile
[ -f Makefile ] || mk=makefile

missing_flags=""
for flag in -std=c11 -Wall -Wextra -Werror -g -O0; do
        grep -q -- "$flag" "$mk" || missing_flags="$missing_flags $flag"
done
if [ -n "$missing_flags" ]; then
        fail "6. flags missing from $mk:$missing_flags" \
             "-g -O0 matter here: optimized code single-steps out of order in gdb"
elif ! grep -qE '^[[:space:]]*CFLAGS[[:space:]]*[:+?]?=' "$mk"; then
        fail "6. the flags are there but not in a CFLAGS variable" \
             "repeating them on every line is how they drift apart"
else
        pass "6. compile flags are complete and live in CFLAGS"
fi

# --- 7. .PHONY -----------------------------------------------------------
if grep -qE '^[[:space:]]*\.PHONY:.*\bclean\b' "$mk"; then
        pass "7. clean is declared .PHONY"
else
        fail "7. clean is not declared .PHONY" \
             "try: touch clean; make clean   -- and see what make tells you"
fi

# --- advisory, not counted ----------------------------------------------
echo
if grep -qE '^[[:space:]]*%\.o[[:space:]]*:' "$mk"; then
        echo "note: you used a pattern rule. Fine once the long form is written --"
        echo "      make sure you can still write both compile lines out by hand."
fi
if grep -qE '\$\(wildcard|\$\(shell' "$mk"; then
        echo "note: wildcard/shell in a four-file Makefile is hiding the dependencies"
        echo "      you are supposed to be learning to state."
fi
if grep -qP '^ +[^ ]' "$mk" 2>/dev/null; then
        echo "note: some lines start with spaces. Recipe lines need a literal TAB."
fi

echo
if [ "$failures" -eq 0 ]; then
        echo "Clean. All seven checks pass."
        echo "Now: delete list.h from the prerequisites, touch list.h, run make,"
        echo "and watch it do nothing. That is the bug check 3 exists to catch."
        exit 0
fi
echo "$failures of 7 checks failed."
exit 1
